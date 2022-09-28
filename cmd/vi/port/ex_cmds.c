/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/* Copyright (c) 1981 Regents of the University of California */
#ident	"@(#)vi:port/ex_cmds.c	1.24"
#include "ex.h"
#include "ex_argv.h"
#include "ex_temp.h"
#include "ex_tty.h"
#include "ex_vis.h"

bool	pflag, nflag;
int	poffset;

#define	nochng()	lchng = chng

/*
 * Main loop for command mode command decoding.
 * A few commands are executed here, but main function
 * is to strip command addresses, do a little address oriented
 * processing and call command routines to do the real work.
 */
extern unsigned char *Version;
commands(noprompt, exitoneof)
	bool noprompt, exitoneof;
{
	register line *addr;
	register int c;
	register int lchng;
	int given;
	int seensemi;
	int cnt;
	bool hadpr;
	unsigned char *vgetpass();

	resetflav();
	nochng();
	for (;;) {
		if (!firstpat)
			laste = 0;
		/*
		 * If dot at last command
		 * ended up at zero, advance to one if there is a such.
		 */
		if (dot <= zero) {
			dot = zero;
			if (dol > zero)
				dot = one;
		}
		shudclob = 0;

		/*
		 * If autoprint or trailing print flags,
		 * print the line at the specified offset
		 * before the next command.
		 */
		if (pflag ||
		    lchng != chng && value(vi_AUTOPRINT) && !inglobal && !inopen && endline) {
			pflag = 0;
			nochng();
			if (dol != zero) {
				addr1 = addr2 = dot + poffset;
				if (addr1 < one || addr1 > dol)
error("Offset out-of-bounds|Offset after command too large");
				setdot1();
				goto print;
			}
		}
		nochng();

		/*
		 * Print prompt if appropriate.
		 * If not in global flush output first to prevent
		 * going into pfast mode unreasonably.
		 */
		if (inglobal == 0) {
			flush();
			if (!hush && value(vi_PROMPT) && !globp && !noprompt && endline) {
				putchar(':');
				hadpr = 1;
			}
			TSYNC();
		}

		/*
		 * Gobble up the address.
		 * Degenerate addresses yield ".".
		 */
		addr2 = 0;
		given = seensemi = 0;
		do {
			addr1 = addr2;
			addr = address(0);
			c = getcd();
			if (addr == 0)
				if (c == ',')
					addr = dot;
				else if (addr1 != 0) {
					addr2 = dot;
					break;
				} else
					break;
			addr2 = addr;
			given++;
			if (c == ';') {
				c = ',';
				dot = addr;
				seensemi = 1;
			}
		} while (c == ',');
		if (c == '%') {
			/* %: same as 1,$ */
			addr1 = one;
			addr2 = dol;
			given = 2;
			c = getchar();
		}
		if (addr1 == 0)
			addr1 = addr2;
		if (c == ':')
			c = getchar();

		/*
		 * Set command name for special character commands.
		 */
		tailspec(c);

		/*
		 * If called via : escape from open or visual, limit
		 * the set of available commands here to save work below.
		 */
		if (inopen) {
			if (c=='\n' || c=='\r' || c==CTRL('d') || c==EOF) {
				if (addr2)
					dot = addr2;
				if (c == EOF)
					return;
				continue;
			}
			if (any(c, "o"))
notinvis:
				tailprim(Command, 1, 1);
		}
		switch (c) {

		case 'a':

			switch(peekchar()) {
			case 'b':
/* abbreviate */
				tail("abbreviate");
				setnoaddr();
				mapcmd(0, 1);
				anyabbrs = 1;
				continue;
			case 'r':
/* args */
				tail("args");
				setnoaddr();
				eol();
				pargs();
				continue;
			}

/* append */
			if (inopen)
				goto notinvis;
			tail("append");
			setdot();
			aiflag = exclam();
			donewline();
			vmacchng(0);
			deletenone();
			setin(addr2);
			inappend = 1;
			(void)append(gettty, addr2);
			inappend = 0;
			nochng();
			continue;

		case 'c':
			switch (peekchar()) {

/* copy */
			case 'o':
				tail("copy");
				vmacchng(0);
				vi_move();
				continue;

/* crypt */
			case 'r':
				tail("crypt");
				crflag = -1;
			ent_crypt:
				setnoaddr();
				xflag = 1;
				if(permflag)
					(void)crypt_close(perm);
				permflag = 1;
				if ((kflag = run_setkey(perm, (key = vgetpass("Enter key:")))) == -1) {
					xflag = 0;
					kflag = 0;
					crflag = 0;
					smerror("Encryption facility not available\n");
				}
				if(kflag == 0)
					crflag = 0;
				continue;

/* cd */
			case 'd':
				tail("cd");
				goto changdir;

/* chdir */
			case 'h':
				ignchar();
				if (peekchar() == 'd') {
					register unsigned char *p;
					tail2of("chdir");
changdir:
					if (savedfile[0] == '/' || !value(vi_WARN))
						(void)exclam();
					else
						(void)quickly();
					if (skipend()) {
						p = (unsigned char *)getenv("HOME");
						if (p == NULL)
							error("Home directory unknown");
					} else
						getone(), p = file;
					eol();
					if (chdir(p) < 0)
						filioerr(p);
					if (savedfile[0] != '/')
						edited = 0;
					continue;
				}
				if (inopen)
					tailprim("change", 2, 1);
				tail2of("change");
				break;

			default:
				if (inopen)
					goto notinvis;
				tail("change");
				break;
			}
/* change */
			aiflag = exclam();
			setCNL();
			vmacchng(0);
			setin(addr1);
			delete(0);
			inappend = 1;
			(void)append(gettty, addr1 - 1);
			inappend = 0;
			nochng();
			continue;

/* delete */
		case 'd':
			/*
			 * Caution: dp and dl have special meaning already.
			 */
			tail("delete");
			c = cmdreg();
			setCNL();
			vmacchng(0);
			if (c)
				YANKreg(c);
			delete(0);
			appendnone();
			continue;

/* edit */
/* ex */
		case 'e':
			if(crflag == 2 || crflag == -2)
				crflag = -1;
			tail(peekchar() == 'x' ? "ex" : "edit");
editcmd:
			if (!exclam() && chng)
				c = 'E';
			filename(c);
			if (c == 'E') {
				ungetchar(lastchar());
				(void)quickly();
			}
			setnoaddr();
doecmd:
			init();
			addr2 = zero;
			laste++;
			sync();
			rop(c);
			nochng();
			continue;

/* file */
		case 'f':
			tail("file");
			setnoaddr();
			filename(c);
			noonl();
/*
			synctmp();
*/
			continue;

/* global */
		case 'g':
			tail("global");
			global(!exclam());
			nochng();
			continue;

/* insert */
		case 'i':
			if (inopen)
				goto notinvis;
			tail("insert");
			setdot();
			nonzero();
			aiflag = exclam();
			donewline();
			vmacchng(0);
			deletenone();
			setin(addr2);
			inappend = 1;
			(void)append(gettty, addr2 - 1);
			inappend = 0;
			if (dot == zero && dol > zero)
				dot = one;
			nochng();
			continue;

/* join */
		case 'j':
			tail("join");
			c = exclam();
			setcount();
			nonzero();
			donewline();
			vmacchng(0);
			if (given < 2 && addr2 != dol)
				addr2++;
			join(c);
			continue;

/* k */
		case 'k':
casek:
			pastwh();
			c = getchar();
			if (endcmd(c))
				serror("Mark what?|%s requires following letter", Command);
			donewline();
			if (!islower(c))
				error("Bad mark|Mark must specify a letter");
			setdot();
			nonzero();
			names[c - 'a'] = *addr2 &~ 01;
			anymarks = 1;
			continue;

/* list */
		case 'l':
			tail("list");
			setCNL();
			(void)setlist(1);
			pflag = 0;
			goto print;

		case 'm':
			if (peekchar() == 'a') {
				ignchar();
				if (peekchar() == 'p') {
/* map */
					tail2of("map");
					setnoaddr();
					mapcmd(0, 0);
					continue;
				}
/* mark */
				tail2of("mark");
				goto casek;
			}
/* move */
			tail("move");
			vmacchng(0);
			vi_move();
			continue;

		case 'n':
			if (peekchar() == 'u') {
				tail("number");
				goto numberit;
			}
/* next */
			tail("next");
			setnoaddr();
			ckaw();
			(void)quickly();
			if (getargs()) 
				makargs();
			next();
			c = 'e';
			filename(c);
			goto doecmd;

/* open */
		case 'o':
			tail("open");
			oop();
			pflag = 0;
			nochng();
			continue;

		case 'p':
		case 'P':
			switch (peekchar()) {

/* put */
			case 'u':
				tail("put");
				setdot();
				c = cmdreg();
				eol();
				vmacchng(0);
				if (c)
					putreg(c);
				else
					put();
				continue;

			case 'r':
				ignchar();
				if (peekchar() == 'e') {
/* preserve */
					tail2of("preserve");
					eol();
					if (preserve() == 0)
						error("Preserve failed!");
					else
						error("File preserved.");
				}
				tail2of("print");
				break;

			default:
				tail("print");
				break;
			}
/* print */
			setCNL();
			pflag = 0;
print:
			nonzero();
			if (clear_screen && span() > lines) {
				flush1();
				vclear();
			}
			plines(addr1, addr2, 1);
			continue;

/* quit */
		case 'q':
			tail("quit");
			setnoaddr();
			c = quickly();
			eol();
			if (!c)
quit:
				nomore();
			if (inopen) {
				vgoto(WECHO, 0);
				if (!ateopr())
					vnfl();
				else {
					tostop();
				}
				flush();
				setty(normf);
				ixlatctl(1);
			}
			cleanup(1);
			exit(errcnt);

		case 'r':
			if (peekchar() == 'e') {
				ignchar();
				switch (peekchar()) {

/* rewind */
				case 'w':
					tail2of("rewind");
					setnoaddr();
					if (!exclam()) {
						ckaw();
						if (chng && dol > zero)
							error("No write@since last change (:rewind! overrides)");
					}
					eol();
					erewind();
					next();
					c = 'e';
					ungetchar(lastchar());
					filename(c);
					goto doecmd;

/* recover */
				case 'c':
					tail2of("recover");
					setnoaddr();
					c = 'e';
					if (!exclam() && chng)
						c = 'E';
					filename(c);
					if (c == 'E') {
						ungetchar(lastchar());
						(void)quickly();
					}
					init();
					addr2 = zero;
					laste++;
					sync();
					recover();
					rop2();
					revocer();
					if (status == 0)
						rop3(c);
					if (dol != zero)
						change();
					nochng();
					continue;
				}
				tail2of("read");
			} else
				tail("read");
/* read */
			if(crflag == 2 || crflag == -2) 
			/* restore crflag for new input text */
				crflag = -1;
			if (savedfile[0] == 0 && dol == zero)
				c = 'e';
			pastwh();
			vmacchng(0);
			if (peekchar() == '!') {
				setdot();
				ignchar();
				unix0(0);
				vi_filter(0);
				continue;
			}
			filename(c);
			rop(c);
			nochng();
			if (inopen && endline && addr1 > zero && addr1 < dol)
				dot = addr1 + 1;
			continue;

		case 's':
			switch (peekchar()) {
			/*
			 * Caution: 2nd char cannot be c, g, or r
			 * because these have meaning to substitute.
			 */

/* set */
			case 'e':
				tail("set");
				setnoaddr();
				set();
				continue;

/* shell */
			case 'h':
				tail("shell");
				setNAEOL();
				vnfl();
				putpad(exit_ca_mode);
				flush();
				resetterm();
				unixwt(1, unixex("-i", (char *) 0, 0, 0));
				vcontin(0);
				continue;

/* source */
			case 'o':
#ifdef notdef
				if (inopen)
					goto notinvis;
#endif
				tail("source");
				setnoaddr();
				getone();
				eol();
				source(file, 0);
				continue;
#ifdef SIGTSTP
/* stop, suspend */
			case 't':
				tail("stop");
				goto suspend;
			case 'u':
				tail("suspend");
suspend:
				c = exclam();
				eol();
				if (!c)
					ckaw();
				onsusp(0);
				continue;
#endif

			}
			/* fall into ... */

/* & */
/* ~ */
/* substitute */
		case '&':
		case '~':
			Command = (unsigned char *)"substitute";
			if (c == 's')
				tail(Command);
			vmacchng(0);
			if (!substitute(c))
				pflag = 0;
			continue;

/* t */
		case 't':
			if (peekchar() == 'a') {
				tail("tag");
				tagfind(exclam());
				if (!inopen)
					lchng = chng - 1;
				else
					nochng();
				continue;
			}
			tail("t");
			vmacchng(0);
			vi_move();
			continue;

		case 'u':
			if (peekchar() == 'n') {
				ignchar();
				switch(peekchar()) {
/* unmap */
				case 'm':
					tail2of("unmap");
					setnoaddr();
					mapcmd(1, 0);
					continue;
/* unabbreviate */
				case 'a':
					tail2of("unabbreviate");
					setnoaddr();
					mapcmd(1, 1);
					anyabbrs = 1;
					continue;
				}
/* undo */
				tail2of("undo");
			} else
				tail("undo");
			setnoaddr();
			markDOT();
			c = exclam();
			donewline();
			undo(c);
			continue;

		case 'v':
			switch (peekchar()) {

			case 'e':
/* version */
				tail("version");
				setNAEOL();
				printf("%s",Version);
				noonl();
				continue;

/* visual */
			case 'i':
				tail("visual");
				if (inopen) {
					c = 'e';
					goto editcmd;
				}
				vop();
				pflag = 0;
				nochng();
				continue;
			}
/* v */
			tail("v");
			global(0);
			nochng();
			continue;

/* write */
		case 'w':
			c = peekchar();
			tail(c == 'q' ? "wq" : "write");
wq:
			if (skipwh() && peekchar() == '!') {
				pofix();
				ignchar();
				setall();
				unix0(0);
				vi_filter(1);
			} else {
				setall();
				if (c == 'q')
					write_quit = 1;
				else
					write_quit = 0;
				wop(1);
				nochng();
			}
			if (c == 'q')
				goto quit;
			continue;
/* X: crypt */
		case 'X':
			crflag = -1; /* determine if file is encrypted */
			goto ent_crypt;
		
		case 'C':
			crflag = 1;  /* assume files read in are encrypted */
			goto ent_crypt;

/* xit */
		case 'x':
			tail("xit");
			if (!chng)
				goto quit;
			c = 'q';
			goto wq;

/* yank */
		case 'y':
			tail("yank");
			c = cmdreg();
			setcount();
			eol();
			vmacchng(0);
			if (c)
				YANKreg(c);
			else
				yank();
			continue;

/* z */
		case 'z':
			zop(0);
			pflag = 0;
			continue;

/* * */
/* @ */
		case '*':
		case '@':
			c = getchar();
			if (c=='\n' || c=='\r')
				ungetchar(c);
			if (any(c, "@*\n\r"))
				c = lastmac;
			if (isupper(c))
				c = tolower(c);
			if (!islower(c))
				error("Bad register");
			donewline();
			setdot();
			cmdmac(c);
			continue;

/* | */
		case '|':
			endline = 0;
			goto caseline;

/* \n */
		case '\n':
			endline = 1;
caseline:
			notempty();
			if (addr2 == 0) {
				if (cursor_up != NOSTR && c == '\n' && !inglobal)
					c = CTRL('k');
				if (inglobal)
					addr1 = addr2 = dot;
				else {
					if (dot == dol)
						error("At EOF|At end-of-file");
					addr1 = addr2 = dot + 1;
				}
			}
			setdot();
			nonzero();
			if (seensemi)
				addr1 = addr2;
			getline(*addr1);
			if (c == CTRL('k')) {
				flush1();
				destline--;
				if (hadpr)
					shudclob = 1;
			}
			plines(addr1, addr2, 1);
			continue;

/* " */
		case '"':
			comment();
			continue;

/* # */
		case '#':
numberit:
			setCNL();
			(void)setnumb(1);
			pflag = 0;
			goto print;

/* = */
		case '=':
			donewline();
			setall();
			if (inglobal == 2)
				pofix();
			printf("%d", lineno(addr2));
			noonl();
			continue;

/* ! */
		case '!':
			if (addr2 != 0) {
				vmacchng(0);
				unix0(0);
				setdot();
				vi_filter(2);
			} else {
				unix0(1);
				pofix();
				putpad(exit_ca_mode);
				flush();
				resetterm();
				unixwt(1, unixex("-c", uxb, 0, 0));
				vclrech(1);	/* vcontin(0); */
				nochng();
			}
			continue;

/* < */
/* > */
		case '<':
		case '>':
			for (cnt = 1; peekchar() == c; cnt++)
				ignchar();
			setCNL();
			vmacchng(0);
			shift(c, cnt);
			continue;

/* ^D */
/* EOF */
		case CTRL('d'):
		case EOF:
			if (exitoneof) {
				if (addr2 != 0)
					dot = addr2;
				return;
			}
			if (!isatty(0)) {
				if (intty)
					/*
					 * Chtty sys call at UCB may cause a
					 * input which was a tty to suddenly be
					 * turned into /dev/null.
					 */
					onhup(0);
				return;
			}
			if (addr2 != 0) {
				setlastchar('\n');
				putnl();
			}
			if (dol == zero) {
				if (addr2 == 0)
					putnl();
				notempty();
			}
			ungetchar(EOF);
			zop(hadpr);
			continue;

		default:
			if (!isalpha(c) || !isascii(c))
				break;
			ungetchar(c);
			tailprim("", 0, 0);
		}
		ungetchar(c);
		{
			int length;
			char multic[MULTI_BYTE_MAX];
			wchar_t wchar;
			length = mbftowc(multic, &wchar, getchar, &peekc);
			if(length < 0)
				length = -length;
			multic[length] = '\0';
			error("What?|Unknown command character '%s'", multic);
		}
	}
}
