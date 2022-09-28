/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:cmd/lpsched/files.c	1.4.4.1"

#include "lpsched.h"

static char time_buf[50];

/**
 ** chfiles() - CHANGE OWNERSHIP OF FILES, RETURN TOTAL SIZE
 **/

#if	defined(__STDC__)
off_t chfiles ( char * * list, uid_t uid, gid_t gid )	/* funcdef */
#else
off_t chfiles (list, uid, gid)
	char **list;
	uid_t uid;
	gid_t gid;
#endif
{
    ENTRY ("chfiles")

    size_t	total;
    struct stat	stbuf;
    char	*file;
    
    total = 0;

    while(file = *list++)
    {
	if (STRNEQU(Lp_Temp, file, strlen(Lp_Temp)) ||
	    STRNEQU(Lp_Tmp, file, strlen(Lp_Tmp)))
	{
	    /*
	     * Once this routine (chfiles) is called for a request,
	     * any temporary files are ``ours'', i.e. they are on our
	     * machine. A user running on an RFS-connected remote machine
	     * can't necessarily know our machine name, so can't put
	     * the files where they belong (Lp_Tmp/machine). But now we
	     * can. Of course, this is all done with mirrors, as Lp_Temp
	     * and Lp_Tmp/local-machine are symbolicly linked. So we just
	     * change the name. This saves on wear and tear later.
	     */
	    if (STRNEQU(Lp_Temp, file, strlen(Lp_Temp)))
	    {
		char *newfile = makepath(Lp_Tmp, Local_System,
				file + strlen(Lp_Temp) + 1, NULL);

		Free(file);
		list[-1] = file = newfile;
	    }
	    
	    (void) Chmod(file, 0600);
	    (void) Chown(file, uid, gid);
	}

	if (Stat(file, &stbuf) == -1)
	    return(-1);

	switch (stbuf.st_mode & S_IFMT) {
	case 0:
	case S_IFREG:
	    break;

	case S_IFIFO:
	    if (!isadmin(uid))
		return(-1);
	    /*
	     * If any of the files is a FIFO, the size indicator
	     * becomes meaningless. On the other hand, returning
	     * a total of zero causes the request to be rejected,
	     * so we return something > 0.
	     */
	    stbuf.st_size = 1;
	    break;

	case S_IFDIR:
	case S_IFCHR:
	case S_IFBLK:
	default:
	    return(-1);
	}

	total += stbuf.st_size;
    }
    return(total);
}

/**
 ** rmfiles() - DELETE/LOG FILES FOR DEFUNCT REQUEST
 **/

#if	defined(__STDC__)
void rmfiles ( RSTATUS * rp, int log_it )	/* funcdef */
#else
void rmfiles(rp, log_it)
    RSTATUS	*rp;
    int		log_it;
#endif
{
    ENTRY ("rmfiles")

    char	**file	= rp->request->file_list;
    char	*path;
    char	*p;
    char	num[STRSIZE(MOST_FILES) + 1];
    static FILE	*logfp	= 0;
    int		reqfd;
    int		count	= 0;


    if (rp->req_file) {

	   /*
	    * The secure request file is removed first to prevent
	    * reloading should the system crash while in rmfiles().
	    */
	    path = makepath(Lp_Requests, rp->req_file, (char *)0);
	    (void) Unlink(path);
	    Free(path);

	    /*
	     * Copy the request file to the log file, if asked,
	     * or simply remove it.
	     */
#if	defined(TEMP_OR_TMP)
	    path = makepath(Lp_Temp, rp->req_file, (char *)0);
#else
	    path = makepath(Lp_Tmp, rp->req_file, (char *)0);
#endif
	    if (log_it && rp->secure && rp->secure->req_id) {
		if (!logfp)
		    logfp = open_lpfile(Lp_ReqLog, "a", MODE_NOREAD);
		if (logfp && (reqfd = Open(path, O_RDONLY, 0)) != -1) {
		    register int	n;
		    char		buf[BUFSIZ];

		    cftime(time_buf, NULL, &(rp->secure->date));
		    fprintf (
			logfp,
			"= %s, uid %d, gid %d, size %ld, %s\n",
			rp->secure->req_id,
			rp->secure->uid,
			rp->secure->gid,
			rp->secure->size,
			time_buf
		    );
		    if (rp->slow)
			fprintf (logfp, "x %s\n", rp->slow);
		    if (rp->fast)
			fprintf (logfp, "y %s\n", rp->fast);
		    if (rp->printer && rp->printer->printer->name)
			fprintf (logfp, "z %s\n", rp->printer->printer->name);
		    while ((n = Read(reqfd, buf, BUFSIZ)) > 0)
			fwrite (buf, 1, n, logfp);
		    Close (reqfd);
		    fflush (logfp);
		}
	    }
	    (void)Unlink (path);
	    Free (path);
    }

    if (file)
	while(*file)
	{
		/*
		 * The copies of user files.
		 */
		if (STRNEQU(Lp_Temp, *file, strlen(Lp_Temp)) ||
		    STRNEQU(Lp_Tmp, *file, strlen(Lp_Tmp)))
		    (void) Unlink(*file);
		count++;
		file++;
	}

    if (rp->secure && rp->secure->req_id) {
	p = getreqno(rp->secure->req_id);

	/*
	 * The filtered files. We can't rely on just the RS_FILTERED
	 * flag, since the request may have been cancelled while
	 * filtering. On the other hand, just checking "rp->slow"
	 * doesn't mean that the files exist, because the request
	 * may have been canceled BEFORE filtering started. Oh well.
	 */
	if (rp->slow)
	    while(count > 0)
	    {
		sprintf(num, "%d", count--);
		path = makestr(Lp_Temp, "/F", p, "-", num, (char *)0);
		Unlink(path);
		Free(path);
	    }

	/*
	 * The notify/error file.
	 */
	path = makepath(Lp_Temp, p, (char *)0);
	(void) Unlink(path);
	Free(path);
    }
}

/**
 ** _alloc_req_id() - ALLOCATE NEXT REQUEST ID
 **/

#define	SEQF_DEF_START	1
#define	SEQF_DEF_END	BIGGEST_REQID
#define	SEQF_DEF_INCR	1
#define	SEQF		".SEQF"

long
#if	defined(__STDC__)
_alloc_req_id (
	void
)
#else
_alloc_files ()
#endif
{
	ENTRY ("_alloc_req_id")

	static short		started	= 0;

	static FILE		*fp;

	static long		start;
	static long		end;
	static long		incr;
	static long		curr;

	static char		fmt[
				STRSIZE(BIGGEST_REQID_S)/* start   */
			      + 1			/* :       */
			      + STRSIZE(BIGGEST_REQID_S)/* end     */
			      + 1			/* :       */
			      + STRSIZE(BIGGEST_REQID_S)/* incr    */
			      + 1			/* :       */
			      + 4			/* %ld\n   */
			      + 1			/* (nul)   */
				];

	char *			file;

	long			ret;


	if (!started) {
		file = makepath(Lp_Temp, SEQF, (char *)0);
		if (
			!(fp = fopen(file, "r+"))
		     && !(fp = fopen(file, "w"))
		)
			fail ("Can't open file %s (%s).\n", file, PERROR);

		rewind (fp);
		if (fscanf(fp, "%ld:%ld:%ld:%ld\n", &start, &end, &incr, &curr) != 4) {
			start = SEQF_DEF_START;
			end = SEQF_DEF_END;
			curr = start;
			incr = SEQF_DEF_INCR;
		}

		if (start < 0)
			start = SEQF_DEF_START;
		if (end > BIGGEST_REQID)
			end = SEQF_DEF_END;
		if (curr < start || curr > end)
			curr = start;

		sprintf (fmt, "%ld:%ld:%ld:%%ld\n", start, end, incr);
		started = 1;
	}

	ret = curr;

	if ((curr += incr) >= end)
	    curr = start;

	rewind (fp);
	fprintf (fp, fmt, curr);
	fflush (fp);

	return (ret);
}

/**
 ** _alloc_file() - ALLOCATE FILES FOR A REQUEST
 **/

char *
#if	defined(__STDC__)
_alloc_files (
	int			num,
	char *			prefix,
	uid_t			uid,
	gid_t			gid
)
#else
_alloc_files(num, prefix, uid, gid)
	int			num;
	char			*prefix;
	uid_t			uid;
	gid_t			gid;
#endif
{
	ENTRY ("_alloc_files")

	static char		base[
				1			/* F       */
			      + STRSIZE(BIGGEST_REQID_S)/* req-id  */
			      + 1			/* -       */
			      + STRSIZE(MOST_FILES_S)	/* file-no */
			      + 1			/* (nul)   */
				];

	char *			file;
	char *			cp;

	int			fd;
	int			plus;


	if (num > BIGGEST_REQID)
		return (0);

	if (!prefix) {
		sprintf (base, "%d-%d", _alloc_req_id(), MOST_FILES);
		plus = 0;
	} else {
		if (strlen(prefix) > 6)
			return (0);
		sprintf (base, "F%s-%d", prefix, MOST_FILES);
		plus = 1;
	}

	file = makepath(Lp_Temp, base, (char *)0);
        
	cp = strrchr(file, '-') + 1;
	while (num--) {
		sprintf (cp, "%d", num + plus);
		if ((fd = Open(file, O_CREAT|O_TRUNC, 0600)) == -1) {
			Free (file);
			return (0);
		} else {
			Close (fd);
			Chown (file, uid, gid);
		}
	}

	Free (file);

	if ((cp = strrchr(base, '-')))
		*cp = 0;

	return (base);
}
