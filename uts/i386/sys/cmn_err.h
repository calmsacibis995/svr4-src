/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_CMN_ERR_H
#define _SYS_CMN_ERR_H

#ident	"@(#)head.sys:sys/cmn_err.h	11.8.4.1"

/* Common error handling severity levels */

#define CE_CONT  0	/* continuation				*/
#define CE_NOTE  1	/* notice				*/
#define CE_WARN	 2	/* warning				*/
#define CE_PANIC 3	/* panic				*/

/*	Codes for where output should go.
*/

#define	PRW_BUF		0x01	/* Output to putbuf.		*/
#define	PRW_CONS	0x02	/* Output to console.		*/

extern short	prt_where;

#define VA_LIST _VOID *
#define VA_START(list, name) list = \
  (_VOID*)((char*)&name+((sizeof(name)+(sizeof(int)-1))&~(sizeof(int)-1)))
#define VA_ARG(list, mode) ((mode *) \
  (list=(_VOID*)((char*)list+sizeof(mode))))[-1]

#if defined(__STDC__)
/*PRINTFLIKE2*/
extern void cmn_err(int, char *, ...);
/*PRINTFLIKE1*/
extern void printf(char *, ...);
/*PRINTFLIKE1*/
extern void panic(char *, ...);
extern void nomemmsg(char *, int, int, int);
#else
extern void cmn_err();
extern void printf();
extern void nomemmsg();
extern void panic();
#endif	/* __STDC__ */

#endif	/* _SYS_CMN_ERR_H */
