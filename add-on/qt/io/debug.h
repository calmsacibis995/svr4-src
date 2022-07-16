/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)qt:io/debug.h	1.3"
static int	debug_flag = 31;
static int	z;

#define DB_ENTER	00001
#define DB_STRUCT	00002
#define DB_BYTES	00004
#define DB_RETURN	00010
#define DB_WAIT		00020

#define RETURN(x,y)	return (z = (y), (debug_flag & DB_RETURN ? printf ("r%d:%d", (x), z) : 0), z)

char
msinb (x)
register int	x;
{
	register char	z;
	z = inb (x);
	if (debug_flag & DB_BYTES)
		printf ("i%o:z%o:", x % 2, (int) (z & 0377));
	return (z);
}


msoutb (x, y)
register int	x;
register int	y;
{
	if (debug_flag & DB_BYTES)
/*		printf ("o%o%o:", x % 2, (int) (y & 0377));*/
		printf("x= %o y=%o\n",x,y);
	return (outb (x, y));
}


prtstr ()
{
	/* printf ("q%d%d%d%d%d:%d:%lo:%o:%o:%d:%d:%o:%d:%d;", 
	    (int) iqt->action[0], (int) iqt->action[1], (int) iqt->action[2], 
	    (int) iqt->action[3], (int) iqt->action[4], (int) (iqt->paction - iqt->action), 
	    iqt->flags, (int) iqt->mask, iqt->sleep ? 1 : 0, (int) iqt->sfarg, 
	    (int) iqt->error, (int) iqt->vio_addr, 
	    (int) iqt->io_count, (int) iqt->uexit); */
	printf ("q%lo", iqt->flags);
}


