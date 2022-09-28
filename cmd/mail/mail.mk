#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)mail:mail.mk	1.36.1.3"
# "@(#)ma.mk	2.48 'attmail mail(1) command'"
#
#	mail make file
#
PREFIX=ma
HDR = mail.h maillock.h libmail.h myregexpr.h s_string.h config.h
ROOT = 
USR_LIB = /usr/lib
USR_INC = /usr/include
MPDIR = $(USR_LIB)/mail
SURRDIR = $(USR_LIB)/mail/surrcmd
VERS = SVR4
# If VERS = SVR4, use the following...
MBINDIR = /usr/bin
CBINDIR = /usr/bin
MBOXDIR = /var/mail
FILEDIR = /etc/mail
SHRLIB  = /usr/share/lib
SYMLINK = ln -s
# If VERS == SVR3, use the following...
#MBINDIR = /bin
#CBINDIR = /usr/bin
#MBOXDIR = /usr/mail
#FILEDIR = /etc/mail
#SHRLIB  = /usr/lib
#SYMLINK = :
#
INS = install
DIRS =	$(ROOT)$(MBOXDIR) \
	$(ROOT)$(MBOXDIR)/:saved \
	$(ROOT)$(MPDIR) \
	$(ROOT)$(FILEDIR) \
	$(ROOT)$(FILEDIR)/lists \
	$(ROOT)$(SURRDIR) \
	$(ROOT)$(SHRLIB)/mail
LIBMAIL = libmail.a
CPPDEFS = -I. -D$(VERS)
CFLAGS =  -O $(CPPDEFS)
#CFLAGS = -g $(CPPDEFS)
LD_FLAGS = -s $(PERFLIBS) $(LDFLAGS)
#LD_FLAGS = $(PERFLIBS) $(LDFLAGS)
LD_LIBS = $(LIBMAIL)
PRODUCT = mail
LPDEST =
TMPDIR = /usr/tmp

# More Build Boilerplate
MKSL=pmksl
GET=pget
MAKEFILE=$(PREFIX).mk
MAKE= make
SLIST=$(PREFIX).sl
ID=$(PREFIX)id

LINT= $(PFX)lint
.SUFFIXES: .ln
.c.ln:
	$(LINT) $(CPPDEFS) -c $*.c > $*.lerr

MPSRC = _mail_pipe.c
MPOBJS = $(MPSRC:.c=.o) $(ID).o

CBSRC = ckbinarsys.c
CBOBJS = $(CBSRC:.c=.o) $(ID).o

NSRC =	notify notify2.c
NOBJS = notify2.o

VSRC = vacation.sh vacation2.sh STD_VAC_MSG

LTOOLS = pmkid

SRC = \
	add_recip.c arefwding.c cat.c ckdlivopts.c \
	cksurg_rc.c cksaved.c clr_hinfo.c cmdexpand.c \
	copyback.c copylet.c copymt.c createmf.c del_recipl.c Dout.c \
	delete.c doFopt.c done.c doopen.c dumpaff.c dumprcv.c errmsg.c \
	findSurg.c gendeliv.c getarg.c getcomment.c \
	gethead.c getsurr.c goback.c init.c initsurr.c \
	isheader.c isit.c islocal.c istext.c legal.c lock.c \
	mailcomp.c main.c mkdead.c mta_ercode.c new_recipl.c notme.c \
	parse.c pckaffspot.c pckrcvspot.c pickFrom.c pipletr.c \
	poplist.c printhdr.c printmail.c pushlist.c \
	savehdrs.c sel_disp.c send.c sendlist.c sendmail.c \
	sendsurg.c setsurg_rc.c setsurg_bt.c setsig.c stamp.c \
	Tout.c tokdef.c translate.c
OBJS = $(SRC:.c=.o) $(ID).o

LIBSRC = abspath.c basename.c casncmp.c config.c copystream.c delempty.c \
	getdomain.c \
	maillock.c myregexpr.c notifyu.c popenvp.c \
	s_string.c setup_exec.c strmove.c \
	skipspace.c substr.c systemvp.c trimnl.c xgetenv.c
LIBOBJS = $(LIBSRC:.c=.o)

DSRC =	my_open.c
DOBJS = 

LOBJS = $(SRC:.c=.ln) $(ID).ln
LERR = $(SRC:.c=.lerr) $(ID).lerr

ALSRC = alias.c
ALOBJS = alias.o init.o $(ID).o

ALLSRC = $(SRC) $(DSRC) $(MPSRC) $(CBSRC) $(NSRC) $(VSRC) $(HDR) $(MAKEFILE) \
	$(LIBSRC) $(LTOOLS) mailsurr binarsys namefiles $(ALSRC)

all:	allmail allsmtp
allmail: mail mail_pipe ckbinarsys notify notify2 mailalias $(LIBMAIL) std_vac_msg vacation vacation2
allsmtp: ; cd smtp; $(MAKE) -f smtp.mk smtp

mail:	$(OBJS)  $(DOBJS) $(LIBMAIL)
	$(CC) $(LD_FLAGS) -o $(PRODUCT) $(OBJS) $(DOBJS) $(LD_LIBS)

mail_pipe:	$(MPOBJS) $(LIBMAIL)
	$(CC) $(LD_FLAGS) -o mail_pipe $(MPOBJS) $(LD_LIBS)

ckbinarsys:	$(CBOBJS) $(LIBMAIL)
	$(CC) $(LD_FLAGS) -o ckbinarsys $(CBOBJS) $(LD_LIBS)

notify2:	$(NOBJS) $(LIBMAIL)
	$(CC) $(LD_FLAGS) -o notify2 $(NOBJS) $(LD_LIBS)

mailalias: $(ALOBJS) $(LIBMAIL)
	$(CC) $(LD_FLAGS) -o mailalias $(ALOBJS) $(LD_LIBS)

debug:
	make "CFLAGS=-g -DDEBUG" DOBJS=my_open.o LD_FLAGS= PRODUCT=Dmail mail

$(LIBMAIL): $(LIBOBJS)
	$(AR) rv $(LIBMAIL) $?

llib-lmail.ln: llib-lmail maillock.h libmail.h
	case $(VERS) in \
		SVR4 )	$(LINT) -D$(VERS) -I. -I$(INC) -I$(ROOT)/usr/include -o mail llib-lmail ;; \
		*    )	$(CC) -E -C -Dlint -D$(VERS) -I. llib-lmail | \
			/usr/lib/lint1 -vx -H$(TMPDIR)/hlint.$$$$ > $(TMPDIR)/nlint$$$$ && \
			mv $(TMPDIR)/nlint$$$$ llib-lmail.ln; \
			rm -f $(TMPDIR)/hlint$$$$ ;; \
	esac

std_vac_msg: STD_VAC_MSG
	grep -v '^#.*ident' < STD_VAC_MSG > std_vac_msg

EDITPATH=	\
	case $(VERS) in \
	    SVR4 ) sed -e 's!REAL_PATH!/usr/bin!g' \
		       -e 's!USR_SHARE_LIB!$(SHRLIB)!g' \
		       -e 's!VAR_MAIL!$(MBOXDIR)!g' ;; \
	    *    ) sed -e 's!REAL_PATH!/bin:/usr/bin!g' \
		       -e 's!USR_SHARE_LIB!$(SHRLIB)!g' \
		       -e 's!VAR_MAIL!$(MBOXDIR)!g' ;; \
	esac < $? > $@

vacation: vacation.sh
	$(EDITPATH)

vacation2: vacation2.sh
	$(EDITPATH)

notify: notify.sh
	$(EDITPATH)

ckdirs:
	@echo
	@echo mail requires the directories:
	@for i in $(DIRS); \
	do \
		echo "\t$${i}"; \
	done;
	@echo
	@echo Checking for existence of directories
	@echo
	@for i in $(DIRS); \
	do \
		if [ -d $${i} ]; \
		then \
			echo "\t$${i} exists";\
		else \
			echo "\t$${i} does not exist";\
			echo "\tCreating $${i}"; \
			mkdir $${i}; \
		fi; \
		echo "\t$(CH)chmod 775 $${i}"; \
		$(CH)chmod 775 $${i}; \
		echo "\t$(CH)chgrp mail $${i}"; \
		$(CH)chgrp mail $${i}; \
		echo; \
	done

calls:
	cflow $(CPPDEFS) $(SRC) > cflow.out
	cflow -r $(CPPDEFS) $(SRC) > cflow-r.out
	calls $(CPPDEFS) $(SRC) > calls.out
	cscope -b $(SRC) > calls.out
	ccalls | sort -u > ccalls.edges
	ccalls -p | sort -u > ccalls.params

install: ckdirs installmail installsmtp

installmail: allmail
	case $(VERS) in \
		SVR4 )	rm -f $(ROOT)$(MBINDIR)/$(PRODUCT) $(ROOT)$(MBINDIR)/r$(PRODUCT) ; \
			$(INS) -f $(ROOT)$(MBINDIR) -m 2511 -g mail -u bin $(PRODUCT) ; \
			ln $(ROOT)$(MBINDIR)/$(PRODUCT) $(ROOT)$(MBINDIR)/r$(PRODUCT) ; \
			$(INS) -f $(ROOT)/usr/share/lib/mail -m 644 -g mail -u bin std_vac_msg ; \
			;; \
		* )	rm -f $(ROOT)/bin/$(PRODUCT) $(ROOT)/bin/r$(PRODUCT) ; \
			$(INS) -f $(ROOT)/bin -m 2511 -g mail -u bin $(PRODUCT) ; \
			ln $(ROOT)/bin/$(PRODUCT) $(ROOT)/bin/r$(PRODUCT) ; \
			$(INS) -f $(ROOT)/usr/lib/mail -m 644 -g mail -u bin std_vac_msg ; \
			;; \
	esac

	rm -f $(ROOT)$(MPDIR)/mail_pipe
	$(INS) -f $(ROOT)$(MPDIR) -m 4511 -g mail -u root mail_pipe

	$(INS) -f $(ROOT)$(SURRDIR) -m 555 -g mail -u bin ckbinarsys

	$(INS) -f $(ROOT)$(CBINDIR) -m 755 -g bin -u bin notify
	$(INS) -f $(ROOT)$(MPDIR) -m 755 -g bin -u bin notify2

	$(INS) -f $(ROOT)$(CBINDIR) -m 755 -g bin -u bin vacation
	$(INS) -f $(ROOT)$(MPDIR) -m 755 -g bin -u bin vacation2
	$(INS) -f $(ROOT)$(FILEDIR) -m 644 -g mail -u bin namefiles

	$(INS) -f $(ROOT)$(FILEDIR) -m 644 -g mail -u bin mailsurr
	rm -f $(ROOT)$(USR_LIB)/mail/mailsurr
	$(SYMLINK) $(ROOT)$(FILEDIR)/mailsurr $(ROOT)$(USR_LIB)/mail/mailsurr

	$(INS) -f $(ROOT)$(FILEDIR) -m 644 -g mail -u bin binarsys
	rm -f $(ROOT)$(USR_LIB)/binarsys
	$(SYMLINK) $(ROOT)$(FILEDIR)/binarsys $(ROOT)$(USR_LIB)/binarsys
	
	$(INS) -f $(ROOT)$(USR_LIB) -m 644 -g bin -u bin $(LIBMAIL)
	$(INS) -f $(ROOT)$(USR_INC) -m 644 -g bin -u bin maillock.h

	$(INS) -f $(ROOT)$(CBINDIR) -m 755 -g bin -u bin mailalias

installsmtp: allsmtp
	cd smtp; $(MAKE) -f smtp.mk install

$(OBJS) $(DOBJS):	$(HDR)

$(LOBJS): $(HDR)

print:
	pr -n $(ALLSRC) | lp $(LPDEST)

lint: $(LOBJS) llib-lmail.ln /tmp
	@echo ==== libmail ====
	$(LINT) $(CPPDEFS) $(LIBSRC)
	@echo ==== mail ====
	for i in $(LERR);do echo ==== $$i ====; cat $$i; done
	$(LINT) $(LOBJS) llib-lmail.ln
	@echo ==== ckbinarsys ====
	$(LINT) $(CPPDEFS) $(CBSRC) casncmp.c $(ID).c
	@echo ==== mail_pipe ====
	$(LINT) $(CPPDEFS) $(MPSRC) xgetenv.c setup_exec.c skipspace.c $(ID).c

clean: cleanmail cleansmtp

cleanmail:
	-rm -f *.o
cleansmtp:
	cd smtp; $(MAKE) -f smtp.mk clean

clobber: clobbermail clobbersmtp

clobbermail: cleanmail
	rm -f mail rmail Dmail a.out mail_pipe ckbinarsys notify2 \
		std_vac_msg core garb makeout nohup.out mailalias \
		llib-lmail.ln vacation vacation2 notify libmail.a
clobbersmtp:
	cd smtp; $(MAKE) -f smtp.mk clobber

populate:
	$(GET) -P$(PREFIX) -S$(SLIST) -M $(ALLSRC)

# Bring slist up-to-date.
$(SLIST):	$(ALLSRC)
	pget -e $(SLIST)
	rm  -f $(SLIST)
	$(MKSL) $(ALLSRC) > $(SLIST)
	pupdate $(SLIST)

$(ID).c:	
	$(CH)chmod 775 ./pmkid
	./pmkid	$(SLIST)

chgrp: 
	chgrp mail mail
	chmod g+s mail
