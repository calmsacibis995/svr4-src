#ident	"@(#)at:uts/i386/master.d/master.AT386.m	1.1"

BUS=AT386
ARCH=AT386
NET=
NET2=
NET3=
LCPP = $(CPP) -P $(MORECPP)

AT386MODS =\
	asy	\
	dma	\
	rtc	\
	cram	\
	hd	\
	fd	\
	lp	\
	kd	\
	kdvm	\
	cmux	\
	gvid

all:
	for i in $(AT386MODS)						;\
	do								 \
	echo  "$$i \c"							;\
	cd $$i								;\
	[ -d $(CONF)/pack.d/$$i ] || mkdir $(CONF)/pack.d/$$i		;\
	[ -f *.c ]   && cp *.c $(CONF)/pack.d/$$i			;\
	[ -f mdev ] && $(LCPP) mdev |					\
		grep -v "^[\*#]ident" >> $(CONF)/cf.d/mdevice		;\
	[ -f sdev ] && $(LCPP) sdev |					\
		grep -v "^[\*#]ident" > $(CONF)/sdevice.d/$$i 		;\
	[ -f mfsys ] && $(LCPP) mfsys |					\
		grep -v "^[\*#]ident" > $(CONF)/mfsys.d/$$i		;\
	[ -f sfsys ] && $(LCPP) sfsys |					\
		grep -v "^[\*#]ident" > $(CONF)/sfsys.d/$$i 		;\
	[ -f node ] && $(LCPP) node |					\
		grep -v "^[\*#]ident" > $(CONF)/node.d/$$i 		;\
	[ -f init ] && $(LCPP) init |					\
		grep -v "^[\*#]ident" > $(CONF)/init.d/$$i 		;\
	[ -f $(CONF)/pack.d/$$i/* ] && chmod +w $(CONF)/pack.d/$$i/*	;\
	cd ..								;\
	done

clean:

clobber:	clean
	for i in $(AT386MODS)                ;\
	do						 \
		rm -rf $(CONF)/pack.d/$$i		;\
	done
