/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)sh:mode.h	1.8.3.1"
/*
 *	UNIX shell
 */

#ifdef pdp11
typedef char BOOL;
#else
typedef short BOOL;
#endif

#define BYTESPERWORD	(sizeof (char *))
#define	NIL	((char*)0)


/* the following nonsense is required
 * because casts turn an Lvalue
 * into an Rvalue so two cheats
 * are necessary, one for each context.
 */
#define Rcheat(a)	((int)(a))


/* address puns for storage allocation */
typedef union
{
	struct forknod	*_forkptr;
	struct comnod	*_comptr;
	struct fndnod	*_fndptr;
	struct parnod	*_parptr;
	struct ifnod	*_ifptr;
	struct whnod	*_whptr;
	struct fornod	*_forptr;
	struct lstnod	*_lstptr;
	struct blk	*_blkptr;
	struct namnod	*_namptr;
	char	*_bytptr;
} address;


/* heap storage */
struct blk
{
	struct blk	*word;
};

#define	BUFSIZ	128
struct fileblk
{
	int	fdes;
	unsigned flin;
	BOOL	feof;
	unsigned char	fsiz;
	unsigned char	*fnxt;
	unsigned char	*fend;
	unsigned char	**feval;
	struct fileblk	*fstak;
	unsigned char	fbuf[BUFSIZ];
};

struct tempblk
{
	int fdes;
	struct tempblk *fstak;
};


/* for files not used with file descriptors */
struct filehdr
{
	int	fdes;
	unsigned	flin;
	BOOL	feof;
	unsigned char	fsiz;
	unsigned char	*fnxt;
	unsigned char	*fend;
	unsigned char	**feval;
	struct fileblk	*fstak;
	unsigned char	_fbuf[1];
};

struct sysnod
{
	char	*sysnam;
	int	sysval;
};

/* this node is a proforma for those that follow */
struct trenod
{
	int	tretyp;
	struct ionod	*treio;
};

/* dummy for access only */
struct argnod
{
	struct argnod	*argnxt;
	unsigned char	argval[1];
};

struct dolnod
{
	struct dolnod	*dolnxt;
	int	doluse;
	unsigned char	**dolarg;
};

struct forknod
{
	int	forktyp;
	struct ionod	*forkio;
	struct trenod	*forktre;
};

struct comnod
{
	int	comtyp;
	struct ionod	*comio;
	struct argnod	*comarg;
	struct argnod	*comset;
};

struct fndnod
{
	int 	fndtyp;
	unsigned char	*fndnam;
	struct trenod	*fndval;
};

struct ifnod
{
	int	iftyp;
	struct trenod	*iftre;
	struct trenod	*thtre;
	struct trenod	*eltre;
};

struct whnod
{
	int	whtyp;
	struct trenod	*whtre;
	struct trenod	*dotre;
};

struct fornod
{
	int	fortyp;
	struct trenod	*fortre;
	unsigned char	*fornam;
	struct comnod	*forlst;
};

struct swnod
{
	int	swtyp;
	unsigned char *swarg;
	struct regnod	*swlst;
};

struct regnod
{
	struct argnod	*regptr;
	struct trenod	*regcom;
	struct regnod	*regnxt;
};

struct parnod
{
	int	partyp;
	struct trenod	*partre;
};

struct lstnod
{
	int	lsttyp;
	struct trenod	*lstlef;
	struct trenod	*lstrit;
};

struct ionod
{
	int	iofile;
	unsigned char	*ioname;
	unsigned char	*iolink;
	struct ionod	*ionxt;
	struct ionod	*iolst;
};

struct fdsave
{
	int org_fd;
	int dup_fd;
};


#define		fndptr(x)	((struct fndnod *)x)
#define		comptr(x)	((struct comnod *)x)
#define		forkptr(x)	((struct forknod *)x)
#define		parptr(x)	((struct parnod *)x)
#define		lstptr(x)	((struct lstnod *)x)
#define		forptr(x)	((struct fornod *)x)
#define		whptr(x)	((struct whnod *)x)
#define		ifptr(x)	((struct ifnod *)x)
#define		swptr(x)	((struct swnod *)x)
