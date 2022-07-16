/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)head:crypt.h	1.1"

/* Password and file encryption functions */

#if defined(__STDC__)

extern char *crypt(const char *, const char *);
extern int crypt_close(int *);
extern char *des_crypt(const char *, const char *);
extern void des_encrypt(char *, int);
extern void des_setkey(const char *);
extern void encrypt(char *, int);
extern int run_crypt(long, char *, unsigned, int *);
extern int run_setkey(int *, const char *);
extern void setkey(const char *);

#endif 
