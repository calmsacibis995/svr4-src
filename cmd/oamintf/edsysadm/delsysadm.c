/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamintf:edsysadm/delsysadm.c	1.3.3.2"

/*	delsysadm task | -r menu
		
		errors: 	
			0	success
			2	invalid syntax
			3	menu or task does not exist
			4	menu not empty ("-r" option)
			5	unable to update interface menu structure
*/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <pwd.h>
#include <limits.h>
#include "intf.h"
#include "menu_io.h"

/*	Define success and error codes */

#define SUCCESS   0
#define SYNTAX	  2
#define	NOTEXIST  3
#define	NOTEMPTY  4
#define NOUPDATE  5
#define REMPTY    6
#define USERWARN 10
#define XPRESSWARN   11
#define PKGTAGWARN   12
#define DIRWARN   13
#define NOTROOT		99

/*	Define char. array sizes and other items */

#define IE_BUILD  	"/usr/sadm/install/bin/ie_build"
#define SHORT_MOD 	"ie_build"	/* for fork exec - also using "-o" argument */
#define	LNSZ	  	128
#define MLNSZ	 	1024
#define LGLNSZ 		12800
#define DOTMENU		".menu"
#define GO		"yes"
#define ONLINE		"_ONLINE"
#define ADDONS		"add-ons"
#define VOIDDEL 	"#VOID#"	/* use VOID to delete, DELETE to change! */
#define EU_PATH		"menu"
#define PATHDEL 	"/"
#define PATHDELCHR 	'/'
#define LVLDEL  	':'
#define PTAGDEL  	"#"
#define PARSEDEL  	"^"

extern uid_t	geteuid();

extern FILE *fopen();

extern struct menu_file *input_menu();
extern struct menu_item *find_menu();

extern char 	*getenv(),
		*menutok();

extern void	*calloc(),
		exit();

extern long	fork();

extern int 	errno,
		execlp(),
		getopt(),
		putenv(),
		wait(),
		unlink(),
		link(),
		ismenu(),
		write_item();

int		ckmn_open();

static int	add_tags(),
		ask_user(),
		chng_express(),
		findx(),
		item_gone(),
		link_tags(),
		menu_pick(),
		parnt_gone(),
		parnt_xfer(),
		parnt_travel(),
		parse_all(),
		parse_menus(),
		read_tags(),
		remove_dirs(),
		temp_files(),
		write_log();

static void 	catch_errs(),
		getfull(),
		getlocn();


static char	
		*prog,			/* program name for find_menu */
		tempmenu[PATH_MAX],	/* temp menuname for menu io routines */
		exprlog[PATH_MAX],	/* express file changes log */
		chngmenu[PATH_MAX],	/* tmp menu file w/ changes */
		uplvlmenu[PATH_MAX],	/* tmp parent's parent menu w/changes */
		saveitem[PATH_MAX],	/* save menu or task argument */
		menulocn[PATH_MAX],	/* for specific menu dir. */
		pathname[PATH_MAX],	/*for phys. pathname - getlocn call */
		intflocn[PATH_MAX], 
		locn[PATH_MAX], 
		oampath[PATH_MAX],	/* for oambase path */
		relpath[PATH_MAX],	/* for rel path for add-on dirs. */
		lclpath[PATH_MAX],	/* for local path for dirs. */
		abspath[PATH_MAX],	/* for absolute path for dirs. */
		intfpath[PATH_MAX],	/* for intf. path (incl. menu) dirs. */
		fulldir[PATH_MAX],	/* for removal path for item dir. */
		exprpath[PATH_MAX],	/* for express file paths  */
		x_name[PATH_MAX];	/* for absolute relative pathname */

FILE *pkglist;

static FILE	*logfile,
		*chngptr,
		*uplvlptr;

struct search_menus{
	char mkywd[17];			/* for mname ifit's a .menu */
	char mwhere[128];		/* for dir part of maction */
	char mwhat[128];		/* for .menu part of maction */
	struct search_menus *mom;	/* for pulling parent pathname out */
	struct search_menus *next;	/* for next .menu to parse */
};
static struct search_menus	*first_menu,
				*last_menu,
				*look_menu;

struct dfine_tags{
	char tag[100];
	struct dfine_tags *next;
};
static struct dfine_tags	*first_tag,
				*last_tag,
				*cur_tag,
				*check_tag;

struct item_def *subitem;	/* for parsing sub menus entries */
char *subactn;
char remdir[PATH_MAX];			/* for dir. path from item  */
char tagdir[PATH_MAX];			/* for dir. path from pkg tag  */


static int	init_menu,
		flagexpress,	/* flag - some files not in log */
		flagpkgdpnd,	/* flag - some pkg dep. not found */
		flagparnt,	/* flag for xfering parent menu changes */
		errwrite, 
		errrmv, 
		errigone,
		warnuser, 
		flagdirs,
		vacant, 	/* flag that parent menu is empty or 
		   		   has only a placeholder in it */
		dotask,
		rflag = 0,	/* recursion flag for menus */
		flaguser = 0,	/* flag user that have pkg dep.s */
		goahead = 0;	/* for dependency check question */


/* GETFULL */
static void
getfull()
{
	char *checkit;
	char *graboam;
	int start_rel;

	start_rel = 0;
	checkit=strtok(intflocn,PATHDEL);
	graboam = getenv(checkit);
	(void) sprintf(oampath,"%s",graboam);

	/* ignore the "menu" section of the EU path */
	checkit=strtok(NULL,PATHDEL);
	(void) sprintf(intfpath,"%s%s%s",oampath,PATHDEL,checkit);

	/* add the rest of the path for EU and grap the relative path too */
	while ((checkit=strtok(NULL,PATHDEL)) != NULL) {
		/* grap the relative path name for use with add-ons */
		if(! start_rel) {
			(void) sprintf(relpath,"%s%s",PATHDEL,checkit);
			(void) sprintf(exprpath,"%s",checkit);
			start_rel = 1;
		}
		else {
			(void) sprintf(relpath,"%s%s%s",relpath,PATHDEL,checkit);
			(void) sprintf(exprpath,"%s%s%s",exprpath,PATHDEL,checkit);
		}
	}
	start_rel = 0;
}	/* end of getfull routine */

/* GETLOCN */
static void
getlocn()
{
	char *checkit;
	char *oambase;
	char menupath[MLNSZ]; 

	checkit=strtok(locn,PATHDEL);
	oambase = getenv(checkit);
	(void) sprintf(menupath,"%s",oambase);

	if((checkit=strtok(NULL,PATHDEL)) != NULL) {
		(void) sprintf(menupath,"%s%s%s",menupath,PATHDEL,checkit);
	}

	/* add the rest of the path for EU */
	while ((checkit=strtok(NULL,PATHDEL)) != NULL) {
		(void) sprintf(menupath,"%s%s%s",menupath,PATHDEL,checkit);
	}

	(void) strcpy (pathname, menupath) ;
}	/* end of getlocn routine */


static int
menu_pick(mptrz, rcrsv)
struct item_def *mptrz;
int rcrsv;
{
	int fullmenu;		/* flag for full/empty menu */
	int errparse;

	fullmenu = 1;		/* initialize to full menu */
	errparse = 0;

	/* add selected menu's pkginst tag ifexists */
	if((mptrz->pkginsts) != NULL) 
	{
		/* parse out pkg inst tag(s) and add to linked list */
		(void) add_tags(mptrz->pkginsts);
	}

	errparse = parse_menus(mptrz);
	if(errparse)
	{
		warnuser = 1;
	}	/* warn user for lost integrity */
	(void) findx(look_menu,"");
	if(*exprpath == NULL)
	{
		(void) sprintf(lclpath,"%s",x_name);
	}
	else
	{
		(void) sprintf(lclpath,"%s%s%s",exprpath,PATHDEL,x_name);
	}
	(void) sprintf(abspath,"%s%s%s",intfpath,PATHDEL,exprpath);
	fullmenu = ckmn_open();

	/* okay for user to have "-r" and empty menu - will delete ok */
	if(! fullmenu)	/* empty menu */
	{
		/* use to warn user that may still have
		   - some express file entries
		   - pkg dependencies
		   NOTE: only applicable if"-r" used,
		         otherwise expect empty conditions 
		*/
		flagexpress = 1;
		flagpkgdpnd = 1;
	}	/* end of empty menu */

	if((fullmenu) && (! rcrsv)) 
	{
		/* ifthis menu is full and the "-r" option was not
		   used, then exit with an error code 
		*/
		catch_errs(NOTEMPTY);
		return(NOTEMPTY);
	}

	/* go to next node of menu for recursively searching & processing */
	look_menu=look_menu->next;
	(void) parse_all();
	 
	/* the menu requested and all sub-menus have been searched */

	/* set flag for user ifpkg dep. still exist */

	if(flaguser) 
	{ 
		/* still have pkg. dep.s */
		goahead = ask_user();
		if(! goahead) 
		{	
			/* user doesn't want to cont. */
			return (1);	/* user quits */
		}
	}


	return (0);

}	/* end of menu_pick */

/*
** WRITE_LOG (submenu)
**	This routine will 
**
** INPUT:
**      called by:              main 		(iptrz)
**				ckmn_open 	(subitem)
**
**      input parameters:       submenu 	struct item_def
**
** OUTPUT:
**      return type & value:    integer
**				iferror occurs, return -1
**				else return "SUCCESS" (0)
**
** GLOBAL:
**      variables used:         x_name
**				exprpath
**      variables changed:      none
*/
static int
write_log(submenu)		/* write to log file */
struct item_def *submenu;
{
	char outpbuf[BUFSIZ];			/* output buffer */
	char fullexpr[MLNSZ];			/* for full expr path */
	char exprlocn[MLNSZ];			/* parse out action */
	char expractn[MLNSZ];			/* parse out action */
	char addpath[MLNSZ];			/* parse out action */

	char * ptr;
	char *addaction;			/* submenu->maction */

	(void) sprintf(expractn,"%s","");		/* initialize array */
	(void) sprintf(addpath,"%s","");		/* initialize array */
	(void) sprintf(exprlocn,"%s","");		/* initialize array */

	addaction = submenu->maction;
	/* 
	   get exprpath and add both x_name and first part of "maction" to it 
	   NOTE: break apart maction for express file format 
	flagexpress = 1;		 warn user 
	*/

	if(*x_name == NULL)
	{
		(void) sprintf(exprlocn, "%s", exprpath);
	}
	else
	{
		(void) sprintf(exprlocn, "%s%s%s", exprpath, PATHDEL, x_name);
	}

	ptr = strrchr(addaction, PATHDELCHR);
	if(!ptr)
	{
		/* NO directory part - only a file name */
		(void) strcpy(expractn, addaction);
	}
	else
	{
		/* get part 1 of submenu->maction */
		(void) strncpy(addpath, addaction, ptr - addaction);
		addpath[ ptr - addaction ] = (char)0;
		(void) sprintf(fullexpr, "%s%s%s", exprlocn, PATHDEL, addpath);

		/* get part 2 of submenu->maction */
		(void) strncpy(expractn, ptr + 1, strlen(ptr + 1));
		expractn[ strlen (ptr + 1) ] = (char)0;
	}

	/* actually put into buffer for express file removal */
	(void) sprintf(outpbuf, "%s", submenu->mname);
	(void) sprintf(outpbuf, "%s^%s", outpbuf, fullexpr);
	(void) sprintf(outpbuf, "%s^%s", outpbuf, expractn);

	/* add in keywords for ie_build to delete - VOID */
	(void) sprintf(outpbuf,"%s^%s",outpbuf, VOIDDEL);

	(void) strcat(outpbuf, "\n");
	if(fputs(outpbuf, logfile) == EOF) {
		return(1);	/* was -1 */
	}
	return (0);
}	/* end of "write_log" (write to logfile) */

/*
** CKMN_OPEN ()
**	This routine will check the current menu by opening it,
**	and ifany entries are found will process these entries.
**	If the menu is empty, this routine exits without processing.
**	The processing includes:  adding the subitems to the
**	linked list ifthey too are .menu(s), adding the pkg
**	instance tags if(existing) to that file, and adding the
**	subitems to the file for express file deletion.
**
** INPUT:
**      called by:              main
**				parse_all
**
**      input parameters:       none
**
** OUTPUT:
**      return type & value:    int mflag
**				ifmenu is empty, return 0 
**				else return 1
**
** GLOBAL:
**      variables used:         x_name
**				abspath
**				look_menu	ptr to current structure 
**						within linked list
**      variables changed:      none
*/

int
ckmn_open()
{
	struct menu_file *recs_item;	/* for parsing sub menus */
	char curlocn[PATH_MAX];		/* for current menu opening */
	int mflag;			/* set flag for empty or full */
	int errparse;
	int errwrite;

	mflag = 0;			/* initialize to 0 for empty */
	errparse = 0;
	errwrite = 0;

	/* 
	   abspath = absolute path withing the interface structure for
		     item requested:
		/usr/sadm/sysadm/menu/applmgmt (for main:applications:msvr)
	   x_name = current relative path for this menu entry:
		msvr/db/index
	   curlocn = current menu location including all relative paths 
		     for entries w/in linked list:
		mxvr/db/index/index.menu
	*/

	(void) sprintf(curlocn,"%s%s%s",x_name,PATHDEL,look_menu->mwhat);
	(void) sprintf(menulocn,"%s%s%s",abspath,PATHDEL,curlocn);
	if((recs_item = input_menu(prog,menulocn, tempmenu)) == NULL) 
	{
		/* this menu is empty */
		mflag = 0;
	}
	else
	{
		if((recs_item->entries) == NULL)
		{
			/* ifthere are no entries, this menu is empty! */
			mflag = 0;
		}
		else 
		{
			/* menu is not empty */
			mflag = 1;
			subitem = recs_item->entries;
			while (subitem != NULL) 
			{
				subactn = subitem->maction;
				if(ismenu(subactn)) 
				{
					/* then .menu, add to search list */
					errparse = parse_menus(subitem);
					if(errparse)
					{
						warnuser = 1;
					}
				}

				/* add to list of pkg insts tags */
				if((subitem->pkginsts) != NULL) 
				{
					/* add tag(s) to linked list */
					(void) add_tags(subitem->pkginsts);
				}

				/* write menu items */
				errwrite = write_log(subitem);
				if(errwrite)
				{
					warnuser = 1;
				}

				subitem = subitem->next;

			}	/* end of while subitem(s) exist  */
		}	/* end of processing this menu's entries */
	}	/* end of processing this menu */

	return(mflag);
}	/* end of ckmn_open */

/*
** PARSE_MENUS ()
**	This routine will create and add to a linked list of .menus.
**	This linked list will supply the keyword, the directory that
**	the item belongs in, the "action", the "mom" directory (a ptr
**	to the linked list item that has this item as an entry), and
**	the "next" linked list item ptr.
**	This routine will supply the ptr(s) for the first item, the
**	"currently being looked at" item, and the last item added for
**	this linked list.
**
** INPUT:
**      called by:              main (iptrz)
**				ckmn_open (subitem)
**
**      input parameters:       submenu	- struct item_def
**
** OUTPUT:
**      return type & value:    integer 
**				iflinked list node created successfully,
**				returns "SUCCESS"
**				else returns -1
**
** GLOBAL:			first_menu	Ptr to first node added
**						within linked list
**				last_menu	Ptr for last added node
**						within linked list
**				look_menu	Ptr to current node 
**						within linked list
**
**      variables changed:      last_menu
*/
static int
parse_menus(submenu)
struct item_def *submenu;
{
	register struct search_menus *this_menu;
	register char *submnm;
	register char *submact;
	register char *ptr;

	if((this_menu = (struct search_menus *) calloc(1,sizeof(struct search_menus))) == NULL)  
		return (1);	/* was -1 */

	(void) strncpy((char *)this_menu, "", sizeof(struct search_menus));

	submnm = submenu->mname;
	submact = submenu->maction;

	(void) strncpy(this_menu->mkywd, submnm, strlen(submnm));

	ptr = strrchr(submact, PATHDELCHR);
	if(!ptr) 
	{
		/* NO directory part - only a file name */
		(void) strcpy(this_menu->mwhat, submact);
		
	} 
	else 
	{
		(void) strncpy(this_menu->mwhere, submact, ptr - submact);
		this_menu->mwhere[ ptr - submact ] = (char)0;

		/* (void) strcpy(this_menu->mwhat, ptr + 1); */
		(void) strncpy(this_menu->mwhat, ptr + 1, strlen(ptr + 1));
		this_menu->mwhat[ strlen (ptr + 1) + 1 ] = (char)0;
	}

	this_menu->mom = look_menu;

	if(init_menu) {
		/* this is the first item in the linked list */
		init_menu = 0;
		first_menu = this_menu;
		look_menu = this_menu;
	}
	else {
		/* this is for subsequent items in the linked list */
		last_menu->next = this_menu;
	}

	last_menu = this_menu;
	last_menu->next = NULL;

	return(SUCCESS);

}	/* end of parse_menus */

/* PARSE_ALL */
static int
parse_all()			/* parse recursively thru .menu(s) */
{
	int fullmenu;

	while (look_menu != NULL) {
		/* get absolute relative path name by recursion */
		(void) findx(look_menu,"");
		fullmenu = ckmn_open();
		if(!fullmenu)
		{
			flagexpress = 1;
			flagpkgdpnd = 1;
		}

		look_menu = look_menu->next;


	}
	return (0);
}		/* end of parse_all */

/* FINDX */
static int
findx(thing_menu, catstr)
struct search_menus *thing_menu;
char *catstr;
{
	char tmpstr[MLNSZ];

	(void) sprintf(tmpstr,"%s%s%s",PATHDEL,thing_menu->mwhere, catstr);

	if(thing_menu->mom == 0)
	{
		/* ifit's a null, then stop recursively adding to path */
		(void) strcpy(x_name, tmpstr+1);
		/* +1 because don't need first path delimeter - / */
		return(0);
	}
	else
	{
		/* keep recursing backward through linked list to get
		   absolute relative pathname! */
		(void) findx(thing_menu->mom, tmpstr);
	}

	return(0);
}		/* end of findx */

/* ASK_USER */

/*	int ask_user 
 *
 *	asks user about continue w/ pkg dependencies
 *	
 *	returns 1 ifuser wants to continue
 *		0 ifuser does not want to continue
 */
static int
ask_user()		
{
	char ans[80] ;

	(void) fprintf(stdout,"The following installed package(s) depend on %s\n\n",saveitem);

	(void) read_tags();	/* add pkg inst tags */

	(void) fprintf(stdout,"\nWARNING: ");
	(void) fprintf(stdout,"Removing %s\nusing this command is NOT recommended ",saveitem);
	(void) fprintf(stdout,"except by removing the related package(s).\n\n");

	(void) fprintf(stdout,"Do you wish to continue [yes/no]?  ");
	(void) scanf ("%3s", ans);

	if((ans[0] == 'y')  || (ans[0] == 'Y')) {
		return (1);
	}

	return (0);
}		/* end  of ask_user */

/* ITEM_GONE */
/*	item_gone
 *
 *	hdr_ptr		structure of menu_line for menu file header lines 
 *	file		file pointer for file to write to
 *	chng_item	menu item to change in temporary parent menu file
 *
 *	returns
 *
 */
static int
item_gone(menu_ptr, file, chng_item)
struct menu_file *menu_ptr;		/* menu structure to write */
FILE *file;
struct item_def *chng_item;		/* menu item to change */
{
	struct menu_line *hdr_ptr;		/* menu structure to write */
	struct item_def  *xfer_item;		/* menu structure to write */
	char outbuf[BUFSIZ];		/* output buffer */
	int sameitem; 		/* for comparing mnames of item definitions */

	sameitem = 0;		/* set flag false for same item */
	vacant = 1;		/* set flag true for empty menu file */
	hdr_ptr = menu_ptr->head;
	xfer_item = menu_ptr->entries;

	/* write header */

	if(write_item(hdr_ptr, file) != 0) {
		return(1);	/* was -1 */
	}

	/* write menu items */

	while(xfer_item != NULL) {
		if(strncmp(xfer_item->mname, chng_item->mname, 
			     strlen(xfer_item->mname)) == 0) {
			sameitem = 1;
		}
		else {
			sameitem = 0;
		}
		if(sameitem) {	/* skip ifnot placeholder,
					   copy limited ifplaceholder */
			if((xfer_item->pholder == P_INACTIVE) ||  
		     	     (xfer_item->pholder == P_ACTIVE)) {
				(void) sprintf(outbuf, "%s^%s^", 
					       xfer_item->mname, 
					       xfer_item->mdescr);
				(void) strcat(outbuf, "placeholder^");
				(void) strcat(outbuf, xfer_item->maction);
				(void) strcat(outbuf, "\n");
				if(fputs(outbuf, file) == EOF) return(1); /* was -1*/
				vacant = 0;	/* parent menu has placeholder item */
			}		/* end of copy only placeholder info */
			/* else skip this entry */

		}	/* end of ifsame item */

		else { 		/* not same item, so copy */
			(void) sprintf(outbuf, "%s^%s^", xfer_item->mname, 
					xfer_item->mdescr);
			if(xfer_item->pholder == P_INACTIVE) {
				(void) strcat(outbuf, PHOLDER);
				(void) strcat(outbuf, TAB_DELIMIT);
			}
			(void) strcat(outbuf, xfer_item->maction);
			if(xfer_item->pholder == P_ACTIVE) {
				(void) strcat(outbuf, TAB_DELIMIT);
				(void) strcat(outbuf, PHOLDER);
			}
		
			if(*(xfer_item->orig_name) != NULL) {
				(void) strcat(outbuf, TAB_DELIMIT);
				(void) strcat(outbuf, "[");
				(void) strcat(outbuf, xfer_item->orig_name);
				(void) strcat(outbuf, "]");
			}
			if(*(xfer_item->pkginsts) != NULL) {
				(void) strcat(outbuf, TAB_DELIMIT);
				(void) strcat(outbuf, xfer_item->pkginsts);
			}
	
			(void) strcat(outbuf, "\n");
			if(fputs(outbuf, file) == EOF) return(1);	/* was -1*/
			vacant = 0;	/* set flag false - parent menu not empty */
		}		/* end of not same item */

		xfer_item = xfer_item->next;
	}	/* end of while still items */

	return(0);

}	/* end of item_gone */

static void
catch_errs(errcode)
int errcode;
{
	switch (errcode) {
		case SYNTAX :	 /*	for usage display 	*/
			(void) fprintf(stdout,"\tUX delsysadm: invalid syntax.\n");
			(void) fprintf(stdout,"\tUsage: delsysadm task\n");
			(void) fprintf(stdout,"\t       delsysadm [-r] menu\n");
			break;
		case NOTEXIST :	 /*	for menu/task not found */
			(void) fprintf(stdout, "\tUX delsysadm: ERROR: %s not found.\n",saveitem);
			break;
		case NOTEMPTY :	 /*	for menu not empty and -r not used */
			(void) fprintf(stdout, "\tUX delsysadm: %s is not empty.\n",saveitem);
			break;
		case NOUPDATE :	 /*	for updating interface problems */
			(void) fprintf(stdout, "\tUX delsysadm: unable to update interface menu structure.\n");
			break;
		case REMPTY :	 /*	for parent menu empty */
			(void) fprintf(stdout, "\tUX delsysadm: nothing to delete.\n");
			break;
		case USERWARN :
			(void) fprintf(stdout,"\tUX delsysadm: changes to menu or submenu may be incomplete.\n");
			break;
		case XPRESSWARN :
			(void) fprintf(stdout,"\tUX delsysadm: changes to the express file may be incomplete.\n");
			break;
		case PKGTAGWARN :
			(void) fprintf(stdout,"\tUX delsysadm: changes to packageinstance tag(s) may be incomplete.\n");
			break;
		case DIRWARN :
			(void) fprintf(stdout,"\tUX delsysadm: changes to directory(s) may be incomplete.\n");
			break;
		case NOTROOT :
			(void) fprintf(stdout,"\tUX delsysadm: must be root to execute.\n");
			break;
		case SUCCESS :
		default :
			break;
	}
}	/* end of catch_errs */
/*
** temp_files() 
**
**	GLOBAL:		pathname
**			(need to keep temp files in same filesystem !)
*/
static int
temp_files(forname)
char *forname;
{
	(void) sprintf(tempmenu,"%s/del.%s",pathname, forname);
	(void) sprintf(exprlog,"%s/expr.%s",pathname, forname);
	(void) sprintf(chngmenu,"%s/chngm.%s",pathname, forname);
	(void) sprintf(uplvlmenu,"%s/uplvl.%s",pathname, forname);
	return (0) ;
}	/* end of temp_files */

/*
**	add_tags
*/
static int
add_tags(gtagged)
char *gtagged;
{
	char *grabtag;
	int errtags;

	errtags = 0;
	if(*gtagged != NULL)
	{
		/* parse for >= 1 tag */
		grabtag = strtok(gtagged, PTAGDEL);
		while (*grabtag != NULL)
		{
			if(strncmp(grabtag, PARSEDEL, strlen(PARSEDEL)) != 0)
			{
				/* add to linked list */
				errtags = link_tags(grabtag);
				if(errtags)
				{
					flagpkgdpnd = 1;
				}
			}	/* end of ifnot the "^" */
			grabtag = strtok(NULL, PTAGDEL);
		}	/* end of while still tags */ 
	}	/* end of iftag(s) available */
	return (0);
}	/* end of add_tag */

/* 
** remove_dirs 
*/
static int
remove_dirs(userpick,taskpick)
struct item_def *userpick;
int taskpick;
{
	char srmcall[PATH_MAX];		/* set up 'system' call */
	char *tagged;
	char *grabtag;
	char partdir[MLNSZ];
	char taskdir[MLNSZ];
	char *partx;
	char *ptract;
	int numerrs;
	int syserrs;

	numerrs = 0;	/* no errors to start with */
	syserrs = 0;
	tagged = userpick->pkginsts;
	if(!taskpick)
	{
		/* get  "menu"'s directory(s) to remove */
		check_tag = first_tag;
		while (check_tag != NULL)
		{
			/* has nodes in linked list */
			(void) sprintf(fulldir,"%s%s%s%s%s%s%s",oampath,PATHDEL,ADDONS,PATHDEL,check_tag->tag,PATHDEL,lclpath);
			(void) sprintf(srmcall,"rm -rf %s",fulldir);
			syserrs = system(srmcall);
			if(syserrs < 1)
			{
				numerrs = 1;
			}
			check_tag = check_tag->next;
		}	/* end of grabbing add-on directory(s) */

		/* need to also remove main directory */
		(void) sprintf(fulldir,"%s%s%s",intfpath,PATHDEL,lclpath);

		(void) sprintf(srmcall,"rm -rf %s",fulldir);
		syserrs = system(srmcall);
		if(syserrs < 1)
		{
			numerrs = 1;
		}
	}	/* end of ifit's a menu */
	else
	{
		/* only ONE directory to remove for a task */

		partx = userpick->maction;
		ptract = strrchr(partx,PATHDELCHR);
		if(!ptract)
		{
			/* NO directory part so ignore */
			(void) sprintf(taskdir,"%s","");
		}
		else
		{
			/* get part 1 of userpick->maction for task directory */
			(void) strncpy(partdir,partx, ptract - partx);
			partdir[ ptract - partx ] = (char)0;
			(void) sprintf(taskdir,"%s%s",PATHDEL,partdir);
		}

		if(*tagged  != NULL)
		{
			/* a task can have only ONE tag ! */
			grabtag = strtok(tagged, PTAGDEL);
			(void) sprintf(fulldir,"%s%s%s%s%s%s%s",oampath,PATHDEL,ADDONS,PATHDEL,grabtag,relpath,taskdir);
		}
		else
		{
			/* use main interface directory structure */
			(void) sprintf(fulldir,"%s%s",pathname,taskdir);
		}
		(void) sprintf(srmcall,"rm -rf %s",fulldir);
		syserrs = system(srmcall);
		if(syserrs < 1)
		{
			numerrs = 1;
		}
	}	/* end ifelse it's a task ! */

	return (numerrs);
}	/* end of remove_dirs */

/*
** link_tags()
*/
static int
link_tags(passtag)
char *passtag;
{
	struct dfine_tags *this_tag;
	char *subtag;
	int added;		/* flag for tag added to linked list */
	int cmptag;		/* flag for comparing two tags in linked list */

	added = 0;
	cmptag = 0;
	last_tag = NULL;
	subtag = passtag;

	if((this_tag = (struct dfine_tags *) calloc(1,sizeof(struct dfine_tags))) == NULL)  
		return (1);	/* was -1*/

	(void) strncpy((char *)this_tag, "", sizeof(struct dfine_tags));

	(void) strncpy(this_tag->tag, subtag, strlen(subtag));
	this_tag->next = NULL;

	if(strncmp(this_tag->tag, ONLINE, strlen(ONLINE)) != 0)
	{
		/* the ONLINE tag needs to be in linked list, but flag
		   for asking user about other pkg insts dependencies
		*/
		flaguser = 1;
	}

	if(first_tag == NULL)
	{
		first_tag = this_tag;
	}	/* end of setting up linked list for first time */
	else
	{
		/* check out for same tag (ifsame - ignore), add in SORTED order */
		cur_tag = first_tag;
		last_tag = first_tag;
		while (! added)
		{
			cmptag = strncmp(this_tag->tag, cur_tag->tag, strlen(this_tag->tag));
			if(cmptag == 0)
			{
				/* they are equal */
				added = 1;
			}	/* end of tags are the same */
			else if(cmptag < 0)
			{
				/* this_tag is < (before) cur_tag */
				this_tag->next = cur_tag;
				if(cur_tag == first_tag)
				{
					/* first node in list - need to reassign */
					first_tag = this_tag;
				}
				if(last_tag != cur_tag)
				{
					/* not first node - add to list in the middle by knowing 
					   where the last node looked at points to */
					last_tag->next = this_tag;
				}
				added = 1;
			} 	/* end of this tag getting put before current tag */
			else if(cmptag > 0)
			{
				/* add after the current node - but remember to check 
				   the rest of the list first ! */
				if(cur_tag->next == NULL)
				{
					/* at end of list - add it */
					cur_tag->next = this_tag;
					added = 1;
				}
				else
				{
					/* update current pointer and go to next node */
					last_tag = cur_tag;
					cur_tag = cur_tag->next;
				}
			}	/* end of this tag comes after current tag */
			else
			{
				/* garbage - ignore ! */
				added = 1;
			}	/* end of not one of the above */
		}	/* end of while adding into linked list */
	}	/* end of else this is not the first node */
	return(0);
}	/* end of link_tags */

/*
**	read_tags()
*/
static int
read_tags()
{
	check_tag = first_tag;
	while (check_tag != NULL)
	{
		/* has nodes in linked list */
		if((strncmp(check_tag->tag, ONLINE, strlen(ONLINE))) != 0)
		{
		(void) fprintf(stdout, "\t\t%s\n",check_tag->tag);
		}	/* end of not an ONLINE tag */
		check_tag = check_tag->next;
	}	/* end of while */
	return (0);
}	/* end of read_tags */

/*
**	chng_express()
**
**	Changes express file by removing entries created and added
**	to "/tmp/expr.{item}"
**
**	GLOBAL:		exprlog
**			logfile
*/

static int
chng_express()
{
	pid_t kid;				/* process id for fork */
	int oksort;				/* flag for sort went ok */
	char sortcall[MLNSZ];			/* string for system call */
	char *tempsort;				/* ptr for temp file name */

	tempsort = tmpnam(NULL); 		/* create temp file for sort */
	(void) sprintf(sortcall,"sort -u %s > %s",exprlog, tempsort);
	oksort = system(sortcall);	/* sort & unique express file changes */
	if(oksort < 0)
	{
		/* sort had errors */
		(void) unlink (tempsort);
		return(-1);	
	}	/* end of error with sort */

	kid=fork();
	switch (kid)
	{
	case -1:
		/* Fork failed */
		(void) unlink (tempsort);
		return (-1);
	case 0:
		/* child process */
		(void) execlp (IE_BUILD, SHORT_MOD, (char *) 0);
		/* perror ("exec in child");
		*/
		exit (-2); 			/* child must die */
	default:
		/* parent process */
		/* printf(" parent ready to wait.\n"); */
		(void) wait (&kid);
		/* printf (" parent done waiting kid was equal to %d\n",kid>>8); */
	}	/* end of switch */
	
	/* still the parent process */

	(void) unlink (tempsort);

	return (0);

}	/* end of chng_express */

/*
**	parnt_xfer()
**
**	This will move the parent menu to a "safe" copy for the time
**	and will move the temporary menu (with appropriate changes)
**	into the parent menu and ifall is ok, the "safe" copy will
**	be removed - ifan error occurs, the "safe" copy goes back
**	to being the real version.
*/

static int
parnt_xfer(real, newreal)
char *real;			/* original parent menu */
char *newreal;			/* original tmp file, becomes new parent menu */
{
	char *old_real;		/* temp file for moving original */
	int errlink;		/* for linking & unlinking - check for error */

	errlink = 0;			/* initialize flag */

	old_real = tempnam(pathname, NULL);	/* create a temp file */
	errlink = unlink(old_real);	/* just for safety */

	errlink = link(real, old_real);	/* copy original to tmp */
	if(errlink < 0)
	{
		/* error with linking original parent to tmp file */
		return(-1);
	}
	errlink = unlink(real);		/* removes old parent */
	if(errlink < 0)
	{
		/* error with unlinking original parent file */
		return(-1);
	}
	errlink = link(newreal,real);	/* makes temp file the new parent */
	if(errlink < 0)
	{
		/* error with linking original temp to parent file */
		(void) link(old_real, real);	/* back to orig menu */
		return(-1);
	}
	errlink = unlink(newreal);	/* get rid of original temp file */
	if(errlink < 0)
	{
		/* error with unlinking original temp file */
		return(-1);
	}

	return(0);
}	/* end of parnt_xfer */
/*
**	parnt_travel
**
**	GLOBAL:		saveitem
**			prog
*/
static int
parnt_travel()
{
	struct menu_item *pitemz;
	struct menu_file *parnt_recs;
	struct item_def *piptrz;
	char parntlocn[PATH_MAX];
	char parntpath[PATH_MAX];
	char parntmenu[PATH_MAX];
	char parntgot[PATH_MAX];
	char dropitem[PATH_MAX];
	char getparent[PATH_MAX];
	register char *findparent;
	register char *pptr;
	int errplace;			/* flag error with xfering files */
	int errgone;

	errplace = 0;			/* init to false */
	(void) strncpy(dropitem, saveitem, strlen(saveitem)); 
	pptr = strrchr(dropitem, LVLDEL);
	if(pptr)
	{
		(void) strncpy(getparent, dropitem, pptr - dropitem);
		getparent[ pptr -dropitem ] = (char)0;
	}
	else 
	{
		/* ignore this whole endevour ! */
		return (1);
	}

	findparent = getparent;

	if((pitemz = find_menu(prog, findparent)) != NULL) {
		(void) strcpy(parntlocn, pitemz->path);
		/*getlocn(parntlocn, parntpath); */
		getlocn(); 

		(void) sprintf(parntmenu,"%s%s%s",parntpath,PATHDEL,pitemz->menu_name);

		(void) strcpy(parntgot, pitemz->item);

	}
	else {			
		/* find_menu returned NULL, exit without modifying parent's parent */
		return(1);
	}

	/*	Check for Empty Parent Menu Name	*/
	if((pitemz->menu_name == 0) || (pitemz->exists == 0)) {
		return(1);
	}
	/*	Check for empty parent menu 		*/
	if((parnt_recs = input_menu(prog, parntmenu, tempmenu)) == NULL) {
		/*	no menu entries, parent's parent menu is empty	*/
		return(1);
	}

	/* each of these has a separate structure format */
	piptrz = parnt_recs->entries;

	if(piptrz == NULL) {
		/* no entries available for parent's parent menu */
		return(1);
	}
	else {		/* entries available in parent's parent menu */			
		while (strncmp(parntgot, piptrz->mname,strlen(parntgot)) != 0 
			&& (piptrz != NULL)) {
			piptrz = piptrz->next;
		/* this section provides :
			- comparison of parntgot & item ptr's's menu entry name
			- piptrz->mname = requested menu 
				config for config.menu (task)
		*/
		}
		if(piptrz == NULL) {
			return(1);
		}
		else {
			uplvlptr = fopen(uplvlmenu, "w");
			errgone = parnt_gone(parnt_recs, uplvlptr, piptrz);
		}	/* end of else grab parent info from parent's parent */

	}	/* end of else entries available */

	(void) fclose(uplvlptr);	/* close changing parent menu */
	errplace = 0;			/* to initialize */
	errplace = parnt_xfer(parntmenu, uplvlmenu);
	if((errplace) || (errgone))
	{
		return (1) ;
	}	/* end of errors in xfering parent's parent menu info. */

	return (0);
}	/* end of parnt_travel */

/* PARNT_GONE */
/*	parnt_gone
 *
 *	hdr_ptr		structure of menu_line for menu file header lines 
 *	file		file pointer for file to write to
 *	chng_item	menu item to change in temporary parent menu file
 *
 *	returns
 *
 */
static int
parnt_gone(menu_ptr, file, chng_item)
struct menu_file *menu_ptr;		/* menu structure to write */
FILE *file;
struct item_def *chng_item;		/* menu item to change */
{
	struct menu_line *hdr_ptr;		/* menu structure to write */
	struct item_def  *xfer_item;		/* menu structure to write */
	char outbuf[BUFSIZ];		/* output buffer */
	int sameitem; 		/* for comparing mnames of item definitions */

	sameitem = 0;		/* init to not same items */
	hdr_ptr = menu_ptr->head;
	xfer_item = menu_ptr->entries;

	/* write header */

	if(write_item(hdr_ptr, file) != 0) {
		return(-1);
	}

	/* write menu items */

	while(xfer_item != NULL) {
		if(strncmp(xfer_item->mname, chng_item->mname, 
			     strlen(xfer_item->mname)) == 0) {
			sameitem = 1;
		}
		else {
			sameitem = 0;
		}
		if((sameitem) && (vacant)) 
		{	
			/* skip ifnot placeholder,
					   copy limited ifplaceholder */
			if((xfer_item->pholder == P_INACTIVE) ||  
		     	     (xfer_item->pholder == P_ACTIVE)) {
				(void) sprintf(outbuf, "%s^%s^", 
					       xfer_item->mname, 
					       xfer_item->mdescr);
				(void) strcat(outbuf, "placeholder^");
				(void) strcat(outbuf, xfer_item->maction);
				(void) strcat(outbuf, "\n");
				if(fputs(outbuf, file) == EOF) return(-1);
			}		/* end of copy only placeholder info */

		}	/* end of ifsame item */

		else { 		/* not same item,  or not empty, so copy */
			(void) sprintf(outbuf, "%s^%s^", xfer_item->mname, 
					xfer_item->mdescr);
			if(xfer_item->pholder == P_INACTIVE) {
				(void) strcat(outbuf, PHOLDER);
				(void) strcat(outbuf, TAB_DELIMIT);
			}
			(void) strcat(outbuf, xfer_item->maction);
			if(xfer_item->pholder == P_ACTIVE) {
				(void) strcat(outbuf, TAB_DELIMIT);
				(void) strcat(outbuf, PHOLDER);
			}
		
			if(*(xfer_item->orig_name) != NULL) {
				(void) strcat(outbuf, TAB_DELIMIT);
				(void) strcat(outbuf, "[");
				(void) strcat(outbuf, xfer_item->orig_name);
				(void) strcat(outbuf, "]");
			}
			if(*(xfer_item->pkginsts) != NULL) {
				(void) strcat(outbuf, TAB_DELIMIT);
				/* FUTURE CHANGE*/
				(void) strcat(outbuf, xfer_item->pkginsts);
			}
	
			(void) strcat(outbuf, "\n");
			if(fputs(outbuf, file) == EOF) return(-1);
		}		/* end of not same item */

		xfer_item = xfer_item->next;
	}	/* end of while still items */

	return(0);

}	/* end of parnt_gone */
main(argc, argv)
int argc;
char *argv[];
{
	/* main declarations */
	struct menu_item *mitemz;
	struct menu_file *recs_menu;	/* ptr to menu file structure */
	struct item_def *iptrz;
	extern int optind;	/* getopt */
	int opt;		/* return from getopt */
	int stopactn; 		/* stop processing of menu per user  */
	int errexpress;		/* express file changes  - 
				   0=no error, 1=error */
	int errxfer;		/* parent menu xfer - 
				   0=no error, 1=error */
	int errparnt;		/* parent's parent menu check -
				   0=no error, 1=error */
	int argitem = 0;	/* argv item count for find_menu */
	int ismenuobj = 0 ;	/* flag if is a menu object */

	char *itemactn;
	char gotitem[MLNSZ];
	char menufile[MLNSZ];

	flagexpress = 0;
	flagpkgdpnd = 0;
	flagparnt = 0;
	errwrite = 0;
	errrmv = 0;
	errigone = 0;
	warnuser = 0;
	flagdirs = 0;
	init_menu = 1;
	last_menu = NULL;
	look_menu = NULL;

	first_tag = NULL;
	last_tag = NULL;

	/* Must be root to execute delsysadm */

	if ( geteuid() !=  0 )
	{
		/* not root, cannot continue */
		catch_errs(NOTROOT);
		return(NOTROOT);
	}

	/* MODIFYING ENVIRONMENT VARIABLE !  - 
	 * need to make it a #define */
	(void) putenv("OAMBASE=/usr/sadm/sysadm");
	/* to initialize array */
	(void) sprintf(x_name,"%s","");	

	/* Process command line options */
	while ((opt = getopt(argc, argv, "r:?")) != EOF) {
		switch (opt) {
			/* recursive option - for menu only */
			case 'r' :
				rflag=1;
				break;
			case '?' :
				catch_errs(SYNTAX);
				return(SYNTAX);
			default :
				catch_errs(SYNTAX);
				return(SYNTAX);
		}
	}
	
	if((	optind == 1) && (argc != 2) && (! rflag)) {
			catch_errs(SYNTAX);
			return(SYNTAX);
	}

	argitem = argc - 1;
	prog = argv[0];

	/*	Argument Parsing	*/

	(void) strcpy(saveitem,argv[argitem]);
	if((mitemz = find_menu(argv[0], argv[argitem])) != NULL) {
		(void) strcpy(locn, mitemz->path);
		(void) strcpy(intflocn, mitemz->path);
		getfull();		/* to get full dir. structure */
		getlocn();

		(void) sprintf(menufile,"%s%s%s",pathname,PATHDEL,mitemz->menu_name);

		(void) strcpy(gotitem, mitemz->item);
	/* this section will provide:
	 *	locn = OAMBASE
	 *	pathname = parent menu dir
	 *		   /usr/sadm/sysadm/menu/applmgmt/msvr/db/index
	 *	menufile = parent menu
	 *	 /usr/sadm/sysadm/menu/applmgmt/msvr/db/index/index.menu
	 *	mitemz->menu_name = parent menu only (no dir path)
	 *		   index.menu
	 *	mitemz->item = requested menu or task
	 *		   delind (task)
	 *	gotitem = mitemz->item
	*/
		/* create file names for temp files that might be used */
		(void) temp_files(gotitem);

	}
	else {			
		/* find_menu returned NULL, exit with error code = 3 */
		catch_errs(NOTEXIST);
		return(NOTEXIST);
	}

	/*	Check for Empty Menu or Task Name	*/
	if((mitemz->menu_name == 0) || (mitemz->exists == 0)) {
		catch_errs(NOTEXIST);
		return(NOTEXIST);
	}
	/*	Check for empty menu 		*/
	if((recs_menu = input_menu(prog, menufile, tempmenu)) == NULL) {
		/*	no menu entries, parent menu is empty	*/
		catch_errs(REMPTY);
		return(REMPTY);
	}

	/* each of these has a separate structure format */
	iptrz = recs_menu->entries;

	if(iptrz == NULL) {
		/* no entries available for menu/task */
		catch_errs(NOTEXIST);
		return(NOTEXIST);
	}
	else {		/* entries available in parent menu */			
		while (strncmp(gotitem, iptrz->mname,strlen(gotitem)) != 0 
			&& (iptrz != NULL)) {
			iptrz = iptrz->next;
		/* use this section to get PKGINSTS for parent for FUTURE CHANGE */
		/* this section provides :
			comparison of gotitem and item pointer's menu entry name

			iptrz->mname = requested menu or task
				delind (task)
		*/
		}
		if(iptrz == NULL) {
			catch_errs(NOTEXIST);
			return(NOTEXIST);
		}
		else {
			itemactn = iptrz->maction;
			/* this section provides :
			 *	iptrz->maction = 
			 * 	requested menu/task action
			 *		delind/Form.delind (task)
			 *	itemactn = iptrz->maction
			*/

			/* open log file to keep track of express file changes
			 * (deletions) needed, write first entry (requested
			 * menu or task) to log file - ifa task, this is the
			 * only entry 
			 */
			logfile = fopen(exprlog, "w");
			errwrite = write_log(iptrz);
			if(errwrite)
			{
				warnuser = 1;
			}	/* warn user */

			/* Check for action being a ".menu"
			 * or Form., Text., Menu. or other object 
	 		 */
			ismenuobj = ismenu(itemactn);
			if(ismenuobj) 
			{
				/*
				 * it's a menu: 
			   	 * process through menu selected, check for 
			   	 * empty/non-empty status, create file of 
			   	 * items to remove from the express file
			   	 * & start parsing for pkg dependencies.
				 */

				stopactn = menu_pick(iptrz, rflag);
				if(stopactn)
				{
					/* user chose not to continue */
					catch_errs(SUCCESS);
					return(SUCCESS);
				}

				/* remove directories */
				dotask = 0;
				errrmv = remove_dirs(iptrz, dotask);
				if(errrmv)
				{
					flagdirs = 1;
				}	/* problems removing directories */
			
			}	/* end of .menu object processing */
			else 
			{	
				/* For task processing - not a .menu */
				if(rflag) {
					catch_errs(SYNTAX);
					return(SYNTAX);
				}

				/* remove directory */
				dotask = 1;
				(void) remove_dirs(iptrz, dotask);

			}	/* end of task object processing */

		} /* end of item is correct one in parent menu */

		/* close temp files */
		(void) fclose(logfile);

		/* make express file changes */
		errexpress = 0;			/* to initialize */
		errexpress = chng_express();
		if(errexpress)
		{
			/* 
			   errors occurred in express file changes
			   warn user 
			*/
			flagexpress = 1;
		}	/* end of errors in changing express file */

		/* open temp menu file for changes to parent menu */
		chngptr = fopen(chngmenu, "w");
		errigone = item_gone(recs_menu, chngptr, iptrz);
		if(errigone)
		{
			warnuser = 1;
		}	/* warn user that structure has lost its integrity */
		
		(void) fclose(chngptr);	/* close changing parent menu */
		errxfer = 0;			/* to initialize */
		errxfer = parnt_xfer(menufile, chngmenu);
		if(errxfer)
		{
			/* errors occurred in xfering parent menu changes */
			warnuser = 1;
		}	/* end of errors in xfering parent menu info. */
		 
		if(vacant)
		{
			errparnt = parnt_travel();
			if(errparnt)
			{
				warnuser = 1;
			}	/* end of iferror, flag that xfer problems */
		}	/* end of resetting parent's parent menu 
			 * if parent empty */
	} /* end of else entry available in parent menu */

	catch_errs(SUCCESS);
	return(SUCCESS);
}	/* END OF MAIN */
