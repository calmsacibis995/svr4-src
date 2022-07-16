#ident	"@(#)e503.mk	1.1	92/09/30	JPB"

ADDON =		e503
COMPONENTS =	ID io sys
FRC=

all:
	for comp in $(COMPONENTS) ; \
	do \
		(cd $$comp ; $(MAKE) "FRC=$(FRC)" all); \
	done
	@echo "\n\t$(ADDON) build completed"

install:	FRC
	[ -d pkg ] || mkdir pkg
	for comp in $(COMPONENTS) ; \
	do \
		(cd $$comp ; $(MAKE) "FRC=$(FRC)" install); \
	done

package:
	[ -d $(ROOT)/usr/src/pkg ] || mkdir $(ROOT)/usr/src/pkg
	rm -rf $(ROOT)/usr/src/pkg/$(ADDON)
	mkdir $(ROOT)/usr/src/pkg/$(ADDON)
	cd pkg; find . -type f -print | cpio -pdl $(ROOT)/usr/src/pkg/$(ADDON)

clean:
	for comp in $(COMPONENTS) ; \
	do \
		(cd $$comp ; $(MAKE) clean); \
	done

clobber:	clean
	for comp in $(COMPONENTS) ; \
	do \
		(cd $$comp ; $(MAKE) clobber); \
	done
	rm -rf pkg

FRC:
