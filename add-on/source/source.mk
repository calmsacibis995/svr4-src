#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)source:source.mk	1.1"

TAPE=/dev/rmt/c0s0r
SECURITY="  "
BACKCOMP=-Hodc

all: media security

install: all
	chmod +x fixcode
	./fixcode change `pwd`
	echo "Source for AT Drivers"

package: Files
	echo "Begin: `date`"
	-( A=`pwd`; \
	cd ${ROOT}; (cat $$A/Files | `cat $$A/security` | \
		xargs ls -d) | \
		cpio -ocC102400 $(BACKCOMP) -O`cat $$A/media` \
	)
	echo "Finish: `date`"

clean:
	-rm -f media security

clobber: clean
	chmod +x fixcode
	./fixcode original `pwd`
	-rm Files

media:
	@( echo "Are you creating a 9 track tape or a cartridge tape?"; \
	echo "   (enter 9, c, or return for c)"; read a; \
	if [ "$$a" = "9" ]; then echo "/dev/rmt0" > media; echo "9 track"; \
		else echo "$(TAPE)" > media; echo "cartridge tape"; fi \
	) 

security:
	@( echo "Are you creating an international or domestic tape?"; \
	echo "   (enter d, i, or return for i)"; read a; \
	if [ "$$a" = "d" ]; then echo 'cat -' > security; echo "Domestic Tape"; \
		else echo 'sed -e /usr\/src\/cmd\/crypt\/crypt.c/d -e /usr\/src\/lib\/libcrypt\/des_decrypt.c/d -e /usr\/src\/uts\/i386\/des\/des_crypt.c/d -e /usr\/src\/uts\/i386\/des\/des_soft.c/d -e /usr\/src\/lib\/libnsl\/des\/des_crypt.c/d -e /usr\/src\/lib\/libnsl\/des\/des_soft.c/d' > security; echo "International Tape"; fi \
	) 

Files: Files.base Files.scde
	cat Files.base Files.scde | grep -v "^#" | sort -u > Files
