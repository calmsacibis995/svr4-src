/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oampkg:pkginfo/pkgparam.c	1.7.4.1"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <pkgstrct.h>
#include <pkginfo.h>

extern char	*optarg, *pkgfile;
extern int	optind, errno;

extern char	*pkgparam();
extern void	exit(),
		progerr();
extern int	getopt(),
		pkgnmchk(),
		pkghead();

#define ERRMESG	"unable to locate parameter information for \"%s\""
#define ERRFLT	"parsing error in parameter file"

char	*prog;

static char	*device = NULL;
static int	errflg = 0;
static int	vflag = 0;

static void
usage()
{
	(void) fprintf(stderr, "usage:\n");
	(void) fprintf(stderr,
		"\t%s [-v] [-d device] pkginst [param [param ...]]\n", prog);
	(void) fprintf(stderr,
		"\t%s [-v] -f file [param [param ...]]\n", prog);
	exit(1);
}

main(argc, argv)
int argc;
char *argv[];
{
	char *value, *pkginst;
	char *param, parambuf[128];
	int c;

	prog = strrchr(argv[0], '/');
	if(!prog++)
		prog = argv[0];

	while ((c = getopt(argc,argv,"vd:f:?")) != EOF) {
		switch(c) {
		  case 'v':
			vflag++;
			break;

		  case 'f':
			/* -d could specify stream or mountable device */
			pkgfile = optarg;
			break;

		  case 'd':
			/* -d could specify stream or mountable device */
			device = optarg;
			break;

		  default:
		  case '?':
			usage();
		}
	}

	if(pkgfile) {
		if(device)
			usage();
		pkginst = pkgfile;
	} else {
		if((optind+1) > argc)
			usage();

		if(pkghead(device))
			return(1); /* couldn't obtain info about device */
		pkginst = argv[optind++];
	}

	do {
		param = argv[optind];
		if(!param) {
			param = parambuf;
			*param = '\0';
		}
		value = pkgparam(pkginst, param);
		if(value == NULL) {
			if(errno == EFAULT) {
				progerr(ERRFLT);
				errflg++;
				break;
			} else if(errno != EINVAL) {
				/* some other error besides no value for this
				 * particular parameter
				 */
				progerr(ERRMESG, pkginst);
				errflg++;
				break;
			}
			if(!argv[optind])
				break;
			continue;
		}
		if(vflag) {
			(void) printf("%s='", param);
			while(*value) {
				if(*value == '\'') {
					(void) printf("'\"'\"'");
					value++;
				} else
					(void) putchar(*value++);
			}
			(void) printf("'\n");
		} else 
			(void) printf("%s\n", value);
		
	} while(!argv[optind] || (++optind < argc));
	(void) pkgparam(NULL, NULL); /* close open FDs so umount won't fail */

	(void) pkghead(NULL);
	return(errflg ? 1 : 0);
}
