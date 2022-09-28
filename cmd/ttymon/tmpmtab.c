/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ttymon:tmpmtab.c	1.14.5.1"

#include	<unistd.h>
#include	<stdlib.h>
#include	<sys/types.h>
#include	<ctype.h>
#include	<string.h>
#include 	<pwd.h>
#include 	<grp.h>
#include	<signal.h>
#include	"ttymon.h"
#include	"tmstruct.h"
#include	"tmextern.h"

extern	char	*strsave();
extern	int	vml();
void	purge();
static	int	get_flags();
static	int	get_ttyflags();
static	int	same_entry();
static	int	check_pmtab();
static	void	insert_pmtab();
static	void	free_pmtab();
static	char	*expand();

int	check_identity();

int	strcheck();

/*
 * read_pmtab() 
 *	- read and parse pmtab 
 *	- store table in linked list pointed by global variable "PMtab"
 *	- exit if file does not exist or error detected.
 */
void
read_pmtab()
{
	register struct pmtab *gptr;
	register char *ptr, *wptr;
	FILE 	 *fp;
	int 	 input,state,size,rawc,field;
	char 	 oldc;
	char 	 line[BUFSIZ];
	char 	 wbuf[BUFSIZ];
	static 	 char *states[] = {
	      "","tag","flags","identity","reserved1","reserved2","reserved3",
	      "device","ttyflags","count","service", "timeout","ttylabel",
	      "modules","prompt","disable msg"
	};

# ifdef DEBUG
	debug("in read_pmtab");
# endif

	if ((fp = fopen(PMTABFILE,"r")) == NULL) {
		logexit(1, "open pmtab failed");
	}

	Nentries = 0;
	if (check_version(PMTAB_VERS, PMTABFILE) != 0)
		logexit(1,"check pmtab version failed");

	for (gptr = PMtab; gptr; gptr = gptr->p_next) {
		if ((gptr->p_status == SESSION) || (gptr->p_status == LOCKED)) {
			if (gptr->p_fd > 0) {
				(void)close(gptr->p_fd);
				gptr->p_fd = 0;
			}
			gptr->p_inservice = gptr->p_status;
		}
		gptr->p_status = NOTVALID;
	}

	wptr = wbuf;
	input = ACTIVE;
	do {
		line[0] = '\0';
		for (ptr= line,oldc = '\0'; ptr < &line[sizeof(line)-1] &&
		 (rawc=getc(fp))!= '\n' && rawc != EOF; ptr++,oldc=(char)rawc){
			if ((rawc == '#') && (oldc != '\\'))
				break;
			*ptr = (char)rawc;
		}
		*ptr = '\0';

		/* skip rest of the line */
		if (rawc != EOF && rawc != '\n') {
			if (rawc != '#') 
				log("Entry too long.\n");
			while ((rawc = getc(fp)) != EOF && rawc != '\n') 
				;
		}

		if (rawc == EOF) {
			if (ptr == line) break;
			else input = FINISHED;
		}

		/* if empty line, skip */
		for (ptr=line; *ptr != '\0' && isspace(*ptr); ptr++)
			;
		if (*ptr == '\0') continue;

#ifdef DEBUG
		(void)sprintf(Scratch,"**** Next Entry ****\n%s",line);
		debug(Scratch);
#endif

		/* Now we have the complete line */

		if ((gptr = ALLOC_PMTAB) == PNULL)
			logexit(1,"memory allocation failed");

		/* set hangup flag, this is the default */
		gptr->p_ttyflags |= H_FLAG;

		for (state=P_TAG,ptr=line;state !=FAILURE && state !=SUCCESS;) {
			switch(state) {
			case P_TAG:
				gptr->p_tag = strsave(getword(ptr,&size,0));
				break;
			case P_FLAGS:
				(void)strcpy(wptr, getword(ptr,&size,0));
				if ((get_flags(wptr, &gptr->p_flags)) != 0) {
					field = state;
					state = FAILURE;
				}
				break;
			case P_IDENTITY:
				gptr->p_identity=strsave(getword(ptr,&size,0));
				break;
			case P_RES1:
				gptr->p_res1=strsave(getword(ptr,&size,0));
				break;
			case P_RES2:
				gptr->p_res2=strsave(getword(ptr,&size,0));
				break;
			case P_RES3:
				gptr->p_res3=strsave(getword(ptr,&size,0));
				break;
			case P_DEVICE:
				gptr->p_device = strsave(getword(ptr,&size,0));
				break;
			case P_TTYFLAGS:
				(void)strcpy(wptr, getword(ptr,&size,0));
				if ((get_ttyflags(wptr,&gptr->p_ttyflags))!=0) {
					field = state;
					state = FAILURE;
				}
				break;
			case P_COUNT:
				(void)strcpy(wptr, getword(ptr,&size,0));
				if (strcheck(wptr, NUM) != 0) {
					log("wait_read count must be a positive number"); 
					field = state;
					state = FAILURE;
				}
				else
				    gptr->p_count = atoi(wptr);
				break;
			case P_SERVER:
				gptr->p_server = 
				strsave(expand(getword(ptr,&size,1), 
					gptr->p_device));
				break;
			case P_TIMEOUT:
				(void)strcpy(wptr, getword(ptr,&size,0));
				if (strcheck(wptr, NUM) != 0) {
					log("timeout value must be a positive number"); 
					field = state;
					state = FAILURE;
				}
				else
				    gptr->p_timeout = atoi(wptr);
				break;
			case P_TTYLABEL:
				gptr->p_ttylabel=strsave(getword(ptr,&size,0));
				break;
			case P_MODULES:
				gptr->p_modules = strsave(getword(ptr,&size,0));
				if (vml(gptr->p_modules) != 0) {
					field = state;
					state = FAILURE;
				}
				break;
			case P_PROMPT:
				gptr->p_prompt = strsave(getword(ptr,&size,TRUE));
				break;
			case P_DMSG:
				gptr->p_dmsg = strsave(getword(ptr,&size,TRUE));
				break;
			} /* end switch */
			ptr += size;
			if (state == FAILURE) 
				break;
			if (state == P_DMSG) {
				if ((*ptr == '\0') || (*ptr == ':')) {
					state = SUCCESS;
				}
				else {
					field = state;
					state = FAILURE;
				}
			}
			else {
				if (*ptr != ':') {
					field = state;
					state = FAILURE;
				} else {
					ptr++;	/* Skip the ':' */
					state++ ;
				}
			}
		} /* end for loop */

		if (state == SUCCESS) {
			if (check_pmtab(gptr) == 0) {
				if (Nentries < Maxfds) 
					insert_pmtab(gptr);
				else {
					(void)sprintf(Scratch, 
			"can't add more entries to pmtab, Maxfds = %d", Maxfds);
					log(Scratch);
					free_pmtab(gptr);
					(void)fclose(fp);
					return;
				}
			}
			else {
				(void)sprintf(Scratch,
				"Parsing failure for entry: \n%s",line);
				log(Scratch);
			log("-------------------------------------------");
				free_pmtab(gptr);
			}
		} else {
			*++ptr = '\0';
			(void)sprintf(Scratch,
			"Parsing failure in the \"%s\" field,\n%s<--error detected here", states[field],line);
			log(Scratch);
			log("-------------------------------------------");
			free_pmtab(gptr);
		}
	} while (input == ACTIVE);

	(void)fclose(fp);
	return;
}

/*
 * get_flags	- scan flags field to set U_FLAG and X_FLAG
 */
static	int
get_flags(wptr, flags)
char	*wptr;		/* pointer to the input string	*/
long *flags;		/* pointer to the flag to set	*/
{
	register char	*p;
	for (p = wptr; *p; p++) {
		switch (*p) {
		case 'x':
			*flags |= X_FLAG;
			break;
		case 'u':
			*flags |= U_FLAG;
			break;
		default:
			(void)sprintf(Scratch,"Invalid flag -- %c", *p);
			log(Scratch);
			return(-1);
		} 
	}
	return(0);
}

/*
 * get_ttyflags	- scan ttyflags field to set corresponding flags
 */
static	int
get_ttyflags(wptr, ttyflags)
char	*wptr;		/* pointer to the input string	*/
long 	*ttyflags;	/* pointer to the flag to be set*/
{
	register char	*p;
	for (p = wptr; *p; p++) {
		switch (*p) {
		case 'c':
			*ttyflags |= C_FLAG;
			break;
		case 'h': /* h means don't hangup */
			*ttyflags &= ~H_FLAG;
			break;
		case 'b':
			*ttyflags |= B_FLAG;
			*ttyflags |= R_FLAG;
			break;
		case 'r':
			*ttyflags |= R_FLAG;
			break;
		default:
			(void)sprintf(Scratch,"Invalid ttyflag -- %c", *p);
			log(Scratch);
			return(-1);
		} 
	}
	return(0);
}

# ifdef DEBUG
/*
 * pflags - put service flags into intelligible form for output
 */

char *
pflags(flags)
long flags;	/* binary representation of the flags */
{
	register int i;			/* scratch counter */
	static char buf[BUFSIZ];	/* formatted flags */

	if (flags == 0)
		return("-");
	i = 0;
	if (flags & U_FLAG) {
		buf[i++] = 'u';
		flags &= ~U_FLAG;
	}
	if (flags & X_FLAG) {
		buf[i++] = 'x';
		flags &= ~X_FLAG;
	}
	if (flags)
		log("Internal error in pflags");
	buf[i] = '\0';
	return(buf);
}

/*
 * pttyflags - put ttyflags into intelligible form for output
 */

char *
pttyflags(flags)
long flags;	/* binary representation of ttyflags */
{
	register int i;			/* scratch counter */
	static char buf[BUFSIZ];	/* formatted flags */

	if (flags == 0)
		return("h");
	i = 0;
	if (flags & C_FLAG) {
		buf[i++] = 'c';
		flags &= ~C_FLAG;
	}
	if (flags & H_FLAG) 
		flags &= ~H_FLAG;
	else
		buf[i++] = 'h';
	if (flags & B_FLAG) {
		buf[i++] = 'b';
		flags &= ~B_FLAG;
	}
	if (flags & R_FLAG) {
		buf[i++] = 'r';
		flags &= ~B_FLAG;
	}
	if (flags)
		log("Internal error in p_ttyflags");
	buf[i] = '\0';
	return(buf);
}

void
dump_pmtab()
{
	struct	pmtab *gptr;

	debug("in dump_pmtab");
	log("********** dumping pmtab **********");
	log(" ");
	for (gptr=PMtab; gptr; gptr = gptr->p_next) {
		log("-------------------------------------------");
		(void)sprintf(Scratch,"tag:\t\t%s",gptr->p_tag);
		log(Scratch);
		(void)sprintf(Scratch,"flags:\t\t%s",pflags(gptr->p_flags));
		log(Scratch);
		(void)sprintf(Scratch,"identity:\t%s",gptr->p_identity);
		log(Scratch);
		(void)sprintf(Scratch,"reserved1:\t%s",gptr->p_res1);
		log(Scratch);
		(void)sprintf(Scratch,"reserved2:\t%s",gptr->p_res2);
		log(Scratch);
		(void)sprintf(Scratch,"reserved3:\t%s",gptr->p_res3);
		log(Scratch);
		(void)sprintf(Scratch,"device:\t%s",gptr->p_device);
		log(Scratch);
		(void)sprintf(Scratch,"ttyflags:\t%s",pttyflags(gptr->p_ttyflags));
		log(Scratch);
		(void)sprintf(Scratch,"count:\t\t%d",gptr->p_count);
		log(Scratch);
		(void)sprintf(Scratch,"server:\t%s",gptr->p_server);
		log(Scratch);
		(void)sprintf(Scratch,"timeout:\t%d",gptr->p_timeout);
		log(Scratch);
		(void)sprintf(Scratch,"ttylabel:\t%s",gptr->p_ttylabel);
		log(Scratch);
		(void)sprintf(Scratch,"modules:\t%s",gptr->p_modules);
		log(Scratch);
		(void)sprintf(Scratch,"prompt:\t%s",gptr->p_prompt);
		log(Scratch);
		(void)sprintf(Scratch,"disable msg:\t%s",gptr->p_dmsg);
		log(Scratch);
		(void)sprintf(Scratch,"status:\t\t%d",gptr->p_status);
		log(Scratch);
		(void)sprintf(Scratch,"inservice:\t%d",gptr->p_inservice);
		log(Scratch);
		(void)sprintf(Scratch,"fd:\t\t%d",gptr->p_fd);
		log(Scratch);
		(void)sprintf(Scratch,"pid:\t\t%ld",gptr->p_pid);
		log(Scratch);
		(void)sprintf(Scratch,"uid:\t\t%ld",gptr->p_uid);
		log(Scratch);
		(void)sprintf(Scratch,"gid:\t\t%ld",gptr->p_gid);
		log(Scratch);
		(void)sprintf(Scratch,"dir:\t%s",gptr->p_dir);
		log(Scratch);
		log(" ");
	}
	log("********** end dumping pmtab **********");
}
# endif

/*
 * same_entry(e1,e2) -    compare 2 entries of pmtab
 *			if the fields are different, copy e2 to e1
 * 			return 1 if same, return 0 if different
 */
static	int
same_entry(e1,e2)
struct	pmtab	*e1,*e2;
{
	if (strcmp(e1->p_identity, e2->p_identity) != 0)
		return(0);
	if (strcmp(e1->p_res1, e2->p_res1) != 0)
		return(0);
	if (strcmp(e1->p_res2, e2->p_res2) != 0)
		return(0);
	if (strcmp(e1->p_res3, e2->p_res3) != 0)
		return(0);
	if (strcmp(e1->p_device, e2->p_device) != 0)
		return(0);
	if (strcmp(e1->p_server, e2->p_server) != 0)
		return(0);
	if (strcmp(e1->p_ttylabel, e2->p_ttylabel) != 0)
		return(0);
	if (strcmp(e1->p_modules, e2->p_modules) != 0)
		return(0);
	if (strcmp(e1->p_prompt, e2->p_prompt) != 0)
		return(0);
	if (strcmp(e1->p_dmsg, e2->p_dmsg) != 0)
		return(0);
	if (e1->p_flags != e2->p_flags)
		return(0);
	/*
	 * compare lowest 4 bits only, 
	 * because A_FLAG is not part of original ttyflags
	 */
	if ((e1->p_ttyflags & 017) != e2->p_ttyflags) /*cmp lowest 4 bit only*/
		return(0);
	if (e1->p_count != e2->p_count)
		return(0);
	if (e1->p_timeout != e2->p_timeout)
		return(0);
	if (e1->p_uid != e2->p_uid)
		return(0);
	if (e1->p_gid != e2->p_gid)
		return(0);
	if (strcmp(e1->p_dir, e2->p_dir) != 0)
		return(0);
	return(1);
}


/*
 * insert_pmtab - insert a pmtab entry into the linked list
 */

static	void
insert_pmtab(sp)
register struct pmtab *sp;	/* ptr to entry to be inserted */
{
	register struct pmtab *tsp, *savtsp;	/* scratch pointers */
	int ret;				/* strcmp return value */

# ifdef DEBUG
	debug("in insert_pmtab");
# endif
	savtsp = tsp = PMtab;

/*
 * find the correct place to insert this element
 */

	while (tsp) {
		ret = strcmp(sp->p_tag, tsp->p_tag);
		if (ret > 0) {
			/* keep on looking */
			savtsp = tsp;
			tsp = tsp->p_next;
			continue;
		}
		else if (ret == 0) {
			if (tsp->p_status) {
				/* this is a duplicate entry, ignore it */
				(void) sprintf(Scratch, "Ignoring duplicate entry for <%s>", tsp->p_tag);
				log(Scratch);
			}
			else {
				if (same_entry(tsp,sp)) {  /* same entry */
					tsp->p_status = VALID;
				}
				else {	/* entry changed */
					if ((sp->p_flags & X_FLAG) && 
						((sp->p_dmsg == NULL) ||
						(*(sp->p_dmsg) == '\0'))) {
						/* disabled entry */
						tsp->p_status = NOTVALID;
					}
					else {
# ifdef DEBUG
					(void)sprintf(Scratch, "replacing <%s>", sp->p_tag);
					debug(Scratch);
# endif
						/* replace old entry */
						sp->p_next = tsp->p_next;
						if (tsp == PMtab) {
						   PMtab = sp;
						}
						else {
						   savtsp->p_next = sp;
						}
						sp->p_status = CHANGED;
						sp->p_fd = tsp->p_fd;
						sp->p_pid = tsp->p_pid;
					        sp->p_inservice =
							tsp->p_inservice;
						sp = tsp;
					}
				}
				Nentries++;
			}
			free_pmtab(sp);
			return;
		}
		else {
			if ((sp->p_flags & X_FLAG) && 
				((sp->p_dmsg == NULL) ||
				(*(sp->p_dmsg) == '\0'))) { /* disabled entry */
				free_pmtab(sp);
				return;
			}
			/* insert it here */
			if (tsp == PMtab) {
				sp->p_next = PMtab;
				PMtab = sp;
			}
			else {
				sp->p_next = savtsp->p_next;
				savtsp->p_next = sp;
			}
# ifdef DEBUG
			(void) sprintf(Scratch, "adding <%s>", sp->p_tag);
			debug(Scratch);
# endif
			Nentries++;
			/* this entry is "current" */
			sp->p_status = VALID;
			return;
		}
	}

/*
 * either an empty list or should put element at end of list
 */

	if ((sp->p_flags & X_FLAG) && 
		((sp->p_dmsg == NULL) ||
		(*(sp->p_dmsg) == '\0'))) { /* disabled entry */
		free_pmtab(sp);		 /* do not poll this entry */
		return;
	}
	sp->p_next = NULL;
	if (PMtab == NULL)
		PMtab = sp;
	else
		savtsp->p_next = sp;
# ifdef DEBUG
	(void) sprintf(Scratch, "adding <%s>", sp->p_tag);
	debug(Scratch);
# endif
	++Nentries;
	/* this entry is "current" */
	sp->p_status = VALID;
}


/*
 * purge - purge linked list of "old" entries
 */


void
purge()
{
	register struct pmtab *sp;		/* working pointer */
	register struct pmtab *savesp, *tsp;	/* scratch pointers */

# ifdef DEBUG
	debug("in purge");
# endif
	sp = savesp = PMtab;
	while (sp) {
		if (sp->p_status) {
			savesp = sp;
			sp = sp->p_next;
		}
		else {
			tsp = sp;
			if (tsp == PMtab) {
				PMtab = sp->p_next;
				savesp = PMtab;
			}
			else
				savesp->p_next = sp->p_next;
# ifdef DEBUG
			(void) sprintf(Scratch, "purging <%s>", sp->p_tag);
			debug(Scratch);
# endif
			sp = sp->p_next;
			free_pmtab(tsp);
		}
	}
}

/*
 *	free_pmtab	- free one pmtab entry
 */
static	void
free_pmtab(p)
struct	pmtab	*p;
{
#ifdef	DEBUG
	debug("in free_pmtab");
#endif
	free(p->p_tag);
	free(p->p_identity);
	free(p->p_res1);
	free(p->p_res2);
	free(p->p_res3);
	free(p->p_device);
	free(p->p_server);
	free(p->p_ttylabel);
	free(p->p_modules);
	free(p->p_prompt);
	free(p->p_dmsg);
	if (p->p_dir)
		free(p->p_dir);
	free(p);
}

/*
 *	check_pmtab - check the fields to make sure things are correct
 *		    - return 0 if everything is ok
 *		    - return -1 if something is wrong
 */

static	int
check_pmtab(p)
struct	pmtab	*p;
{
	if (p == NULL) {
		log("pmtab ptr is NULL");
		return(-1);
	}

	/* check service tag */
	if ((p->p_tag == NULL) || (*(p->p_tag) == '\0')) {
		log("port/service tag is missing");
		return(-1);
	}
	if (strlen(p->p_tag) > (size_t)(MAXID - 1)) {
		(void)sprintf(Scratch,"port/service tag <%s> is longer than %d",
			p->p_tag, MAXID-1);
		log(Scratch);
		return(-1);
	}
	if (strcheck(p->p_tag, ALNUM) != 0) {
		(void)sprintf(Scratch,
		"port/service tag <%s> is not alphanumeric", p->p_tag);
		log(Scratch);
		return(-1);
	}

	if (check_identity(p) != 0) {
		return(-1);
	}

	if (check_device(p->p_device) != 0)
		return(-1);

	if (check_cmd(p->p_server) != 0)
		return(-1);
	return(0);
}

extern  struct 	passwd *getpwnam();
extern  void 	endpwent();
extern  struct 	group *getgrgid();
extern  void 	endgrent();

/*
 *	check_identity - check to see if the identity is a valid user
 *		       - log name in the passwd file,
 *		       - and if its group id is a valid one
 *		  	- return 0 if everything is ok. Otherwise, return -1
 */

int
check_identity(p)
struct	pmtab	*p;
{
	register struct passwd *pwdp;

	if ((p->p_identity == NULL) || (*(p->p_identity) == '\0')) {
		log("identity field is missing");
		return(-1);
	}
	if ((pwdp = getpwnam(p->p_identity)) == NULL) {
		(void)sprintf(Scratch, "missing or bad passwd entry for <%s>", 
			p->p_identity);
		log(Scratch);
		endpwent();
		return(-1);
	}
	if (getgrgid(pwdp->pw_gid) == NULL) {
		(void)sprintf(Scratch, "no group entry for %ld", pwdp->pw_gid);
		log(Scratch);
		endgrent();
		endpwent();
		return(-1);
	}
	p->p_uid = pwdp->pw_uid;
	p->p_gid = pwdp->pw_gid;
	p->p_dir = strsave(pwdp->pw_dir);
	endgrent();
	endpwent();
	return(0);
}

/*
 * expand(cmdp, devp)	- expand %d to device name and %% to %,
 *				- any other characters are untouched.
 *				- return the expanded string
 */
static char	*
expand(cmdp,devp)
char	*cmdp;		/* ptr to cmd string	*/
char	*devp;		/* ptr to device name	*/
{
	register char	*cp, *dp, *np;
	static char	buf[BUFSIZ];
	cp = cmdp;
	np = buf;
	dp = devp;
	while (*cp) {
		if (*cp != '%') {
			*np++ = *cp++;
			continue;
		}
		switch (*++cp) {
		case 'd':
			while (*dp) {
				*np++ = *dp++;
			}
			cp++;
			break;
		case '%':
			*np++ = *cp++;
			break;
		default:
			*np++ = *cp++;
			break;
		}
	}
	*np = '\0';
	return(buf);
}

