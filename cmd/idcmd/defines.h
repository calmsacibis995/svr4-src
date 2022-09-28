/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */

#ident	"@(#)idcmd:defines.h	1.3.1.2"

/* macro to check if a given char is in a given string */
#define	INSTRING( str, c )	( strchr( str, c ) == NULL ? 0 : 1 )

/* Check if two line segments, (a,b) and (x,y),  overlap. */
#define	OVERLAP(a,b,x,y)	(!(b < x || a > y))
#define MAX(a, b)		(a > b ? a : b)
#define	MIN(a, b)		(a < b ? a : b)

#define CIVN	0		/* clock IVN */
#define CTYPE	1		/* clock type field */
#define	SIVN	0		/* start of IVN range */
#define EIVN	32		/* end of IVN range */
#define	SIOA	0x0L		/* start of I/O address range */
#define EIOA	0xFFFFL		/* end of I/O address range */
#define	SCMA	0x10000L	/* start of controller memory address range */
#define DMASIZ	15		/* highest dma channel number permitted */

#define	TSIZE	256		/* max System table */
#define	DSIZE	256		/* max Master table */
#define	BLSIZE	256		/* max block device table */
#define	CSIZE	256		/* max character device table */
#define FSSIZE	20		/* max fs type table */
#define BHIGH	255		/* max block device major number */
#define CHIGH	255		/* max character device major number */
#define BLOW	0		/* min block device major number */
#define CLOW	0		/* min character device major number */
#define	LMINOR	0		/* min minor number */
#define HMINOR	255		/* max minor number */
#define DEVSIZE	16		/* number of device numbers */
#define DEVNOINT 1		/* start of device numbers without interrupts */
#define MAXOBJNM	128	/* from vm/bootconf.h for swap pathname */

#define MASK	2		/* identifies second field of mdevice */
#define TYPE	3		/* identifies third field of mdevice */
/* if adding a new flag, update MASK_FLAGS or TYPE_FLAGS appropriately */

#define MASK_FLAGS	"ocrwisfexphzILEXMS-"
#define TYPE_FLAGS	"inaosrbctemdufHNOGRSDM-"

/* chars that go into type[] */
#define INST	'i'		/* installable				*/
#define NOT	'n'		/* not installable			*/
#define AUTO	'a'		/* automatically installed		*/
#define ONCE	'o'		/* allow only one spec. of device	*/
#define NOCNT	's'		/* suppress device count field		*/
#define REQ	'r'		/* required device			*/
#define	BLOCK	'b'		/* block type device			*/
#define	CHAR	'c'		/* character type device		*/
#define	TTYS	't'		/* device is a tty			*/
#define EXECSW	'e'		/* software exec module			*/
#define	MOD	'm'		/* STREAMS module type			*/
#define HW	'H'		/* driver interfaces to hardware	*/
#define NODRIVE	'N'		/* no driver (no Driver.o and Space.c)	*/
#define IOASPY	'O'		/* IOA regions can overlap		*/
#define	GROUP	'G'		/* interrupt indicates device group	*/
#define DRESET	'R'		/* reset routine			*/
#define STREAM  'S'		/* STREAMS installable			*/
#define	DMASHR	'D'		/* can shar DMA channel			*/
#define	DISP	'd'		/* dispatcher class			*/
#define UNIQ	'u'		/* same blk and char majors are assigned*/

#define	NEWDRV	'f'		/* 4.0 based new driver			*/
#define MULTMAJ 'M'		/* for drivers requiring multiple majors*/

/* chars that go into mask[] */
#define	OPEN	'o'		/* open routine exists			*/
#define	CLOSE	'c'		/* close routine exists			*/
#define	READ	'r'		/* read routine exists			*/
#define	WRITE	'w'		/* write routine exists			*/
#define	IOCTL	'i'		/* ioctl routine exists			*/
#define START	's'		/* start table func exists */
#define INIT	'I'		/* initialization routine exists	*/
#define	FORK	'f'		/* fork routine exists			*/
#define	EXEC	'e'		/* exec routine exists			*/
#define	EXIT	'x'		/* exit routine exists			*/
#define	POLL	'p'		/* poll (io_poll) routine exists (XENIX)*/
#define	POLLSYS	'L'		/* chpoll routine exists (poll syscall)	*/
#define	HALT	'h'		/* halt (io_halt) routine exists (XENIX)*/
#define KENTER	'E'		/* kenter routine exists		*/
#define KEXIT	'X'		/* kexit routine exists			*/

#define MMAP	'M'		/* character mmap func exists		*/
#define SEGMAP	'S'		/* character segmap func exists		*/
#define SIZE	'z'		/* size func exists			*/

/* location of file */
#define	IN	0		/* file in input directory		*/
#define OUT	1		/* file in output directory		*/
#define FULL	2		/* file has full pathname		*/

/* error messages */
#define USAGE	"Usage: idconfig [-i|o|r dir] [-D|T|d|t|a|c|h|v|p file]\n"
#define OIOA	"Start I/O Address, %lx, greater than End I/O Address, %lx\n"
#define OCMA	"Start Memory Address, %lx, greater than End Memory Address, %lx\n"
#define RIOA	"I/O Address range, %lx and %lx, must be within (%lx, %lx)\n"
#define RCMA	" Controller Memory Address, %lx must be greater than (%lx)\n"
#define RIVN	"Interrupt vector number, %hd, must be within (%d, %d)\n"
#define CVEC	"Interrupt vector conflict between devices '%s' and '%s'\n"
#define	CIDV	"Each controller for device '%s' must use the same interrupt\n"
#define CIOA	"I/O Address ranges overlap for devices '%s' and '%s'\n"
#define CCMA	"Memory Address ranges overlap for devices '%s' and '%s'\n"
#define CID	"Id '%c' shared by devices '%s' and '%s'\n"
#define IBDM	"Block device major number, %hd, must be within (%d, %d)\n"
#define DBDM	"Identical block device major number, %hd, for '%s' and '%s'\n"
#define ICDM	"Character device major number, %hd, must be within (%d, %d)\n"
#define DCDM	"Identical character device major number, %hd, for '%s' and '%s'\n"
#define UNIT	"Unit, %hd, must be within (%hd, %hd)\n"
#define MISS	"Missing required device '%s'\n"
#define ONESPEC	"Only one specification of device '%s' allowed\n"
#define TUNE	"Unknown tunable parameter '%s'\n"
#define RESPEC	"Tunable parameter '%s' respecified\n"
#define PARM	"The value of parameter '%s', %ld, must be within (%ld, %ld)\n"
#define INCOR	"Incorrect line format\n"
#define UNK	"Unknown device '%s'\n"
#define BLK	"'%s' must be a block device\n"
#define MINOR	"Minor device number must be within (%d, %d)\n"
#define ASNSPEC	"'%s' respecified\n"
#define NBLK	"Must have greater than 0 blocks\n"
#define ASNLO	"Lowest disk block permissible is 0\n"
#define	INSTOPT	"Device, '%s', must have an 'i' and no 'n' in Field 3 of Master\n"
#define NOTINST "Tunable, '%s', must have an 'n' and no 'i' in field 3 of Master\n"
#define DIFF	"Field 'type' must match for all '%s' devices\n"
#define ABBR	"Field 'id' must contain a letter\n"
#define ABSH	"Field 'id' can only contain a '-' if 'share' is 'Y'\n"
#define	OPRT	"Block device '%s' must have an 'open' function\n"
#define CLRT	"Block device '%s' must have a 'close' function\n"
#define	EXIST	"Directory '%s' does not exist\n"
#define MPAR	"Missing value for tunable parameter '%s'\n"
#define FOPEN	"Can not open '%s' for mode '%s'\n"
#define MASTAB	"Master table overflow\n"
#define WRONG	"Wrong number of fields\n"
#define KEY	"Keyword table overflow\n"
#define SUCH	"No such device '%s' in mdevice\n"
#define SYSTEM	"System table overflow\n"
#define	CONFD	"Configured field for device '%s' must contain 'Y' or 'N'\n"
#define DCONF	"DMA channel conflict between devices '%s' and '%s'\n"
#define DRANGE	"DMA channel must be within (0, %d)\n"
#define DEVNM	"Too many hardware devices that do not generate interrupts\n"
#define SWPNM	"Swap device pathname is not given in sassign, the default '%s' is used\n"
#define MISSF	"Device, '%s', must have an 'f' in third field of mdevice\n"
#define MMRANGE	"Invalid multiple majors specification (%s)\n"
#define MMSYNTAX	"Invalid syntax for multiple majors range (%s)\n"
#define MMIRANGE	"Invalid range: start number should be less than end number (%s)\n"
#define	MMERROR		"Multiple major range specified for %s device but no 'M' in mdevice type field"
#define MASK_ERROR "Invalid flag '%c' used in second field of mdevice\n"
#define TYPE_ERROR "Invalid flag '%c' used in third field of mdevice\n"
#define NOMAXMINOR "Can not find value for MAXMINOR tunable, using %lu\n"
