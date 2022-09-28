/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)iconv:kbd.h	1.2.3.1"

/*
 * Until such time as we have a better way to do things, we have
 * to define an ioctl "header".  This is it...hope it doesn't
 * conflict with something else.
 */
#define KBD	(('Z' | 128) << 8)

/*
 * Misc definitions fundamental to the implementation.  If any of them
 * change, everything has to be re-compiled.  Mostly these are boundary
 * conditions.  Entries with "+1" mean that the last position is reserved
 * for the null character, as they are character STRING sizes.
 */
#define KBDNL	16	/* max name length (+1) */
#define KBDVL	20	/* max length (+1) of "verbose" strings */
#define KBDTMAX	65500	/* max size of a table, in absence of other factors */
#define KBDIMAX	128	/* max length of a search string */
#define KBDOMAX 256	/* max length of a result string */
#define KBDLNX	16	/* max links in composite */
#define NTLINKS 10	/* number of new tablinks to alloc */

/*
 * Minimum & maximum timer values.  The max value should be long enough
 * so that "even the slowest typists won't time-out unless they really have
 * their hands away from the keyboard long enough to forget what they typed".
 * The min value should be at least 3 ticks so that we never come back until
 * the scheduler etc. has had time to run (don't hog the callout table).
 */

#define MINTV	5	/* minimum timer value */
#define MAXTV	500	/* max timer value: s/b at least 4sec */

/*
 * cornode c_flags.  Anything not defined here is RESERVED!  Don't
 * touch, please.
 */
#define ND_RESULT	0x01
#define ND_TIMER	0x02
#define ND_INLINE	0x04
#define ND_LAST		0x80
#define ND_RESERVED	0x78	/* all the reserved bits */

/*
 * the final node structure "in core node"
 */
struct cornode {
	unsigned short c_child;
	unsigned char c_val;
	unsigned char c_flag;	/* high bit on == end of list */
};

/*
 * One header per file
 */
struct kbd_header {
	unsigned char h_magic[10];	/* magic number */
	short h_ntabs;	/* number of tables in the file */
};

/*
 * t_flag definitions:
 */
#define KBD_ONE	0x01	/* contains one-one */
#define KBD_FULL 0x02	/* "filled" root */
#define KBD_ERR 0x04	/* contains Error node */
#define KBD_TIME 0x08	/* default timed table */
#define KBD_ALP 0x10	/* reserved ALP bit */
#define KBD_COT 0xFF	/* is a composite table, NOT a real table */

/*
 * One "table" structure per table, "h_ntabs" per file.
 * FIX ME: could be divvied up better so that LESS needs to be
 * kept on disk -- there are lots of unused fields stored out there.
 * Also, remove t_swtch -- it's being phased out.
 */
struct kbd_tab {
	unsigned char t_name[KBDNL];	/* name of table */
	unsigned short t_nodes;	/* number of NODES */
	unsigned short t_text;	/* total length of result text */
	unsigned short t_flag;	/* flags (incl. oneone flag) */
	unsigned short t_error;	/* error "text" space */
	unsigned char t_min;	/* min & max root values */
	unsigned char t_max;

	struct cornode *t_nodep;/* in-core pointer to nodes */
	unsigned char *t_textp;	/* in-core pointer to top of text */
	unsigned char *t_oneone;/* one-one 256-byte direct map */
	struct kbd_tab *t_next;	/* in-core next table */
	struct tablink *t_child;/* in-core for composite tables */
	unsigned int t_asize;	/* in-core sizeof actual allocation */
	unsigned short t_ref;	/* in-core reference count */
};

/*
 * Re-use some fields for ALP when (t_flag & KBD_ALP).  The ALP hooks
 * are here to preserve binary compatibility.
 */

#define t_alp	t_nodep		/* reserved ALP pointer */
#define t_alpname	t_textp	/* reserved ALP name offset */
#define t_alpfunc	t_oneone	/* reserved ALP func */

/*
 * kbd module ioctls and structures.
 */

#define KBD_LOAD	(KBD| 1)	/* initiate a download */
#define KBD_CONT	(KBD| 2)	/* continuation of download */
#define KBD_END		(KBD| 3)	/* end of download */
#define KBD_UNLOAD	(KBD| 4)	/* unload by name */
#define KBD_QUERY	(KBD| 5)	/* administrative */
#define KBD_ATTACH	(KBD| 6)	/* attach to named table */
#define KBD_DETACH	(KBD| 7)	/* detach from named table */
#define KBD_VERB	(KBD| 8)	/* set verbose mode */
#define KBD_HOTKEY	(KBD| 9)	/* set hot-key */
#define KBD_ON		(KBD|10)	/* turn on mapping */
#define KBD_OFF		(KBD|11)	/* turn off mapping */
#define KBD_LIST	(KBD|12)	/* list available (loaded) tables */
#define KBD_LINK	(KBD|13)	/* make composite */
#define KBD_TSET	(KBD|14)	/* set timer */
#define KBD_TGET	(KBD|15)	/* get current timer value */
#define KBD_EXT		(KBD|16)	/* ALP entry (external) */

#define Z_HK0		0	/* hot key modes (low 7 bits) */
#define Z_HK1		1
#define Z_HK2		2
#define Z_HKVERB	0x80	/* verbose mode for hot keys */

/*
 * Structure for initiating a download.  Only one download can be in
 * progress at a time per Stream.  "tabsize+onesize+nodesize+textsize" is
 * the total space that must be malloc'd for a table.  Only root can
 * do a public download.  Others must attach or do private downloads.
 */

struct kbd_load {
	int z_tabsize;	/* size of table structure */
	int z_onesize;	/* size of one-one table */
	int z_nodesize;	/* total size of nodes */
	int z_textsize;	/* total size of text */
	unsigned char z_name[KBDNL];	/* name of table */
};

/*
 * Alignment characteristics.
 */

#define ALIGNER long
#define ALIGNMENT sizeof(long)

/*
 * Flags for t_type field:
 */
#define Z_ON		0x01
#define Z_OFF		0x02
/*
 * Flags for c_type field:
 */
#define Z_SET		0x01
#define Z_GET		0x02
/*
 * Z_UP/Z_DOWN - multiple use.  "u_doing" is one or more of these.
 */
#define Z_UP		0x10
#define Z_DOWN		0x20

/*
 * Attach/detach structure
 */
struct kbd_tach {
	unsigned short t_type;	/* one of Attach, Detach with Up or Down */
	unsigned char t_table[KBDNL];	/* table name */
};

/*
 * Control structure for On/off requests.  Also used with KBD_END to
 * hold Z_PUBLIC or Z_PRIVATE.
 */
struct kbd_ctl {
	unsigned short c_type;	/* one of On or Off with Up or Down */
	unsigned short c_arg;	/* expansion slot */
};

/*
 * Structure for timer requests.  Since tables can't be multiply attached,
 * the name is sufficient to find the table.  If it's a composite,
 * s_nth is set to the component number.  Timer value is 5-150 and
 * represents a value in clock ticks, and is not the numerator of
 * a fraction in seconds (it is thus machine dependent, but may provide
 * greater control).  This structure currently unused.
 */

struct timeset {
	int s_nth;	/* nth child, if composite */
	unsigned short s_time;		/* timer value, 10-200 */
	unsigned char s_table[KBDNL];	/* table name */
};

/*
 * Flags for downloading
 */

#define mi_contra_fa	0
#define Z_PUBLIC	1
#define Z_PRIVATE	2

/*
 * query structure & "q_flag" flags.
 */

#define KBD_QF_PUB	0x01	/* public, else private */
#define KBD_QF_COT	0x02	/* if composite */
#define KBD_QF_TIM	0x04	/* if timed */
#define KBD_QF_EXT	0x08	/* if ALP */

struct kbd_query {
	unsigned char q_name[KBDNL];	/* name of table */
	long q_id;		/* unique identifier */
	int q_nchild;		/* number of elements */
	long q_child[KBDLNX];	/* element identifiers */
	char q_chtim[KBDLNX];	/* element timing data */
	int q_seq;		/* sequence */
	unsigned int q_asize;	/* in-core sizeof allocation */
	unsigned short q_tach;	/* Z_UP and/or Z_DOWN */
	unsigned short q_ref;	/* in-core reference count */
	unsigned short q_flag;	/* PUB, COT, TIM */
	unsigned short q_time;	/* timing data (simple only) */
	unsigned char q_hkin;	/* in hot key */
	unsigned char q_hkout;	/* out hot key */
};
