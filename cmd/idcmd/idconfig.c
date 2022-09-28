/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)idcmd:idconfig.c	1.3.4.1"

/* Config for Installable Drivers and Tunable Parameters */

/*
 *  This program is adapted from the program of the same name
 *  written for the AT&T PC 6300+ computer.  Several things have
 *  been changed, however:
 *
 *      *  Some of the 'type' fields read in the 'mdevice' master
 *         file have been eliminated or reassigned to new meanings.
 *         Other type flags have been added.  The flags were changed
 *         to support STREAMS devices and File System types; features
 *         not included in the 6300+.
 *
 *      *  Some default device names have changed.  For example,
 *         'wini' is now 'hd'.
 *
 *      *  All the stuff included to support 'sharable' device
 *         drivers in a merged UNIX/DOS environment has been
 *         removed for now.
 *
 *      *  The two files 'mfsys' and 'sfsys' have been added
 *         to include specification info for file system types.
 *         This program was modified to read them, and write
 *         the appropriate configuration data into files.
 *
 *      *  The 't4' data structure was added to this program to
 *         hold fs type config information.
 *
 *	*  Idconfig now generates a dispatcher class table and
 *	   and exec switch table to support kernel dispatcher
 *	   and exec modules.
 *
 */


#include <stdio.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "inst.h"
#include "defines.h"
#include "sys/conf.h"
#include <sys/mkdev.h>

/* System table. This structure stores a line from sdevice.
 * The field 'type', has been moved to struct t2 and renamed 'inttype'
 * since it has the same value for identical controllers in sdevice.
 * The device name is not stored since it can be obtained via
 * 'master->dev'.
 */
struct t1 {
        struct  t2 *master;     /* pointer to mdevice entry             */
        short   units;          /* number of units                      */
        short   vector;         /* interrupt vector number              */
        long    sioa;           /* start I/O address                    */
        long    eioa;           /* end I/O address                      */
        long    scma;           /* start controller memory address      */
        long    ecma;           /* end controller memory address        */
        short   count;          /* sequence number (0 to units - 1)     */
        int     devnm;          /* device number                        */
	short	type;		/* interrupt scheme type		*/
} table[TSIZE];


/* Master table - stores mdevice. The values for field 'conf' follow.
 * 0 - not configured in system.
 * 1 - installable. Add device to OS-Merge table.
 * 2 - installable. Not required for OS-Merge.
 * 3 - not installable. Tunable.
 * Value 0 indicates that the device has not been configured
 * (i.e. it has not been specified in file 'sdevice').
 * Values 1-2 indicate that an installable device has been
 * configured (i.e. specified in sdevice).
 * If the device uses an interrupt (master->inttype > 0),
 * I/O address range (sioa & eioa > 0),
 * or controller memory address range (scma & ecma > 0),
 * 'conf' is set to 1. Otherwise, a device configured into the system
 * not using these 3 items, will have 'conf' set to 2.
 * Value 3 indicates that an mdevice entry corresponds to a tunable.
 * If the tunable parameter (or default value if not specified)
 * is greater than zero, the non-installable is considered configured
 * (conf is assigned 3).
 */
struct t2 {
        char    dev[NAMESZ];       /* device name                          */
        char    mask[ 10 ];     /* letters indicating existing handlers */
        char    type[ 20 ];     /* letters indicating device type       */
        char    hndlr[ 6 ];     /* handler name                         */
        short   block;          /* major dev number if block device     */
        short   charr;          /* major dev number if character device */
        short   min;            /* min number of units                  */
        short   max;            /* max number of units                  */
        short   chan;           /* dma channel                          */
        short   ipl;            /* ipl value for interrupt handler      */
        short   dcount;         /* number of controllers present        */
        short   acount;         /* total units for all controllers      */
        short   inttype;        /* interrupt type                       */
        short   conf;           /* is device configured in System file  */
        int     devnm;          /* last device number used              */
	int	*v_flag;	/* new or old driver flag */
        struct t2 * link;       /* list of devices sharing an interrupt */
        short	blk_start;	/* start of multiple majors range - blk */
        short	blk_end;	/* end of multiple majors range - blk   */
        short	char_start;	/* start of multiple majors range - char*/
        short	char_end;	/* end of multiple majors range - char  */
} devinfo[DSIZE];


/* Parameter table - store mtune and stune */
struct  t3      {
        char    oudef[21];              /* output definition symbol     */
        long    def;                    /* default value                */
        long    min;                    /* minimum value                */
        long    max;                    /* maximum value                */
        long    value;                  /* value in stune               */
        short   conf;                   /* was it specified in stune    */
} *parms;


struct t4 {     /* fs type configuration info */
        char fs_name[33];
        char fs_prefix[NAMESZ];
        char fs_conf[3];
        long fs_flags;
        long fs_notify;
        long fs_funcset;
} fstype[FSSIZE];

struct  t2      *bdevices[BLSIZE];      /* pointers to block devices */
struct  t2      *cdevices[CSIZE];       /* pointers to char. devices */

/* size of tables */
short   dbound  = -1;   /* current Master table size */
short   tbound  = -1;   /* current System table size */
short   pbound  = -1;   /* current Parameter table size */
short   bbound  = -1;   /* current end of block device table */
short   cbound  = -1;   /* current end of character device table */
short   fbound  = -1;   /* current end of fs type table */

/* root, pipe, dump, swap */
short   rtmaj   = -1;   /* major device number for root device */
short   swpmaj  = -1;   /* major device number for swap device */
short   pipmaj  = -1;   /* major device number for pipe device */
short   dmpmaj  = -1;   /* major device number for dump device */
short   rtmin   = -1;   /* minor device number for root device */
short   swpmin  = -1;   /* minor device number for swap device */
short   pipmin  = -1;   /* minor device number for pipe device */
short   dmpmin  = -1;   /* minor device number for dump device */
struct  t2 *dmppt;      /* pointer to dump entry */
char	swpname[MAXOBJNM]="/dev/swap";	/* pathname for swap device 4.0 */

/* variables for device number assignments */
struct t2 *reset[DEVSIZE];      /* points to devinfo if it has reset routine */
int devnm = DEVNOINT;           /* start address for devices without interrupts */


/* extern declarations */
struct  t1 *sfind();
struct  t1 *ckassign();
struct  t2 *mfind();
struct  t3 *pfind();
FILE    *open1();       /* open a file */
char    *uppermap();
char    *strchr();
extern  char *optarg;   /* used by getopt */
extern int rdmdevice(), rdmtune(), rdsdevice(), rdstune(), rdsassign();
extern int rdmfsys(), rdsfsys();
extern int prcon(), prdef(), prfs(), prvec(), prfiles(), ckerror();
extern int errno;

/* flags */
short   eflag   = 0;    /* error in configuration */
short   dflag   = 0;    /* debug flag */
short   sflag   = 0;    /* suppress searching for software packages */
short   rflag   = 0;    /* root directory */
short   iflag   = 0;    /* directory containing input files */
short   oflag   = 0;    /* directory for output files */

/* buffers */
char    root[512];              /* root directory containing cf and packages */
char    input[512];             /* directory containing input files */
char    output[512];            /* directory for output files */
char    current[512];           /* current directory */
char    path[512];              /* construct path names */
char    line[102];              /* buffer for input lines */
char    errbuf[100];            /* buffer for error messages */

/* input file names */
char    *mdfile = MDEVICE;      /* default Master device file */
char    *mtfile = MTUNE;        /* default Master tunable file */
char    *sdfile = SDEVICE;      /* default System device file */
char    *stfile = STUNE;        /* default System tunable file */
char    *safile = SASSIGN;      /* default System assign file */
char    *mffile = MFSYS;        /* default Master fs type file */
char    *sffile = SFSYS;        /* default System fs type file */

/* output file names */
char    *cfile = "conf.c";      /* configuration table file */
char    *hfile = "config.h";    /* configuration header file */
char    *ffile = "fsconf.c";    /* file system type table file */
char    *vfile = "vector.c";    /* interrupt vector file */
char    *pfile = "direct";      /* pathnames of driver file for link-edit */
char    *mfile = "idmerge";     /* installed drivers for OS-Merge */


struct LIST  {
        int (*add)();                   /* functions called by main program */
        char name[10];                  /* name of function */
} list[] = {
        rdmdevice,      "rdmdevice",    /* Master device file */
        rdmtune,        "rdmtune",      /* Master tunable parameter file */
        rdmfsys,        "rdmfsys",      /* Master fs type spec file */
        rdsdevice,      "rdsdevice",    /* System device file */
        rdstune,        "rdstune",      /* System tunable parameter file */
        rdsfsys,        "rdsfsys",      /* System fs type spec file */
        rdsassign,      "rdsassign",    /* System assign file */
        ckerror,        "ckerror",      /* check for errors */
        prcon,          "prcon",        /* config.c */
        prdef,          "prdef",        /* config.h */
        prfs,           "prfs",         /* fsconf.c */
        prvec,          "prvec",        /* vector.c */
        prfiles,        "prfiles",      /* remaining output files */
        ckerror,        "ckerror",      /* check for errors */
        NULL,           ""
};

/* data structure for linked list of exec switch elements.
 * This will be used to order the execsw table by priority
 * order.
 */

struct linklist {
	struct t2 *mdev_entry;
	struct linklist *next;
};


main(argc,argv)
int argc;
char *argv[];
{
        int m;
        struct LIST *p;

        while((m=getopt(argc, argv, "?#si:o:r:D:T:d:t:a:c:h:v:p:m:")) != EOF )
                switch(m) {

                case 'D':
                        mfile = optarg;
                        break;
                case 'T':
                        mtfile = optarg;
                        break;
                case 'd':
                        sdfile = optarg;
                        break;
                case 't':
                        stfile = optarg;
                        break;
                case 'a':
                        safile = optarg;
                        break;
                case 'c':
                        cfile = optarg;
                        break;
                case 'h':
                        hfile = optarg;
                        break;
                case 'v':
                        vfile = optarg;
                        break;
                case 'p':
                        pfile = optarg;
                        break;
                case 'm':
                        mfile = optarg;
                        break;
                case 's':
                        sflag++;
                        break;
                case '#':
                        dflag++;
                        break;
                case 'r':
                        strcpy(root, optarg);
                        rflag++;
                        break;
                case 'i':
                        strcpy(input, optarg);
                        iflag++;
                        break;
                case 'o':
                        strcpy(output, optarg);
                        oflag++;
                        break;
                case '?':
                        fprintf(stderr, USAGE);
                        exit(1);
                }

        /* Get full path names for root, input, and output directories */
        getcwd(current, sizeof(current));
        getpath(rflag, root, ROOT);
        sprintf(path, "%s/%s", root, BUILD);
        getpath(iflag, input, path);
        getpath(oflag, output, path);

        if (dflag)
                fprintf(stdout, "Root: %s\nInput: %s\nOutput: %s\n\n",
                        root, input, output);

        /* call each function */
        for (p = &list[0]; p->add != NULL; p++) {
                if (dflag)
                        fprintf(stdout, "Main: Before %s\n", p->name);
                (*p->add)();
        }

        exit(0);
}



equal(s1,s2)
register char *s1, *s2;
{
        while (*s1++ == *s2) {
                if (*s2++ == NULL)
                        return(1);
        }
        return(0);
}



/* print error message and set flag indicating error */

error(b)
int b;
{
        if (b)
                fprintf(stderr, "LINE: %s\n", line);
        fprintf(stderr, "ERROR: %s\n", errbuf);
        eflag++;
}



/* print warning message */

warning(b)
int b;
{
        if (b)
                fprintf(stderr, "LINE: %s\n", line);
        fprintf(stderr, "WARNING: %s\n", errbuf);
}



/* fatal error - exit */

fatal(b)
int b;
{
        if (b)
                fprintf(stderr, "LINE: %s\n", line);
        fprintf(stderr, "FATAL ERROR: %s\n", errbuf);
        exit(1);
}



/* check if error occurred */

ckerror()
{
        if (eflag) {
                sprintf(errbuf, "Errors encountered. Configuration terminated.\n");
                fatal(0);
        }
}



/* This routine is used to search through the Master table for
 * some specified device.  If the device is found, we return a pointer to
 * the device.  If the device is not found, we return a NULL.
 * Since pointers that are incremented past a segment wrap around,
 * pointer comparisons have been replaced with integer comparisons.
 */
struct t2 *
mfind(device)
char *device;
{
        register struct t2 *q;
        register int i;

        for (q = devinfo, i = 0; i <= dbound; i++, q++) {
                if (equal(device, q->dev))
                        return(q);
        }
        return(NULL);
}



/* This routine is used to search the System table for some
 * specified device.  If the device is found we return a pointer to
 * that device.  If the device is not found, we return a NULL.
 * Pointer comparisons have been replaced with integer comparisons.
 */
struct t1 *
sfind(device)
char *device;
{
        register struct t1 *p;
        register int i;

        for (p = table, i = 0; i <= tbound; i++, p++) {
                if (equal(device, p->master->dev))
                        return(p);
        }
        return(NULL);
}



/* This routine is used to search the Parameter table
 * for the keyword that was specified in the configuration.  If the
 * keyword cannot be found in this table, a NULL is returned.
 * If the keyword is found, a pointer to that entry is returned.
 * Pointer comparisons have been replaced with integer comparisons.
 */
struct t3 *
pfind(keyword)
char *keyword;
{
        register struct t3 *p;
        register int i;

        for (p = parms, i = 0; i <= pbound; i++, p++) {
                if (equal(keyword, p->oudef))
                        return(p);
        }
        return(NULL);
}



/* Enter device in block device table, bdevices, and/or character device
 * table, cdevices.
 */
enter(p)
struct t2 *p;
{
        register struct t2 **q;

        if (INSTRING(p->type, BLOCK)) {

		/* check and enter those block major numbers       */
		/* specified by a device requiring multiple majors */
		if (INSTRING(p->type, MULTMAJ)) {
			register int i;
			int HowMany;

			HowMany = p->blk_end - p->blk_start + 1;

	                /* check range of block device major numbers */
			for(i = 0; i < HowMany; i++) {
		                if ((p->blk_start + i) < BLOW || (p->blk_start + i) > BHIGH) {
		                        sprintf(errbuf, IBDM, (p->blk_start + i), BLOW, BHIGH);
		                        error(1);
		                        return(0);
		                }
			}
			for(i = 0; i < HowMany; i++) {
				q = &bdevices[p->blk_start +i];
		                if (*q != NULL && *q != p) {
		                        sprintf(errbuf, DBDM, (p->blk_start+i), p->dev, (*q)->dev);
		                        error(1);
		                        return(0);
		                }
		                *q = p;
		                bbound = MAX(bbound, p->blk_start+i);
			}
		}

		else {		/* single major */

	                /* check range of block device major number */
	                if (p->block < BLOW || p->block > BHIGH) {
	                        sprintf(errbuf, IBDM, p->block, BLOW, BHIGH);
	                        error(1);
	                        return(0);
	                }

	                /* check if major number used by a different block device */
	                q = &bdevices[p->block];
	                if (*q != NULL && *q != p) {
	                        sprintf(errbuf, DBDM, p->block, p->dev, (*q)->dev);
	                        error(1);
	                        return(0);
	                }
	
	                *q = p;
	                bbound = MAX(bbound, p->block);
	        }

	}

        if (INSTRING(p->type, CHAR)) {

		/* check and enter those character major numbers   */
		/* specified by a device requiring multiple majors */
		if (INSTRING(p->type, MULTMAJ)) {
			register int i;
			int HowMany;

			HowMany = p->char_end - p->char_start + 1;

	                /* check range of character device major numbers */
			for(i = 0; i < HowMany; i++) {
		                if ((p->char_start + i) < CLOW || (p->char_start + i) > CHIGH) {
		                        sprintf(errbuf, ICDM, (p->char_start + i), CLOW, CHIGH);
		                        error(1);
		                        return(0);
		                }
			}
			for(i = 0; i < HowMany; i++) {
				q = &cdevices[p->char_start +i];
		                if (*q != NULL && *q != p) {
		                        sprintf(errbuf, DCDM, (p->char_start+i), p->dev, (*q)->dev);
		                        error(1);
		                        return(0);
		                }
		                *q = p;
		                cbound = MAX(cbound, p->char_start+i);
			}
		}

		else {		/* single major */

	                /* check range of character device major number */
	                if (p->charr < CLOW || p->charr > CHIGH) {
	                        sprintf(errbuf, ICDM, p->charr, CLOW, CHIGH);
	                        error(1);
	                        return(0);
	                }

	                /* check if major number used by a different character device */
	                q = &cdevices[p->charr];
	                if (*q != NULL && *q != p) {
	                        sprintf(errbuf, DCDM, p->charr, p->dev, (*q)->dev);
	                        error(1);
	                        return(0);
	                }
	
	                *q = p;
	                cbound = MAX(cbound, p->charr);
		}
        }

        return(1);
}



/* check if vector, I/O addresses, and device memory addresses overlap */

check(s, m)
struct sdev *s;
struct t2 *m;
{
        register struct t1 *p;
        register int i;
        static struct {                 /* reserved I/O addresses */
                int start;		/* updated for 6386 WGS */
                int end;
                char name[NAMESZ];
        } reserved[] = {
#ifdef AT386
                { 0x00, 0x1f, "reserved" },	/* DMAC 1 */
                { 0x20, 0x3f, "reserved" },	/* Interrupt Controller 1 */
                { 0x40, 0x5f, "reserved" },	/* timers */
                { 0x70, 0x7f, "reserved" },	/* NMI mask/real time clock */
                { 0x80, 0x9f, "reserved" },	/* DMA Page registers */
                { 0xa0, 0xbf, "reserved" },	/* Interrupt Controller 2 */
                { 0xc0, 0xdf, "reserved" },	/* DMAC 2 */
#endif
                { -1, -1, "" }
        }, *q;

	/* check if type field is the same for identical controllers.
	 * an exception is allowed for EISA-based systems that need
	 * the same controller to share different ivectors: one
	 * shareable and the other non-shareable (i.e., combines interrupt
	 * types 1 and 4).
	 */
	if (m->dcount > 0) {
		if(( m->inttype != 1 && m->inttype != 4) ||
		   (s->type != 1 && s->type != 4)) {
			if (m->inttype != s->type) {
				sprintf(errbuf, DIFF, m->dev);
				error(1);
				return(0);
			}
		}
	}

        /* check interrupt vector number */
        if (s->type > 0) {
                /* check range of IVN */
                if (s->vector < SIVN || s->vector > EIVN) {
                        sprintf(errbuf, RIVN, s->vector, SIVN, EIVN);
                        error(1);
                        return(0);
                }
        }

        /* check I/O address */
        if (s->sioa > 0) {
                /* check for increasing range */
                if (s->sioa > s->eioa) {
                        sprintf(errbuf, OIOA, s->sioa, s->eioa);
                        error(1);
                        return(0);
                }
                /* check range of I/O addresses */
                if (s->sioa < SIOA || s->eioa > EIOA) {
                        sprintf(errbuf, RIOA, s->sioa, s->eioa, SIOA, EIOA);
                        error(1);
                        return(0);
                }
                /* check conflicts with reserved address ranges */
                for (q = reserved; q->start != -1; q++) {
                        if (!OVERLAP(s->sioa, s->eioa, q->start, q->end))
                                continue;
                        if (equal(s->device, q->name))
                                continue;
                        sprintf(errbuf, CIOA, s->device, q->name);
                        error(1);
                        return(0);
                }
        }

        /* check controller memory address */
        if (s->scma > 0) {
                /* check for increasing range */
                if (s->scma > s->ecma) {
                        sprintf(errbuf, OCMA, s->scma, s->ecma);
                        error(1);
                        return(0);
                }
                /* check range of device memory address */
                if (s->scma < SCMA ) {
                        sprintf(errbuf, RCMA, s->scma, SCMA);
                        error(1);
                        return(0);
                }
        }

        for (p = table, i = 0; i <= tbound; i++, p++) {
                /* check interrupt vector conflicts.
                 * type = 0, does not require interrupt.
                 * type = 1, requires interrupt. Will not share. Identical
                 *              controllers require separate interrupts.
                 * type = 2, requires interrupt. Identical controllers
                 *              must share the interrupt.
		 * type = 3, cooperating devices may share interrupt
		 * type = 4, requires an interrupt. Same as type 3 but
		 *		in addition, the same controller may support
		 *		multiple ivects. The sharing of interrupts is
		 *		on a level-sensitive basis (an EISA feature).
		 *		All drivers that are level-sensitive must have
		 *		the same type. A combination of interrupt
		 *		types 1 and 4 is the only one allowed.
		 */

                /* do they both use interrupts */
                if ((s->type > 0) && (p->master->inttype > 0)) {

                        /* do they both use the same interrupt number */
                        if (s->vector == p->vector) {
                                int err = 0;

                                switch (s->type) {
                                case 1:
                                        err++;
                                        break;
                                case 2:
                                        err = !equal(s->device, p->master->dev);
                                        break;
                                case 3:
                                        if ( p->master->inttype == 3 )
                                                break;
                                case 4:
                                        if ( p->master->inttype == 4 )
                                                break;
                                default:
                                        err = s->type != p->master->inttype;
                                }
                                if (err) {
                                        sprintf(errbuf, CVEC,
                                                p->master->dev, s->device);
                                        error(1);
                                        return(0);
                                }
                        }

			/* check if identical controllers of type greater
                         * than 1, are using the same interrupt. The exception
			 * is type 4 which allows support of multiple (shareable)
			 * interrupts.
			 */
			if (equal(s->device, p->master->dev) &&
				(s->type > 1 && s->type != 4) &&
				(s->vector != p->vector)) {
				sprintf(errbuf, CIDV, s->device);
				error(1);
				return(0);
			}
		}
	
                /* check I/O address conflicts */
                if (s->sioa > 0)
                        if (OVERLAP(s->sioa, s->eioa, p->sioa, p->eioa)) {
                                if (!INSTRING(m->type, IOASPY) &&
                                    !INSTRING(p->master->type, IOASPY)) {
                                        sprintf(errbuf, CIOA, p->master->dev, s->device);
                                        error(1);
                                        return(0);
                                }
                        }

                /* check device memory address conflicts */
                if (s->scma > 0)
                        if (OVERLAP(s->scma, s->ecma, p->scma, p->ecma)) {
                                sprintf(errbuf, CCMA, p->master->dev, s->device);
                                error(1);
                                return(0);
                        }
        }
        return(1);
}



/* This routine is used to map lower case alphabetics into upper case. */

char *
uppermap(device,caps)
char *device;
char *caps;
{
        register char *ptr;
        register char *ptr2;
        ptr2 = caps;
        for (ptr = device; *ptr != NULL; ptr++) {
                if ('a' <= *ptr && *ptr <= 'z')
                        *ptr2++ = *ptr + 'A' - 'a';
                else
                        *ptr2++ = *ptr;
        }
        *ptr2 = NULL;
        return (caps);
}



/* construct full path name */

getpath(flag, buf, def)
int flag;
char *buf, *def;
{
        switch (flag) {
        case 0:
                strcpy(buf, def);
                break;
        case 1:
                if (chdir(buf) != 0) {
                        sprintf(errbuf, EXIST, buf);
                        fatal(1);
                }
                getcwd(buf, 512);  /* sizeof root[], input[] and output[] */
                chdir(current);
                break;
        }
}



/* open a file */

FILE *
open1(file, mode, dir)
char *file, *mode;
int dir;
{
        FILE *fp;
        char *p;

        switch (dir) {
        case IN:
                sprintf(path, "%s/%s", input, file);
                p = path;
                break;
        case OUT:
                sprintf(path, "%s/%s", output, file);
                p = path;
                break;
        case FULL:
                p = file;
                break;
        }

        if (dflag)
                fprintf(stdout, "Open: mode=%s path=%s\n", mode, p);

        if ((fp = fopen(p, mode)) == NULL) {
                sprintf(errbuf, FOPEN, p, mode);
                fatal(0);
        }
        return(fp);
}


int getmajors(mstring, start, end)
char *mstring;
int *start;
int *end;
{
	register char *p;
	char savestring[20];
	int dash = 0;

	strcpy(savestring, mstring);
	for(p = mstring; *p != 0; p++) {
		if(!isdigit(*p) && *p != '-') {
			sprintf(errbuf,MMRANGE, savestring);
			error(1);
			return(0);
		}
		if (*p == '-') {
			*p++ = 0;
			dash++;
			break;
		}
	}

	if(!isdigit(*mstring) || (dash && !isdigit(*p))) {
		sprintf(errbuf,MMSYNTAX, savestring);
		error(1);
		return(0);
	}

	*start = atoi(mstring);

	if (!dash)
		*end = *start;
	else
		*end   = atoi(p);

	if (*end < *start) {
		sprintf(errbuf,MMIRANGE,savestring);
		error(1);
		return(0);
	}

	return(1);
}

void checkflags();	/* validates the mask and type fields */

/* read mdevice - Master device file */

rdmdevice()
{
        register int l;
        register FILE *fp;
        register struct t2 *q;
        register struct multmaj mm;
	int start, end;

        fp = open1(mdfile, "r", IN);
        
        for (q = devinfo; fgets(line,100,fp) != NULL;) {

                /* check for comment or blank line */
                if (line[0]=='*' || line [0]=='#' || line [0]=='\n')
                        continue;

                if (++dbound == DSIZE)
                {
                        sprintf(errbuf, MASTAB);
                        fatal(1);
                }

                l = sscanf(line,"%14s%9s%19s%6s%19s%19s%hd%hd%hd",
                        q->dev, q->mask, q->type, q->hndlr,
                        mm.brange, mm.crange, &(q->min), &(q->max), &(q->chan));

                if( l != 9 )
                {
                        sprintf(errbuf, "mdevice: %s", WRONG);
                        fatal(1);
                        continue;
                }

		/* validate the flags specified in the second ('mask') and
		 * third ('type') fields of the mdevice file.
		 */

		checkflags(q->mask, MASK);	/* check that 'mask' flags are valid */
		checkflags(q->type, TYPE);	/* check that 'type' flags are valid */

		/* check block majors and update entry fields */
		if(getmajors(mm.brange, &start, &end)) {
			if ((start != end) && !(INSTRING(q->type, MULTMAJ))) {
				sprintf(errbuf, MMERROR, q->dev);
				error(1);
				continue;
			}
			q->block = start;
			q->blk_start = start;
			q->blk_end   = end;
		}
		else continue;
		

		/* check char majors and update entry fields */
		if(getmajors(mm.crange, &start, &end)) {
			if ((start != end) && !(INSTRING(q->type, MULTMAJ))) {
				sprintf(errbuf, MMERROR, q->dev);
				error(1);
				continue;
			}
			q->charr = start;
			q->char_start = start;
			q->char_end   = end;
		}
		else continue;

                /* check for unique dma channel */
                dmacheck(q);

                q++;
        }
        fclose(fp);
}



void checkflags(s, type)
char *s;
int type;
{
	char *ptr;
	char *strchr();

	for ( ; *s != NULL; s++) {
		if (type == MASK) {
			if ((ptr = strchr(MASK_FLAGS, *s)) == NULL) {
				sprintf(errbuf, MASK_ERROR, *s);
				error(1);
			}
		}
		else if (type == TYPE) {
			if ((ptr = strchr(TYPE_FLAGS, *s)) == NULL) {
				sprintf(errbuf, TYPE_ERROR, *s);
				error(1);
			}
		}
	}
}



/* check for unique dma channel */

dmacheck(q)
struct t2 *q;
{
        struct t2 *p;

        /* -1 => no dma channel */
        if (q->chan == -1)
                return;

#ifndef MBUS
        /* channel 0 is refresh channel */
        if (q->chan == 0) {
                sprintf(errbuf, DCONF, q->dev, "refresh");
                error(1);
                return;
        }
#endif /* MBUS */

        /* check range */
        if (q->chan < -1 || q->chan > DMASIZ) {
                sprintf(errbuf, DRANGE, DMASIZ);
                error(1);
                return;
        }

        for (p = devinfo; p < q; p++) {
                if (q->chan == p->chan) {
                        if ( !INSTRING(p->type,DMASHR)
                        ||   !INSTRING(q->type,DMASHR) )
                        {
                                sprintf(errbuf, DCONF, q->dev, p->dev);
                                error(1);
                                return;
                        }
                }
        }
}



/* read mtune - Master tune file */

rdmtune()
{
        register int l;
        register FILE *fp;
        register struct t3 *q;
#if defined(vax) || defined (uts)
        char s1[21], s2[21], s3[21];
#endif /* vax || uts */
	int ntune;
	char *malloc();

        fp = open1(mtfile, "r", IN);

/*
 * Allocate parameter table.  Read through mtune file, counting up entries,
 *	then call malloc to allocate parms array
 */
	ntune = 0;
	while (fgets(line, 100, fp) != NULL) {
                if (line[0]=='*' || line [0]=='#' || line [0]=='\n')
                        continue;
		++ntune;
	}

	parms = (struct t3 *)malloc(ntune * sizeof(struct t3));
	if (parms == NULL) {
		sprintf (errbuf, "Cannot allocate memory for parameter table with %d entries.\n", ntune);
		fatal (1);
	}

/*
 * now re-read mtune file, this time filling in allocated parms table
 */
	rewind (fp);
        for (q = parms; fgets(line,100,fp) != NULL;) {

                /* check for comment or blank line */
                if (line[0]=='*' || line [0]=='#' || line [0]=='\n')
                        continue;

                if (++pbound == ntune) {
                        sprintf(errbuf, KEY);
                        fatal(1);
                }

#if !defined(vax) && !defined(uts)
                l = sscanf(line,"%20s%li%li%li",
                        q->oudef,&q->def,&q->min,&q->max);
#else /* vax || uts case */
                l = sscanf(line,"%20s%20s%20s%20s",
                        q->oudef,s1, s2, s3);
                if (s1[0] == '0' && s1[1] == 'x')
                        q->def=strtol( s1, 0, 16);
                else
                        q->def=atoi(s1);
                if (s2[0] == '0' && s2[1] == 'x')
                        q->min=strtol( s2, 0, 16);
                else
                        q->min=atoi(s2);
                if (s3[0] == '0' && s3[1] == 'x')
                        q->max=strtol( s3, 0, 16);
                else
                        q->max=atoi(s3);
#endif /* !vax && !uts */
                if (l != 4) {
                        sprintf(errbuf, "mtune: %s", WRONG);
                        fatal(1);
                        continue;
                }

                q++;
        }
        fclose(fp);
}

/* read mfsys - Master file system type file */
int
rdmfsys()
{
        register int l;
        register FILE *fp;
        register struct t4 *q;
        char string[33];

        fp = open1(mffile, "r", IN);

        for( q=fstype; fgets(line,100,fp) != NULL; )
        {
                /* toss comment and blank lines... */
                if (line[0]=='*' || line [0]=='#' || line [0]=='\n')
                        continue;

                /* check for full table */
                if( ++fbound == FSSIZE )
                {
                        sprintf(errbuf, "File System Type Table Overflow:  Remove one or more fs types from the 'mfsys' file and re-attempt configuration.");
                        fatal(1);
                }

                /* read 1 line = one fs type specification */
                l = sscanf(line,"%s%s", q->fs_name, q->fs_prefix);

                if( l != 2 )
                {
                        sprintf(errbuf,"Insufficient number of fields in mfsys spec line.  Skipping line.");
                        error(1);
                        continue;
                }

                if( ((int)strlen(q->fs_prefix)) > 14 )
                {
                        sprintf(errbuf,"FS type prefix too long.  Skipping entry.");
                        error(1);
                        continue;
                }

                /* done with this line */
                q++;
        }
        fclose(fp);
}


/* read sdevice - System device file */

rdsdevice()
{
        register FILE *fp;
        register struct t1 *p;
        register struct t2 *q;
        register int i;
        char buff[102];

        fp = open1(sdfile, "r", IN);

        /* read sdevice */
        for (p = table; fgets(line, 100, fp) != NULL;)
                if (parse(p)) p++;
        fclose(fp);

        /* get automatically installed devices and generate error
           for missing required devices */
        for (q = devinfo, i = 0; i <= dbound; i++, q++)
                if (INSTRING(q->type, AUTO) && (q->conf == 0)) {
                        /* sflag suppresses searching driver packages.
                           Automatics will be left out of conf.c */
                        if (!sflag) {
                                sprintf(buff, "%s/pack.d/%s/System", root, q->dev);
                                fp = open1(buff, "r", FULL);
                                fgets(line, 100, fp);
                                if (parse(p)) p++;
                                fclose(fp);
                        }
                 } else if (INSTRING(q->type, REQ) && (q->conf == 0)) {
                        sprintf(errbuf, MISS, q->dev);
                        fatal(0);
                }
}



/* parse an entry from sdevice */

parse(p)
struct t1 *p;
{
        register int l;
        register struct t2 *q;
        struct sdev sys;

        /* check for comment and blank lines */
        if (line[0]=='*' || line [0]=='#' || line [0]=='\n')
                return(0);

        l = sscanf(line, "%15s %c %hd %hd %hd %hd %lx %lx %lx %lx %c %c\n",
                sys.device, &sys.conf,
                &sys.units, &sys.ipl, &sys.type, &sys.vector,
                &sys.sioa, &sys.eioa, &sys.scma, &sys.ecma
                );

        if (l != 10) {
                sprintf(errbuf, "sdevice: %s", WRONG);
                fatal(1);
                return(0);
        }

        /* check if device is to be configured into Kernel */
        if (sys.conf == 'N')
                return(0);

        if (sys.conf != 'Y') {
                sprintf(errbuf, CONFD, sys.device);
                error(1);
                return(0);
        }

        if ((q = mfind(sys.device)) == NULL) {
                sprintf(errbuf, SUCH, sys.device);
                error(1);
                return(0);
        }

        if (!INSTRING(q->type, INST) || INSTRING(q->type, NOT)) {
                sprintf(errbuf, INSTOPT, sys.device);
                error(1);
                return(0);
        }

        if (!INSTRING(q->type, NEWDRV) && INSTRING(q->mask, SIZE)) {
                sprintf(errbuf, MISSF, sys.device);
                error(1);
                return(0);
        }

        if (INSTRING(q->type, ONCE) && (sfind(sys.device) != NULL)) {
                sprintf(errbuf, ONESPEC, q->dev);
                error(1);
                return(0);
        }

        /* check range of units */
        if (sys.units < q->min || sys.units > q->max) {
                sprintf(errbuf, UNIT, sys.units, q->min, q->max);
                error(1);
                return(0);
        }

        /* check ipl value - must be 1 to 8, inclusive */
	/* This check has no meaning for an exec module */

	if (!INSTRING(q->type, EXECSW))
	        if( (sys.vector > 0 ) && (sys.ipl<1) || (sys.ipl>8) )
	        {
	                sprintf(errbuf,"ipl value out of range");
	                error(1);
	                return(0);
	        }

        /* check for conflicts between interrupt vectors,
         * I/O addresses,and device memory addresses */
        if (!check(&sys, q))
                return(0);

        /* enter device in block or character device tables */
        if (!enter(q))
                return(0);

        if (++tbound == TSIZE) {
                sprintf(errbuf, SYSTEM);
                fatal(1);
        }

        /* enter device in System table and update Master table */
        p->master = q;
        p->units = sys.units;
        p->vector = sys.vector;
        p->sioa = sys.sioa;
        p->eioa = sys.eioa;
        p->scma = sys.scma;
        p->ecma = sys.ecma;
        p->count = (q->dcount)++;
        q->ipl = sys.ipl;
        q->acount += p->units;
        q->inttype = sys.type;
        q->conf = INSTRING(q->type, HW) ? 1 : 2;

        /* assign device number if hardware device */
        if (q->conf == 1) {
                if (q->inttype > 0)
                        p->devnm = p->vector;
                else if (q->devnm != 0)
                        p->devnm = q->devnm;
                else {
                        if (devnm == DEVSIZE) {
                                sprintf(errbuf, DEVNM);
                                error(1);
                                return(0);
                        }
                        p->devnm = q->devnm = devnm++;
                }


                /* if device has a reset routine, save pointer to devinfo */
                if (INSTRING(q->type, DRESET))
                        reset[p->devnm] = q;
        }

        return(1);
}



/* rdstune - System tunable parameter file */

rdstune()
{
        register FILE *fp;
        register struct t3 *p;
        register int l, i;
        char input[22];
#if defined(vax) || defined (uts)
        char s1[21];
#endif /* vax || uts */
        struct t2 *q;

        fp = open1(stfile, "r", IN);

        while (fgets(line, 100, fp) != NULL) {

                /* check for comment and blank lines */
                if (line[0] == '*')
                        continue;

                l = sscanf(line, "%20s", input);
                if (l == 0) {
                        sprintf(errbuf, INCOR);
                        error(1);
                        continue;
                }

                /* find tunable in Parameter table */
                p = pfind(input);
                if (p == NULL) {
                        sprintf(errbuf, TUNE, input);
                        error(0);
                        continue;
                }

                /* check if already specified */
                if (p->conf != 0) {
                        sprintf(errbuf, RESPEC, input);
                        error(0);
                        continue;
                }

                /* store value in Parameter table */
#if !defined(vax) && !defined(uts)
                l = sscanf(line, "%*s %li", &p->value);
#else /* vax || uts case */
                l = sscanf(line,"%*s %20s", s1);
                if (s1[0] == '0' && s1[1] == 'x')
                        p->value=strtol( s1, 0, 16);
                else
                        p->value=atoi(s1);
#endif /* !vax and !uts */
                if (l != 1) {
                        sprintf(errbuf, MPAR, input);
                        error(1);
                        continue;
                }

                /* indicate tunable parameter specified */
                p->conf = 1;

                /* check whether parameter is within min and max */
                if (p->value < p->min || p->value > p->max) {
                        sprintf(errbuf, PARM, input, p->value, p->min, p->max);
                        error(1);
                }
        }

        /* Find those tunables that also appear in mdevice.
         * If their value is greater than zero, mark mdevice conf
         * field with 3, indicating that it is configured.
         */
        for (p = parms, i = 0; i <= pbound; i++, p++) {
                if (p->conf == 0)
                        p->value = p->def;
                if ((q = mfind(p->oudef)) != NULL)
                        if (!INSTRING(q->type, NOT) || INSTRING(q->type, INST)) {
                                sprintf(errbuf, NOTINST, q->dev);
                                error(0);
                        } else if (p->value > 0)
                                q->conf = 3;
        }
}


/* read sfsys - specification file for added file system types */
int
rdsfsys()
{
        register int l, i;
        register FILE *fp;
        struct t4 f;

        fp = open1(sffile, "r", IN);

top:
        while (fgets(line,100, fp) != NULL )
        {
                /* skip comment and blank lines */
                if (line[0]=='*' || line [0]=='#' || line [0]=='\n')
                        continue;

                l = sscanf(line,"%s%s",f.fs_name, f.fs_conf);

                if( l != 2 )
                {
                        sprintf(errbuf, "Incorrect number of fields in sfsys line.  Skipping entry.");
                        error(1);
                        continue;
                }
                for( i=0; i<=fbound; i++ ) /* mark configured fstypes */
                {
                        if( strcmp(f.fs_name, fstype[i].fs_name) == 0 )
                        {
                                if( f.fs_conf[0] == 'N' || f.fs_conf[0] == 'n' )
                                {
                                        fstype[i].fs_conf[0] = 'N';
                                        fstype[i].fs_conf[1] = '\0';
                                }
                                else
                                {
                                        fstype[i].fs_conf[0] = 'Y';
                                        fstype[i].fs_conf[1] = '\0';
                                }
                                goto top;
                        }
                }
                if( i > fbound ) /* didn't find it */
                {
                        sprintf(errbuf,"FS type %s not found in mfsys file.",
                                f.fs_name);
                        fatal(1);
                }
                /* if we made it to here, everything is all right */
        }
        fclose(fp);
}


/* read sassign - System assignment file */

rdsassign()
{
        register FILE *fp;
        char input[9], major[9], pathname[MAXOBJNM];
        int l, minor, swp_l;
        struct t1 *p;

        fp = open1(safile, "r", IN);

        while (fgets(line, 100, fp) != NULL) {

                /* check for comment and blank lines */
                if (line[0]=='*' || line [0]=='#' || line [0]=='\n')
                        continue;

                if (sscanf(line, "%8s", input) == 0) {
                        sprintf(errbuf, INCOR);
                        error(1);
                        continue;
                }

                /* root device specification */
                if (equal(input, "root")) {
                        l = sscanf(line,"%*s%8s%d", major, &minor);
                        p = ckassign(input, l, 2, rtmin, major, minor);
                        if (p != NULL) {
                                rtmin = minor;
                                rtmaj = p->master->block;
                        }

                /* swap device specification */
		/* 
		 * For 4.0, pathname must be given as the 4th field
		 * if not,  /dev/swap will be used as default
		 * There does not seem to be an easy way to check
		 * that the path name matches the major, minor spec
		 * no checking is done.
		 */
                } else if (equal(input,"swap")) {
                        l = sscanf(line,"%*s%8s%d%s",
                                major, &minor, pathname );
			if (equal(pathname, "")) {
                		fprintf(stderr, "LINE: %s\n", line);
                		sprintf(errbuf, SWPNM, swpname);
        			fprintf(stderr, "ERROR: %s\n", errbuf);
				swp_l=2;
			} else {
				strcpy(swpname, pathname);
				swp_l=3;
			}

                        p = ckassign(input, l, swp_l, swpmin,
                                major, minor);
                        if (p != NULL) {        
                                swpmin = minor;
                                swpmaj = p->master->block;
                        }

                /* pipe device specification */
                } else if (equal(input,"pipe")) {
                        l = sscanf(line,"%*s%8s%d", major, &minor);
                        p = ckassign(input, l, 2, pipmin, major, minor);
                        if (p != NULL) {
                                pipmin = minor;
                                pipmaj = p->master->block;
                        }

                /* dump device specification */
                } else if (equal(input,"dump")) {
                        l = sscanf(line,"%*s%8s%d",
                                major, &minor);
                        p = ckassign(input, l, 2, dmpmin,
                                major, minor);
                        if (p != NULL) {
                                dmpmin = minor;
                                dmpmaj = p->master->block;
                                dmppt = p->master;
                        }
                }
        }
        fclose(fp);

        /* Make sure that the root, swap, pipe and dump devices were specified. */
        if (rtmaj < 0) {
                sprintf(errbuf, MISS, "root");
                error(0);
        }
        if (swpmaj < 0) {
                sprintf(errbuf, MISS, "swap");
                error(0);
        }
        if (pipmaj < 0) {
                sprintf(errbuf, MISS, "pipe");
                error(0);
        }
        if (dmpmaj < 0) {
                sprintf(errbuf, MISS, "dump");
                error(0);
        }
}



/* check parameters of root, swap, pipe, dump */

struct t1*
ckassign(dev, l, u, exist, maj, min)
int l, u, exist, min;
char *dev, *maj;
{
        struct t1 *p;
	minor_t highminor = OMAXMIN;
	struct t3 *maxminor;

        if (l != u) {
                sprintf(errbuf, INCOR);
                error(1);
                return(NULL);
        }
        if ((p = sfind(maj)) == NULL) {
                sprintf(errbuf, UNK, maj);
                error(1);
                return(NULL);
        }
        if (!INSTRING(p->master->type, BLOCK)) {
                sprintf(errbuf, BLK, maj);
                error(1);
        }
	if (INSTRING(p->master->type, NEWDRV)) {
		if ((maxminor = pfind("MAXMINOR")) == (struct t3 *)NULL) {
			sprintf(errbuf, NOMAXMINOR, (u_long)highminor);
			warning(0);
		} else 
			highminor = (minor_t)(maxminor->conf?
					      maxminor->value : maxminor->def);
	}
        if (min < -1 || min > (int)highminor) {
                sprintf(errbuf, MINOR, LMINOR, highminor);
                error(1);
        }
        if (exist >= 0) {
                sprintf(errbuf, ASNSPEC, dev);
                error(1);
        }
        return(p);
}


/* Support routine for prvec(). Similar to sfind(), except it
 * looks for all entries in sdevice matching device until it finds
 * one with the same vector number specified in the argument list.
 * Needed to support combination of interrupt types 1 and 4 for SCSI
 * support.
 */

struct t1 *
find_sdev(device, vector)
char *device;
short vector;
{
        register struct t1 *p;
        register int i;

        for (p = table, i = 0; i <= tbound; i++, p++) {
                if (equal(device, p->master->dev)) {
			if (p->vector == vector)
                        	return(p);
		}
        }
        return(NULL);
}



/* Print out interrupt vector file. Devices that have a 'G' (group)
 * in field 3 of their Master entry do not use an interrupt, though
 * they have one specified. The interrupt indicates that the device
 * belongs to a group. All devices within a group will be assigned
 * to MS-DOS when any device within the group is assigned. One of
 * the devices in that group must have an interrupt servicing routine
 * (i.e. it will not have a 'G' in field 3 of Master).
 */

prvec()
{
        register FILE *fp;
        register struct t1 *p;
        register int i;
        static struct t2 *vec[256];	/* points to the start of link
					 * lists of devices sharing the
					 * same interrupt vector. There's
					 * one link list per vector number.
					 */
	struct t2 *cur;			/* current element in the list */
	unsigned long level_intr_mask = 0; /* level-sensitive mask to handle
					 * level-sensitive interrupts. (For
					 * EISA bus architectures.)
					 */
        int nintr = 0;

        for (p = table, i = 0; i <= tbound; i++, p++) {
                p->master->link = NULL;	/* ensure lists are NULL terminated */
        }

	/* initialize link lists */
	for (i = 0; i < 256; i++)
		vec[i] = NULL;

        fp = open1(vfile, "w", OUT);
        chmod (vfile, 0644);

        fprintf(fp, "/* Table of Interrupt Vectors */\n\n");
        fprintf(fp,"extern intnull();\n");
        fprintf(fp, "extern clock();\n");
        for (p = table, i = 0; i <= tbound; i++, p++) {
                if (p->master->inttype != 0 && !INSTRING(p->master->type, GROUP)) {
                        if (p->count == 0)
                                fprintf(fp,"extern %sintr();\n", p->master->hndlr);
			if (!vec[p->vector]) {	/* empty list */
				/* add first element to the list */
				vec[p->vector] = p->master;
				p->master->link = NULL;
			}
			else {	/* attach elements to the end.
				 * If the device entry appears more
				 * than once in the sdevice file,
				 * add ONLY once to the link list.
				 */
				for (cur = vec[p->vector]; cur != NULL; cur = cur->link) {
					if (cur == p->master) /* exists already */
						break;
					if (cur->link == NULL) {
						p->master->link = NULL;
						cur->link = p->master;
					}
				}
			}
                }
        }

        for ( i = 1; i < 256; i++ ) {
                struct t2 * m;
                if ( vec[i] && vec[i]->inttype >= 3 && (m = vec[i])->link ) {
			if ((p = find_sdev(vec[i]->dev, i)) != NULL) {
				if (p->type != 1) {
	                        	fprintf(fp,"shrint%d() {\n", i );
	                        	do {
	                                	fprintf(fp, "\t%sintr(%d);\n", m->hndlr,i );
	                        	} while ( m = m->link );
	                        	fprintf(fp, "}\n" );
				}
			}
                }
        }

	fprintf(fp, "int	(*ivect[])() = {\n");

        /* the clock always goes first */
        fprintf(fp, "\tclock\t\t/* 0\t*/");

	for (i = 1; i < 256; i++)
	{
		fprintf(fp,",\n");
		if( vec[i] != NULL )
		{
                        if ( vec[i]->inttype >= 3 && vec[i]->link != 0 ) {
				if ((p = find_sdev(vec[i]->dev, i)) != NULL) {
					if (p->type != 1)
	                                	fprintf(fp, "\tshrint%d\t\t/* %d\t*/",i,i );
	                        	else
	                                	fprintf(fp, "\t%sintr\t\t/* %d\t*/",
	                                        	vec[i]->hndlr, i);
				}
	                        else
	                               	fprintf(fp, "\t%sintr\t\t/* %d\t*/",
	                                       	vec[i]->hndlr, i);
				}
                        else
                               	fprintf(fp, "\t%sintr\t\t/* %d\t*/",
                                       	vec[i]->hndlr, i);
                        if( nintr < i )  nintr = i;
                }
                else
                        fprintf(fp,"\tintnull\t\t/* %d\t*/", i);
        }
        fprintf(fp, "\n};\n");

        fprintf(fp,"int nintr = %d;\n", (nintr+1));

	/* handle level sensitive interrupt sharing (type = 4).
	 * Need to build a bit mask to mark the vector numbers
	 * that are level-sensitive. The assumption here is that
	 * there's a max of 16 hw interrupts. The data type used
	 * is a long int, though, to handle further expansion.
	 */

	for (i = SIVN; i < EIVN; i++) {
		if (vec[i] && vec[i]->inttype == 4) {
			if (i == 0)
				level_intr_mask |= 1;
			else
				level_intr_mask |= (1 << (i-1));
		}
	}

        /* Indices in this table correspond to indices in the ivect table. */
        fprintf(fp,"/* Table of ipl values for interrupt handlers. */\n");
        fprintf(fp,"\nunsigned char intpri[] = {\n");

        /* do the clock's IPL first */
        fprintf(fp,"\t8");      /* sys clock runs at IPL = 8 */

        for( i=1; i<256; i++ )
        {
                fprintf(fp,"\t/* %d */,\n", (i-1));

                if( vec[i] != NULL )
                        fprintf(fp,"\t%d", vec[i]->ipl );
                else
                        fprintf(fp,"\t0");
        }
        fprintf(fp,"\t/* %d */\n};\n", (i-1));

	/* print level sensitive mask */
	fprintf(fp, "unsigned long level_intr_mask = 0x%x;\n", level_intr_mask);

        fclose(fp);
}



/* print out configuration header file */

prdef()
{
        register FILE *fp;
        register int i;
        char caps[9];

        fp = open1(hfile, "w", OUT);
        chmod(hfile, 0644);

        /* go through Master table */
        {
        register struct t2 *p;
	register int j;
	int HowMany;

        fprintf(fp, "/* defines for each device */\n");
        for (p = devinfo, i = 0; i <= dbound; i++, p++) {

                /* skip devices that are not configured or represent
                 * tunable parameters */
                if (p->conf == 0 || p->conf == 3)
                        continue;

                uppermap(p->hndlr, caps);
                fprintf(fp, "\n#define\t%s\t\t1\n", caps);
                fprintf(fp, "#define\t%s_CNTLS\t%hd\n", caps, p->dcount);
                fprintf(fp, "#define\t%s_UNITS\t%hd\n", caps, p->acount);
                fprintf(fp, "#define\t%s_CHAN\t%hd\n", caps, p->chan);
                fprintf(fp, "#define\t%s_TYPE\t%hd\n", caps, p->inttype);

		if(INSTRING(p->type, BLOCK)) {
			HowMany = p->blk_end - p->blk_start + 1;
                	fprintf(fp, "#define\t%s_BMAJORS\t%hd\n",
					caps, HowMany);
			for(j=0; j < HowMany; j++)
				fprintf(fp, "#define\t%s_BMAJOR_%hd\t%hd\n",
					caps,j,p->blk_start+j);
		}
		if(INSTRING(p->type, CHAR)) {
			HowMany = p->char_end - p->char_start + 1;
                	fprintf(fp, "#define\t%s_CMAJORS\t%hd\n",
					caps, HowMany);
			for(j=0; j < HowMany; j++)
				fprintf(fp, "#define\t%s_CMAJOR_%hd\t%hd\n",
					caps,j,p->char_start+j);
		}
        }
        }

        /* go through System table */
        {
        register struct t1 *p;

        fprintf(fp, "\n\n/* defines for each controller */\n");
        for (p = table, i = 0; i <= tbound; i++, p++) {
                uppermap(p->master->hndlr, caps);
                fprintf(fp, "\n#define\t%s_%hd\t\t%hd\n",
                        caps, p->count, p->units);
                fprintf(fp, "#define\t%s_%hd_VECT\t%hd\n",
                        caps, p->count, p->vector);
                fprintf(fp, "#define\t%s_%hd_SIOA\t%ld\n",
                        caps, p->count, p->sioa);
                fprintf(fp, "#define\t%s_%hd_EIOA\t%ld\n",
                        caps, p->count, p->eioa);
                fprintf(fp, "#define\t%s_%hd_SCMA\t%ld\n",
                        caps, p->count, p->scma);
                fprintf(fp, "#define\t%s_%hd_ECMA\t%ld\n",
                        caps, p->count, p->ecma);
                if (p->master->conf != 1)
                        continue;
        }
        }
        /* go through tunable Parameter table */
        {
        register struct t3 *p;
        
        fprintf(fp, "\n/* defines for each tunable parameter */\n");
        for (p = parms, i = 0; i <= pbound; i++, p++)
                fprintf(fp, "#define\t%s\t%ld\n",
                        p->oudef, p->value);
        }

        fclose(fp);
}

void sort_entry();
void print_execsw();

/* print out configuration table file */
/***********conf.c!************/
prcon()
{
        register FILE *fp;
        register struct t2 *q;
        int i, j, b;
	int driver, module;
	struct	linklist *listhead;
        char    initbf[513];    /* initialization routine buffer */
        char    startbf[513];   /* start table routine buffer */
        char    pollbf[513];    /* poll table routine buffer */
        char    haltbf[513];    /* halt table routine buffer */
        char    forkbf[513];    /* fork routine buffer */
        char    execbf[513];    /* exec routine buffer */
        char    exitbf[513];    /* exit routine buffer */
        char    kenterbf[513];  /* kenter routine buffer */
        char    kexitbf[513];   /* kexit routine buffer */
        char    xbf[256];       /* buffer for external symbol definitions */
	char	caps[9];	/* buffer for upper case device handler */

        fp = open1(cfile, "w", OUT);
        chmod(cfile, 0644);

        fprintf(fp,"#include\t\"%s\"\n",hfile);
        fprintf(fp,"#include\t\"sys/param.h\"\n");
        fprintf(fp,"#include\t\"sys/types.h\"\n");
        fprintf(fp,"#include\t\"sys/sysmacros.h\"\n");
        fprintf(fp,"#include\t\"sys/conf.h\"\n");
        fprintf(fp,"#include\t\"sys/stream.h\"\n");
        fprintf(fp,"#include\t\"sys/class.h\"\n");
        fprintf(fp,"#include\t\"sys/vnode.h\"\n");
        fprintf(fp,"#include\t\"sys/exec.h\"\n");
        fprintf(fp,"#include\t\"vm/bootconf.h\"\n");

        fprintf(fp,"extern int nodev(), nulldev(), nuldevreset();\n");

/*
 * Search the Master table and generate an extern statement for
 * any routines that are needed.
 *
 * Declare the required streamtab structures here, as well.
 *
 */
	fprintf(fp, "extern int nodevflag;\n");
        for (q = devinfo, i = 0; i <= dbound; i++, q++) {
                /* if not configured, continue */
                if (q->conf == 0)
                        continue;

                /* is this block device */
                b = INSTRING(q->type, BLOCK);

		/* print extern definition for new, 4.0 style driver */
		if (INSTRING(q->type, NEWDRV))
			fprintf(fp, "extern int %sdevflag;\n",q->hndlr);

                sprintf(xbf,"extern ");
                if (INSTRING(q->mask, OPEN)) {
                        strcat (xbf,q->hndlr);
                        strcat (xbf,"open(), ");
                } else if (b) {
                        sprintf(errbuf, OPRT, q->dev);
                        error(0);
                }
                if (INSTRING(q->mask, CLOSE)) {
                        strcat (xbf,q->hndlr);
                        strcat (xbf,"close(), ");
                } else if (b) {
                        sprintf(errbuf, CLRT, q->dev);
                        error(0);
                }
                if (INSTRING(q->mask, READ)) {
                        strcat (xbf,q->hndlr);
                        strcat (xbf,"read(), ");
                }
                if (INSTRING(q->mask, WRITE)) {
                        strcat (xbf,q->hndlr);
                        strcat (xbf,"write(), ");
                }
                if (INSTRING(q->mask, IOCTL)) {
                        strcat (xbf,q->hndlr);
                        strcat (xbf,"ioctl(), ");
                }
                if (b) {
                        strcat (xbf,q->hndlr);
                        strcat (xbf,"strategy(), ");
                        strcat (xbf,q->hndlr);
                        strcat (xbf,"print(), ");
			if (INSTRING(q->type, NEWDRV)) {
				if (INSTRING(q->mask, SIZE)) {
                        		strcat (xbf,q->hndlr);
                        		strcat (xbf,"size(), ");
				}
			}
                }
                if (INSTRING(q->mask, FORK)) {
                        strcat (xbf,q->hndlr);
                        strcat (xbf,"fork(), ");
                }
                if (INSTRING(q->mask, EXEC)) {
                        strcat (xbf,q->hndlr);
                        strcat (xbf,"exec(), ");
                }
                if (INSTRING(q->mask, EXIT)) {
                        strcat (xbf,q->hndlr);
                        strcat (xbf,"exit(), ");
                }
                if (INSTRING(q->type, DRESET)) {
                        strcat (xbf,q->hndlr);
                        strcat (xbf,"reset(), ");
                }
                if (INSTRING(q->mask, INIT)) {
                        strcat (xbf,q->hndlr);
                        strcat (xbf,"init(), ");
                }
                if (INSTRING(q->mask, START)) {
                        strcat (xbf,q->hndlr);
                        strcat (xbf,"start(), ");
                }
                if (INSTRING(q->mask, POLLSYS)) {
                        strcat (xbf,q->hndlr);
                        strcat (xbf,"chpoll(), ");
                }
                if (INSTRING(q->mask, POLL)) {
                        strcat (xbf,q->hndlr);
                        strcat (xbf,"poll(), ");
                }
                if (INSTRING(q->mask, HALT)) {
                        strcat (xbf,q->hndlr);
                        strcat (xbf,"halt(), ");
                }
                if (INSTRING(q->mask, KENTER)) {
                        strcat (xbf,q->hndlr);
                        strcat (xbf,"kenter(), ");
                }
                if (INSTRING(q->mask, KEXIT)) {
                        strcat (xbf,q->hndlr);
                        strcat (xbf,"kexit(), ");
                }
/*  add new char. dev. funs: mmap and segmap declaration   */
/*  Also Set the version_flag                              */

                if (INSTRING(q->mask, MMAP)) {
                        strcat (xbf,q->hndlr);
                        strcat (xbf,"mmap(), ");
                }
                if (INSTRING(q->mask, SEGMAP)) {
                        strcat (xbf,q->hndlr);
                        strcat (xbf,"segmap(), ");
                }


printextern:
                /* If there were no decls, trunc xbf[] to 0 length */
                if (xbf[strlen(xbf)-2] != ',')
                        xbf[0] = '\0';
                else
                        strcpy(&xbf[strlen(xbf)-2], ";");

                if (INSTRING(q->type, TTYS)) {
                        strcat(xbf,"\nextern struct tty ");
                        strcat(xbf,q->hndlr);
                        strcat(xbf,"_tty[];");
                }
                fprintf(fp,"%s\n",xbf);

                if( INSTRING(q->type, STREAM) )
                {
                     /* declare streamtab structures here */
                     fprintf(fp,"extern struct streamtab %sinfo;\n", q->hndlr);
                }

		/* declare exec switch structures here */

                if (INSTRING(q->type, EXECSW)) {
                        fprintf(fp, "extern short %smagic[%d];\n",q->hndlr,
				q->max);
			fprintf(fp, "extern int %sexec();\n",q->hndlr);
			fprintf(fp, "extern int %score();\n",q->hndlr);
		}
        }


/*
 * Go through block device table and indicate addresses of required routines.
 * If a particular device is not present, fill in "nodev" entries.
 */

        fprintf(fp,"\nstruct bdevsw bdevsw[] = {\n");
        for (b = 0; b <= bbound; b++) {
                q = bdevices[b];
                if( b!=0 )  fprintf(fp, ",\n");
                fprintf(fp,"/*%2d*/\t",b);
                if (q) {

/* print out version flag, size fields */
/* decision logic for d_flag depends on new 4.0 feature */
/* right now it is the NEWDRV in 3rd field of mdev */

                        fprintf(fp,"%sopen,\t%sclose,\t%sstrategy,\t%sprint,",
                                q->hndlr,
                                q->hndlr,
                                q->hndlr,
                                q->hndlr);

			if (INSTRING(q->type, NEWDRV)) {
			   if (INSTRING(q->mask, SIZE))
				fprintf(fp, "\t%ssize,", q->hndlr);
			   else
                        	fprintf(fp,"\tnulldev,");
			}
			else  /*older type-nosize (nodev)*/
                        	fprintf(fp,"\tnulldev,");

                        if (INSTRING(q->mask, POLL))
                                fprintf(fp,"\t%spoll,",q->hndlr);
                        else
                                fprintf(fp,"\tnodev,");

                        if (INSTRING(q->mask, HALT))
                                fprintf(fp,"\t%shalt,",q->hndlr);
                        else
                                fprintf(fp,"\tnodev,");

			fprintf(fp,"\t\"%s\",\t(struct iobuf *)0,", q->dev);

			if (INSTRING(q->type, NEWDRV))
				fprintf(fp, "\t&%sdevflag", q->hndlr);
			else
				fprintf(fp, "\t&nodevflag");
                } else {

                        fprintf(fp,"nodev,\tnodev,\tnodev,\tnodev,\tnulldev,\tnodev,\tnodev,\t\"nodev\",\t0,\t&nodevflag");
                }
        }
        fprintf(fp,"\n};\n\n");
        fprintf(fp,"struct cdevsw cdevsw[] = {\n");
/*
 * Go through character device table and indicate addresses of required
 * routines, or indicate "nulldev" if routine is not present.  If a
 * particular device is not present, fill in "nodev" entries.
 *
 * Add streamtab pointers for STREAMS drivers; they don't need
 * any other fields in this table to be filled in. 
 */
        for (j = 0; j <= cbound; j++) {
                q = cdevices[j];
                fprintf(fp,"/*%2d*/", j);
                if (q) {
                        if (INSTRING(q->mask, OPEN))
                                fprintf(fp,"\t%sopen,",q->hndlr);
                        else
                                fprintf(fp,"\tnulldev,");
                        if (INSTRING(q->mask, CLOSE))
                                fprintf(fp,"\t%sclose,",q->hndlr);
                        else
                                fprintf(fp,"\tnulldev,");
                        if (INSTRING(q->mask, READ))
                                fprintf(fp,"\t%sread,",q->hndlr);
                        else
                                fprintf(fp,"\tnodev, ");
                        if (INSTRING(q->mask, WRITE))
                                fprintf(fp,"\t%swrite,",q->hndlr);
                        else
                                fprintf(fp,"\tnodev, ");
                        if (INSTRING(q->mask, IOCTL))
                                fprintf(fp,"\t%sioctl,",q->hndlr);
                        else
                                fprintf(fp,"\tnodev, ");
/* 4.0 added */
                        if (INSTRING(q->mask, MMAP))
                                fprintf(fp,"\t%smmap,",q->hndlr);
                        else
                                fprintf(fp,"\tnodev, ");
                        if (INSTRING(q->mask, SEGMAP))
                                fprintf(fp,"\t%ssegmap,",q->hndlr);
                        else
                                fprintf(fp,"\tnodev, ");
                        if (INSTRING(q->mask, POLLSYS))
                                fprintf(fp,"\t%schpoll,",q->hndlr);
                        else
                                fprintf(fp,"\tnodev, ");

                        if (INSTRING(q->mask, POLL))
                                fprintf(fp,"\t%spoll,",q->hndlr);
                        else
                                fprintf(fp,"\tnodev, ");

                        if (INSTRING(q->mask, HALT))
                                fprintf(fp,"\t%shalt,",q->hndlr);
                        else
                                fprintf(fp,"\tnodev, ");

                        if (INSTRING(q->type, TTYS))
                                fprintf(fp,"\t%s_tty,",q->hndlr);
                        else
                                fprintf(fp,"\t0,");
                        if( INSTRING(q->type, STREAM) )
                                fprintf(fp,"\t&%sinfo,", q->hndlr);
                        else
                                fprintf(fp,"\t0,");
                        fprintf(fp,"\t\"%s\",\n",q->dev);

			if (INSTRING(q->type, NEWDRV))
                        	fprintf(fp,"\t&%sdevflag,\n", q->hndlr);
			else
                        	fprintf(fp,"\t&nodevflag,\n");
		}
                else
                        fprintf(fp,"\tnodev, \tnodev, \tnodev, \tnodev, \tnodev, \tnodev, \tnodev, \tnodev, \tnodev, \tnodev, \t0,\t0,\t\"nodev\",\t&nodevflag,\n");
        }
        fprintf(fp,"};\n\n");

/*
 * Print the fmodsw table.  STREAMS installables are processed as follows:
 *
 * if mdevice entry field 2 has
 *      "S" this is a module, give it fmodsw entry (retain for back compat.)
 *      "Sm" this is a module, give it fmodsw entry (correct form)
 *      "Sc" this is a driver, no fmodsw entry.
 *      "Smc" this is a driver & module, give it fmodsw entry.
 */

        fprintf(fp, "\nstruct fmodsw fmodsw[] = {\n");
        for( q=devinfo, i=0, j=0; i<=dbound; i++, q++ )
        {
                if( q->conf == 0 )  /* not configured */
                        continue;
                driver=module=0;
                if( INSTRING(q->type, STREAM) ){
                        if( INSTRING(q->type, CHAR))
                                driver++;
                        if( INSTRING(q->type, MOD))
                                module++;
                        if(!driver)
                                module++;
                        if(module){
                                /* This one's a stream module. */
                                if( j++ != 0 )  fprintf(fp,",\n");
				if (INSTRING(q->type, NEWDRV))
					fprintf(fp, "\t\"%s\", &%sinfo,\t&%sdevflag", q->dev, q->hndlr, q->hndlr);
				else
					fprintf(fp, "\t\"%s\", &%sinfo,\t&nodevflag", q->dev, q->hndlr);
                	}
        	}
	}
        if( j == 0 )    /* empty table */
/* add 0 f_flag value */
                fprintf(fp,"\t\"\", (struct streamtab *)0, &nodevflag");
        fprintf(fp, "\n};\n");  /* end of fmodsw initialization */

        fprintf(fp,"int fmodcnt = %d;\n", j );

/*
 * Print out block and character device counts, root, swap, and dump device
 * information.
 */
        fprintf(fp,"int\tbdevcnt = %d;\nint\tcdevcnt = %d;\n\n",(bbound+1),(cbound+1));

/* Declare and allocate block and character shadow tables (new for 4.0) */

	fprintf(fp, "struct bdevsw\tshadowbsw[%d];\n", bbound+1);
	fprintf(fp, "struct cdevsw\tshadowcsw[%d];\n\n", cbound+1);

        fprintf(fp,"dev_t\trootdev = makedevice(%hd, %hd);\n",rtmaj,rtmin);
        fprintf(fp,"dev_t\tpipedev = makedevice(%hd, %hd);\n",pipmaj,pipmin);
        fprintf(fp,"dev_t\tdumpdev = makedevice(%hd, %hd);\n",dmpmaj,dmpmin);
        fprintf(fp,"extern %sdump();\n", dmppt->hndlr);
/*      fprintf(fp, "int\t(*dump)() = %sdump;\n",dmppt->hndlr); */
        fprintf(fp,"dev_t\tswapdev = makedevice(%hd, %hd);\n",swpmaj,swpmin);
/*
 * swap name is from sassign or default
 * offset from swap device is zero
 * flag is set to 0; see vm/bootconf.h
 */
	fprintf(fp,"struct bootobj swapfile = {\"\", \"%s\", 0, 0, 0, 0};\n", 
		swpname);


/*
 * Initialize the init, fork, exec, and exit handler buffers.
 */
        sprintf(initbf,"\nint\t(*io_init[])() = \n{\n");
        sprintf(startbf,"\nint\t(*io_start[])() = \n{\n");
        sprintf(pollbf,"\nint\t(*io_poll[])() = \n{\n");
        sprintf(haltbf,"\nint\t(*io_halt[])() = \n{\n");
        sprintf(kenterbf,"\nint\t(*io_kenter[])() = \n{\n");
        sprintf(kexitbf,"\nint\t(*io_kexit[])() = \n{\n");
/*
 * for every entry, set up to print their init and start
 * routines, if they exist
 */
        for (q = devinfo, i = 0; i <= dbound; i++, q++) {
                if (q->conf == 0)
                        continue;
                if (INSTRING(q->mask, INIT))
                                sprintf(&initbf[strlen(initbf)],"\t%sinit,\n",q->hndlr);
                if (INSTRING(q->mask, START))
                                sprintf(&startbf[strlen(startbf)],"\t%sstart,\n",q->hndlr);
                if (INSTRING(q->mask, POLL))
                                sprintf(&pollbf[strlen(pollbf)],"\t%spoll,\n",q->hndlr);
                if (INSTRING(q->mask, HALT))
                                sprintf(&haltbf[strlen(haltbf)],"\t%shalt,\n",q->hndlr);
                if (INSTRING(q->mask, KENTER))
                                sprintf(&kenterbf[strlen(kenterbf)],"\t%skenter,\n",q->hndlr);
                if (INSTRING(q->mask, KEXIT))
                                sprintf(&kexitbf[strlen(kexitbf)],"\t%skexit,\n",q->hndlr);
        }
/*
 * Write out NULL entry into init, fork, poll and halt buffers.
 * Then write the buffers out into the configuration file.
 */
        sprintf(&initbf[strlen(initbf)],"\t(int (*)())0\n};\n");
        fprintf(fp,"%s",initbf);

        sprintf(&startbf[strlen(startbf)],"\t(int (*)())0\n};\n");
        fprintf(fp,"%s",startbf);

        sprintf(&pollbf[strlen(pollbf)],"\t(int (*)())0\n};\n");
        fprintf(fp,"%s",pollbf);

        sprintf(&haltbf[strlen(haltbf)],"\t(int (*)())0\n};\n");
        fprintf(fp,"%s",haltbf);

        sprintf(&kenterbf[strlen(kenterbf)],"\t(int (*)())0\n};\n");
        fprintf(fp,"%s",kenterbf);

        sprintf(&kexitbf[strlen(kexitbf)],"\t(int (*)())0\n};\n");
        fprintf(fp,"%s",kexitbf);

	/*  configures in dispacher classes */

	fprintf (fp, "extern void  sys_init();\n");
	for (q = devinfo, i = 0; i <dbound; i++, q++) {
		if (INSTRING(q->type, DISP) && q->conf != 0)
			fprintf (fp, "extern void  %s_init();\n",q->hndlr);
	}

	fprintf (fp, "class_t class[] = {\n");
	fprintf (fp, "\t\t\"SYS\", sys_init, NULL,\n");
	for (q = devinfo, i = 0, j = 1; i <dbound; i++, q++) {
		if (INSTRING(q->type, DISP) && q->conf != 0) {
                	uppermap(q->hndlr, caps);
			fprintf (fp, "\t\t\"%s\", %s_init, NULL,\n",
				caps, q->hndlr);
			j++;
		}
	}
		
	fprintf (fp, "};\n\n");
	
	fprintf (fp, "int\tnclass = %d;\n", j);

/*
 * Print the execsw table. Exec modules are identified by the
 * presence of flag 'e' in the type (third) field of mdevice.
 * The number of entries per module depends on the number
 * specified in the eighth field of mdevice. The priority field
 * (fourth field in sdevice) is used to sort the order of the
 * entries in this table.
 */

	listhead = NULL;
        for( q=devinfo, i=0; i<=dbound; i++, q++ )
        {
                if( q->conf == 0 )  /* not configured */
                        continue;
		if (INSTRING(q->type, EXECSW))
			sort_entry(q, &listhead);

	}
	print_execsw(fp, listhead);

        fclose(fp);
}



void sort_entry(entry, head)
struct t2 *entry;
struct linklist **head;
{
	struct linklist *new;	/* new element to create */
	struct linklist *cur;	/* traverses the list - current position */
	struct linklist *trail;	/* keeps track of previous element */
	char *malloc();

	new = (struct linklist *)malloc(sizeof(struct linklist));
	new->mdev_entry = entry;
	if (*head == NULL) {	/* empty list */
		*head = new;
		new->next = NULL;
	}
	else {
		trail = *head;
		for (cur = *head; cur != NULL; cur=cur->next) {
			/* if higher priority, place ahead of the list */
			if (new->mdev_entry->ipl > cur->mdev_entry->ipl) {
				if(cur == *head) { /* add to head of list */
					new->next = *head;
					*head = new;
				}
				else {
					new->next = trail->next;
					trail->next = new;
				}
				break;
			}
			if (cur->next == NULL) {  /* insert at the end of list */
				new->next = NULL;
				cur->next = new;
				break;
			}
			trail = cur;
		}
	}
}



void print_execsw(fp, head)
FILE *fp;
struct linklist *head;
{
	struct linklist *cur;	/* traverses the sorted list */
	struct t1 *sdev_entry;
	int j, k;

        fprintf(fp, "\nstruct execsw execsw[] = {\n");
	j = 0;
	if (head == NULL) {	/* empty table */
		fprintf(fp,"\t{ 0, 0, 0 }\n");
	}
	else {
		for (cur = head; cur != NULL; cur = cur->next) {
			for(k=0; k < cur->mdev_entry->max; k++) {
				if( j++ != 0 )  fprintf(fp,",\n");
				fprintf(fp,"\t{ %smagic+%d, %sexec, %score }",
					cur->mdev_entry->hndlr, k,
					cur->mdev_entry->hndlr,cur->mdev_entry->hndlr);
			}
			if ((sdev_entry = sfind(cur->mdev_entry->dev)) != NULL) {
				/* A units entry of 0 means the module has a
				 * "default" exec handler.
				 */
				if(sdev_entry->units == 0) {
					if( j++ != 0 )  fprintf(fp,",\n");
					fprintf(fp,"\t{ NULL, %sexec, %score }",
					cur->mdev_entry->hndlr,cur->mdev_entry->hndlr);
				}
	
			}
		}
	}
        fprintf(fp, "\n};\n");  /* end of execsw initialization */

        fprintf(fp,"int nexectype = %d;\n", j );
}




/* Create 1) file listing path names of configured driver packages.
 * and all file system types!
 */

prfiles()
{
        struct t2 *p;
        struct t4 *f;
        int i;
        FILE *pp;
        char path[512];

        pp = open1(pfile, "w", OUT);
        chmod(pfile, 0644);

        strcpy(path,root);
        strcat(path,"/pack.d");

        if( chdir(path) != 0 )
        {
                fprintf(stderr, "%s dir; errno=%d\n", path,errno);
                sprintf(errbuf, "Can't change to pack.d directory.");
                fatal(0);
        }

        /* record drivers */
        for (p = devinfo, i = 0; i <= dbound; i++, p++)
        {
                if( p->conf == 3 )  /* obsolete designation */
                        continue;
                if( p->conf == 0 )      /* not configured 'N'  */
                        wr_file(0, p->dev, pp);
                else                    /* configured 'Y' */
                        wr_file(1, p->dev, pp);
        }
        for( f=fstype, i=0; i<=fbound; i++, f++ )
        {
                if( f->fs_conf[0] == 'Y' ) /* no stubs.c files here */
                {
                        if( strcmp(f->fs_prefix,"du") == 0 )
                                wr_file(1, "dufst", pp);
                        else
                                wr_file(1, f->fs_name, pp);
                } else 
			wr_file(0, f->fs_name, pp);
        }
        fclose(pp);

        if( chdir("..") != 0 )
        {
                sprintf(errbuf, "Cannot change back to build directory.");
                fatal(0);
        }
                
}

/*
 * Write files in the directory 'name' to the file of objects for
 * the kernel we're building.
 */
wr_file(flag,dirname,ifp)
int flag;
char *dirname;
FILE *ifp;
{
        struct stat bstat;
        char buff[100];

        /* move to driver package directory */
        if (chdir(dirname) != 0)
        {
                fprintf(stderr, "%s dir; errno=%d\n", dirname,errno);
                sprintf(errbuf, EXIST, dirname);
                error(0);
                return;
        }

        /* pick up stubs.c if driver's not configured */
        if( flag==0 )  /* Only configure stubs if 'N' in sdevice  */
        {
                if( stat("stubs.c",&bstat)==0 )
                        fprintf(ifp, "%s/pack.d/%s/stubs.c\n", root, dirname);
                chdir("..");
                return; /* done with this one */
        }

        /* check for Driver.o and add to file */
        if (stat("Driver.o", &bstat) == 0 )
                fprintf(ifp, "%s/pack.d/%s/Driver.o\n", root, dirname);

        else{           /* Require a Driver.o if 'Y' in sdevice  */
                sprintf(errbuf, "Cannot find Driver.o for %s.", dirname);
                error(0);
        }

        /* add space.c to file */
        if (stat("space.c", &bstat) == 0)
                fprintf(ifp, "%s/pack.d/%s/space.c\n", root, dirname);

        chdir("..");

        return;
}
/*
 *  Print the configuration file for file system types that
 *  are to be included in the system.
 *
 *  By default, this file is "fsconf.c".
 */
prfs()
{
        register FILE *fp;
        register struct t4 *f;
        int i, j;
        int k=0;

        fp = open1(ffile, "w", OUT);
        chmod(cfile, 0644);

        fprintf(fp,"#include\t\"%s\"\n",hfile);
        fprintf(fp,"#include\t\"sys/types.h\"\n");
        fprintf(fp,"#include\t\"sys/vfs.h\"\n");

/* First, search through the fstype table and print an
 * extern declaration for all the switch functions that
 * are supposed to be provided in the configured file types.
 */
        /*  
         *  start declaring the file system external init routines
         *  format is 
         *  extern int prefix+init();
         */

        /* loop for each configured type */
        for( f=fstype, i=0, j=0; i<=fbound; f++, i++ )
        {
                /* skip over empty table entries */
                if( f->fs_prefix[0] == '\0' )  continue;

                /* skip unconfigured fs types */
                if( f->fs_conf[0] == 'N' )  continue;

                fprintf(fp,"extern int  %sinit() ;", f->fs_prefix);
		j++;
        }

        fprintf(fp,"int nfstype = %d;\n", (j+1));

/*
 * Next, set up and initialize the vfssw data structure table.
 */
        fprintf(fp,"\nstruct vfssw vfssw[] = {\n");
	/* first, initialize the NULL fs type  */
        fprintf(fp, "\"EMPTY\", 0x0, 0x0, 0x0");
        for( f=fstype, i=0; i<=fbound; f++, i++ ) 
        {
                /* skip unconfigured fs types */
                if( f->fs_conf[0] == 'N' )  continue;
                fprintf(fp,",\n\"%s\", %sinit, 0x0, 0x0", f->fs_name, f->fs_prefix);
        }
        fprintf(fp,"\n};\n\n");

}
