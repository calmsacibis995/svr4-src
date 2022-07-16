/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/getdate_data.c	1.1"

#include "synonyms.h"
#include <stddef.h>
#include <nl_types.h>
#include <langinfo.h>

const nl_item _time_item[] = {
	ABMON_1, ABMON_2, ABMON_3, ABMON_4, ABMON_5, ABMON_6,
	ABMON_7, ABMON_8, ABMON_9, ABMON_10, ABMON_11, ABMON_12,

	MON_1, MON_2, MON_3, MON_4, MON_5, MON_6,
	MON_7, MON_8, MON_9, MON_10, MON_11, MON_12,

	ABDAY_1, ABDAY_2, ABDAY_3, ABDAY_4, ABDAY_5, ABDAY_6, ABDAY_7,

	DAY_1, DAY_2, DAY_3, DAY_4, DAY_5, DAY_6, DAY_7,

	T_FMT, D_FMT, D_T_FMT, AM_STR, PM_STR, NULL,
};
