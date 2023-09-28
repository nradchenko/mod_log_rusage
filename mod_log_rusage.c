/*
 mod_log_rusage.c: resource usage logging for apache2
 Copyright (C) 2023 Nikita Radchenko <nradchenko@protonmail.com>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

/*
 * This module implements utime and stime logging using %{format}z
 * format string for LogFormat and CustomLog directives.
 *
 * Supported format tokens are:
 * - utime: number of microseconds spent executing in user mode
 * - stime: number of microseconds spent executing in kernel mode
 */

#include "apr_strings.h"
#include "apr_lib.h"
#include "apr_hash.h"
#include "apr_optional.h"

#define APR_WANT_STRFUNC
#include "apr_want.h"

#include "ap_config.h"
#include "mod_log_config.h"
#include "httpd.h"
#include "http_core.h"
#include "http_config.h"
#include "http_connection.h"
#include "http_protocol.h"

#include <sys/time.h>
#include <sys/resource.h>

module AP_MODULE_DECLARE_DATA log_rusage_module;

static long utime;
static long stime;

static const char *log_handler(request_rec *r, char *a)
{
    if (*a == '\0' || !strcmp(a, "utime")) {
        return apr_ltoa(r->pool, utime);
    }
    else if (!strcmp(a, "stime")) {
        return apr_ltoa(r->pool, stime);
    }
    else {
        return a;
    }
}

static int pre_config(apr_pool_t *p, apr_pool_t *plog, apr_pool_t *ptemp)
{
    static APR_OPTIONAL_FN_TYPE(ap_register_log_handler) *log_pfn_register;

    log_pfn_register = APR_RETRIEVE_OPTIONAL_FN(ap_register_log_handler);

    if (log_pfn_register) {
        log_pfn_register(p, "z", log_handler, 0);
    }

    return OK;
}

static int log_transaction(request_rec *r)
{
    static struct rusage usage, usage_prev;

    // get current resource usage
    getrusage(RUSAGE_SELF, &usage);

    // calculate resource usage difference between two requests
    utime = (usage.ru_utime.tv_sec - usage_prev.ru_utime.tv_sec) * 1000000
                      + usage.ru_utime.tv_usec - usage_prev.ru_utime.tv_usec;
    stime = (usage.ru_stime.tv_sec - usage_prev.ru_stime.tv_sec) * 1000000
                      + usage.ru_stime.tv_usec - usage_prev.ru_stime.tv_usec;

    // save data for next request
    usage_prev = usage;

    return OK;
}

static void register_hooks(apr_pool_t *p)
{
    ap_hook_pre_config(pre_config, NULL, NULL, APR_HOOK_REALLY_FIRST);
    ap_hook_log_transaction(log_transaction, NULL, NULL, APR_HOOK_FIRST);
}

module AP_MODULE_DECLARE_DATA log_rusage_module =
{
    STANDARD20_MODULE_STUFF,
    NULL,                  /* create per-dir    config structures */
    NULL,                  /* merge  per-dir    config structures */
    NULL,                  /* create per-server config structures */
    NULL,                  /* merge  per-server config structures */
    NULL,                  /* table of config file commands       */
    register_hooks         /* register hooks                      */
};
