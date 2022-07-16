#ident	"@(#)Makefile	1.1	92/09/30	JPB"

STRIPFILES =	Master System Node
OTHERFILES =	Space.c
FILES =	$(STRIPFILES) $(OTHERFILES)

all:	$(FILES)

install:	all
	[ -d ../pkg ] || mkdir ../pkg
	for i in $(STRIPFILES) ; \
	do \
		grep -v "^\*" $$i | grep -v "^$$" > ../pkg/$$i ; \
	done
	cp $(OTHERFILES) ../pkg

clean:
	-for i in $(STRIPFILES) $(OTHERFILES); \
	do \
		rm -f ../pkg/$$i ; \
	done

clobber: clean
