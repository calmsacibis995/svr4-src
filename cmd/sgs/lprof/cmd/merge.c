/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)lxprof:cmd/merge.c	1.7"

#include "hidelibc.h"		/* uses "_" to hide libc functions */
#include "hidelibelf.h"		/* uses "_" to hide libelf functions */

#include <fcntl.h>
#include <stdio.h>
#include <signal.h>

#include "cor_errs.h"
#include "retcode.h"
#include "glob.h"
#include "env.h"

static char *temp_in, *temp_out;
extern int _tso_flag;		/* time stamp override flag set in main() */

/* Catch core dumps.  If the time stamp override has been specified and the */
/* count files are based on two different versions of the source code, */
/* _CAcov_join() might core dump in several places.  The core dump is caught */
/* and the appropriate error message printed.		*/


void _dexit(sig)
int sig;	/* #of signal which woke us up - ignored */
{
    unlink(temp_in);
    unlink(temp_out);
    /* in this case, we can quit */
    exit(2);
}

void Tout(sig)
int sig;	/* #of signal which woke us up - ignored */
{
	/* -T option set */
	if (_tso_flag) 
		fprintf(stderr,"\nCAmerge: Time stamp override specified, but can't merge count \
files\nCount files might not be based on the same version of the source program\n"); 
	else
		fprintf(stderr,"CAmerge: Can't merge count files\n");

	_dexit(0);	/* Unlink temp files and quit */
}


CAmerge(envir)
struct command *envir;
{
	short bad,good,count,flag,n,ret_flag,ret_code;
	char  *temp,*dest_covfile,
	      sysbuf[100];

	int fildes;
	char *tmpnam();
	void perror();


	if (signal(SIGINT,   SIG_IGN) != SIG_IGN)
		(void) signal(SIGINT,  _dexit);

	if (signal(SIGHUP,   SIG_IGN) != SIG_IGN)
		(void) signal(SIGHUP,  _dexit);

	if (signal(SIGQUIT,  SIG_IGN) != SIG_IGN)
		(void) signal(SIGQUIT, _dexit);

	ret_code = OK;
	count = envir->cov_next;   /* number of files to merge */
	flag = FALSE;
	if (count >= 2)
	{
	    dest_covfile = envir->dest_ptr;
	    if ( ((temp_in = tempnam(NULL,"lxp")) != NULL) &&
		((temp_out = tempnam(NULL, "lxp")) != NULL) )
	    /*if ( (tmpnam(temp_in) != NULL) && (tmpnam(temp_out) != NULL) )*/
	    {
		/* Catch core dumps during the count file merge process */
    		if (signal(SIGBUS, SIG_IGN) != SIG_IGN)
			(void) signal(SIGBUS, Tout);

		/* merge first two files */
		if((ret_flag = _CAcov_join(envir->cov_ptr[0],envir->cov_ptr[1],temp_out)) == MRG_OK)
		{
			flag = TRUE;
			/* swap scratch files */
			temp = temp_in;
			temp_in = temp_out;
			temp_out = temp;
		   }
		else
		{
			/* merge attempt failed, which covfile was culprit? */
			switch(ret_flag)
			{
				case MRG_FA1:
					bad = 0;
					good = 1;
					break;
				case MRG_FA2:
					bad = 1;
					good = 0;
					break;
				case MRG_FA3:
					fprintf(stderr,"CAmerge: %s\n",COR104);
					ret_code = BUG_FAIL;
					count = 0;
					break;
				default:
					fprintf(stderr,"CAmerge: %s\n",COR105);
					ret_code = BUG_FAIL;
					count = 0;
					break;
			  }
			if ((ret_code != BUG_FAIL) && (count > 2))
			/* copy 'good' file for possible use in subsequent merge */
			{
			   /* since 'copyfile' would remove input file,
			      create string for system call to 'cp' */
			   sprintf(sysbuf,"cp %s %s",envir->cov_ptr[good],temp_in);
			   if (system(sysbuf) < 0 )
			   {
			   	perror("CAmerge");
				fprintf(stderr,"CAmerge: %s\n",COR107);
				ret_code = BUG_FAIL;
			      }
			   else
			   {
				fprintf(stderr,"***'%s' discarded***\n",envir->cov_ptr[bad]);
				ret_code = COND_FAIL;
			     }
			   }
		  }


		/* are there more files to merge? */
		if( (count > 2) && (ret_code != BUG_FAIL))
		{
			n = 2;
			while (n < count)
			{
				if(_CAcov_join(envir->cov_ptr[n++],temp_in,temp_out) == MRG_OK)
				{
					flag = TRUE;
					/* swap scratch files */
					temp = temp_in;
					temp_in = temp_out;
					temp_out = temp;
				  }
				else
					ret_code = COND_FAIL;
			   }    /* while */
		    }   /* if count . . .*/
		if (flag == FALSE)
		{
			fprintf(stderr,"*** no merged output ***\n");
			ret_code = ALL_FAIL;
		   }
		else
		{
			if (count > 2) unlink(temp_out);
			/* create destination file before call to 'copyfile' */
			if ((fildes=open(dest_covfile,O_CREAT,0644)) > 0)
			{
			   close(fildes);
			   if (copyfile(temp_in,dest_covfile) != 0 )
			   {
				perror("CAmerge");
				fprintf(stderr,"***unable to put output into '%s', it can be retrieved from '%s'***\n",
				dest_covfile,temp_in);
				ret_code = COND_FAIL;
			      }
			   else
				fprintf(stderr,"\n\t`%s' created\n",dest_covfile);
			   }
			else
			{
			   perror("CAmerge");
			   fprintf(stderr,"***unable to put output into '%s', it can be retrieved from '%s'***\n",
			   dest_covfile,temp_in);
			   ret_code = COND_FAIL;
			  }
		  }

	     }  /* if tempnam . . . */

	    else {
			fprintf(stderr,"CAmerge: %s\n",COR103);
			if (temp_in != NULL)
			    free(temp_in);
			if (temp_out != NULL)
			    free(temp_out);
			ret_code = BUG_FAIL;
	  	}

	  }   /* if count >= 2. . . */

	else {
		fprintf(stderr,"Too few CNTFILE names specified.\n");
		ret_code = ALL_FAIL;
	}
	return(ret_code);
}

#ifndef CA_DEBUG
#include <sys/types.h>
#include <sys/stat.h>

copyfile(tempfile, oldfile)
char *tempfile, *oldfile;
{
	register fi,fo,ln;
	struct	stat	st;
	char buf[BUFSIZ];

#ifdef  __STDC__
	void (*oldi)(int), (*oldh)(int), (*oldg)(int);
#else
	void (*oldi)(), (*oldh)(), (*oldg)();
#endif

	oldi = signal(SIGINT, SIG_IGN);
	oldh = signal(SIGHUP, SIG_IGN);
	oldg = signal(SIGQUIT, SIG_IGN);

	if (stat(oldfile, &st) != 0)
		return(EOF);
	if (unlink(oldfile) != 0)
		return(EOF);
	if (link(tempfile, oldfile) != 0) {
		if ((fi = open(tempfile, 0)) < 0)
			return(EOF);
		if ((fo = creat(oldfile, 0644)) < 0)
			return(EOF);
		while ((ln = read(fi, buf, sizeof(buf))) > 0)
			if(write(fo, buf, ln) != ln)
				return(EOF);
		close(fi);
		close(fo);

	}
	if (chmod(oldfile, st.st_mode) != 0)
		return(EOF);
	if (chown(oldfile, st.st_uid, st.st_gid) != 0)
		return(EOF);
	if (unlink(tempfile) != 0)
		return(EOF);
	signal(SIGINT, oldi);
	signal(SIGHUP, oldh);
	signal(SIGQUIT, oldg);
	return(0);
}
#endif
