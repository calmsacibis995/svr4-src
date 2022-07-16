#ident	"@(#)makefile	1.2	91/09/14	JPB"
#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)curses:screen/makefile	1.40.1.18"

#
# cursor package maker
# SYSV makefile for UNIX System V
#
#

# Sources
LIBNAME=	libcurses.a
CTAGS=		ctags
BUILT=		keycaps keyname.c termcap.c tiget.c tnames.c tinames.c tifnames.c tifget.c
PHEADERS= 	term.h curses.h
TMP=		./tmp

DESTLIB=	$(ROOT)/usr/lib
DESTBIN=	$(ROOT)/usr/bin
DESTDIR=	$(ROOT)

AR=		TMPDIR=$(TMP) ar
CHMOD=		/bin/chmod
CPIO=		/usr/bin/cpio
ECHO= 		/bin/echo
ED= 		/bin/ed
LINT=		$(PFX)lint
LN=		/bin/ln
LS=		/bin/ls
MKDIR=		/bin/mkdir
RM=		/bin/rm
SHELL=		/bin/sh
UNIQ=		/usr/bin/uniq

INC = $(ROOT)/usr/include
INCLUDE = -I. -I $(INC)


CPPDEFS= 	-I.
DCPPDEFS= 	-DDEBUG $(CPPDEFS)
DFLAGS=		-g $(DCPPDEFS)
TFLAGS=		-O $(DCPPDEFS)
CFLAGS=		-O $(CPPDEFS)
SUBOBJS=
LIBS= 
LINK_MODE=
CCSBIN=$(ROOT)/usr/ccs/bin
CCSLIB=$(ROOT)/usr/ccs/lib

.SUFFIXES: .p .d .t

HEADERS=	curses.h unctrl.h curses_inc.h curshdr.h term.h

#
#	tic source and header files
#
TICHDRS=	compiler.h object.h

TICSRC=		tic_main.c tic_hash.c tic_captab.c tic_scan.c tic_error.c \
		tic_parse.c tic_read.c tnames.c tinames.c tifnames.c

TICOBJS=	$(TICSRC:.c=.o)

CHDRS=		print.h otermcap.h

BSRC=		print.c iexpand.c
BOBJ=		print.o iexpand.o

CSRC=		captoinfo.c otermcap.c $(BSRC)
COBJ=		captoinfo.o otermcap.o $(BOBJ)

ISRC=		infocmp.c cexpand.c infotocap.c $(BSRC)
IOBJ=		infocmp.o cexpand.o infotocap.o $(BOBJ)

tools:		$(LIBNAME) tic tput captoinfo infocmp

CFILES1=	_addch.c _addchnstr.c _addchstr.c _addnstr.c \
		_addstr.c _attroff.c _attron.c _attrset.c \
		_beep.c _bkgd.c _bkgdset.c _border.c _box.c \
		_clear.c _clrtobot.c _clrtoeol.c _crmode.c \
		_del_curterm.c _delch.c _deleteln.c _echo.c \
		_echochar.c _erase.c _fixterm.c _flash.c \
		_garbagdlins.c _garbagedwin.c _getch.c \
		_getstr.c _halfdelay.c _hline.c _inch.c \
		_inchnstr.c _inchstr.c _innstr.c _insch.c \
		_insdelln.c _insertln.c _insnstr.c _insstr.c \
		_instr.c _intrflush.c _meta.c _move.c _mvaddch.c \
		_mvaddchnstr.c _mvaddchstr.c _mvaddnstr.c \
		_mvaddstr.c _mvdelch.c _mvgetch.c _mvgetstr.c \
		_mvhline.c _mvinch.c _mvinchnstr.c _mvinchstr.c \
		_mvinnstr.c _mvinsch.c _mvinsnstr.c _mvinsstr.c \
		_mvinstr.c _mvvline.c _mvwaddch.c _mvwaddchnst.c \
		_mvwaddchstr.c _mvwaddnstr.c _mvwaddstr.c _mvwdelch.c \
		_mvwgetch.c _mvwgetstr.c _mvwhline.c _mvwinch.c \
		_mvwinchnst.c _mvwinchstr.c _mvwinnstr.c _mvwinsch.c \
		_mvwinsnstr.c _mvwinsstr.c _mvwinstr.c _mvwvline.c \
		_newterm.c _nl.c _nocrmode.c _noecho.c _nonl.c \
		_noqiflush.c _overlay.c _overwrite.c _qiflush.c \
		_refresh.c _resetterm.c _saveterm.c _scr_init.c \
		_scr_restore.c _scr_set.c _scrl.c _scroll.c \
		_set_curterm.c _set_term.c _setscrreg.c _slk_init.c \
		_standend.c _standout.c _subpad.c _timeout.c \
		_touchline.c _unctrl.c _vline.c _waddchstr.c \
		_waddstr.c _wclear.c _wdeleteln.c _werase.c \
		_winsertln.c _winsstr.c _wstandend.c _wstandout.c \
		V2.__sscans.c V2._sprintw.c V2.makenew.c V3.box.c \
		V3.initscr.c V3.m_addch.c V3.m_addstr.c \
		V3.m_clear.c V3.m_erase.c V3.m_initscr.c V3.m_move.c \
		V3.m_newterm.c V3.m_refresh.c V3.newterm.c \
		V3.pechochar.c V3.upd_old_y.c V3.vidattr.c V3.vidputs.c \
		V3.waddch.c V3.wattroff.c V3.wattron.c V3.wattrset.c \
		V3.wechochar.c V3.winsch.c baudrate.c can_change.c \
		color_cont.c cbreak.c chkinput.c clearok.c copywin.c \
		curs_set.c curserr.c curses.c def_prog.c delay.c \
		delay_out.c delkey.c delkeymap.c delscreen.c delterm.c \
		delwin.c derwin.c doupdate.c draino.c dupwin.c endwin.c \
		erasechar.c flushinp.c getattrs.c getbegyx.c getbkgd.c \
		getmaxyx.c getparyx.c getsyx.c gettmode.c getwin.c \
		getyx.c has_colors.c has_ic.c has_il.c idcok.c idlok.c \
		immedok.c init_acs.c init_color.c init_costs.c \
		init_pair.c initscr.c is_linetou.c is_wintou.c \
		keyname.c keypad.c killchar.c leaveok.c

CFILES2=	longname.c makenew.c memSset.c meta.c mouse.c mvcur.c \
		mvderwin.c mvprintw.c mvscanw.c mvwin.c mvwprintw.c \
		mvwscanw.c napms.c newkey.c newpad.c newscreen.c \
		newwin.c nocbreak.c nodelay.c noraw.c \
		notimeout.c outch.c overlap.c pair_cont.c \
		pechochar.c pnoutref.c prefresh.c printw.c putp.c \
		putwin.c quick_echo.c raw.c redrawwin.c reset_sh.c \
		resetty.c restart.c ring.c ripoffline.c savetty.c \
		scanw.c scr_all.c scr_dump.c scr_ll_dump.c scr_reset.c \
		scrollok.c setcurscreen.c setcurterm.c setecho.c \
		setkeymap.c setnonl.c setqiflush.c setsyx.c setterm.c \
		setupterm.c slk_atroff.c slk_atron.c slk_atrset.c \
		slk_clear.c slk_label.c slk_noutref.c slk_refresh.c \
		slk_restore.c slk_set.c slk_start.c slk_touch.c \
		start_col.c subwin.c syncok.c tcsearch.c termattrs.c \
		termcap.c termerr.c termname.c tgetch.c tgoto.c tifget.c \
		tifnames.c tiget.c tinames.c tinputfd.c tnames.c \
		touchwin.c tparm.c tputs.c trace.c tstp.c ttimeout.c \
		typeahead.c unctrl.c ungetch.c use_env.c \
		untouchwin.c vidputs.c vidupdate.c vsscanf.c vwprintw.c \
		vwscanw.c waddch.c waddchnstr.c waddnstr.c wattroff.c \
		wattron.c wattrset.c wbkgd.c wbkgdset.c wborder.c \
		wclrtobot.c wclrtoeol.c wdelch.c wechochar.c wgetch.c \
		wgetstr.c whline.c winch.c winchnstr.c winchstr.c \
		winnstr.c winsch.c winsdelln.c winsnstr.c winstr.c \
		wmove.c wnoutref.c wprintw.c wredrawln.c wrefresh.c \
		wscanw.c wscrl.c wsetscrreg.c wsyncdown.c wsyncup.c \
		wtimeout.c wtouchln.c wvline.c 

all: date $(LIBNAME) llib-lcurses.ln tools
 
date:
	@$(ECHO) Started: `date`

# Regular, optimized objects
OBJS1=	$(CFILES1:.c=.o)
OBJS2=	$(CFILES2:.c=.o)
$(OBJS1): $(HEADERS)
$(OBJS2): $(HEADERS)

# Profiled, optimized objects
#POBJS=	$(CFILES:.c=.p)
#$(POBJS): $(HEADERS)

# Debugging objects
#DOBJS=	$(CFILES:.c=.d)
#$(DOBJS): $(HEADERS)

# Tracing objects
#TOBJS=	$(CFILES:.c=.t)
#$(TOBJS): $(HEADERS)

.c.p:
	-mv $*.o $*.O
	$(CC) $(CFLAGS) -p -c $*.c
	mv $*.o $*.p
	-mv $*.O $*.o
.c.d:
	-mv $*.o $*.O
	$(CC) $(DFLAGS) -c $*.c
	mv $*.o $*.d
	-mv $*.O $*.o
.c.t:
	-mv $*.o $*.O
	$(CC) $(TFLAGS) -c $*.c
	mv $*.o $*.t
	-mv $*.O $*.o

#obj1:	$(OBJS1)

#obj2:	$(OBJS2)

#$(LIBNAME): obj1 obj2

$(LIBNAME): $(OBJS1) $(OBJS2)
	@$(ECHO)
	@$(ECHO) Building libcurses archive
	@$(ECHO)
	$(AR) rv $(LIBNAME) $(OBJS1)
	$(AR) rv $(LIBNAME) $(OBJS2)
	@$(ECHO)
	@$(ECHO) libcurses has been archived
	@$(ECHO)

p:	p$(LIBNAME)
p$(LIBNAME): $(POBJS)
	$(RM) -f x.c
	$(AR) rv p$(LIBNAME) $?

d:	d$(LIBNAME)
d$(LIBNAME): $(HEADERS) $(DOBJS)
	$(RM) -f x.c
	$(AR) lrv d$(LIBNAME) $?

t:	t$(LIBNAME)
t$(LIBNAME): $(OBJS)
	$(RM) -f x.c
	$(AR) rv t$(LIBNAME) $?

setupterm.o: term.h 
term.h tnames.c tinames.c tifnames.c: caps maketerm.ed
	@if [ ! -d "$(TMP)" ] ;\
	then	$(ECHO) "\t$(TMP) not found.";\
		$(ECHO) "\n\tCreating $(TMP)\n";\
		$(MKDIR) $(TMP); \
	fi;
	@$(ECHO)
	@$(ECHO) Building term.h
	@$(ECHO)
	$(ED) - < maketerm.ed
	$(CH) $(CHMOD) 0664 term.h tnames.c tinames.c tifnames.c
	@$(ECHO)
	@$(ECHO) "term.h has been built"
	@$(ECHO)

keycaps: caps
	@$(ECHO)
	@$(ECHO) Building keycaps
	@$(ECHO)
	$(RM) -f $(TMP)/keycaps keycaps
	grep "KEY_" caps > keycaps
	@$(ECHO) 
	@$(ECHO) "keycaps has been built"
	@$(ECHO)

curses.h: keycaps curses.ed
	@if [ ! -d "$(TMP)" ] ;\
	then	$(ECHO) "\t$(TMP) not found.";\
		$(ECHO) "\n\tCreating $(TMP)\n";\
		$(MKDIR) $(TMP); \
	fi;
	@$(ECHO) 
	@$(ECHO) Building ./curses.h
	@$(ECHO) 
	$(RM) -f curses.h
	$(ED) - < curses.ed
	$(CH) $(CHMOD) 0664 curses.h
	$(RM) -f $(TMP)/keycaps
	@$(ECHO) 
	@$(ECHO) "curses.h has been built"
	@$(ECHO) 

keyname.c: keycaps keyname.sh
	@$(ECHO)
	@$(ECHO) Building keyname.c
	@$(ECHO)
	$(RM) -f keyname.c
	sh keyname.sh
	$(CH) $(CHMOD) 0664 keyname.c
	@$(ECHO)
	@$(ECHO) "keyname.c has been built"
	@$(ECHO)

setkeymap.c: keycaps
	@if tail -1 keycaps | grep 'KEY_MOUSE' > /dev/null; \
	then touch setkeymap.c; \
	else $(ECHO) new keys!! must change setkeymap.c; \
	     exit 1; \
	fi


termcap.c:	termcap.ed caps
	@if [ ! -d "$(TMP)" ] ;\
	then	$(ECHO) "\t$(TMP) not found.";\
		$(ECHO) "\n\tCreating $(TMP)\n";\
		$(MKDIR) $(TMP); \
	fi;
	@$(ECHO) 
	@$(ECHO) Building termcap.c
	@$(ECHO)
	$(RM) -f termcap.c
	$(ED) - < termcap.ed
	$(RM) -f bool num str
	$(CH) $(CHMOD) 0664 termcap.c
	@$(ECHO) 
	@$(ECHO) "termcap.c has been built"
	@$(ECHO)

tifget.c:	tifget.ed caps
	@if [ ! -d "$(TMP)" ] ;\
	then	$(ECHO) "\t$(TMP) not found.";\
		$(ECHO) "\n\tCreating $(TMP)\n";\
		$(MKDIR) $(TMP); \
	fi;
	@$(ECHO)
	@$(ECHO) Building tifget.c
	@$(ECHO)
	$(ED) - < tifget.ed
	$(CH) $(CHMOD) 0664 tifget.c
	@$(ECHO)
	@$(ECHO) "tifget.c has been built"
	@$(ECHO)

tiget.c:	tiget.ed caps
	@if [ ! -d "$(TMP)" ] ;\
	then	$(ECHO) "\t$(TMP) not found.";\
		$(ECHO) "\n\tCreating $(TMP)\n";\
		$(MKDIR) $(TMP); \
	fi;
	@$(ECHO)
	@$(ECHO) Building tiget.c
	@$(ECHO)
	$(ED) - < tiget.ed
	$(CH) $(CHMOD) 0664 tiget.c
	@$(ECHO)
	@$(ECHO) "tiget.c has been built"
	@$(ECHO)

ticclob:
	$(RM) -f $(TICOBJS) tic

tic_captab.o:	tic_captab.c compiler.h
tic_error.o:	tic_error.c compiler.h
tic_hash.o:	tic_hash.c compiler.h term.h
tic_main.o:	tic_main.c compiler.h curses.h
tic_parse.o:	tic_parse.c compiler.h object.h
tic_read.o:	tic_read.c object.h
tic_scan.o:	tic_scan.c compiler.h

new:	clean tags $(LIBNAME)

allnew:	clean tags $(LIBNAME) d$(LIBNAME) p$(LIBNAME) t$(LIBNAME)

allfour: $(LIBNAME) d$(LIBNAME) p$(LIBNAME) t$(LIBNAME)

clean:
	@$(ECHO)
	@$(ECHO) "Removing all .o's"
	@$(ECHO)
	@$(RM) -f $(OBJS1) $(LIBNAME)
	@$(RM) -f $(OBJS2)
	@$(ECHO)
	@$(ECHO) "Removing all .d's"
	@$(ECHO)
	@$(RM) -f $(DOBJS) d$(LIBNAME)
	@$(ECHO)
	@$(ECHO) "Removing all .p's"
	@$(ECHO)
	@$(RM) -f $(POBJS) p$(LIBNAME)
	@$(ECHO)
	@$(ECHO) "Removing all .t's"
	@$(ECHO)
	$(RM) -f *.o trace a.out
	$(RM) -f $(BUILT)

clobber: clean
	@$(RM) -rf $(TOBJS) t$(LIBNAME) $(LIBNAME) ?$(LIBNAME) $(TMP)
	$(RM) -f captoinfo chgtinfo dumpti infocmp show llib-lcurses.ln tic tput 
	$(RM) -f $(PHEADERS)

rmhdrs:
	@$(ECHO)
	@$(ECHO) The build of libcurses generates programs and header files.
	@$(ECHO) We remove any old versions of these files before we begin.
	@$(ECHO)
	$(RM) -f $(BUILT)


tags:
	$(CTAGS) $(CFILES) $(HEADERS)

lint:
	-$(LN) llib-lcurses llib.c
	@$(ECHO) $(LINT) -vxu $(CPPDEFS) -DNOMACROS > lint.out
	-$(LINT) -vxu $(CPPDEFS) -DNOMACROS $(CFILES) llib.c >> lint.out
	$(RM) -f llib.c

lpr:
	-pr `$(LS) $(HEADERS) | sort` | lpr
	-pr `$(LS) $(CFILES) | sort` | lpr
	-lpq

TPFILES= makefile makefile.bsd makefile.sysv \
	$(HEADERS) $(CFILES) \
	$(TICHDRS) $(TICSRC) \
	$(CHDRS) $(BSRC) $(CSRC) $(ISRC) \
	curses.ed \
	termcap.ed tiget.ed tifget.ed \
	tags llib-lcurses TODO caps getopt.c \
	keyname.sh \
	show.c testterminfo.c tput.c

tp:
	tp crm0 `$(LS) $(TPFILES) | $(UNIQ)`

tar:
	tar crvf curses.tar `$(LS) $(TPFILES) | $(UNIQ)`

cpio:
	$(LS) $(TPFILES) | $(UNIQ) | $(CPIO) -o > curses.cpio

cpio-c:
	$(LS) $(TPFILES) | $(UNIQ) | $(CPIO) -oc > curses.cpio

cpiotape:
	$(LS) $(TPFILES) | $(UNIQ) | $(CPIO) -oBc > /dev/rmt8

llib-lcurses.ln: curses.h term.h
	$(LINT) -DNOMACROS -o curses -vx $(INCLUDE) llib-lcurses

install:	all
	@if [ ! -d "$(CCSLIB)" ] ;\
	then	$(MKDIR) $(CCSLIB); \
	fi;
	install -f $(CCSLIB) -m 644 -u bin -g bin $(LIBNAME)
	$(RM) -f $(CCSLIB)/libtermcap.a $(CCSLIB)/libtermlib.a
	$(LN) $(CCSLIB)/libcurses.a $(CCSLIB)/libtermcap.a
	$(LN) $(CCSLIB)/libcurses.a $(CCSLIB)/libtermlib.a

	@if [ ! -d "$(DESTLIB)" ] ;\
	then	$(MKDIR) $(DESTLIB); \
	fi;
	@if [ ! -d "$(DESTDIR)/usr/include" ] ;\
	then	$(MKDIR) $(DESTDIR)/usr/include; \
	fi;
	cp curses.h term.h unctrl.h $(DESTDIR)/usr/include
	@if [ -d "$(INC)" ] ;\
	then	cp curses.h term.h unctrl.h $(INC) ;\
	fi

	cp llib-lcurses llib-lcurses.ln $(CCSLIB)

	@if [ ! -d "$(CCSBIN)" ] ;\
	then	$(MKDIR) $(CCSBIN); \
	fi;
	install -f $(CCSBIN) -m 555 -u bin -g bin tic
	install -f $(CCSBIN) -m 555 -u bin -g bin captoinfo
	install -f $(CCSBIN) -m 555 -u bin -g bin infocmp
	install -f $(CCSBIN) -m 555 -u bin -g bin tput

cktmp:
	@$(ECHO) 
	@$(ECHO) libcurses.a is built using $(TMP) for work space.
	@$(ECHO) Verifying the existence of $(TMP).
	@$(ECHO)
	@if [ -d "$(TMP)" ] ;\
	then \
		$(ECHO) "\t$(TMP) exists";\
		$(ECHO);\
	else  \
		$(ECHO) "\t$(TMP) not found.";\
		$(ECHO) "\n\tCreating $(TMP)\n";\
		$(MKDIR) $(TMP); \
	fi;

dumpti: setupterm.c curses.h term.h tnames.o tinames.o tifnames.o termerr.o curses.o
	@$(ECHO)
	@$(ECHO) Building dumpti
	@$(ECHO)
	-$(LN) setupterm.c dumpti.c
	$(CC) $(CFLAGS) -DDUMPTI -s dumpti.c curses.o tnames.o tinames.o tifnames.o termerr.o -o dumpti $(LINK_MODE); rm -f dumpti.c
	@$(ECHO)
	@$(ECHO) dumpti has been built
	@$(ECHO)

captoinfo: $(COBJ) $(LIBNAME)
	$(CC) $(COBJ) $(LIBNAME) -s -o captoinfo $(LINK_MODE)

infocmp: $(IOBJ) $(LIBNAME)
	$(CC) $(IOBJ) $(LIBNAME) -s -o infocmp $(LINK_MODE)

captoinfo.o:	captoinfo.c curses.h otermcap.h print.h
otermcap.o:	otermcap.c otermcap.h
print.o:	print.c curses.h print.h
iexpand.o:	iexpand.c print.h
cexpand.o:	cexpand.c print.h
infotocap.o:	infotocap.c curses.h
infocmp.o:	infocmp.c curses.h term.h print.h

tic: $(TICOBJS)
	@$(ECHO)
	@$(ECHO) Building the tic compiler
	@$(ECHO)
	$(CC) -s -o tic $(TICOBJS) $(LINK_MODE)
	@$(ECHO)
	@$(ECHO) the tic compiler has been built
	@$(ECHO)

linttic: $(TICSRC)
	$(LINT) $(CPPDEFS) $(TICSRC)

lintinfocmp: $(ISRC)
	$(LINT) $(CPPDEFS) $(ISRC)

lintcaptoinfo: $(CSRC)
	$(LINT) $(CPPDEFS) $(CSRC)

show:	$(LIBNAME) show.o
	$(CC) $(CFLAGS) -s -o show show.o $(LIBNAME) $(LINK_MODE)

pshow:	p$(LIBNAME) show.p
	$(CC) -pg $(CFLAGS) -s -o pshow show.p p$(LIBNAME) $(LINK_MODE)

dshow:	d$(LIBNAME) show.d
	$(CC) $(DFLAGS) -s -o dshow show.d d$(LIBNAME) $(LINK_MODE)

tshow:	t$(LIBNAME) show.t
	$(CC) $(TFLAGS) -s -o tshow show.t t$(LIBNAME) $(LINK_MODE)

tput:	$(LIBNAME) tput.o tnames.o tinames.o tifnames.o
	$(CC) $(CFLAGS) -s -o tput tput.o $(LIBNAME) $(LINK_MODE)

testterminfo:	$(LIBNAME) testterminfo.o
	$(CC) $(CFLAGS) -s -o testterminfo testterminfo.o $(LIBNAME) $(LINK_MODE)

chgtinfo:	curses.h chgtinfo.c
	$(CC) $(CFLAGS) -s -o chgtinfo chgtinfo.c $(LINK_MODE)
