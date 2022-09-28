/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)idcmd:inst.h	1.3"

/* Header file for Installable Drivers commands */

/* path names of ID directories */
#define ROOT	"/etc/conf"		/* root of ID */
#define EROOT	"/etc"			/* directory for installing environment */
#define DROOT	"/dev"			/* directory for installing nodes */
#define	BUILD	"cf.d"			/* Master & System files for building */
#define INSTALL	"cf.d"			/* Master & System files for installation */

/* file names of Master and System files */
#define	MDEVICE	"mdevice"		/* Master device file */
#define MFSYS	"mfsys"			/* Master fs type file */
#define	MTUNE	"mtune"			/* Master tunable device file */
#define SDEVICE	"sdevice"		/* System device file */
#define SFSYS	"sfsys"			/* System fs type file */
#define STUNE	"stune"			/* System tunable parameter file */
#define SASSIGN	"sassign"		/* System assign - root, pipe, dump, swap */
#define MASTER	"Master"

#define FIRST	"0"
#define	NEXT	"1"
#define RESET	"2"
#define	MDEV   	0		/* Master device file */
#define	MTUN	1		/* Master tunable parameter file */
#define BDEV	2		/* Master device file for booted Kernel */
#define BTUN	3		/* Master tunable file for booted Kernel */
#define	SDEV   	4		/* System device file */
#define	STUN	5		/* System tunable parameter file */
#define SASN	6		/* System assign file - root, pipe, dump, swap */
#define MFS	7		/* Master file system type file */
#define SFS	8		/* System fs type specification file */

#define NAMESZ	15

/*   ELF compiler version string  */
#define ELFVERSTR	"'(SCDE) 5.0'"

struct mdev {			/* Master file structure for devices */
	char	device[NAMESZ];	/* device name */
	char	mask[10];	/* letters indicating existing handlers */
	char	type[20];	/* letters indicating device type */
	char	handler[6];	/* handler name */
	short	blk;		/* major dev number if block device */
	short	chr;		/* major dev number if character device */
	short	min;		/* minimum number of units */
	short	max;		/* maximum number of units */
	short	chan;		/* DMA channel */
	short	blk_start;	/* start of multiple majors range - blk */
	short	blk_end;	/* end of multiple majors range -blk */
	short	char_start;	/* start of multiple majors range - char */
	short	char_end;	/* end of multiple majors range - char */
};

struct mtun {			/* Master file structure for tunable parameters */
	char	oudef[21];	/* output parameter keyword */
	long	def;		/* default value */
	long	min;		/* minimum value */
	long	max;		/* maximum value */
};

struct sdev {			/* System file structure for devices */
	char	device[NAMESZ];	/* device name */
	char	conf;		/* Y/N - Configured in Kernel */
	short	units;		/* number of units */
	short	ipl;		/* ipl level for intr handler */
	short	type;		/* type of interrupt scheme */
	short	vector;		/* interrupt vector number */
	long	sioa;		/* start I/O address */
	long	eioa;		/* end I/O address */
	long	scma;		/* start controller memory address */
	long	ecma;		/* end controller memory address */
};

struct stun {			/* System file structure for tunable parms. */
	char	name[21];	/* name of tunable parameter */
	long	value;		/* number specified */
};

struct sasn {			/* System file struct. for root, pipe, dump, swap */
	char	device[9];	/* device name */
	char	major[9];	/* major device name */
	short	minor;		/* minor device number */
	long	low;		/* lowest disk block in area */
	short	blocks;		/* number of disk blocks in area */
};

struct sfsys {
	char name[9];		/* fstype name */
	unsigned int funcmask;	/* functions provided in fs type */
};

/* variables for multiple major numbers */

struct	multmaj	{
	char	brange[20];
        char	crange[20];
};

/* funcmask values: fstypsw functions. */
#define FINIT	0
#define IPUT	1
#define IREAD	2
#define FILLER	3
#define IUPDAT	4
#define READI	5
#define WRITEI	6
#define ITRUNC	7
#define STATF	8
#define NAMEI	9
#define MOUNT	10
#define UMOUNT	11
#define GETINODE	12
#define OPENI	13
#define CLOSEI	14
#define UPDATE	15
#define STATFS	16
#define ACCESS	17
#define GETDENTS	18
#define ALLOCMAP	19
#define FREEMAP	20
#define READMAP	21
#define SETATTR	22
#define NOTIFY	23
#define FCNTL	24
#define FSINFO	25
#define FIOCTL	26
#define FILL	27
