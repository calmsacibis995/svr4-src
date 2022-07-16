/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_MKDEV_H
#define _SYS_MKDEV_H

#ident	"@(#)head.sys:sys/mkdev.h	1.6.2.1"

/* SVR3 device number constants */

#define ONBITSMAJOR	7	/* # of SVR3 major device bits */
#define ONBITSMINOR	8	/* # of SVR3 minor device bits */
#define OMAXMAJ		0x7f	/* SVR3 max major value */
#define OMAXMIN		0xff	/* SVR3 max major value */


#define NBITSMAJOR	14	/* # of SVR4 major device bits */
#define NBITSMINOR	18	/* # of SVR4 minor device bits */
#define MAXMAJ		0xff	/* Although 14 bits are reserved, 
				** the 3b2 major number is restricted
				** to 8 bits. 
				*/

#define MAXMIN		0x3ffff	/* MAX minor for 3b2 software drivers.
				** For 3b2 hardware devices the minor is
				** restricted to 256 (0-255)
				*/

#if !defined(_KERNEL)

/* undefine sysmacros.h device macros */

#undef makedev
#undef major
#undef minor

#if defined(__STDC__)

dev_t makedev(const major_t, const minor_t);
major_t major(const dev_t);
minor_t minor(const dev_t);
dev_t __makedev(const int, const major_t, const minor_t);
major_t __major(const int, const dev_t);
minor_t __minor(const int, const dev_t);

#else

dev_t makedev();
major_t major();
minor_t minor();
dev_t __makedev();
major_t __major();
minor_t __minor();

#endif	/* defined(_STDC_) */

#define OLDDEV 0	/* old device format */
#define NEWDEV 1	/* new device format */


static dev_t
makedev(maj, min)
major_t maj;
minor_t min;
{
int ver;
#if !defined(_STYPES)
	ver = NEWDEV;
#else
	ver = OLDDEV;
#endif

	return(__makedev(ver, maj, min));
}

static major_t 
major(dev)
dev_t dev;
{
int ver;
#if !defined(_STYPES)
	ver = NEWDEV;
#else
	ver = OLDDEV;
#endif

	return(__major(ver, dev));
}

static minor_t 
minor(dev)
dev_t dev;
{
int ver;
#if !defined(_STYPES)
	ver = NEWDEV;
#else
	ver = OLDDEV;
#endif

	return(__minor(ver, dev));
}

#endif	/* !defined(_KERNEL) */

#endif	/* _SYS_MKDEV_H */
