/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:meth.d/file_req.c	1.9.2.1"

#include	<limits.h> 	/* get PATH_MAX from here, not stdio */
#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<bkrs.h>
#include	<method.h>
#include	<varargs.h>
#include	<errno.h>

#define STR_MAX	5

extern char	*bkstrtok();
extern int	brlog();
extern void	*malloc();
extern char	*result();
extern int	safe_stat();
extern int	strcmp();
extern unsigned	strlen();
extern long	strtol();
extern char	*strrchr();   
static int	write_check();

file_rest_t *
file_req(mp, argv, left_todo)
m_info_t	*mp;
unsigned char	*argv[];
int		*left_todo;
{
	register file_rest_t	*f;
	file_rest_t	*file_base;
	int		i;
	int		size;
	char		*typ;

#ifdef TRACE
	brlog("file_req: n_names = %d", mp->n_names);
#endif
	size = mp->n_names * sizeof(file_rest_t);
	file_base = (file_rest_t *) malloc((unsigned) size);

	if (file_base == NULL) {
		brlog(" file_req malloc failed %s", SE);
		return(NULL);
	}
	for (i = 0, f = file_base; i < mp->n_names ; i++, f++) {
		f->result_msg = NULL;
		f->file_count = f->ino = 0;
		f->rindx = R_NOTFOUND;
		f->jobid = bkstrtok(argv[(mp->fnames + i)], ":");
		f->uid = bkstrtok(NULL, ":");
		f->rest_fd = -1;

		if (f->uid != NULL) {
			(void) sscanf(f->uid, "%ld", &(f->idnum));
		}
		f->date = bkstrtok(NULL, ":");

		if ((f->date != NULL) && strlen(f->date)) {
			f->ldate = strtol(f->date, (char **)NULL, 16);
		}
		typ = bkstrtok(NULL, ":");

		if (typ && (!strcmp(typ, "D")))
			f->type = 1;		/* directory */
		else
			f->type = 0;		/* file */
		f->name = bkstrtok(NULL, ":");
		f->rename = bkstrtok(NULL, ":");
#ifdef TRACE
	brlog("file_req: i=%d, name=%s, rename=%s", i, f->name, f->rename);
#endif
		f->inode = bkstrtok(NULL, ":");

		if (f->inode != NULL) {
			f->ino = strtol( f->inode, (char **)NULL, 10 );
		}
		if (f->name) {
			f->status = 0;
			f->name_len = strlen(f->name);
			(*left_todo)++;
		}
		else {
			f->rindx = R_NAMLEN;
			f->status = F_UNSUCCESS;
			continue;
		}
		if (f->rename) {
			f->rename_len = strlen(f->rename);
		}
		else {
			f->rename_len = 0;
		}
		if (f->uid == NULL) {			/* o/w uid = ? */
			f->status = F_UNSUCCESS;
			f->rindx = R_NOUID;
			(*left_todo)--;
			continue;
		}
		else if (f->idnum) {
			if (write_check(f)) {
				f->status = F_UNSUCCESS;
				f->rindx = R_NOWRITE_PERM;
				(*left_todo)--;
				continue;
			}
		}
	}
	return(file_base);
} /* file_req() */

static int
write_check(f)
register file_rest_t	*f;
{
	char	*wrk = NULL;
	char	*wrksav;
	int	nfail = 0;
	struct stat	st;
	register char	*target;

	target = (f->rename_len) ? f->rename : f->name;

	while ((safe_stat(target, &st) == -1) && *target) {
		nfail++;
		wrksav = wrk;
		wrk = (char *) strrchr(target, '/');

		if (wrksav)
			*wrksav = '/';
		if (wrk)
			*wrk = '\0';
		else {
			*target = '\0';
		}
	}
	if (!(*target)) {
		*target = '/';
		return(1);
	}
	if (wrk)
		*wrk = '/';

	if (!nfail) {		/* target exists */
		if ((!(f->type)) && ((st.st_mode & S_IFMT) == S_IFDIR)) {
			if (!(f->result_msg)) {
				f->result_msg = result(1,
					"target file %s exists, is directory",
							target);
			}
			return(1);
		}
	}
	if (f->idnum == (st.st_uid)) {
		if (!((st.st_mode) & S_IWUSR)) {
			return(1);
		}
	}
	else {
		if (!((st.st_mode) & S_IWOTH)) {
			return(1);
		}
	}
	return(0);
} /* write_check() */

/*VARARGS*/
char *
result(va_alist)
va_dcl
{
	va_list	args;
	char	*fmt;
	char	*str[STR_MAX];
	char	*msg;
	int	nstring;
	int	i;
	int	nchar = 0;

	va_start(args);

	nstring = va_arg(args, int);

	if (nstring > STR_MAX)
		return(NULL);

	fmt = va_arg(args, char *);

	if (nstring) {
		for (i = 0; i < nstring; i++) {
			str[i] = va_arg(args, char *);
			nchar += strlen(str[i]);
		}
	}
	va_end(args);
	nchar += (int) (strlen(fmt) + 1);
	msg = (char *) malloc((unsigned) nchar);

	if (msg == NULL) {
		return(NULL);
	}
	(void) vsprintf(msg, fmt, (va_list) str);

	return(msg);
} /* result() */
