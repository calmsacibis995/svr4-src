/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:meth.d/nxt_media.c	1.9.2.1"

#include	<limits.h> 	/* get PATH_MAX from here, not stdio */
#include	<stdio.h>
#include	<sys/types.h>
#include	<setjmp.h>
#include	<method.h>
#include	<errno.h>

extern int	brinvlbl();
extern int	brlog();
extern void	*malloc();
extern char	*nxt_list();
extern char	*strchr();

static char	*null_label = "\0";

get_media_name(reuse_dmname, m_info, M, MP)
short		*reuse_dmname;
media_list_t	**m_info;
media_info_t	*M;
m_info_t	*MP;
{
	char	*dmname;

	if (*reuse_dmname) {		/* suspended, not io error */
		*reuse_dmname = 0;
		*m_info = M->cur;
	}
	else {					/* get new media */
		if ((dmname = nxt_list(MP)) == NULL) {
			if (((OVRIDE(MP)) && !(AUTOM(MP))) && MP->volpromt) {
				dmname = null_label;
			}
			else {
				return(1);
			}
		}
		*m_info = (media_list_t *)malloc(sizeof(media_info_t));

		if (*m_info == NULL) {
			brlog(" getmname malloc failed %s", SE);
			sprintf(ME(MP), "Job ID %s: out of memory", MP->jobid);
			return(1);
		}
		(*m_info)->label = dmname;
		(*m_info)->next = NULL;
		(*m_info)->type = 0;		/* data ot toc */

		if (M->cur) {			/* last vol was good */
			if (M->first == NULL) 
				M->first = M->cur;
			if (M->last)
				(M->last)->next = M->cur;
			M->last = M->cur;
		}
	}
	if (*dmname && (!(MP->volpromt))) {
		(void) brinvlbl (dmname);
	}
	M->cur = *m_info;
	return(0);
} /* get_media_name() */

char *
nxt_list(MP)
m_info_t	*MP;
{
	char	*dmname;
	char	*wrk;

	dmname = MP->dmnames;

	if (dmname == NULL || !(*dmname)) {
		brlog(" nxt_list: no more media names ");
		sprintf(ME(MP), "Job ID %s: no more media names", MP->jobid);
		return(NULL);
	}
	if (wrk = strchr(MP->dmnames, ',')) {
		*wrk++ = 0;

		if (*wrk)
			MP->dmnames = wrk;
		else { 
			MP->dmnames = NULL;
		}
	}
	else {
		MP->dmnames = NULL;
	}
	return(dmname);
} /* nxt_list() */
