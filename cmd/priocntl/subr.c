/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)priocntl:subr.c	1.5.3.1"
#include	<stdio.h>
#include	<string.h>
#include	<unistd.h>
#include	<sys/types.h>
#include	<sys/procset.h>
#include	<sys/priocntl.h>

#include "priocntl.h"

/*
 * Utility functions for priocntl command.
 */

/* VARARGS1 */
void
fatalerr(format, a1, a2, a3, a4, a5)
char	*format;
{
	fprintf(stderr, format, a1, a2, a3, a4, a5);
	exit(1);
}


/*
 * Structure defining idtypes known to the priocntl command
 * along with the corresponding names and a liberal guess
 * of the max number of procs sharing any given ID of that type.
 * The idtype values themselves are defined in <sys/procset.h>.
 */
static struct idtypes {
	idtype_t	idtype;
	char		*idtypnm;
	int		maxprocsperid;
} idtypes [] = {
	{ P_PID,	"pid",		1 },
	{ P_PPID,	"ppid",		200 },
	{ P_PGID,	"pgid",		500 },
	{ P_SID,	"sid",		1024 },
	{ P_CID,	"class",	1024 },
	{ P_UID,	"uid",		1024 },
	{ P_GID,	"gid",		1024 },
	{ P_ALL,	"all",		1024 }
};

#define	IDCNT	(sizeof(idtypes)/sizeof(struct idtypes))


int
str2idtyp(idtypnm, idtypep)
char		*idtypnm;
idtype_t	*idtypep;
{
	register struct idtypes	*curp;
	register struct idtypes	*endp;

	for (curp = idtypes, endp = &idtypes[IDCNT]; curp < endp; curp++) {
		if (strcmp(curp->idtypnm, idtypnm) == 0) {
			*idtypep = curp->idtype;
			return(0);
		}
	}
	return(-1);
}


int
idtyp2str(idtype, idtypnm)
idtype_t	idtype;
char		*idtypnm;
{
	register struct idtypes	*curp;
	register struct idtypes	*endp;

	for (curp = idtypes, endp = &idtypes[IDCNT]; curp < endp; curp++) {
		if (idtype == curp->idtype) {
			strcpy(idtypnm, curp->idtypnm);
			return(0);
		}
	}
	return(-1);
}


/*
 * Given an idtype, return a very liberal guess of the max number of
 * processes sharing any given ID of that type.
 */
int
idtyp2maxprocs(idtype)
idtype_t	idtype;
{
	register struct idtypes	*curp;
	register struct idtypes	*endp;

	for (curp = idtypes, endp = &idtypes[IDCNT]; curp < endp; curp++) {
		if (idtype == curp->idtype)
			return(curp->maxprocsperid);
	}
	return(-1);
}

	
/*
 * Compare two IDs for equality.
 */
int
idcompar(id1p, id2p)
id_t	*id1p;
id_t	*id2p;
{
	if (*id1p == *id2p)
		return(0);
	else
		return(-1);
}


id_t
clname2cid(clname)
char	*clname;
{
	pcinfo_t	pcinfo;

	strcpy(pcinfo.pc_clname, clname);
	if (priocntl(0, 0, PC_GETCID, &pcinfo) == -1)
		return((id_t)-1);
	return(pcinfo.pc_cid);
}


int
getmyid(idtype, idptr)
idtype_t	idtype;
id_t		*idptr;
{
	pcparms_t	pcparms;

	switch(idtype) {

	case P_PID:
		*idptr = (id_t)getpid();
		break;

	case P_PPID:
		*idptr = (id_t)getppid();
		break;

	case P_PGID:
		*idptr = (id_t)getpgrp();
		break;

	case P_SID:
		*idptr = (id_t)getsid(getpid());
		break;

	case P_CID:
		pcparms.pc_cid = PC_CLNULL;
		if (priocntl(P_PID, P_MYID, PC_GETPARMS, &pcparms) == -1)
			return(-1);
		*idptr = pcparms.pc_cid;
		break;

	case P_UID:
		*idptr = (id_t)getuid();
		break;

	case P_GID:
		*idptr = (id_t)getgid();
		break;

	default:
		return(-1);
	}
	return(0);
}


int
getmyidstr(idtype, idstr)
idtype_t	idtype;
char		*idstr;
{
	pcparms_t	pcparms;
	pcinfo_t	pcinfo;

	switch(idtype) {

	case P_PID:
		itoa((long)getpid(), idstr);
		break;

	case P_PPID:
		itoa((long)getppid(), idstr);
		break;

	case P_PGID:
		itoa((long)getpgrp(), idstr);
		break;
	case P_SID:
		itoa((long)getsid(getpid()), idstr);
		break;

	case P_CID:
		if (priocntl(P_PID, P_MYID, PC_GETPARMS, &pcparms) == -1)
			return(-1);
		pcinfo.pc_cid = pcparms.pc_cid;
		if (priocntl(0, 0, PC_GETCLINFO, &pcinfo) == -1)
			return(-1);
		strcpy(idstr, pcinfo.pc_clname);
		break;

	case P_UID:
		itoa((long)getuid(), idstr);
		break;

	case P_GID:
		itoa((long)getgid(), idstr);
		break;

	default:
		return(-1);
	}
	return(0);
}


/*
 * itoa() and reverse() taken almost verbatim from K & R Chapter 3.
 */
static void	reverse();

/*
 * itoa(): Convert n to characters in s.
 */
void
itoa(n, s)
long	n;
char	*s;
{
	long	i, sign;

	if ((sign = n) < 0)	/* record sign */
		n = -n;		/* make sign positive */
	i = 0;
	do {	/* generate digits in reverse order */
		s[i++] = n % 10 + '0';	/* get next digit */
	} while ((n /= 10) > 0);	/* delete it */
	if (sign < 0)
		s[i++] = '-';
	s[i] = '\0';
	reverse(s);
}


/*
 * reverse(): Reverse string s in place.
 */
static void
reverse(s)
char	*s;
{
	int	c, i, j;

	for (i = 0, j = strlen(s) - 1; i < j; i++, j--) {
		c = s[i];
		s[i] = s[j];
		s[j] = (char)c;
	}
}
