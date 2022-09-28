/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)mkpart:mkpart.h	1.1"

/*
 *	Memory copying macro
 * use only with constant sized copy operations.
 */
#define COPY(x,y,n)							\
	{	struct k {char c[(n)];} ;				\
		*((struct k *)&(x)) = *((struct k *)&(y));		\
	}

/*
 *	Symbol Table declaractions
 */
typedef struct symbol {
	struct symbol *next;	/* next symbol in this hash list */
	char	flags;		/* symbol flags */
	char	*name;		/* symbol name */
	char	*ref;		/* generic pointer to arbitrary data */
} symbol;

	/* Symbol flags */
#define SY_DECL	0x01	/* symbol name referenced */
#define SY_DEFN	0x02	/* symbol name defined */

#define NHASH	67		/* number of hash buckets; prime number */
extern symbol	*hashbuckets[NHASH];

#define MAXLINE 256		/* longest allowed input line */
	
/*
 *	Parsing declarations
 */

	/* node tag types */
#define T_NULL		0
#define T_STANZA	1
#define T_USES		2
#define T_DEVNAME	3

	/* stanza types */
#define S_NULL		0
#define S_DEVICE	1
#define S_PART		2


	/*
	 *	Parse Nodes
	 */
typedef struct node {
	int	token;		/* token value */
	union {
		char	*string;
		char	*name;
		char	*filename;
		void	*ref;
		unsigned long number;
		struct	{
			unsigned long low, high;
			} range;
		struct	{
			struct node *next;
			union {
				struct node *data;
				void *ref;
				} d;
			} list_elem;
	} v;
} node;
#define String		v.string
#define Name		v.name
#define Filename	v.filename
#define Number		v.number
#define RangeLo         v.range.low
#define RangeHi         v.range.high
#define ListElem	v.list_elem.d.data
#define ListRef		v.list_elem.d.ref
#define ListNext	v.list_elem.next

	/*
	 *	Device Stanza
	 */
typedef struct devstanza {
	char	ds_tag;		/* node type: T_STANZA */
	char    ds_type;        /* stanza type: S_DEVICE */
	symbol  *ds_name;       /* my name */
	symbol  *ds_use;        /* use stanza */
	char	*ds_boot;	/* bootstrap code filename */
	char	*ds_device;	/* device name for whole disk */
	char	*ds_dserial;	/* serial number string */
	unsigned short ds_heads;	/* number of heads */
	unsigned short ds_cyls;		/* number of cylinders */
	unsigned short ds_sectors;	/* number of sectors/track */
	unsigned short ds_bpsec;	/* number of bytes/sector */
	unsigned long ds_vtocsec;	/* vtoc sector */
	unsigned long ds_altsec;	/* alternate track table */
	node	*ds_badsec;	/* bad sector list */
	node	*ds_badtrk;	/* bad track list */
} devstanza;

	/*
	 *	Partition Stanza
	 */
typedef struct partstanza {
	char	ps_tag;		/* node type: T_STANZA */
	char    ps_type;        /* stanza type: S_PART */
	symbol  *ps_name;       /* my name */
	symbol  *ps_use;        /* use stanza name */
	int	ps_partno;	/* partition number in vtoc */
	short   ps_ptag;        /* partition tag */
	short   ps_perm;        /* i/o permission */
	unsigned long ps_start;	/* starting sector */
	unsigned long ps_size;	/* number of sectors in partition */
} partstanza;

#define UNDEFINED_SECTOR	0xffffffff	/* invalid sector number, size */
#define UNDEFINED_NUMBER	0x55555555	/* an unlikely number */

typedef union stanza {
	devstanza dev;
	partstanza part;
} stanza;

typedef union YYSTYPE {
	int	token;
	stanza	*stanza;
	node	*node;
	symbol	*symbol;
} YYSTYPE;

extern YYSTYPE yylval;

/*
 *	External Declarations
 */

#define PARTFILE	"/etc/partitions"

	/* partitions.y */
stanza		*newpstanza(),
		*newdstanza(),
		*newpartstanza(),
		*newdevstanza(),
		*add2pstanza(),
		*add2dstanza();
node		*mergeranges(),
		*newnode();
int             overlap();
void            initnodes(),
		add_nl_elem(),
		freenode();

	/* scan.c */
void		myerror();
unsigned int	hash();
symbol 		*lookup();

	/* mkpart.c */
void            giveusage(),
		formatdevice(),
		builddevice(),
		writevtoc(),
		writeboot(),
		updateparts(),
		updatedevice(),
		buildstanza(),
		printstanza(),
		printdevinfo(),
		printalts(),
		printpart();

int		readbootblock(),
		getdevice(),
		getboot();

extern int      MaxUseDepth;

	/* outside world */
char		*malloc(),
		*realloc();

void		qsort();
