APXS ?= apxs

all: build

build: mod_log_rusage.c
	$(APXS) -c mod_log_rusage.c

install: mod_log_rusage.c
	$(APXS) -a -i -c mod_log_rusage.c

clean:
	rm -rf *.la *.lo *.o *.slo .libs
