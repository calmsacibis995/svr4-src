#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)adm:.adm.mk	1.18.1.20"

ROOT =
LIB = $(ROOT)/usr/lib
CRONTABS = $(ROOT)/var/spool/cron/crontabs
LIBCRON = $(ROOT)/etc/cron.d
INS = install
INSDIR = $(ROOT)/etc
INSDIR1 = $(ROOT)/sbin
INSDIR2 = $(ROOT)/usr/sbin
TOUCH=touch

CRON_ENT= adm root sys

CRON_LIB= .proto at.allow at.deny cron.allow cron.deny queuedefs

ETC_SCRIPTS= bcheckrc bupsched brc bsetdate checkall checklist cleanup \
	dfspace fsanck fstab filesave gettydefs \
	_sactab _sysconfig ttydefs ttysrch \
	group inittab ioctl.syscon issue master motd mountall passwd powerfail \
	profile rc rc0 rc2 shutdown stdprofile system system.mtc11 \
	system.mtc12 system.un32 tapesave TIMEZONE umountall uucp \
	adduser deluser cshrc ttytype

OTHER= spellhist ascii

all:	etc_scripts crontab cronlib other

crontab: $(CRON_ENT)

cronlib: $(CRON_LIB)

etc_scripts: $(ETC_SCRIPTS)

other: $(OTHER)

clean:

clobber: clean

install:
	if [ ! -d $(ROOT)/var/spool ] ; then mkdir $(ROOT)/var/spool ; fi
	if [ ! -d $(ROOT)/var/spool/cron ] ; then mkdir $(ROOT)/var/spool/cron ; fi
	if [ ! -d $(ROOT)/var/spool/cron/crontabs ] ; then mkdir $(ROOT)/var/spool/cron/crontabs ; fi
	if [ ! -d $(ROOT)/var/spool/cron/atjobs ] ; then mkdir $(ROOT)/var/spool/cron/atjobs ; fi
	if [ ! -d $(ROOT)/etc/cron.d ] ; then mkdir $(ROOT)/etc/cron.d ; fi
	if [ ! -d $(ROOT)/etc/rc0.d ] ; then mkdir $(ROOT)/etc/rc0.d ; fi
	if [ ! -d $(ROOT)/etc/rc1.d ] ; then mkdir $(ROOT)/etc/rc1.d ; fi
	if [ ! -d $(ROOT)/etc/rc2.d ] ; then mkdir $(ROOT)/etc/rc2.d ; fi
	if [ ! -d $(ROOT)/etc/rc3.d ] ; then mkdir $(ROOT)/etc/rc3.d ; fi
	if [ ! -d $(INSDIR)/init.d ] ; then mkdir $(INSDIR)/init.d ; fi
	if [ ! -d $(ROOT)/usr/share ] ; then mkdir $(ROOT)/usr/share; fi
	if [ ! -d $(ROOT)/usr/share/lib ] ; then mkdir $(ROOT)/usr/share/lib; fi
	if [ ! -d $(ROOT)/var/news ] ; then mkdir $(ROOT)/var/news; fi
	if [ ! -d $(ROOT)/etc/saf ] ; \
	then \
		mkdir $(ROOT)/etc/saf;\
		$(CH)chmod 755 $(ROOT)/etc/saf;\
		$(CH)chown bin $(ROOT)/etc/saf;\
		$(CH)chgrp bin $(ROOT)/etc/saf;\
	fi
	if [ ! -d $(ROOT)/var/saf ] ; \
	then \
		mkdir $(ROOT)/var/saf;\
		$(CH)chmod 755 $(ROOT)/var/saf;\
		$(CH)chown bin $(ROOT)/var/saf;\
		$(CH)chgrp bin $(ROOT)/var/saf;\
	fi
	make -f .adm.mk $(ARGS)

adm::
	-if i386;\
	then cd i386 ;\
	elif i286;\
	then cd i286;\
	fi; \
	cp adm $(CRONTABS)/adm;\
	$(CH)chmod 644 $(CRONTABS)/adm;\
	$(CH)chgrp sys $(CRONTABS)/adm;\
	$(TOUCH) 0101000070 $(CRONTABS)/adm;\
	$(CH)chown root $(CRONTABS)/adm

root::
	cp root $(CRONTABS)/root
	$(CH)chmod 644 $(CRONTABS)/root
	$(CH)chgrp sys $(CRONTABS)/root
	$(TOUCH) 0101000070 $(CRONTABS)/root
	$(CH)chown root $(CRONTABS)/root

sys::
	cp sys $(CRONTABS)/sys
	$(CH)chmod 644 $(CRONTABS)/sys
	$(CH)chgrp sys $(CRONTABS)/sys
	$(TOUCH) 0101000070 $(CRONTABS)/sys
	$(CH)chown root $(CRONTABS)/sys

.proto::
	cp .proto $(LIBCRON)/.proto
	$(CH)chmod 744 $(LIBCRON)/.proto
	$(CH)chgrp sys $(LIBCRON)/.proto
	$(TOUCH) 0101000070 $(LIBCRON)/.proto
	$(CH)chown root $(LIBCRON)/.proto

ascii::
	cp ascii $(ROOT)/usr/share/lib/ascii
	$(CH)chmod 644 $(ROOT)/usr/share/lib/ascii
	$(CH)chgrp bin $(ROOT)/usr/share/lib/ascii
	$(CH)chown bin $(ROOT)/usr/share/lib/ascii

at.allow::
	cp at.allow $(LIBCRON)/at.allow
	$(CH)chmod 644 $(LIBCRON)/at.allow
	$(CH)chgrp sys $(LIBCRON)/at.allow
	$(TOUCH) 0101000070 $(LIBCRON)/at.allow
	$(CH)chown root $(LIBCRON)/at.allow

at.deny::
	if [ ! -f $@ ] ; then touch $@; fi
	cp at.deny $(LIBCRON)/at.deny
	$(CH)chmod 644 $(LIBCRON)/at.deny
	$(CH)chgrp sys $(LIBCRON)/at.deny
	$(TOUCH) 0101000070 $(LIBCRON)/at.deny
	$(CH)chown root $(LIBCRON)/at.deny

cron.allow::
	cp cron.allow $(LIBCRON)/cron.allow
	$(CH)chmod 644 $(LIBCRON)/cron.allow
	$(CH)chgrp sys $(LIBCRON)/cron.allow
	$(TOUCH) 0101000070 $(LIBCRON)/cron.allow
	$(CH)chown root $(LIBCRON)/cron.allow

cron.deny::
	if [ ! -f $@ ] ; then touch $@; fi
	cp cron.deny $(LIBCRON)/cron.deny
	$(CH)chmod 644 $(LIBCRON)/cron.deny
	$(CH)chgrp sys $(LIBCRON)/cron.deny
	$(TOUCH) 0101000070 $(LIBCRON)/cron.deny
	$(CH)chown root $(LIBCRON)/cron.deny

queuedefs::
	cp queuedefs $(LIBCRON)/queuedefs
	$(CH)chmod 644 $(LIBCRON)/queuedefs
	$(CH)chgrp sys $(LIBCRON)/queuedefs
	$(TOUCH) 0101000070 $(LIBCRON)/queuedefs
	$(CH)chown root $(LIBCRON)/queuedefs

#for i386 and i286, bcheckrc is installed by initpkg
bcheckrc::
	-if vax;\
	then cd vax;\
	elif pdp11;\
	then cd pdp11;\
	elif u3b5;\
	then cd u3b5;\
	elif u3b;\
	then cd u3b;\
	fi; \
	if i286 || i386 ;\
	then : ;\
	else \
	    cp bcheckrc.sh $(INSDIR1)/bcheckrc;\
	    cp bcheckrc.sh $(INSDIR2)/bcheckrc;\
	    $(CH)chmod 744 $(INSDIR1)/bcheckrc $(INSDIR2)/bcheckrc;\
	    $(CH)chgrp sys $(INSDIR1)/bcheckrc $(INSDIR2)/bcheckrc;\
	    $(TOUCH) 0101000070 $(INSDIR1)/bcheckrc $(INSDIR2)/bcheckrc;\
	    $(CH)chown root $(INSDIR1)/bcheckrc $(INSDIR2)/bcheckrc;\
	fi

#for i386 and i286, brc is installed by initpkg
brc::
	-if vax;\
	then cd vax;\
	elif pdp11;\
	then cd pdp11;\
	elif u3b5;\
	then cd u3b5;\
	elif u3b;\
	then cd u3b;\
	fi; \
	if i286 || i386 ;\
	then : ;\
	else \
	    cp brc.sh $(INSDIR1)/brc;\
	    cp brc.sh $(INSDIR2)/brc;\
	    $(CH)chmod 744 $(INSDIR1)/brc $(INSDIR2)/brc;\
	    $(CH)chgrp sys $(INSDIR1)/brc $(INSDIR2)/brc;\
	    $(TOUCH) 0101000070 $(INSDIR1)/brc $(INSDIR2)/brc;\
	    $(CH)chown root $(INSDIR1)/brc $(INSDIR2)/brc;\
	fi

#bsetdate is not valid for i286 and i386
bsetdate::
	-if i286 || i386 ;\
	then : ;\
	else \
	    cp bsetdate $(INSDIR)/bsetdate;\
	    $(CH)chmod 744 $(INSDIR)/bsetdate;\
	    $(CH)chgrp sys $(INSDIR)/bsetdate;\
	    $(CH)chown root $(INSDIR)/bsetdate;\
	fi

bupsched::
	-if i386;\
	then cd i386;\
	fi;\
	$(INS) -f $(ROOT)/etc -m 0644 -u root -g sys bupsched

checkall::
	-if vax || pdp11 || u3b5 || u3b;\
	then\
		cp checkall.sh $(INSDIR)/checkall;\
		$(CH)chmod 744 $(INSDIR)/checkall;\
		$(CH)chgrp bin $(INSDIR)/checkall;\
		$(TOUCH) 0101000070 $(INSDIR)/checkall;\
		$(CH)chown bin $(INSDIR)/checkall;\
	fi

checklist::
	-if vax;\
	then cd vax;\
	elif pdp11;\
	then cd pdp11;\
	elif u3b5;\
	then cd u3b5;\
	elif u3b;\
	then cd u3b;\
	elif i286;\
	then cd i286;\
	elif i386;\
	then cd i386;\
	fi; \
	cp checklist $(INSDIR)/checklist;\
	$(CH)chmod 664 $(INSDIR)/checklist;\
	$(CH)chgrp sys $(INSDIR)/checklist;\
	$(TOUCH) 0101000070 $(INSDIR)/checklist;\
 	$(CH)chown root $(INSDIR)/checklist

cleanup::
	cp cleanup $(INSDIR1)/cleanup 
	cp cleanup $(INSDIR2)/cleanup
	$(CH)chmod 744 $(INSDIR1)/cleanup $(INSDIR2)/cleanup
	$(CH)chgrp sys $(INSDIR1)/cleanup $(INSDIR2)/cleanup
	$(CH)chown root $(INSDIR1)/cleanup $(INSDIR2)/cleanup

cshrc::
	cp cshrc $(INSDIR)/cshrc
	$(CH)chmod 644 $(INSDIR)/cshrc
	$(CH)chgrp sys $(INSDIR)/cshrc
	$(CH)chown root $(INSDIR)/cshrc

dfspace::
	cp dfspace $(INSDIR1)/dfspace 
	cp dfspace $(INSDIR2)/dfspace
	$(CH)chmod 755 $(INSDIR1)/dfspace $(INSDIR2)/dfspace
	$(CH)chgrp sys $(INSDIR1)/dfspace $(INSDIR2)/dfspace
	$(CH)chown root $(INSDIR1)/dfspace $(INSDIR2)/dfspace

fsanck::
	-if vax;\
	then cd vax;\
	elif pdp11;\
	then cd pdp11;\
	elif u3b5;\
	then cd u3b5;\
	elif u3b;\
	then cd u3b;\
	elif i286;\
	then cd i286;\
	elif i386;\
	then cd i386;\
	fi; \
	cp fsanck $(INSDIR1)/fsanck;\
	cp fsanck $(INSDIR2)/fsanck;\
	$(CH)chmod 744 $(INSDIR1)/fsanck $(INSDIR2)/fsanck;\
	$(CH)chgrp sys $(INSDIR1)/fsanck $(INSDIR2)/fsanck
	$(CH)chown root $(INSDIR1)/fsanck $(INSDIR2)/fsanck

#for i386 and i286, fstab is installed by initpkg
fstab::
	-if vax;\
	then cd vax;\
	elif pdp11;\
	then cd pdp11;\
	elif u3b5;\
	then cd u3b5;\
	elif u3b;\
	then cd u3b;\
	fi; \
	if i286 || i386 ;\
	then : ;\
	else \
	    cp fstab $(INSDIR)/fstab;\
	    $(CH)chmod 644 $(INSDIR)/fstab;\
	    $(CH)chgrp sys $(INSDIR)/fstab;\
	    $(CH)chown root $(INSDIR)/fstab;\
	fi

filesave::
	-if vax;\
	then cd vax;\
	elif pdp11;\
	then cd pdp11;\
	elif u3b5;\
	then cd u3b5;\
	elif u3b;\
	then cd u3b;\
	elif i286;\
	then cd i286;\
	elif i386;\
	then cd i386;\
	fi; \
	cp filesave.sh $(INSDIR1)/filesave;\
	cp filesave.sh $(INSDIR2)/filesave;\
	$(CH)chmod 744 $(INSDIR1)/filesave $(INSDIR2)/filesave;\
	$(CH)chgrp sys $(INSDIR1)/filesave $(INSDIR2)/filesave;\
	$(TOUCH) 0101000070 $(INSDIR1)/filesave $(INSDIR2)/filesave;\
	$(CH)chown root $(INSDIR1)/filesave $(INSDIR2)/filesave

gettydefs::
	-if vax;\
	then cd vax;\
	elif pdp11;\
	then cd pdp11;\
	elif u3b5;\
	then cd u3b5;\
	elif u3b;\
	then cd u3b;\
	elif u3b2;\
	then cd u3b2;\
	elif i286;\
	then cd i286;\
	elif i386;\
	then cd i386;\
	fi;\
	cp gettydefs $(INSDIR)/gettydefs;\
	$(CH)chmod 644 $(INSDIR)/gettydefs;\
	$(CH)chgrp sys $(INSDIR)/gettydefs;\
	$(TOUCH) 0101000070 $(INSDIR)/gettydefs;\
	$(CH)chown root $(INSDIR)/gettydefs
	
_sactab::
	-if i386;\
	then cd i386;\
	fi;\
	cp _sactab $(ROOT)/etc/saf
	$(CH)chmod 444 $(ROOT)/etc/saf/_sactab;\
	$(CH)chgrp other $(ROOT)/etc/saf/_sactab;\
	$(TOUCH) 0101000070 $(INSDIR)/gettydefs;\
	$(CH)chown root $(ROOT)/etc/saf/_sactab
	
ttydefs::
	-if i386;\
	then cd i386;\
	fi;\
	cp ttydefs $(INSDIR)/ttydefs;\
	$(CH)chmod 664 $(INSDIR)/ttydefs;\
	$(CH)chgrp sys $(INSDIR)/ttydefs;\
	$(TOUCH) 0101000070 $(INSDIR)/ttydefs;\
	$(CH)chown root $(INSDIR)/ttydefs

ttysrch::
	-if i386;\
	then cd i386;\
	fi;\
	$(INS) -f $(ROOT)/etc -m 0644 -u root -g sys ttysrch
	
group::
	cp group $(INSDIR)/group
	$(CH)chmod 644 $(INSDIR)/group
	$(CH)chgrp sys $(INSDIR)/group
	$(TOUCH) 0101000070 $(INSDIR)/group
	$(CH)chown root $(INSDIR)/group

#for i386 and i286, inittab is installed by initpkg
inittab::
	-if vax;\
	then cd vax;\
	elif pdp11;\
	then cd pdp11;\
	elif u3b5;\
	then cd u3b5;\
	elif u3b;\
	then cd u3b;\
	fi; \
	if i286 || i386 ;\
	then : ;\
	else \
	    cp inittab $(INSDIR)/inittab;\
	    $(CH)chmod 644 $(INSDIR)/inittab;\
	    $(CH)chgrp sys $(INSDIR)/inittab;\
	    $(CH)chown root $(INSDIR)/inittab;\
	fi

ioctl.syscon::
	-if vax;\
	then cd vax;\
	elif pdp11;\
	then cd pdp11;\
	elif u3b5;\
	then cd u3b5;\
	elif u3b;\
	then cd u3b;\
	elif i286;\
	then cd i286;\
	elif i386;\
	then cd i386;\
	elif u3b2;\
	then cd u3b2;\
	fi;\
	cp ioctl.syscon $(INSDIR)/ioctl.syscon;\
	$(CH)chmod 644 $(INSDIR)/ioctl.syscon;\
	$(CH)chgrp sys $(INSDIR)/ioctl.syscon;\
	$(TOUCH) 0101000070 $(INSDIR)/ioctl.syscon;\
	$(CH)chown root $(INSDIR)/ioctl.syscon

issue::
	    cp issue $(INSDIR)/issue;\
	    $(CH)chmod 744 $(INSDIR)/issue;\
	    $(CH)chgrp sys $(INSDIR)/issue;\
	    $(CH)chown root $(INSDIR)/issue

master::
	-if vax;\
	then cd vax;\
	elif pdp11;\
	then cd pdp11;\
	elif u3b5;\
	then cd u3b5;\
	elif u3b;\
	then cd u3b;\
	fi; \
	if i286 || i386 ;\
	then : ;\
	else \
		cp master $(INSDIR)/master;\
		$(CH)chmod 644 $(INSDIR)/master;\
		$(CH)chgrp sys $(INSDIR)/master;\
		$(CH)chown root $(INSDIR)/master;\
	fi

motd::
	-if vax;\
	then cd vax;\
	elif pdp11;\
	then cd pdp11;\
	elif u3b5;\
	then cd u3b5;\
	elif u3b;\
	then cd u3b;\
	elif i286;\
	then cd i286;\
	elif i386;\
	then cd i386;\
	elif u3b2;\
	then cd u3b2;\
	fi;\
	sed 1d motd > $(INSDIR)/motd;\
	$(CH)chmod 644 $(INSDIR)/motd;\
	$(CH)chgrp sys $(INSDIR)/motd;\
	$(CH)chown root $(INSDIR)/motd

#for i286 and i386, mountall is installed by initpkg
mountall::
	if i286 || i386 ;\
	then : ;\
	else \
	    cp mountall $(INSDIR1)/mountall;\
	    cp mountall $(INSDIR2)/mountall;\
	    $(CH)chmod 744 $(INSDIR1)/mountall $(INSDIR2)/mountall;\
	    $(CH)chgrp sys $(INSDIR1)/mountall $(INSDIR2)/mountall;\
	    $(CH)chown root $(INSDIR1)/mountall $(INSDIR2)/mountall;\
	fi

passwd::
	-if vax;\
	then cd vax;\
	elif pdp11;\
	then cd pdp11;\
	elif u3b5;\
	then cd u3b5;\
	elif u3b;\
	then cd u3b;\
	elif i286;\
	then cd i286;\
	elif i386;\
	then cd i386;\
	elif u3b2;\
	then cd u3b2;\
	fi;\
	cp passwd $(INSDIR)/passwd;\
	$(CH)chmod 444 $(INSDIR)/passwd;\
	$(CH)chgrp sys $(INSDIR)/passwd;\
	$(TOUCH) 0101000070 $(INSDIR)/passwd;\
	$(CH)chown root $(INSDIR)/passwd

powerfail::
	-if vax;\
	then cd vax;\
	elif pdp11;\
	then cd pdp11;\
	elif u3b5;\
	then cd u3b5;\
	elif u3b;\
	then cd u3b;\
	fi; \
	if i286 || i386 ;\
	then : ;\
	else \
		cp powerfail.sh $(INSDIR1)/powerfail;\
		cp powerfail.sh $(INSDIR2)/powerfail;\
		$(CH)chmod 744 $(INSDIR1)/powerfail $(INSDIR2)/powerfail;\
		$(CH)chgrp sys $(INSDIR1)/powerfail $(INSDIR2)/powerfail;\
		$(TOUCH) 0101000070 $(INSDIR1)/powerfail $(INSDIR2)/powerfail;\
		$(CH)chown root $(INSDIR1)/powerfail $(INSDIR2)/powerfail;\
	fi

profile::
	    cp profile $(INSDIR)/profile;\
	    $(CH)chmod 644 $(INSDIR)/profile;\
	    $(CH)chgrp sys $(INSDIR)/profile;\
	    $(CH)chown root $(INSDIR)/profile

rc::
	-if vax;\
	then cd vax;\
	elif pdp11;\
	then cd pdp11;\
	elif u3b5;\
	then cd u3b5;\
	elif u3b;\
	then cd u3b;\
	fi ; \
	if i286 || i386;\
	then : ;\
	else \
		cp rc.sh $(INSDIR1)/rc;\
		cp rc.sh $(INSDIR2)/rc;\
		$(CH)chmod 744 $(INSDIR1)/rc $(INSDIR2)/rc;\
		$(CH)chgrp sys $(INSDIR1)/rc $(INSDIR2)/rc;\
		$(TOUCH) 0101000070 $(INSDIR1)/rc $(INSDIR2)/rc;\
		$(CH)chown root $(INSDIR1)/rc $(INSDIR2)/rc;\
	fi

#for i386 and i286, rc0 is installed by initpkg
rc0::
	if i286 || i386;\
	then : ;\
	else \
	    cp rc0 $(INSDIR1)/rc0;\
	    cp rc0 $(INSDIR2)/rc0;\
	    $(CH)chmod 744 $(INSDIR)/rc0 $(INSDIR)/rc0;\
	    $(CH)chgrp sys $(INSDIR)/rc0 $(INSDIR)/rc0;\
	    $(CH)chown root $(INSDIR)/rc0 $(INSDIR)/rc0;\
	fi

#for i386 and i286, rc2 is installed by initpkg
rc2::
	if i286 || i386;\
	then : ;\
	else \
	    cp rc2 $(INSDIR1)/rc2;\
	    cp rc2 $(INSDIR2)/rc2;\
	    $(CH)chmod 744 $(INSDIR1)/rc2 $(INSDIR2)/rc2;\
	    $(CH)chgrp sys $(INSDIR1)/rc2 $(INSDIR2)/rc2;\
	    $(CH)chown root $(INSDIR1)/rc2 $(INSDIR2)/rc2;\
	fi

#for i386 and i286, shutdown is installed by initpkg
shutdown::
	-if vax;\
	then cd vax;\
	elif pdp11;\
	then cd pdp11;\
	elif u3b5;\
	then cd u3b5;\
	elif u3b;\
	then cd u3b;\
	fi ; \
	if i286 || i386;\
	then : ;\
	else \
	    cp shutdown.sh $(INSDIR1)/shutdown;\
	    cp shutdown.sh $(INSDIR2)/shutdown;\
	    $(CH)chmod 744 $(INSDIR1)/shutdown $(INSDIR2)/shutdown;\
	    $(CH)chgrp sys $(INSDIR1)/shutdown $(INSDIR2)/shutdown;\
	    $(TOUCH) 0101000070 $(INSDIR1)/shutdown $(INSDIR2)/shutdown;\
	    $(CH)chown root $(INSDIR1)/shutdown $(INSDIR2)/shutdown;\
	fi

tapesave::
	-if vax;\
	then cd vax;\
	elif pdp11;\
	then cd pdp11;\
	elif u3b5;\
	then cd u3b5;\
	elif u3b;\
	then cd u3b;\
	fi ; \
	if i286 || i386;\
	then : ;\
	else \
		cp tapesave.sh $(INSDIR1)/tapesave;\
		cp tapesave.sh $(INSDIR2)/tapesave;\
		$(CH)chmod 744 $(INSDIR)/tapesave $(INSDIR)/tapesave;\
		$(CH)chgrp sys $(INSDIR)/tapesave $(INSDIR)/tapesave;\
		$(TOUCH) 0101000070 $(INSDIR)/tapesave $(INSDIR)/tapesave;\
		$(CH)chown root $(INSDIR)/tapesave $(INSDIR)/tapesave;\
	fi

ttytype:
	-if i386 ; then \
		cd i386; \
	cp ttytype $(INSDIR)/ttytype;\
	$(CH)chmod 644 $(INSDIR)/ttytype;\
	$(CH)chgrp sys $(INSDIR)/ttytype;\
	$(CH)chown root $(INSDIR)/ttytype;\
	fi

stdprofile::
	-if i386 ; then \
		cd i386; \
		cp stdprofile $(INSDIR)/stdprofile; \
		$(CH)chmod 644 $(INSDIR)/stdprofile; \
		$(CH)chgrp sys $(INSDIR)/stdprofile; \
		$(CH)chown root $(INSDIR)/stdprofile; \
	fi 

system::
	-if u3b;\
	then cd u3b;\
		cp system $(INSDIR)/system;\
		$(CH)chmod 644 $(INSDIR)/system;\
		$(CH)chgrp sys $(INSDIR)/system;\
		$(TOUCH) 0101000070 $(INSDIR)/system;\
		$(CH)chown root $(INSDIR)/system;\
	fi

system.mtc11::
	-if u3b;\
	then cd u3b;\
		cp system.32 $(INSDIR)/system.mtc11;\
		$(CH)chmod 644 $(INSDIR)/system.mtc11;\
		$(CH)chgrp sys $(INSDIR)/system.mtc11;\
		$(TOUCH) 0101000070 $(INSDIR)/system.mtc11;\
		$(CH)chown root $(INSDIR)/system.mtc11;\
	fi

system.mtc12::
	-if u3b;\
	then cd u3b;\
		cp system.32 $(INSDIR)/system.mtc12;\
		$(CH)chmod 644 $(INSDIR)/system.mtc12;\
		$(CH)chgrp sys $(INSDIR)/system.mtc12;\
		$(TOUCH) 0101000070 $(INSDIR)/system.mtc12;\
		$(CH)chown root $(INSDIR)/system.mtc12;\
	fi

system.un32::
	-if u3b;\
	then cd u3b;\
		cp system.32 $(INSDIR)/system.un32;\
		$(CH)chmod 644 $(INSDIR)/system.un32;\
		$(CH)chgrp sys $(INSDIR)/system.un32;\
		$(TOUCH) 0101000070 $(INSDIR)/system.un32;\
		$(CH)chown root $(INSDIR)/system.un32;\
	fi

TIMEZONE::
	    cp TIMEZONE $(INSDIR)/TIMEZONE;\
	    $(CH)chmod 744 $(INSDIR)/TIMEZONE;\
	    $(CH)chgrp sys $(INSDIR)/TIMEZONE;\
	    $(CH)chown root $(INSDIR)/TIMEZONE

#for i386 and i286, umountall is installed by initpkg
umountall::
	if i286 || i386 ;\
	then : ;\
	else \
	    cp umountall $(INSDIR1)/umountall;\
	    cp umountall $(INSDIR2)/umountall;\
	    $(CH)chmod 744 $(INSDIR)/umountall $(INSDIR)/umountall;\
	    $(CH)chgrp sys $(INSDIR)/umountall $(INSDIR)/umountall;\
	    $(CH)chown root $(INSDIR)/umountall $(INSDIR)/umountall;\
	fi

#for i386 and i286, uucp is installed by initpkg
uucp::
	-if vax;\
	then cd vax;\
	elif pdp11;\
	then cd pdp11;\
	elif u3b5;\
	then cd u3b5;\
	elif u3b;\
	then cd u3b;\
	fi; \
	if i286 || i386 ;\
	then : ;\
	else \
	    cp uucp $(INSDIR1)/init.d/uucp;\
	    cp uucp $(INSDIR2)/init.d/uucp;\
	    $(CH)chmod 744 $(INSDIR)/init.d/uucp $(INSDIR)/init.d/uucp;\
	    $(CH)chgrp sys $(INSDIR)/init.d/uucp $(INSDIR)/init.d/uucp;\
	    $(CH)chown root $(INSDIR)/init.d/uucp $(INSDIR)/init.d/uucp;\
	fi

spellhist::
	if [ ! -d $(LIB)/spell ] ; then mkdir $(LIB)/spell; fi
	if [ ! -f $@ ] ; then touch $@; fi
	cp $@ $(LIB)/spell/$@
	$(CH)chmod 666 $(LIB)/spell/$@
	$(CH)chgrp sys $(LIB)/spell/$@
	$(CH)chown root $(LIB)/spell/$@

adduser:
	if i386; then cd i386; fi; \
	cp adduser.sh $(INSDIR1)/adduser;  \
	cp adduser.sh $(INSDIR2)/adduser;  \
	chmod 755 $(INSDIR1)/adduser $(INSDIR2)/adduser 

deluser:
	if i386; then cd i386; fi; \
	cp deluser.sh $(INSDIR1)/deluser;  \
	cp deluser.sh $(INSDIR2)/deluser;  \
	chmod 755 $(INSDIR1)/deluser $(INSDIR2)/deluser 
