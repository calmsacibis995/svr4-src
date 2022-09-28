/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)dis:common/structs.h	1.3"

#include        <libelf.h>

#define FAILURE 0
#define SUCCESS 1
#define	TEXT	0
#define DATA	1

typedef struct scnlist SCNLIST;
typedef struct funclist FUNCLIST;
typedef struct nfunction NFUNC;

/*
 * The linked list of scnlist's describes which sections are
 * to be disassembled
 */

struct scnlist
{
	Elf32_Shdr	*shdr;
	char		*scnam;
	SCNLIST		*snext;
	Elf32_Word	scnum;
	int		stype;		/* disassemble as text or data   */
	FUNCLIST	*funcs;		/* the list of functions defined */
					/* in this section		 */
};

/*
 * A list of functions is associated with each section. This list is
 * used for printing the names of the functions and resyncing.
 */

struct funclist
{
	char		*funcnm;
	long		faddr;		/* address of the function	*/
	long		fcnindex;	/* index of the function in 
						the symbol table */
	FUNCLIST	*nextfunc;
};

/*
 * If the -F option is specified, an array of nfunctions is set up
 * contiaing information about the functions
 */

struct nfunction
{
	char	*funcnm;
	long	faddr;
	long	fcnindex;
	char	fnumaux;
	long	fsize;
	int	found;
	unsigned short	fscnum;
};


/* the following structures are used for symbolic disassembly */

/* structures for holding information about external and static symbols */

/* extern-static hash table structure */
typedef	struct node essymrec;
typedef struct node *pessymrec;
struct	node {
			char 	*name;
			long	symval;
			struct node	*next;
};

/* extern-static union-array list structure */
typedef struct ua	uarec;
typedef	struct ua	*puarec;
struct	ua {
			char	*name;
			long	symval;
			unsigned short type;
			int	range;
			struct ua	*next;
};


/* structures for holding information about local symbols.  The local symbol structure
   requires the extra field containing symbol storage class info. (sclass) to
   distinguish between automatic symbols, function arguments and register symbols */

/* local hash table structure */
typedef	struct lnode locsymrec;
typedef struct lnode *plocsymrec;
struct	lnode {
			char 	*name;
			long	symval;
			char	sclass;
			struct lnode	*next;
};

/* local union-array list structure */
typedef struct lua	locuarec;
typedef	struct lua	*plocuarec;
struct	lua {
			char	*name;
			long	symval;
			unsigned short type;
			char	sclass;
			int	range;
			struct lua	*next;
};

#define ESHTSIZ 500	/* external and static hash table size */
#define LOCHTSIZ 50	/* local hash table size */
