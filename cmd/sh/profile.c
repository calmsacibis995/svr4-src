/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)sh:profile.c	1.6.4.1"

char *mktemp();

monitor(lowpc, highpc, buf, bufsiz, cntsiz)
char *lowpc, *highpc;
int *buf, bufsiz;
{
	register o;
	static *sbuf, ssiz;

	if (lowpc == 0) {
		profil(0, 0, 0, 0);
		o = creat(mktemp("profXXXXXX"), 0666);
		write(o, sbuf, ssiz<<1);
		close(o);
		return;
	}
	ssiz = bufsiz;
	buf[0] = lowpc;
	buf[1] = highpc;
	buf[2] = cntsiz;
	sbuf = buf;
	buf += 3*(cntsiz+1);
	bufsiz -= 3*(cntsiz+1);
	if (bufsiz<=0)
		return;
	o = ((highpc - lowpc)>>1) & 077777;
	if(bufsiz < o)
		o = ((long)bufsiz<<15) / o;
	else
		o = 077777;
	profil(buf, bufsiz<<1, lowpc, o<<1);
}
