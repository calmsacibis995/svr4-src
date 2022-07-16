/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/hrtctime.c	1.4"

#include "synonyms.h"
#include <time.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/dl.h>
#include <sys/evecb.h>
#include <sys/hrtcntl.h>


static char cbuf[100];


char *
_hrtctime(tdp)
hrtime_t *tdp;
{
	register struct tm	*tmp;
	char			*_hrtasctime();

	if (tdp->hrt_res <= 0 || tdp->hrt_res > NANOSEC || tdp->hrt_rem < 0)
		return(NULL); 

	if(tdp->hrt_rem >= tdp->hrt_res){
		tdp->hrt_secs	+= tdp->hrt_rem / tdp->hrt_res;
		tdp->hrt_rem     = tdp->hrt_rem % tdp->hrt_res;
	}

	tmp = localtime((long *)&tdp->hrt_secs);
	return(_hrtasctime(tmp, tdp->hrt_rem, tdp->hrt_res));
}


char *
_hrtasctime(tmp, rem, res)
struct tm *tmp;
ulong rem;
ulong res;
{
	char		*p;
	dl_t		llog;
	dl_t		lfrac;
	dl_t		lres;
	dl_t		lrem;
	int		i;
	ulong		result = 1;

	if (res <= 0 || rem < 0 || rem >= res || res > NANOSEC)
		return(NULL);

	lrem.dl_hop = 0;
	lrem.dl_lop = rem;
	lres.dl_hop = 0;
	lres.dl_lop = res;
	llog = llog10(lres);

	for(i = 1; i <= llog.dl_lop; ++i)
		result = result * 10;
	if (result != res)	
		llog.dl_lop++;
	lfrac = ldivide(lmul(lrem, lexp10(llog)), lres);

	sprintf(cbuf, "%.3s %.3s %2d %02d:%02d:%02d",
		&"SunMonTueWedThuFriSat"[3 * tmp->tm_wday],
		&"JanFebMarAprMayJunJulAugSepOctNovDec"
		[3 *tmp->tm_mon],
		tmp->tm_mday, tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

	p = cbuf + strlen(cbuf);
	if (res == 1)
		sprintf(p, " %4d", tmp->tm_year + 1900);
	else
		sprintf(p, ".%0*d %4d", llog.dl_lop, lfrac.dl_lop,
					tmp->tm_year + 1900);
	return(cbuf);
}
