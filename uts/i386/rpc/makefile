#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-rpc:makefile	1.3"

# Makefile to be used for creating Source Code Provision tapes

VOLDIR	= vol1
SRCDIR	= /scp/4.0
FILES	= [a-z]*
CPIO	= cpio -ocB > 
TAPE	= /dev/rmt/180m
TYPE	= INTERNATIONAL
DOMFILE	= DOMESTIC
EXCLUDE	= comm -13 exclude -	|

EXCLUDE_FILES	= *.exclude
EXCLUDE_DOM	= $(DOMFILE)

$(VOLDIR)::
	-@echo "Hit <return> when the $(TYPE) $@ tape is on $(TAPE): \\c" ; \
	read garbage < /dev/tty
	cd $@			; \
	cat -s $(EXCLUDE_FILES)	  \
	       $(EXCLUDE_DOM)	| \
	egrep -v "^#|^$$" 	| \
	sort -iu > exclude	; \
	cat $(FILES)		| \
	egrep -v "^#|^$$" 	| \
	sort -u 		| \
	$(EXCLUDE)		  \
	( cd $(SRCDIR)		; \
	$(CPIO) $(TAPE) )

all:	$(VOLDIR) domestic

international: $(VOLDIR)

domestic:
	-@for dir in $(VOLDIR) ; \
	do \
		if [ -s $$dir/$(DOMFILE) ] ; then \
			make	FILES="$(FILES) $(DOMFILE)" \
				TYPE=DOMESTIC \
				TAPE=$(TAPE) \
				SRCDIR=$(SRCDIR) \
				EXCLUDE_DOM= \
				VOLDIR=$$dir ; \
		fi ; \
	done
