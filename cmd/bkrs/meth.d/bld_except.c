/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:meth.d/bld_except.c	1.11.3.1"

#include	<limits.h> 	/* get PATH_MAX from here, not stdio */
#include	<sys/types.h>
#include	<stdio.h>
#include	<method.h>
#include	<backup.h>
#include	<signal.h>
#include	<fcntl.h>
#include	<table.h>
#include	<bkexcept.h>
#include	<string.h>
#include	<errno.h>

extern char		*br_get_except_path();
extern int		brlog();
extern void		*malloc();
extern int		TLclose();
extern int		TLfreeentry();
extern ENTRY		TLgetentry();
extern unsigned char	*TLgetfield();
extern int		TLopen();
extern int		TLread();

extern int	bklevels;

int
bld_except (mp)
m_info_t	*mp;
{
	char	ex_path[256];
	unsigned char	*c;
	register char	**pt;
	int	len = 0;
	int	strcount = 0;
	int	size;
	int	ret;
	int	tid;
	int	i;
	char	*str;
	ENTRY	ent;
	TLdesc_t	ex_desc;

	if (mp->flags & eflag) { /* passed in exception list file */
		(void) strcpy(ex_path, (char *)mp->ex_tab);
		mp->ex_tab = (char **)NULL;
	}
	else {
		(void) strcpy(ex_path, br_get_except_path());
	}
#ifdef TRACE
	brlog(" bld_except: path=%s ", ex_path); 
#endif
	(void) strncpy((char *) &ex_desc, "", sizeof( TLdesc_t ));

        ex_desc.td_format = EX_EXCEPT_F;

	BEGIN_CRITICAL_REGION;

	ret = TLopen(&tid, ex_path, &ex_desc, O_RDONLY);

	END_CRITICAL_REGION;

	if (ret != TLOK) {
		brlog(" bld_except open of %s failed TLopen ret=%d ", ex_path, ret);
		TLclose(tid);
		return (1);
	}
	ent = TLgetentry(tid);

	if (ent == 0) {
		brlog(" bld_except TLgetentry of %s failed ", ex_path);
		TLclose(tid);
		return (1);
	}
/* pass 1: count patterns and their lengths */

	BEGIN_CRITICAL_REGION;
	
	for(i = 1;; i++) {
		ret = TLread(tid, i, ent);

		if (ret != TLOK) {
			break;
		}
		c = TLgetfield(tid, ent, EX_EXCPATH);

		if (c == NULL)
			continue;

		strcount++;
		len += strlen((char *) c);
		len += sizeof(char);		/* room for terminator */
	}
	END_CRITICAL_REGION;

	if (!strcount) {
		TLfreeentry(tid, ent);
		TLclose(tid);
		brlog(" bld_except no entries found ");
		return (0);
	}
	size = (sizeof(char *) * (strcount + 1)) + len;	/* leave a null ptr */
	pt = (char **) malloc((unsigned )size);

	if ((char *)pt == NULL) {
		brlog(" bld_except malloc failed %s ", SE);
		TLfreeentry(tid, ent);
		TLclose(tid);
		return(1);
	}
	str = (char *)((char **)pt + (strcount + 1));	/* leave a NULL ptr */
	mp->ex_tab = pt;
	mp->ex_count = strcount;

	BEGIN_CRITICAL_REGION;

	for (i = 1, len = 0; len <= strcount ; i++) {
		ret = TLread(tid, i, ent);

		if (ret != TLOK)
			break;

		c = TLgetfield(tid, ent, EX_EXCPATH);

		if (c == NULL)
			continue;
		len++;
		*pt++ = str;
		(void) strcpy(str, (char *) c);
		size = strlen((char *) c);

		if (size) {
			if (*(str+size-1) == '\n')
				*(str+size-1) = 0;
#ifdef TRACE
			brlog(" bld_except: exception entry=%s ", str); 
#endif
		}
		str += (size + sizeof(char));
	}
	END_CRITICAL_REGION;

	*pt = NULL;

	TLfreeentry(tid, ent);

	TLclose(tid);

	return(0);
} /* bld_except() */
