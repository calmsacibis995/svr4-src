/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sco:mapstr.c	1.1"
#include <sys/types.h>
#include <sys/at_ansi.h>
#include <sys/kd.h>
#include <stdio.h>
#include <errno.h>

struct str_dflt buf;
struct fkeyarg fkey;
extern int optind;

main(Narg,ppCarg)
int Narg;
char **ppCarg;
{
	int i;
	FILE *pF;
	char *dflt;
	stridx_t stridx;
	int kern,global;
	int arg;

#define DFLT "/usr/lib/keyboard/strings"

	if (ioctl(0,KIOCINFO,0) < 0) {
		fprintf(stderr,"mapstr can only be run from a virtual terminal\n");
		fprintf(stderr,"on a graphics workstation.\n");
		fprintf(stderr,"usage: %s [-dg] [file]\n",ppCarg[0]);
		exit(2);
	}
	kern = 0;
	global = 0;
	while( EOF != (arg = getopt( Narg, ppCarg, "dg")) ){
		switch(arg){
		case 'd':
			kern = 1;
			break;
		case 'g':
			global = 1;
			break;
		default:
			fprintf(stderr,"unknown option -%c", arg);
			fprintf(stderr,"usage: %s [-dg] [file]\n",ppCarg[0]);
			return 1;
			break;
		}
	}
	dflt = DFLT;
	if( !kern ){
		if (optind < Narg) {
			pF = fopen(ppCarg[optind],"r");
			if (!pF) {
			   fprintf(stderr,"can't open %s for reading\n",ppCarg[optind]);
			   exit(1);
			}
		} else
			if( 0 == (pF = fopen( "/usr/lib/keyboard/strings","r"))){
			   fprintf(stderr,"can't open %s\n", dflt );
			   exit(1);
			}
		if (global) {
			buf.str_direction = KD_DFLTGET;
			if (ioctl(0,KDDFLTSTRMAP,&buf) < 0) {
			   perror("mapstr: unable to perform GIO_STRMAP ioctl\n");
			   exit(1);
			}
		} else
			if (ioctl (0,GIO_STRMAP,&buf.str_map) < 0) {
		 	   perror("mapstr: unable to perform GIO_STRMAP ioctl\n");
			   exit(1);
			}
		strindexreset(&buf.str_map,&stridx[0]);
		for( i = 1 ; i <= K_FUNL - K_FUNF ; i++ ){
			fkey.keynum = i;
			fscanstr(pF,fkey.keydef,&fkey.flen);
			if (!addstring(&buf.str_map,&stridx[0],i,fkey.keydef,fkey.flen)) {
				fprintf(stderr,"mapstr: Not enough space for function key definitions\n");
				exit(1);
			}
		}
		if (global) {
			buf.str_direction = KD_DFLTSET;
			if( ioctl(0,KDDFLTSTRMAP,&buf) < 0 ) {
				perror("KDDFLTSTRMAP ioctl failure");
				exit(1);
			}
		} else
			if (ioctl(0,PIO_STRMAP,&buf.str_map) < 0) {
				perror("PIO_KEYMAP ioctl failure");
				exit(1);
			}
	 } else {
		buf.str_direction = KD_DFLTGET;
		if (global) {
			if (ioctl(0,KDDFLTSTRMAP,&buf) < 0) {
				perror("KDDFLTSTRMAP ioctl failure");
				exit(1);
			}
		} else
			if (ioctl(0,GIO_STRMAP,&buf.str_map) < 0) {
				perror("KDDFLTSTRMAP ioctl failure");
				exit(1);
			}
		strindexreset(&buf.str_map,&stridx[0]);
		fprintf(stdout, "String key values\n");
		for( i = 1 ; i <= K_FUNL - K_FUNF ; i++ ){
			int len;

			if (i == K_FUNL - K_FUNF)
				len = STRTABLN - 1 - stridx[i-1] -1;
			else
				len = stridx[i] - stridx[i-1] -1;
			if (len > 0) {
			   if (fprintstr(stdout,&buf.str_map[stridx[i-1]],len))
			   	fprintfnt(stdout, i);
			}
		}
	}
	exit(0);
}

fscanstr(pF,pCstr, pNchar )
FILE *pF;
char *pCstr;
int *pNchar;
{
	int c;
	
	*pNchar = 0;
	do {
		while( (c = getc(pF)) != '\n' ){
			if(feof(pF))
				return;
		}
	} while( getc(pF) != '"' );
	while( (c = getc(pF)) != '"' ){
		if( c == '\\' ){
			c = getc(pF);
			if( '0' <= c && '9' >= c ){
				c -= '0';
				c = 8 * c + getc(pF) - '0';
				c = 8 * c + getc(pF) - '0';
			}
		}
		*(pCstr++) = c;
		(*pNchar)++;
	}
}

fprintstr(pF, pCstr, Nchar)
FILE *pF;
char *pCstr;
int Nchar;
{
	if (*pCstr == '\0')
		return (0);
	putc('"',pF);
	while(Nchar--){
		if(*pCstr < ' '){
			fprintf(pF,"\\%03o", *pCstr );
		} else {
			putc(*pCstr,pF);
		}
		pCstr++;
	}
	putc('"',pF);
	return (1);
}



char *Lables[] = { "Function #","Shift Function #","Control Function #",
		 "Ctrl/Shft Function #","Home","Up arrow","Page up","-",
		 "Left arrow","5","Right arrow","+","End","Down arrow", 
		 "Page down","Insert","(null)"};

fprintfnt(pF, Nchar)
FILE *pF;
int Nchar;
{
	if ( Nchar < 13 )
		fprintf(stdout,"\t%s%d\n",Lables[0], Nchar );
	else if ( Nchar < 25 )
		fprintf(stdout,"\t%s%d\n",Lables[1], Nchar-12 );
	else if ( Nchar < 37 )
		fprintf(stdout,"\t%s%d\n",Lables[2], Nchar-24 );
	else if ( Nchar < 49 )
		fprintf(stdout,"\t%s%d\n",Lables[3], Nchar-36 );
	else if ( Nchar < 61 )
		fprintf(stdout,"\t%s\n",Lables[Nchar-45] );
	else if ( Nchar < 97 )
		fprintf(stdout,"\t%s\n",Lables[16] );
}

int
strindexreset(strmap,stridx)
unchar *strmap;
ushort *stridx;
{
	register int strix;		/* Index into string buffer */
	register ushort	key;		/* Function key number */
	register ushort *idxp;
	register unchar *bufp;

	bufp = strmap;
	idxp = stridx;

	bufp[STRTABLN - 1] = '\0';	/* Make sure buffer ends with null */
	strix = 0;			/* Start from beginning of buffer */

	for (key = 0; (int) key < NSTRKEYS; key++) {	
		idxp[key] = strix;	/* Point to start of string */
		while (strix < STRTABLN - 1 && bufp[strix++])
			;		/* Find start of next string */
	}
}


int
addstring(strmap, stridx, keynum, str, len)
unchar *strmap;
ushort *stridx;
ushort	keynum, len;
unchar	*str;
{
	register int	amount;		/* Amount to move */
	int		i,		/* Counter */
			cnt,
			oldlen;		/* Length of old string */
	register unchar	*oldstr,	/* Location of old string in table */
			*to,		/* Destination of move */
			*from;		/* Source of move */
	unchar		*bufend,	/* End of string buffer */
			*bufbase,	/* Beginning of buffer */
			*tmp;		/* Temporary pointer into old string */
	ushort		*idxp;
	

	if ( (int)keynum >= NSTRKEYS)		/* Invalid key number? */
		return 0;		/* Ignore string setting */
	len++;				/* Adjust length to count end null */

	idxp = (ushort *) stridx;
	idxp += keynum - 1;

	oldstr = (unchar *) strmap;
	oldstr += *idxp;

	/* Now oldstr points at beginning of old string for key */

	bufbase = (unchar *) strmap;
	bufend = bufbase + STRTABLN - 1;

	tmp = oldstr;
	while (*tmp++ != '\0')		/* Find end of old string */
		;

	oldlen =  tmp - oldstr;		/* Compute length of string + null */

	/*
	 * If lengths are different, expand or contract table to fit
	 */
	if (oldlen > (int) len) {		/* Move up for shorter string? */
		from = oldstr + oldlen;	/* Calculate source */
		to = oldstr + len;	/* Calculate destination */
		amount = STRTABLN - (oldstr - bufbase) - oldlen;
		for (cnt = amount; cnt > 0; cnt--)
			*to++ = *from++;
	}
	else if (oldlen < (int) len) {	/* Move down for longer string? */
		from = bufend - (len - oldlen);	/* Calculate source */
		to = bufend;		/* Calculate destination */
		if (from < (oldstr + len))	/* String won't fit? */
			return 0;	/* Return without doing anything */
		amount	= STRTABLN - (oldstr - bufbase) - len;	/* Move length */
		while (--amount >= 0)		/* Copy whole length */
			*to-- = *from--;	/* Copy character at a time */
	}

	len--;				/* Remove previous addition for null */
	while (len--) 
		*oldstr++ = *str++;

	strindexreset(strmap,stridx);		/* Readjust string index table */
	return 1;
}
