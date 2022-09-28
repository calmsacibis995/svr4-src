/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)lp:cmd/bsd/lpc/process.c	1.2.2.1"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>

#include "lp.h"
#include "printers.h"
#include "class.h"
#include "msgs.h"
#include "requests.h"
#include "oam_def.h"
#include "lpc.h"

struct prnames {
	char		*printer; 	
	struct prnames  *next;
};

struct prnames	*prhead;
	
int		 got_all_prnames;

extern int	 When;
extern char	*Reason;

#if defined (__STDC__)
static	void	unlinkf(char *);
#else
static	void	unlinkf();
#endif

/**
 **   Obtain all printer names from LPSCHED and save them in
 **   a linked list of printer names.
 **/
static void
#if defined(__STDC__)
get_all_prnames(void)
#else
get_all_prnames()
#endif
{



	char	*printer,
		*reject_reason,
		*request_id,
		*form,
		*char_set,
		*disable_reason;

	short	 printer_status,
		 status;

	long	 enable_date,
		 reject_date;
	
	int 	 rc;
	register struct prnames	*ptr, *prcur;


	if (got_all_prnames) 
		return;


	/*
 	 * Send a message to LPSCHED to retrive all printer names
	 * This is done by sending a: S_INQUIRE_PRINTER_STATUS to LPSCHED.
	 * This will return the current status of the job underway on 
         * all the printers on the system. 
	 */
	prhead = NULL;
	snd_msg(S_INQUIRE_PRINTER_STATUS, "");
	do {
		rcv_msg(R_INQUIRE_PRINTER_STATUS, &status,
						  &printer,
						  &form,
						  &char_set,
						  &disable_reason,
						  &reject_reason,
						  &printer_status,
						  &request_id,
						  &enable_date,
						  &reject_date);
		if(!(ptr = (struct prnames *)malloc(sizeof(struct prnames)))) {
			lp_fatal(E_LP_MALLOC);
			/*NOTREACHED*/
		}
		if(prhead == NULL)
			prhead = ptr;
		else
			prcur->next = ptr;
		prcur = ptr;
		/* Copy : printer name
		 * Increment the entry count.
		 */
		ptr->printer = (char *)malloc(strlen(printer));
		strcpy(ptr->printer, printer);
		ptr->next = NULL;
	} while(status== MOKMORE);
	got_all_prnames = 1;
	return;
}



/* 
** The actual work in passing the commands to LPCSHED is done here
** i.e: Enable/disable the queues
**      Enable/disable the printers
**	canceling jobs and the temporary files etc
**/

/*
**	Disable the queue to a printer.
**/
void
#if defined(__STDC__)
disableq(char *dest)
#else
disableq(dest)
char	*dest;
#endif
{
    	short	 status;
	char	*reason = Reason;

	if (reason)
		reason = "unknown reason";

	snd_msg(S_REJECT_DEST, dest, reason);
	rcv_msg(R_REJECT_DEST, &status);
	switch (status) {
	case MOK:
	case MERRDEST:
		printf("%s:\n", dest);
		printf("\tqueuing disabled\n");
		break;
	case MNODEST:
 		printf("unknown printer %s\n", dest);
		break;
	case MNOPERM:
		printf("%s:\n", dest);
		printf("\tcannot disable queuing\n");
		break;
	default:
		lp_fatal(E_LP_BADSTATUS, status); 
		/*NOTREACHED*/
	}
}
/*
**	Enable the queue to a printer.
**/	

void	
#if defined(__STDC__)
enableq(char *dest)
#else
enableq(dest)
char	*dest;
#endif
{
    	short	status;

	snd_msg(S_ACCEPT_DEST, dest);
	rcv_msg(R_ACCEPT_DEST, &status);
	switch (status) {
	case MOK:
	case MERRDEST:
		printf("%s:\n", dest);
		printf("\tqueueing enabled\n");
	    	break;
	case MNODEST:
  		printf("unknown printer %s\n", dest);
	    	break;
	case MNOPERM:
		printf("%s:\n", dest);
		printf("\tcannot enable queueing\n");
	    	break;
	default:
	    	lp_fatal(E_LP_BADSTATUS, status);
		/*NOTREACHED*/
	}

}

/*
**	Enable printing on the given printer.
**/
void
#if defined(__STDC__)
enablepr(char *dest)
#else
enablepr(dest)
char	*dest;
#endif
{
    	short	status;

	snd_msg(S_ENABLE_DEST, dest);
	rcv_msg(R_ENABLE_DEST, &status);
	switch (status) {
	case MOK:
	case MERRDEST:
		printf("%s:\n", dest);
		printf("\tprinting enabled\n");
		break;
	case MNODEST:
		printf("unknown printer %s\n", dest);
	    	break;
	case MNOPERM:
		printf("%s:\n", dest);
		printf("\tcannot enable printing\n");
	    	break;
	default:
	    	lp_fatal(E_LP_BADSTATUS, status);
		/*NOTREACHED*/
	}
}

/*
**	Disable printing on the named printer.
**/
void
#if defined(__STDC__)
disablepr(char *dest)
#else
disablepr(dest)
char	*dest;
#endif
{
	short	 status;
	char	*req_id;
	char	*reason = Reason;

	if (!reason)
		reason = "stopped by user";

	snd_msg(S_DISABLE_DEST, dest, reason, When);
	rcv_msg(R_DISABLE_DEST, &status, &req_id);
	switch (status) {
    	case MOK:
	case MERRDEST:
		printf("%s:\n", dest);
		printf("\tprinting disabled\n");
		break;
	case MNODEST:
		printf("unknown printer %s\n", dest);
		break;
    	case MNOPERM:
		printf("%s:\n", dest);
		printf("\tcannot disable printing\n");
		break;
    	default:
		lp_fatal(E_LP_BADSTATUS, status); 
		/*NOTREACHED*/
    	}
	return;
}
	
void
#if defined(__STDC__)
statuspr(char *printer)
#else
statuspr(printer)
char	*printer;
#endif
{
	char	*tprinter;
			
	int	 rc, entry_count;

	char	*user,
		*reject_reason,
		*request_id,
		*form,
		*char_set,
		*disable_reason;

	short	 printer_status,
		 status,
		 state;

	long	 size,
		 enable_date,
		 reject_date,
		 date;

	entry_count = 0;
	snd_msg(S_INQUIRE_PRINTER_STATUS, printer);
	rcv_msg(R_INQUIRE_PRINTER_STATUS, &status,
					  &tprinter,
					  &form,
					  &char_set,
					  &disable_reason,
					  &reject_reason,
					  &printer_status,
					  &request_id,
					  &enable_date,
					  &reject_date);
	switch (status) {
	case MOK:
	case MOKMORE:
		break;
	case MNODEST:
		printf("unknown printer %s\n", printer);
		return;
	default:
		lp_fatal(E_LP_BADSTATUS, status);
		/*NOTREACHED*/
	}
	printf("%s:\n", printer);
	printf("\tqueueing is %s\n", printer_status & PS_REJECTED ? "disabled" :
								    "enabled");
	printf("\tprinting is %s\n", printer_status & PS_DISABLED ? "disabled" :
								    "enabled");
	snd_msg(S_INQUIRE_REQUEST, "", printer, "", "", "");
	do {
		rcv_msg(R_INQUIRE_REQUEST, &status,
					   &request_id,
					   &user,
					   &size,
					   &date,
					   &state,
					   &tprinter,
					   &form,
					   &char_set);
		switch (status) {
		case MOK:
		case MOKMORE:
			if (!(state & RS_DONE))
				entry_count++;
			break;
		case MNOINFO:
			break;
		default:
			lp_fatal(E_LP_BADSTATUS, status);
			/*NOTREACHED*/
		}
	} while (status == MOKMORE);
	if (entry_count == 0 )
		printf("\tno entries\n");
	else if (entry_count == 1)
		printf("\t1 entry in spool area\n");
	else
		printf("\t%d entries in spool area\n", entry_count);
	if (entry_count)
		if (!(printer_status & (PS_FAULTED|PS_DISABLED)))
			printf("\t%s is ready and printing\n", printer);
		else if (printer_status & PS_FAULTED)
			printf("\twaiting for %s to become ready (offline?)\n",
								printer);
			
	/*??? what to do for remote printers:
		possible status:
			"waiting for RM to come up"
			"waiting for queue to be enabled on RM"
			"sending to RM"
			"no space on remote; waiting for queue to drain"
	*/
}

void
#if defined(__STDC__)
restartpr(char *dest)
#else
restartpr(dest)
char	*dest;
#endif
{
	disablepr(dest);
	enablepr(dest);
}

void
#if defined(__STDC__)
uppr(char *dest)
#else
uppr(dest)
char	*dest;
#endif
{
	enableq(dest);
	enablepr(dest);
}

void
#if defined(__STDC__)
downpr(char *dest)
#else
downpr(dest)
char	*dest;
#endif
{
	disableq(dest);
	disablepr(dest);
}

/* avoids compiler type checking problem */
static
#if defined(__STDC__)
_strcmp(const void *s1, const void *s2)
#else
_strcmp(s1, s2)
const void *s1;
const void *s2;
#endif
{
	return(strcmp(s1, s2));
}

void
#if defined(__STDC__)
cleanpr(char *dest)
#else
cleanpr(dest)
char	*dest;
#endif
{
	char		*sysdir;
	char		*sppath;
	char		*spfile;
	long		 addr = -1;
	int		 nfiles;
	DIR		*dirp;
	struct dirent	*direntp;
	struct sfn { 
			char sfn[20];
		   }	*p;
	static char	*buf;
	static int	 bufsize;

	if (!getprinter(dest)) {
		printf("unknown printer %s\n", dest);
		return;
	}
	if (!buf && !(buf = malloc(bufsize = 100 * sizeof(*p)))) {
		printf("can't malloc %d bytes\n",  bufsize);
		return;
	}
	printf("%s:\n", dest);
	if (chdir(Lp_Tmp) < 0) {
		printf("\tcannot examine spool directory\n");
		return;
	}
	for (; sysdir = next_dir(".", &addr); free(sysdir)) {
		if (*sysdir == '.' || !(dirp = opendir(sysdir)))
			continue;
		nfiles = 0;
		p = (struct sfn *)buf;
		while (direntp = readdir(dirp)) {
			if (*direntp->d_name == '.')
				continue;
			if ((char *)p + sizeof(*p) > buf + bufsize) {
				buf = realloc(buf, bufsize += 20 * sizeof(*p));
				if (!buf) {
					printf("can't malloc %d bytes\n",  bufsize);
					free(sysdir);
					closedir(dirp);
					return;
				}
				p = (struct sfn *)buf + nfiles;
			}
			strncpy(p->sfn, direntp->d_name, sizeof(p->sfn)-1);
			p++;
			nfiles++;
		}
		closedir(dirp);
		if (!nfiles)
			continue;
		qsort(buf, nfiles, sizeof(*p), _strcmp);
		spfile = sppath = malloc(strlen(sysdir) + sizeof(*p) + 1);
		spfile += sprintf(sppath, "%s/", sysdir);
		for (p = (struct sfn *)buf; nfiles--; p++) {
			char		*cp;
			REQUEST		*req;
			struct sfn	*op;

			strcpy(spfile, p->sfn);
			if (!(cp = strrchr(p->sfn, '-')) ||
			    !(STREQU(cp+1, "0")) ||
			    !(req = getrequest(sppath))) {
				unlinkf(sppath);
				continue;
			}
			if (STREQU(req->destination, dest) &&
			    !getsecure(sppath)) {
				unlinkf(sppath);
				continue;
			}
				/* skip "good" files */
			*cp = NULL;
			for (op = p++; nfiles; p++, nfiles--) {
				if (!(cp = strrchr(p->sfn, '-'))) {
					p--;
					break;
				}
				*cp = NULL;
				if (!STREQU(op->sfn, p->sfn)) {
					*cp = '-';
					p--;
					break;
				}
			}
		}
		free(sppath);
	}
}

static void
#if defined(__STDC__)
unlinkf(char *name)
#else
unlinkf(name)
char	*name;
#endif
{
	if (unlink(name) < 0)
		printf("\tcannot remove %s\n", name);
	else
		printf("\tremoved %s\n", name);
}

void
#if defined(__STDC__)
do_all(void (*func)(char *))
#else
do_all(func)
void	(*func)();
#endif
{
	register struct prnames	*ptr;

	get_all_prnames();
	
	for (ptr = prhead; ptr; ptr = ptr->next)
		(*func)(ptr->printer);
}
