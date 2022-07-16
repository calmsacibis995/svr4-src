#ident	"@(#)Makefile	1.1	92/09/30	JPB"

DRIVER = e503
FILES = e503.h

all:	$(FILES)

install: all
	[ -d ../pkg/sys ] || mkdir ../pkg/sys
	for h in $(FILES); \
	do \
	cp $$h ../pkg/sys ; \
	done

clean:
	for h in $(FILES) ; \
	do \
	rm -rf ../pkg/sys/$$h ; \
	done

clobber: clean
