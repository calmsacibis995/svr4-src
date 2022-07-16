/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/localeconv.c	1.7"
#include "synonyms.h"
#include "shlib.h"
#include <locale.h>
#include "_locale.h"
#include <stdio.h>
#include <limits.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

static struct lconv	lformat = {
	".",	/* decimal_point */
	"",	/* thousands grouping */
	"",	/* grouping */
	"",	/* int_curr_symbol */
	"",	/* currency symbol */
	"",	/* mon_decimal_point */
	"",	/* mon_thousands_sep */
	"",	/* mon_grouping */
	"",	/* positive sign */
	"",	/* negative sign */
	CHAR_MAX,	/* int_frac_digits */
	CHAR_MAX,	/* frac_digits */
	CHAR_MAX,	/* p_cs_precedes */
	CHAR_MAX,	/* p_sep_by_space */
	CHAR_MAX,	/* n_scs_precedes */
	CHAR_MAX,	/* n_sep_by_space */
	CHAR_MAX,	/* p_sign_posn */
	CHAR_MAX,	/* n_sign_posn */
};

static char	sv_lc_numeric[LC_NAMELEN] = "C";
static char	sv_lc_monetary[LC_NAMELEN] = "C";


struct lconv *
localeconv()
{
	int	fd;
	VOID	*str;
	struct stat	buf;
	static char	*ostr = NULL; /* pointer to last set locale for LC_MONETARY */
	static char	*onstr = NULL; /* pointer to last set locale for LC_NUMERIC */
	struct lconv	*monetary;

	if (strcmp(_cur_locale[LC_NUMERIC], sv_lc_numeric) != 0) {
		lformat.decimal_point[0] = _numeric[0];
		lformat.thousands_sep[0] = _numeric[1];
		if ((fd = open(_fullocale(_cur_locale[LC_NUMERIC],"LC_NUMERIC"), O_RDONLY)) == -1)
			goto err4;
		if ((fstat(fd, &buf)) != 0 || (str = malloc(buf.st_size)) == NULL)
			goto err5;
		if (buf.st_size > 2) {
			if ((read(fd, str, buf.st_size)) != buf.st_size)
				goto err6;

			/* if a previous locale was set for LC_NUMERIC, free it */
			if (onstr != NULL)
				free(onstr);
			onstr = str;

			lformat.grouping = (char *)str + 2;
		} else
			lformat.grouping = "";
		close(fd);
		strcpy(sv_lc_numeric, _cur_locale[LC_NUMERIC]);
	}

	if (strcmp(_cur_locale[LC_MONETARY], sv_lc_monetary) == 0)
		return(&lformat);

	if ((fd = open(_fullocale(_cur_locale[LC_MONETARY],"LC_MONETARY"), O_RDONLY)) == -1)
		goto err1;
	if ((fstat(fd, &buf)) != 0 || (str = malloc(buf.st_size + 2)) == NULL)
		goto err2;
	if ((read(fd, str, buf.st_size)) != buf.st_size)
		goto err3;
	close(fd);

	/* if a previous locale was set for LC_MONETARY, free it */
	if (ostr != NULL)
		free(ostr);
	ostr = str;

	monetary = (struct lconv *)str;
	str = (char *)str + sizeof(struct lconv);
	lformat.int_curr_symbol = (char *)str + (int)monetary->int_curr_symbol;
	lformat.currency_symbol = (char *)str + (int)monetary->currency_symbol;
	lformat.mon_decimal_point = (char *)str + (int)monetary->mon_decimal_point;
	lformat.mon_thousands_sep = (char *)str + (int)monetary->mon_thousands_sep;
	lformat.mon_grouping = (char *)str + (int)monetary->mon_grouping;
	lformat.positive_sign = (char *)str + (int)monetary->positive_sign;
	lformat.negative_sign = (char *)str + (int)monetary->negative_sign;
	lformat.int_frac_digits = monetary->int_frac_digits;
	lformat.frac_digits = monetary->frac_digits;
	lformat.p_cs_precedes = monetary->p_cs_precedes;
	lformat.p_sep_by_space = monetary->p_sep_by_space;
	lformat.n_cs_precedes = monetary->n_cs_precedes;
	lformat.n_sep_by_space = monetary->n_sep_by_space;
	lformat.p_sign_posn = monetary->p_sign_posn;
	lformat.n_sign_posn = monetary->n_sign_posn;

	strcpy(sv_lc_monetary, _cur_locale[LC_MONETARY]);
	return(&lformat);

err3:	free(str);
err2:	close(fd);
err1:	strcpy(_cur_locale[LC_MONETARY], sv_lc_monetary);
	return(&lformat);

err6:	free(str);
err5:	close(fd);
err4:	strcpy(_cur_locale[LC_NUMERIC], sv_lc_numeric);
	return(&lformat);
}
