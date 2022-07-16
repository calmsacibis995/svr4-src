#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)scsi.in:scsi.in.mk	1.3"

##SYSTEMENV	= 3
SYSTEMENV	= 4
ADDON		= scsi.in
COMPONENTS	= ID install io sys cmd test
##COMPONENTS	= ID io sys 
FRC =

all:
	@echo "\n\tCopying sd01 driver and utility from add-on/scsi"
	cd ../scsi; find ID/sd01 cmd io/sd01 sys -print|cpio -pduv ../scsi.in
##	cd ../scsi; find sys -print|cpio -pduv ../scsi.in
	for comp in $(COMPONENTS); \
	do \
		(cd $$comp; make "ROOT=$(ROOT)" "FRC=$(FRC)" \
			"SYSTEMENV=$(SYSTEMENV)" $@); \
	done
	@echo "\n\t$(ADDON) build completed"

install: FRC all
	[ -d pkg ] || mkdir pkg
	for comp in $(COMPONENTS); \
	do \
		(cd $$comp; make "ROOT=$(ROOT)" "FRC=$(FRC)" \
			"SYSTEMENV=$(SYSTEMENV)" $@); \
	done

package:
	[ -d $(ROOT)/usr/src/pkg ] || mkdir $(ROOT)/usr/src/pkg
	[ -d $(ROOT)/usr/src/pkg/scsi.in.src ] || mkdir $(ROOT)/usr/src/pkg/$(ADDON).src
	cd pkg; find . -type f -print | cpio -pdum $(ROOT)/usr/src/pkg/$(ADDON).src
	rm -rf $(ROOT)/usr/src/pkg/$(ADDON)
	cd pkg; \
		if [ -f Install ];then \
		(echo Size; find . -type f ! -name Size -print) \
			| cpio -ocB > $(ROOT)/usr/src/pkg/$(ADDON) ;\
		fi

clean:
	for comp in $(COMPONENTS); \
	do \
		(cd $$comp; make "ROOT=$(ROOT)" "FRC=$(FRC)" clean); \
	done

clobber: clean
	for comp in $(COMPONENTS); \
	do \
		(cd $$comp; make "ROOT=$(ROOT)" "FRC=$(FRC)" clobber); \
	done
	(cd ../scsi; find ID/sd01 cmd io/sd01 sys -type f -print) | xargs rm -f
	rm -rf pkg

FRC:
