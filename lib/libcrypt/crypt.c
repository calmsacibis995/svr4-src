/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libcrypt:crypt.c	1.4"

#ifdef __STDC__
	#pragma weak setkey = _setkey
	#pragma weak encrypt = _encrypt
	#pragma weak crypt = _crypt
#endif
#include "synonyms.h"

void setkey (key)
const char *key;
{
	extern void	des_setkey();
	des_setkey(key);
}

void encrypt(block, edflag)
char *block;
int edflag;
{
	extern void	des_encrypt();
	des_encrypt(block, edflag);
}

char *
crypt(pw, salt)
const char *pw;
const char *salt;
{
	extern char	*des_crypt();
	
	return(des_crypt(pw, salt));
}
