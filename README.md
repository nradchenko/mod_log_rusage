mod_log_rusage: resource usage logging for apache2
==================================================

Overview
--------

This module implements utime and stime logging using `%{format}z`
format string for `LogFormat` and `CustomLog` directives.

Supported format tokens are:
- `utime`: number of microseconds spent executing in user mode
- `stime`: number of microseconds spent executing in kernel mode

How to build
------------

Install apache2 and apache2-dev packages and run `make`

How to install
--------------

Run `make install` or copy `.libs/mod_log_rusage.so` to `modules` directory
and put the following line in your `apache24.conf`:
```
LoadModule log_rusage_module  modules/mod_log_rusage.so
```

Example configuration:
```
LogFormat "%V %a %l %u %t \"%r\" %>s %b %D %{utime}z:%{stime}z" combined_plain
```

Links
--------------

* http://man7.org/linux/man-pages/man2/getrusage.2.html
