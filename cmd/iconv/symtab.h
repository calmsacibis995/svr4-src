/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)iconv:symtab.h	1.1.1.1"

/*
 * symtab.h	- symbol table structure & flags
 */

#define UNDEF		0	/* for undefined symbols... */

#define S_FUNC		1	/* operator */
#define S_PARM		2	/* parameter (string) */
#define S_NAME		3	/* name of a binding set */
#define S_SWTCH		4	/* is a value */

/*
 * S_FUNC type nodes are parents.  S_PARAM type nodes are children.
 * many children may point to the same parent.
 */

struct sym {
	char *s_name;		/* pointer to name of item */
	char *s_value;		/* symbol value */
	char s_type;		/* symbol type (one of S_?) */
	struct sym *s_next;	/* ptr to next in chain */
};

struct sym *s_find(), *s_create(), *s_lookup(), *s_value();
char *strsave();

#define N_CHILD		0	/* child node ptr */
#define N_RESULT	0x01	/* result pointer */
#define N_EMPTY		0x02	/* temp "filled" node */

struct node {
	unsigned char n_val;
	char n_flag;	/* N_CHILD / N_RESULT / N_EMPTY */
	unsigned short n_num;	/* node number */
	struct node *n_next;
	union {
		struct node *n_child;
		char *n_result;
	} n_what;
	unsigned short n_node;	/* number of child node or ptr to text */
	int n_lnum;	/* during initial tree, N_RESULT line number */
};

#define E_DUP (-2)
#define E_ERROR (-1)

/*
 * For the "file" command, use following lines in /etc/magic:
0	string		kbd!map		kbd map file
>8	byte		>0		Ver %d:
>10	short		>0		with %d table(s)

 */

#define KBD_MAGIC	"kbd!map"
#define KBD_HOFF	8	/* offset in h_magic[] of version number */

/*
 * Version stamps:  The 3B2 and 386 byte order are incompatible, therefore
 * they must have different version numbers to insure that tables compiled
 * for one don't get loaded on the other.  The loader will reject them
 * right away.  Don't define another unless it really won't work for
 * your machine.
 */

#ifdef u3b2
# define KBD_VER		1	/* 3B2 version */
#else
# ifdef i386
#    define KBD_VER		2	/* 386 version */
# else
#    define KBD_VER		3	/* other version */
# endif
#endif

/*
 * kbd_map only used by the compiler.
 */

struct kbd_map {
	char *mapname;
	struct node *maproot;
	unsigned char *maptext;	/* the text */
	struct cornode *mapnodes;	/* the nodes */
	struct kbd_tab *maptab;
	unsigned char *mapone;
	unsigned char *maperr;	/* error string */
	unsigned char map_min;	/* min & max values */
	unsigned char map_max;
};

/*
 * For linkage maps (mapname is (-1), maptext points to string.
 * For external (ALP) maps, it's (-2), maptext points to string.
 */
#define LINKAGE	((char *) -1)
#define EXTERNAL ((char *) -2)
