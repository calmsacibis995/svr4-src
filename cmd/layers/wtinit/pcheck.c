/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)32ld:pcheck.c	2.3.3.1"

/*
**	Packet check bytes calculation
*/

#ifdef	vax
/*
**	Vax "crc" instruction look-up table for polynomial = 0120001
*/

unsigned long	crc16t[] =
{
	       0, 0146001, 0154001,  012000, 0170001,  036000,  024000, 0162001
	,0120001,  066000,  074000, 0132001,  050000, 0116001, 0104001,  042000
};

/*ARGSUSED*/
int
crc(s, n)
	unsigned char *	s;
	int		n;
{
	asm("	crc	_crc16t,$0,8(ap),*4(ap)	");
	asm("	cmpw	r0,(r3)			");
	asm("	beqlu	OK			");
	asm("	movw	r0,(r3)			");
	asm("	movl	$1,r0			");
	asm("	ret				");
	asm("OK:movw	r0,(r3)			");
	return 0;
}

#else	/* vax */


/*
**	crc-16:  x**16 + x**15 + x**2 + 1
*/

typedef unsigned char	uchar;
typedef unsigned short	ushort;

#define	lobyte(X)	(X&0xff)
#define	hibyte(X)	((X>>8)&0xff)

ushort	crc16t_32[2][16]	=
{
	0, 0140301, 0140601, 0500, 0141401, 01700, 01200, 0141101,
	0143001, 03300, 003600, 0143501, 02400, 0142701, 0142201, 02100,
	0, 0146001, 0154001, 012000, 0170001, 036000, 024000, 0162001,
	0120001, 066000, 074000, 0132001, 050000, 0116001, 0104001, 042000
};

#ifndef	mc68000
int
crc(buffer, nbytes)
	register uchar *buffer;
	int		nbytes;
{
	register ushort	tcrc = 0;
	register int	temp;
	register int	i;

	if ( (i = nbytes) > 0 )
	do
	{
		temp = tcrc ^ *buffer++;
		tcrc = crc16t_32[0][temp & 017]
			 ^ crc16t_32[1][(temp>>4) & 017]
			 ^ (tcrc>>8);
	}
	while
		( --i > 0 );

	if ( lobyte(tcrc) != *buffer )
		i++;
	*buffer++ = lobyte(tcrc);

	if ( hibyte(tcrc) != *buffer )
		i++;
	*buffer++ = hibyte(tcrc);

	return i;
}
#else	/* mc68000 */
asm("	text			");
asm("	global	crc		");
asm("crc:			");
asm("	link	%fp,&crcF	");
asm("	movm.l	&crcM,crcS(%fp)	");
asm("	mov.l	8(%fp),%a2	");
asm("	mov.l	&0,%d2		");
asm("	mov.w	12(%fp),%d4	");
asm("	ble	crc%140		");
asm("crc%170:			");
asm("	mov.b	(%a2)+,%d3	");
asm("	eor.b	%d2,%d3		");
asm("	mov.l	&15,%d0		");
asm("	and.b	%d3,%d0		");
asm("	add.l	%d0,%d0		");
asm("	mov.l	&crc16t_3,%a1	");
asm("	mov.w	0(%a1,%d0.l),%d0");
asm("	lsr.b	&3,%d3		");
asm("	and.w	&30,%d3		");
asm("	mov.l	&crc16t_3+32,%a0");
asm("	mov.w	0(%a0,%d3.w),%d1");
asm("	eor.w	%d0,%d1		");
asm("	lsr.w	&8,%d2		");
asm("	eor.w	%d1,%d2		");
asm("	sub.w	&1,%d4		");
asm("	bgt	crc%170		");
asm("crc%140:			");
asm("	cmp.b	%d2,(%a2)	");
asm("	beq	crc%180		");
asm("	add.w	&1,%d4		");
asm("crc%180:			");
asm("	mov.b	%d2,(%a2)+	");
asm("	lsr.w	&8,%d2		");
asm("	cmp.b	%d2,(%a2)	");
asm("	beq	crc%190		");
asm("	add.w	&1,%d4		");
asm("crc%190:			");
asm("	mov.b	%d2,(%a2)+	");
asm("	mov.w	%d4,%d0		");
asm("	movm.l	crcS(%fp),&crcM	");
asm("	unlk	%fp		");
asm("	rts			");
asm("	set	crcS,-16	");
asm("	set	crcF,-22	");
asm("	set	crcM,02034	");
#endif	/* mc68000 */
#endif	/* vax */
