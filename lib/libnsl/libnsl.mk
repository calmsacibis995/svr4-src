#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)libnsl:libnsl.mk	1.5.5.1"

# 
# Network services library
#

ROOT=
USRLIB=$(ROOT)/usr/lib
SHLIB=$(ROOT)/shlib
INC=$(ROOT)/usr/include

INS = install

all:
	@for i in *;\
	do\
		if test -d $$i;then\
		case $$i in\
		*.*)\
			;;\
		*)\
		cd  $$i;\
		echo "===== $(MAKE) -f $$i.mk all";\
		$(MAKE) -f $$i.mk ROOT=$(ROOT) INC=$(INC); \
		cd .. ;;\
		esac;\
		fi;\
	done

	if [ x$(CCSTYPE) = xCOFF ] ; \
	then \
		rm -f libnsl_s;\
		rm -f libnsl_s.a;\
		$(PFX)mkshlib -q -s _spec -h libnsl_s.a -t libnsl_s;\
	else \
		rm -f libnsl_i.so;\
		$(CC) -G -dy -ztext -o libnsl_i.so *.o *.o_i ;\
		if [ -s des_crypt.o_d -a  -s des_soft.o_d ] ;\
		then \
			rm -f libnsl_d.so;\
			$(CC) -G -dy -ztext -o libnsl_d.so *.o *.o_d ;\
		fi ;\
	fi

install:  all
	if [ x$(CCSTYPE) = xCOFF ] ; \
	then \
		$(INS) -f $(USRLIB) -m 0664 libnsl_s.a ; \
		$(INS) -o -f $(SHLIB) -m 0775 libnsl_s ; \
	else \
		$(INS) -f $(USRLIB) -m 0664 libnsl_i.so ; \
		ln $(USRLIB)/libnsl_i.so $(USRLIB)/libnsl.so ; \
		if [ -s libnsl_d.so ]; \
		then \
			$(INS) -f $(USRLIB) -m 0664 libnsl_d.so ; \
			ln $(USRLIB)/libnsl_d.so $(USRLIB)/libnsl.so ; \
		fi ; \
	fi

clean:
	-rm -f *.o *.o_i *.o_d 
	@for i in *;\
	do\
		if test -d $$i;then\
		case $$i in\
		*.*)\
			;;\
		*)\
		cd  $$i;\
		$(MAKE) -f $$i.mk clean; \
		cd .. ;;\
		esac;\
		fi;\
	done

clobber:	clean
	-rm -f libnsl_s.a libnsl_s libnsl_i.so libnsl_d.so
	@for i in *;\
	do\
		if test -d $$i;then\
		case $$i in\
		*.*)\
			;;\
		*)\
		cd  $$i;\
		$(MAKE) -f $$i.mk clobber; \
		cd .. ;;\
		esac;\
		fi;\
	done
