/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1984, 1986, 1987, 1988, 1989 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)sh:bltin.c	1.3.13.1"
/*
 *
 * UNIX shell
 *
 */


#include	"defs.h"
#include	<errno.h>
#include	"sym.h"
#include	"hash.h"
#include	<sys/types.h>
#include	<sys/times.h>

builtin(type, argc, argv, t)
int type, argc;
unsigned char **argv;
struct trenod	*t;
{
	short index = initio(t->treio, (type != SYSEXEC));
	unsigned char *a1 = argv[1];

	switch (type)		
	{

	case SYSSUSP:
		syssusp(argc,argv);
		break;

	case SYSSTOP:
		sysstop(argc,argv);
		break;

	case SYSKILL:
		syskill(argc,argv);
		break;

	case SYSFGBG:
		sysfgbg(argc,argv);
		break;

	case SYSJOBS:
		sysjobs(argc,argv);
		break;

	case SYSDOT:
		if (a1)
		{
			register int	f;

			if ((f = pathopen(getpath(a1), a1)) < 0)
				failed(a1, notfound);
			else
				execexp(0, f);
		}
		break;

	case SYSTIMES:
		{
			struct tms tms;

			times(&tms);
			prt(tms.tms_cutime);
			prc_buff(SP);
			prt(tms.tms_cstime);
			prc_buff(NL);
		}
		break;

	case SYSEXIT:
		flags |= forcexit;	/* force exit */
		exitsh(a1 ? stoi(a1) : retval);

	case SYSNULL:
		t->treio = 0;
		break;

	case SYSCONT:
		if (loopcnt)
		{
			execbrk = breakcnt = 1;
			if (a1)
				breakcnt = stoi(a1);
			if (breakcnt > loopcnt)
				breakcnt = loopcnt;
			else
				breakcnt = -breakcnt;
		}
		break;

	case SYSBREAK:
		if (loopcnt)
		{
			execbrk = breakcnt = 1;
			if (a1)
				breakcnt = stoi(a1);
			if (breakcnt > loopcnt)
				breakcnt = loopcnt;
		}
		break;

	case SYSTRAP:
		systrap(argc,argv);
		break;

	case SYSEXEC:
		argv++;
		ioset = 0;
		t->treio = 0;
		if (a1 == 0) {
			setmode(0);
			break;
		}
		/* FALLTHROUGH */

#ifdef RES	/* Research includes login as part of the shell */

	case SYSLOGIN:
		if (!endjobs(JOB_STOPPED|JOB_RUNNING))
			break;
		oldsigs();
		execa(argv, -1);
		done(0);
#else

	case SYSNEWGRP:
		if (flags & rshflg)
			failed(argv[0], restricted);
		else if (!endjobs(JOB_STOPPED|JOB_RUNNING))
			break;
		else
		{
			flags |= forcexit; /* bad exec will terminate shell */
			oldsigs();
			rmtemp(0);
			rmfunctmp();
#ifdef ACCT
			doacct();
#endif
			execa(argv, -1);
			done(0);
		}

#endif

	case SYSCD:
		if (flags & rshflg)
			failed(argv[0], restricted);
		else if ((a1 && *a1) || (a1 == 0 && (a1 = homenod.namval)))
		{
			unsigned char *cdpath;
			unsigned char *dir;
			int f;

			if ((cdpath = cdpnod.namval) == 0 ||
			     *a1 == '/' ||
			     cf(a1, ".") == 0 ||
			     cf(a1, "..") == 0 ||
			     (*a1 == '.' && (*(a1+1) == '/' || *(a1+1) == '.' && *(a1+2) == '/')))
				cdpath = (unsigned char *)nullstr;

			do
			{
				dir = cdpath;

#ifdef VPIX
				cdpath = catpath(cdpath,a1,(char *)0);
#else
				cdpath = catpath(cdpath,a1);
#endif

			}
			while ((f = (chdir(curstak()) < 0)) && cdpath);

			if (f) {
				switch(errno) {
						case EMULTIHOP:
							failed(a1, emultihop);
							break;
						case ENOTDIR:
							failed(a1, enotdir);
							break;
						case ENOENT:
							failed(a1, enoent);
							break;
						case EACCES:
							failed(a1, eacces);
							break;
						case ENOLINK:
							failed(a1, enolink);
							break;
						default: 
						failed(a1, baddir);
						break;
						}
			}
			else 
			{
				cwd(curstak());
				if (cf(nullstr, dir) &&
				    *dir != ':' &&
					any('/', curstak()) &&
					flags & prompt)
				{
					prs_buff(cwdget());
					prc_buff(NL);
				}
			}
			zapcd();
		}
		else 
		{
			if (a1)
				error(nulldir);
			else
				error(nohome);
		}

		break;

	case SYSSHFT:
		{
			int places;

			places = a1 ? stoi(a1) : 1;

			if ((dolc -= places) < 0)
			{
				dolc = 0;
				error(badshift);
			}
			else
				dolv += places;
		}			

		break;

	case SYSWAIT:
		syswait(argc,argv);
		break;

	case SYSREAD:
		if(argc < 2)
			failed(argv[0],mssgargn);
		rwait = 1;
		exitval = readvar(&argv[1]);
		rwait = 0;
		break;

	case SYSSET:
		if (a1)
		{
			int	cnt;

			cnt = options(argc, argv);
			if (cnt > 1)
				setargs(argv + argc - cnt);
		}
		else if (comptr(t)->comset == 0)
		{
			/*
			 * scan name chain and print
			 */
			namscan(printnam);
		}
		break;

	case SYSRDONLY:
		exitval = 0;
		if (a1)
		{
			while (*++argv)
				attrib(lookup(*argv), N_RDONLY);
		}
		else
			namscan(printro);

		break;

	case SYSXPORT:
		{
			struct namnod 	*n;

			exitval = 0;
			if (a1)
			{
				while (*++argv)
				{
					n = lookup(*argv);
					if (n->namflg & N_FUNCTN)
						error(badexport);
					else
						attrib(n, N_EXPORT);
				}
			}
			else
				namscan(printexp);
		}
		break;

	case SYSEVAL:
		if (a1)
			execexp(a1, &argv[2]);
		break;

#ifndef RES	
	case SYSULIMIT:
		sysulimit(argc, argv);
		break;
			
	case SYSUMASK:
		if (a1)
		{ 
			int c;
			mode_t i;

			i = 0;
			while ((c = *a1++) >= '0' && c <= '7')
				i = (i << 3) + c - '0';
			umask(i);
		}
		else
		{
			mode_t i;
			int j;

			umask(i = umask(0));
			prc_buff('0');
			for (j = 6; j >= 0; j -= 3)
				prc_buff(((i >> j) & 07) +'0');
			prc_buff(NL);
		}
		break;

#endif

	case SYSTST:
		exitval = test(argc, argv);
		break;

	case SYSECHO:
		exitval = echo(argc, argv);
		break;

	case SYSHASH:
		exitval = 0;

		if (a1)
		{
			if (a1[0] == '-')
			{
				if (a1[1] == 'r')
					zaphash();
				else
					error(badopt);
			}
			else
			{
				while (*++argv)
				{
					if (hashtype(hash_cmd(*argv)) == NOTFOUND)
						failed(*argv, notfound);
				}
			}
		}
		else
			hashpr();

		break;

	case SYSPWD:
		{
			exitval = 0;
			cwdprint();
		}
		break;

	case SYSRETURN:
		if (funcnt == 0)
			error(badreturn);

		execbrk = 1;
		exitval = (a1 ? stoi(a1) : retval);
		break;
	
	case SYSTYPE:
		exitval = 0;
		if (a1)
		{
			while (*++argv)
				what_is_path(*argv);
		}
		break;

	case SYSUNS:
		exitval = 0;
		if (a1)
		{
			while (*++argv)
				unset_name(*argv);
		}
		setwidth();
		break;

	case SYSGETOPT: {
		int getoptval;
		struct namnod *n;
		extern unsigned char numbuf[];
		unsigned char *varnam = argv[2];
		unsigned char c[2];
		if(argc < 3) {
			failure(argv[0],mssgargn);
			break;
		}
		exitval = 0;
		n = lookup("OPTIND");
		optind = stoi(n->namval);
		if(argc > 3) {
			argv[2] = dolv[0];
			getoptval = getopt(argc-2, (char **)&argv[2], (char *)argv[1]);
		}
		else
			getoptval = getopt(dolc+1, (char **)dolv, (char *)argv[1]);
		if(getoptval == -1) {
			itos(optind);
			assign(n, numbuf);
			n = lookup(varnam);
			assign(n, nullstr);
			exitval = 1;
			break;
		}
		argv[2] = varnam;
		itos(optind);
		assign(n, numbuf);
		c[0] = getoptval;
		c[1] = 0;
		n = lookup(varnam);
		assign(n, c);
		n = lookup("OPTARG");
		assign(n, optarg);
		}
		break;

	default:
		prs_buff("unknown builtin\n");
	}


	flushb();
	restore(index);
	chktrap();
}
