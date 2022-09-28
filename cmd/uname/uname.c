/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)uname:uname.c	1.30"

/*******************************************************************

		PROPRIETARY NOTICE (Combined)

This source code is unpublished proprietary information
constituting, or derived under license from AT&T's UNIX(r) System V.
In addition, portions of such source code were derived from Berkeley
4.3 BSD under license from the Regents of the University of
California.



		Copyright Notice 

Notice of copyright on this source code product does not indicate 
publication.

	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
	          All rights reserved.
********************************************************************/ 

#include        <stdio.h>
#include        "sys/utsname.h"
#include	"sys/systeminfo.h"

#if u3b2 || i386
#include        <string.h>
#include        "sys/types.h"
#include        "sys/fcntl.h"
#include        "sys/stat.h"
#endif

#ifdef i386
#include        "sys/sysi86.h"
#else
#include        "sys/sys3b.h"
#endif

#define ROOTUID (uid_t)0

struct utsname  unstr, *un;

extern void exit();
extern int uname();
extern int optind;
extern char *optarg;

main(argc, argv)
char **argv;
int argc;
{
#if u3b2 || i386
        char *nodename;
        int Sflg=0;
        char *optstring="asnrpvmS:";
#else
        char *optstring="asnrpvm";
#endif

        int sflg=0, nflg=0, rflg=0, vflg=0, mflg=0, pflg=0, errflg=0, optlet;
	char procbuf[256];

	umask(~(S_IRWXU|S_IRGRP|S_IROTH) & S_IAMB);
        un = &unstr;
        uname(un);

        while((optlet=getopt(argc, argv, optstring)) != EOF)
                switch(optlet) {
                case 'a':
                        sflg++; nflg++; rflg++; vflg++; mflg++; pflg++;
                        break;
                case 's':
                        sflg++;
                        break;
                case 'n':
                        nflg++;
                        break;
                case 'r':
                        rflg++;
                        break;
                case 'v':
                        vflg++;
                        break;
                case 'm':
                        mflg++;
                        break;
		case 'p':
			pflg++;
			break;
#if u3b2 || i386
                case 'S':
                        Sflg++;
                        nodename = optarg;

#ifdef i386
			Createrc2file(nodename);
#endif

                        break;
#endif

                case '?':
                        errflg++;
                }

        if(errflg || (optind != argc))
                usage();

#if u3b2 || u3b15 || i386
        if((Sflg > 1) || 
           (Sflg && (sflg || nflg || rflg || vflg || mflg || pflg))) {
                usage();
        }

        /* If we're changing the system name */
        if(Sflg) {
		FILE *file;
		char curname[SYS_NMLN];
		int len = strlen(nodename);
		int curlen, i;
		
                if (getuid() != ROOTUID) {
                        if (geteuid() != ROOTUID) {
                                (void) fprintf(stderr, "uname: not super user\n");
                                exit(1);
                        }
                }

                /*
                 * The size of the node name must be less than SYS_NMLN.
                 */
                if(len > SYS_NMLN - 1) {
                        (void) fprintf(stderr, "uname: name must be <= %d letters\n",SYS_NMLN-1);
                        exit(1);
                }

		/*
		 * NOTE:
		 * The name of the system is stored in a file for use
		 * when booting because the non-volatile RAM on the
		 * porting base will not allow storage of the full
		 * internet standard nodename.
		 * If sufficient non-volatile RAM is available on
		 * the hardware, however, storing the name there would
		 * be preferable to storing it in a file.
		 */

		/* 
		 * Only modify the file if the name requested is
		 * different than the name currently stored.
		 * This will mainly be useful at boot time
		 * when 'uname -S' is called with the name stored 
		 * in the file as an argument, to change the
		 * name of the machine from the default to the
		 * stored name.  In this case only the string
		 * in the global utsname structure must be changed.
		 */
		if ((file = fopen("/etc/nodename", "r")) != NULL) {
			curlen = fread(curname, sizeof(char), SYS_NMLN, file);
			for (i = 0; i < curlen; i++) {
				if (curname[i] == '\n') {
					curname[i] = '\0';
					break;
				}
			}
			if (i == curlen) {
				curname[curlen] = '\0';
			}
			fclose(file);
		} else {
			curname[0] = '\0';
		}

		if (strcmp(curname, nodename) != 0) {
			if ((file = fopen("/etc/nodename", "w")) == NULL) {
				(void) fprintf(stderr, "uname: error in writing name\n");
				exit(1);
			} 
			if (fprintf(file, "%s\n", nodename) < 0) {
				(void) fprintf(stderr, "uname: error in writing name\n");
				exit(1);
			}
			fclose(file);
		}		
		
                /* replace name in kernel data section */
#ifdef i386
                sysi86(SETNAME, nodename, 0);
#else
                sys3b(SETNAME, nodename, 0);
#endif

                exit(0);
        }
#endif
                                                    /* "uname -s" is the default */
        if( !(sflg || nflg || rflg || vflg || mflg || pflg))
                sflg++;
        if(sflg)
                (void) fprintf(stdout, "%.*s", sizeof(un->sysname), un->sysname);
        if(nflg) {
                if(sflg) (void) putchar(' ');
                (void) fprintf(stdout, "%.*s", sizeof(un->nodename), un->nodename);
        }
        if(rflg) {
                if(sflg || nflg) (void) putchar(' ');
                (void) fprintf(stdout, "%.*s", sizeof(un->release), un->release);
        }
        if(vflg) {
                if(sflg || nflg || rflg) (void) putchar(' ');
                (void) fprintf(stdout, "%.*s", sizeof(un->version), un->version);
        }
        if(mflg) {
                if(sflg || nflg || rflg || vflg) (void) putchar(' ');
                (void) fprintf(stdout, "%.*s", sizeof(un->machine), un->machine);
        }
	if (pflg) {
		if (sysinfo(SI_ARCHITECTURE, procbuf, sizeof(procbuf)) == -1) {
			fprintf(stderr,"uname: sysinfo failed\n");
			exit(1);
		}
                if(sflg || nflg || rflg || vflg || mflg) (void) putchar(' ');
		(void) fprintf(stdout, "%.*s", strlen(procbuf), procbuf);
	}
        (void) putchar('\n');
        exit(0);
}

usage()
{
#if u3b2 || i386
        (void) fprintf(stderr, "usage:  uname [-snrvmap]\n\tuname [-S system name]\n");
#else
        (void) fprintf(stderr, "usage:  uname [-snrvmap]\n");
#endif

        exit(1);
}



#ifdef i386
Createrc2file(nodename)
char *nodename;
{
	FILE *fp;
	
	if (strlen(nodename) > (size_t) SYS_NMLN ) {
		(void) fprintf(stderr, "uname: name must be <= %d letters\n", SYS_NMLN);
               	exit(1);
	}

	if ((fp = fopen("/etc/rc2.d/S11uname", "w")) == NULL) {
		fprintf(stderr,"uname: Cannot open /etc/rc2.d/S11uname\n");
       		exit(1);
	} 

	fprintf(fp, "uname -S %s", nodename);
}
#endif
