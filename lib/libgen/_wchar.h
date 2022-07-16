/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libgen:_wchar.h	1.1.3.2"

#define SS2     0x8e
#define SS3     0x8f
#define EUCMASK 0x30000000
#define P11     0x30000000          /* Code Set 1 */
#define P01     0x10000000          /* Code Set 2 */
#define P10     0x20000000          /* Code Set 3 */
/* masks for characters < 3 bytes */
#define H_EUCMASK 0x8080
#define H_P11     0x8080          /* Code Set 1 */
#define H_P01     0x0080          /* Code Set 2 */
#define H_P10     0x8000          /* Code Set 3 */
#ifdef __STDC__
#define multibyte       (__ctype[520]>1)
#define eucw1   __ctype[514]
#define eucw2   __ctype[515]
#define eucw3   __ctype[516]
#else
#define multibyte       (_ctype[520]>1)
#define eucw1   _ctype[514]
#define eucw2   _ctype[515]
#define eucw3   _ctype[516]
#endif
