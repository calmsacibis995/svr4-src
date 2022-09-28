/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)vi:port/bcopy.c	1.10"
bcopy(from, to, count)
#ifdef vax
	unsigned char *from, *to;
	int count;
{

	asm("	movc3	12(ap),*4(ap),*8(ap)");
}
#else
#ifdef u3b		/* movblkb only works with register args */
	register unsigned char *from, *to;
	register int count;
{
	asm("	movblkb	%r6, %r8, %r7");
}
#else
	register unsigned char *from, *to;
	register int count;
{
	while ((count--) > 0)
		*to++ = *from++;
}
#endif
#endif
