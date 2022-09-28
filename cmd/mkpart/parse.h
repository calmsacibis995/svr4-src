/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)mkpart:parse.h	1.1"

/*
 *	Partition File Tokens
 */
#define NULLTOKEN	0
	/* basic tokens */
#define NAME		10	/* identifier: [a-zA-Z_][a-zA-Z_0-9]* */
#define NUMBER		11	/* whole number */
#define STRING		12	/* double quoted string */

	/* punctuation */
#define BLANK_LINE	20	/* \n[ \t]*\n */
#define COLON		21
#define COMMA		22
#define EQ		23	/* = */
#define LPAREN		24
#define RPAREN		25
#define DASH		26

	/* keywords */

		/* device keywords */
#define BOOT		40	/* specifies a.out filename of boot code */
#define HEADS		41	/* number of heads on physical device */
#define CYLS		42	/* number of cylinders on device */
#define SECTORS	43	/* number of sectors per track */
#define BPSEC		44	/* number of bytes per sector */
#define DSERIAL	45	/* device serial number */
#define VTOCSEC	46	/* sector number of vtoc */
#define ALTSEC		47	/* sector number of alternate block table */
#define DEVICE		48	/* unix raw device filename for entire device */
#define BADSEC		49	/* bad sector list */
#define BADTRK		50	/* bad track list */
#define USEDEVICE	51
#define		ISDEVICE(x) ((x)>=BOOT && (x)<TAG)

		/* partition keywords */
#define TAG		60	/* partition tag value */
#define TAGNAME	61
#define PERM		62	/* partition permission value */
#define PERMNAME	63
#define START		64	/* first sector in partition */
#define SIZE		65	/* number of sectors in partition */
#define PARTDEVICE	66	/* unix raw device filename for this partition */
#define USEPART	67
#define		ISPART(x) ((x)>=TAG && (x)<RANGE)


		/* nonterminal nodes */
#define RANGE		80	/* a range of numbers for bad block list */
#define LIST		81	/* a list of nodes */
