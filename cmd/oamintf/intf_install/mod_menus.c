/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident  "@(#)oamintf:intf_install/mod_menus.c	1.6.3.1"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include "intf.h"
#include "menu_io.h"
#include "inst_err.h"

/*
 * mod_menus modifies OAM Interface ".menu" files based on information
 * in input file.  Input file must be in ".mif" format:
 *    path^description^action
 *
 * where path is an OAM Interface logical pathname.  Examples:
 *    main:devices^Storage Device Management^
 *    main:devices:copy^Storage Device Copy^Form.copy
 *
 * description is the description to appear beside the menu name
 * action is the action that takes place.
 */

extern char	*getenv(),
		*menutok(),
		*read_item();
extern void	*calloc(), 
		exit(),
		free_menu(),
		inst_err();
extern int	atoi(),
		getopt(),
		putenv(),
		mk_tdir(),
		output_menu(),
		mk_dir();
extern struct menu_item *find_menu();
extern struct menu_file *input_menu();


static int oflag;		/* set if "-o" option on command line */
static int tflag;		/* set if "-t" option on command line */
static FILE *miffile;			/* mif file pointer */
static FILE *logfile;			/* file pointer of log file */
static FILE *exprfile;			/* file ptr for express mode log */

static char cur_path[PATH_MAX];		/* current physical path */
static char cur_menu[PATH_MAX];		/* current menu file */
static char tmp_menu[PATH_MAX];		/* temp menu file */
static char new_dir[PATH_MAX];		/* new directory to create */
static char new_menu[PATH_MAX];		/* new menu to create */
static char new_tmp[PATH_MAX];		/* new tmp menu name for placeholder */
static char outstr[PATH_MAX * 2];	/* output line for new entries */
static char oam_env[LNSZ];		/* for reset OAMBASE in env for tflag */
static char *prog;			/* calling program name */
static char *path;		/* pointer to .mif entry path */
static char *descr;		/* ptr to .mif entry description */
static char *action;		/* ptr to .mif entry action */
static char *pkginst;		/* package instance from install feature */
static char *nil = "";		/* NULL string */

static struct item_def 
		*find_last();
static void	new_name(), add_pkg(),
		log_chgs(), log_expr();
static int	write_new(), process_item(), 
		merge_menu(), compare_item(),
		srch_pkg(), proc_ph(),
		new_desc(), find_mif();

main(argc, argv)
int argc;
char **argv;
{
	extern char *optarg;		/* used by getopt */
	extern int optind, opterr;	/* used by getopt */
	struct menu_item *m_item;	/* return from find_menu() */
	struct menu_file *file_str;	/* return from input_menu() */
	struct item_def *itemptr;	/* pointer to menu item */
	struct item_def *previtem;	/* pointer to previous menu item */
	char instr[LNSZ];	/* input line from miffile */
	char *oambase;		/* interface base directory */
	char *cur_dir;		/* pointer to position in current dir. */
	char *log_file;		/* pointer to log file name */
	char *mi_file;		/* pointer to .mi file name */
	char *exp_file;		/* pointer to express log file name */
	char *pos;		/* position in action string to see if menu */
	int ch;			/* return from getopt */
	int filechanged;	/* log if changes made to .menu file */
	int errflg;		/* flag if getopt errors found */
	int errors;		/* keeps track of how many errors found */
	int opt;		/* set if option present */

	prog = argv[0];

	oflag = tflag = 0;		/* init */
	opterr = errflg = 0;		/* no print errors from getopt */

	while((ch = getopt(argc, argv, "ot")) != EOF)  {
		switch(ch) {
			case 'o':
				oflag++;
				break;
			case 't':
				tflag++;
				break;
			case '?':
				errflg++;
		}
		opt = 1;
	}

	if(errflg || ((!opt) && (optind >= argc)) || (argc < 3)) {
		inst_err(prog, ERR, USAGE);
		return(ERR_RET);
	}

	errors = 0;

	if ((pkginst = getenv("PKGINST")) == (char *)NULL)
		pkginst = nil; /* null string */

	if(tflag) {
		if ((oambase = getenv(TESTBASE)) == (char *)NULL)
			oambase = nil; /* null string */
		(void) sprintf(oam_env, "OAMBASE=%s", oambase);
		if(putenv(oam_env) != 0) {
			inst_err(prog, ERR, ENV_MOD);
			return(ERR_RET);
		}
		
	}
	else
		if ((oambase = getenv(OAMBASE)) == (char *)NULL)
			oambase = nil; /* null string */
	
	mi_file = argv[optind++];
	log_file = argv[optind++];
	exp_file = argv[optind];

	if((miffile = fopen(mi_file, "r")) == NULL) {
		/*failed to open */
		inst_err(prog, ERR, FILE_OPN, mi_file);
		return(ERR_RET);
	}
	if((logfile = fopen(log_file, "a")) == NULL) {
		inst_err(prog, ERR, FILE_OPN, log_file);
		return(ERR_RET);
	}

	if((exprfile = fopen(exp_file, "a")) == NULL) {
		inst_err(prog, ERR, FILE_OPN, exp_file);
		return(ERR_RET);
	}
	
	/* read each line in .mif file and process */
	while(fgets(instr, sizeof(instr), miffile)) {
		for(pos=instr; isspace(*pos); )
			pos++; /* eat leading whitespace */
		if((*pos == '\0') || (*pos == '#')) 
			continue; /* comment or blank line */

		/* get path */
		path = menutok(pos);
		if(path == NULL) {
			inst_err(prog, ERR, INV_ENTRY, instr);
			errors++;
			continue;
		}

		/* get description */
		descr = menutok(NULL);
		if(descr == NULL) {
			inst_err(prog, ERR, INV_ENTRY, instr);
			errors++;
			continue;
		}

		/* get action */
		action = menutok(NULL);

		filechanged = 0;	/* reset log */
		m_item = find_menu(prog, path);
		if(m_item == NULL) {
			inst_err(prog, ERR, INV_PATH, instr);
			errors++;
			continue;
		}
		/* valid menu */

		cur_dir = m_item->path;

		/* build path name to temp file in cur_dir */
		if(strncmp(cur_dir, OAMBASE, sizeof(OAMBASE)-1) ||
			(cur_dir[sizeof(OAMBASE)-1] != '/')) {
			inst_err(prog, ERR, INV_PATH, instr);
			errors++;
			continue;
		}
		/* skip "OAMBASE" in cur_dir string */
		cur_dir += sizeof(OAMBASE);

		/* build temporary menu path */
		(void) sprintf(tmp_menu,"%s/%s/%s", oambase, cur_dir, TMP_NAME);

		/* build full current path */
		(void) sprintf(cur_path, "%s/%s", oambase, cur_dir);

		/* save cur_path in new_dir */
		(void) strcpy(new_dir, cur_path);

		/* path to menu file containing the item */
		(void) sprintf(cur_menu, "%s/%s/%s", oambase, cur_dir, 
			m_item->menu_name);
		if((file_str = input_menu(prog, cur_menu, tmp_menu)) == NULL) {
			errors++;
			continue;
		}

		itemptr = file_str->entries;
		previtem = itemptr;

		/* search menu file to find .mif entry */
		filechanged = find_mif(&itemptr, &previtem, 
			m_item, file_str, cur_dir);
		if(filechanged < 0) {
			errors++;
			continue;
		} else if(filechanged) {
			if(output_menu(prog, file_str, tmp_menu) < 0) {
				errors++;
				break;
			}
			log_chgs(tmp_menu, cur_menu); /* log changes */
			free_menu(file_str);
			filechanged = 0;	/* reset */
		}

		if(m_item->par_menu != NULL) {
			/* parent placeholder */
			/* build path name to temp file in cur_dir */
			if(strncmp(m_item->par_menu, OAMBASE, 
				sizeof(OAMBASE)-1) == 0) {
				/* skip "OAMBASE" in cur_dir string */
				m_item->par_menu += sizeof(OAMBASE);
				(void) sprintf(cur_menu, "%s/%s", oambase,
					m_item->par_menu);
			} else (void) strcpy(cur_menu, m_item->par_menu);

			(void) strcpy(tmp_menu, cur_menu);
			if((pos = strrchr(tmp_menu, (int) '/')) != NULL)
				*(pos + 1) = NULL;
			(void) strcat(tmp_menu, TMP_NAME);
			if((file_str = input_menu(prog, cur_menu, tmp_menu))
				== NULL) {
				errors++;
				continue;
			}

			itemptr = file_str->entries;
			for(;;) {

				if(strncmp(itemptr->mname, m_item->par_item, 
					strlen(m_item->par_item)) == 0) {
					/* found it */
					filechanged = proc_ph(itemptr, 
						ISPHOLDER, cur_dir);
					break;
				}
				if(itemptr->next == NULL)
					break;
				else 
					itemptr = itemptr->next;
			}
			if(filechanged) {
				if(output_menu(prog, file_str, tmp_menu) < 0) {
					errors++;
					break;
				}
				log_chgs(tmp_menu, cur_menu); /* log changes */
				free_menu(file_str);
			}
		}
	} /* end while fgets instr (get each mif file line) */

	(void) fclose(logfile);
	(void) fclose(miffile);
	return(errors);
} /* end main */

static int
write_new(tflag, name)
int tflag;		/* set if mod_menus invoked with -t option */
char *name;		/* name of new menu item to write into menu file */
{
	FILE *newtmp;		/* new temp file */
	char newline[LNSZ];	/* new output line so won't clobber others */
	char menu_nam[MENULEN];	/* menu name - truncated to MAXSTR */
	int ret;		/* return from mk_dir */
	int len;		/* length of string in new_dir */

	ret = 0;

	/* new_dir already has correct path name up to name */
	(void) strcat(new_dir, DIR_DELIMIT);
	len = strlen(new_dir);
	(void) strcat(new_dir, name);

	/* create new directory */
	if(tflag) {
		if((ret = mk_tdir(new_dir, logfile)) < 0) return(-1);
	} else {
		if((ret = mk_dir(new_dir, pkginst, logfile)) < 0) return (-1);
	}

	/* be sure that total name is maximum of MAXNAM */
	(void) strncpy(menu_nam, name, MAXNAM);
	*(menu_nam + MAXNAM) = NULL;
	
	/* create new menu file name and temp file name to go with it */
	(void) sprintf(new_menu, "%s/%s%s", new_dir, menu_nam, MENU_SFX);
	(void) sprintf(new_tmp, "%s/%s", new_dir, TMP_NAME);

	if((newtmp = fopen(new_tmp, "a")) == NULL) {
		inst_err(prog, ERR, FILE_OPN, new_tmp);
		ret = -1;
	}
	else { /* write menu description line */
		(void) sprintf(newline, "%s%s\n", MENUHDR, descr);
		(void) fputs(newline, newtmp);
		(void) fputs(LIFELINE, newtmp);
		(void) fclose(newtmp);
	}
	log_chgs(new_tmp, new_menu);
	return(ret);
}

static void
log_chgs(new, old)
char *new;	/* name of tmp menu file that holds change */
char *old;	/* name of menu file corresponding to tmp file */
{
	/*
	 * record in log file that file name indicated by 'new' has
	 * changes contained in 'old'.
	 */
	(void) fprintf(logfile, "%s %s\n", new, old);
}

static void
log_expr(itemptr, cur_dir, ispholder, delflag)
struct item_def *itemptr;	/* ptr to item to log in expr file */
char *cur_dir;			/* current directory */
int ispholder;			/* pholder flag */
int delflag;	/* tells if "^#DELETE#" gets added to end of entry or not */
{
	char *cur;		/* position in cur_dir of beginning of path */
	char *pos;		/* position of last '/' in action */
	/*
	 * record in the express mode log file that a new task/menu
	 * was added to the Interface structure.  
	 */

	if(strncmp(cur_dir, MAIN_PATH, strlen(MAIN_PATH)) == 0) {
		cur = cur_dir + strlen(MAIN_PATH);
	} else 
		cur = cur_dir;

	if(ispholder == NONPHOLDER) {	/* cur_dir doesn't include full path */
		if(strncmp(itemptr->maction,LOC_IDENT,strlen(LOC_IDENT)) != 0){
			(void) sprintf(outstr, "%s%c%s/%s", itemptr->mname, 
				TAB_CHAR, cur, itemptr->maction);
			pos = strrchr(outstr, '/');
			if(pos != NULL) 
				*pos = TAB_CHAR;
		} else (void) sprintf(outstr, "%s%c-%c%s", 
			itemptr->mname, TAB_CHAR, TAB_CHAR, itemptr->maction);
	} else {	/* placeholder - cur_dir will include full path */
		pos = strrchr(itemptr->maction, '/');
		if(pos == NULL) pos = itemptr->maction;
		else pos++;
		(void) sprintf(outstr, "%s%c%s%c%s", itemptr->mname, 
			TAB_CHAR, cur, TAB_CHAR, pos);
	}

	(void) fputs(outstr, exprfile);
	if(delflag) {
		(void) fputs(TAB_DELIMIT, exprfile);
		(void) fputs(DEL_STR, exprfile);
	}
	(void) fputs("\n", exprfile);
}

static int
srch_pkg(itemptr)
struct item_def *itemptr;
{
	char	*pt;	/* pointer to pkginsts */
	int	n;

	n = strlen(pkginst);
	for(pt = itemptr->pkginsts; ; ) {
		/* find next '#' */
		while(*pt != '#') {
			if(*pt++ == '\0')
				return(NOTFOUND);
		}
		pt++; /* get to start of pkginst */

		if(!strncmp(pt, pkginst, n) && (pt[n] == '#'))
			return(FOUND);

		/* not found, so find next TAB_CHAR */
		while(*pt != TAB_CHAR) {
			if(*pt++ == '\0')
				return(NOTFOUND);
		}
	}
	/*NOTREACHED*/
}

static struct item_def *
find_last(name, itemptr)
char *name;			/* name to find last entry for */
struct item_def *itemptr;	/* item to start searching from */
{

	struct item_def *last;	/* last item of orig_name to return */

	last = itemptr;
	itemptr = itemptr->next;
	while(strncmp(itemptr->orig_name, name, strlen(name)) ==  0) {
		last = itemptr;
		if(itemptr->next != NULL) itemptr = itemptr->next;
		else break;
	}
	return(last);
}

static void
new_name(to, from, orig)
char *to;		/* place to copy new name to */
char *from;		/* derive the new name from this name */
char *orig;		/* original name */
{
	char *pos;	/* character position */
	char *svpos;	/* save character position */
	char buildbuf[MAXITEMLEN+1];	/* buffer to build new name into */
	
	int cnt;	/* count of digits */
	int num;	/* rename number */
	
	cnt = 0;
	pos = from;

	while(isalpha(*pos)) pos++;
	/* pos is now at first non alpha char */

	if((isspace(*pos)) || (*pos == NULL)) {	/* first rename of orig */
		num = 2;
		cnt = 1;
	} else {	/* multiple rename */
		svpos = pos;
		while(isdigit(*svpos)) {	/* count how many digits */
			cnt += 1;
			svpos++;
		}
		num = atoi(pos);
		if(num == 9) cnt += 1;	/* add 1 digit if next is 10 */
		num++;
	}
	
	(void) strncpy(buildbuf, orig, (MAXITEMLEN - cnt));
	*(buildbuf + MAXITEMLEN - cnt) = NULL;
	(void) sprintf(to, "%s%d", buildbuf, num);
	inst_err(prog, WARN, RENAME, orig, to);
}

static int
process_item(tflag, itemptr, m_item, cpflag)
int tflag;			/* set if mod_menus invoked with -t option */
struct item_def *itemptr;	/* pointer to item to process */
struct menu_item *m_item;	/* menu item from find_menu() */
int cpflag;			/* flag to indicate if copy pkginst */
{
	int len;		/* string length of action */

	(void) strcpy(itemptr->mdescr, descr);
	itemptr->pholder = P_NONE;
	if ((action == NULL) ||
	    (strncmp(action, LOC_IDENT, strlen(LOC_IDENT)) != 0)) {
		/* orig name is NULL, then no rename present */
		if(*(itemptr->orig_name) == NULL)
			(void) strcpy(itemptr->maction, itemptr->mname);
		else if(action != NULL) /* task, use orig name */
			(void) strcpy(itemptr->maction, itemptr->orig_name);
		else if(action == NULL)	/* menu, use itemptr->mname */
			(void) strcpy(itemptr->maction, itemptr->mname);
		(void) strcat(itemptr->maction, DIR_DELIMIT);
	} else 
		*(itemptr->maction) = NULL;

	if(cpflag)
		(void) sprintf(itemptr->pkginsts, "#%s#", pkginst);

	if(action != NULL) 
		(void) strcat(itemptr->maction, action);
	else {
		len = strlen(itemptr->maction);
		(void) strncat(itemptr->maction, itemptr->mname, MAXNAM);
		*(itemptr->maction + len + MAXNAM) = NULL;
		(void) strcat(itemptr->maction, MENU_SFX);
		if(write_new(tflag, itemptr->mname) < 0) {
			inst_err(prog, ERR, D_CREAT, m_item->item);
			return(-1);
		}
	}
	return(0);
}

static int
proc_ph(itemptr, pflag, cur_dir)
struct item_def *itemptr;	/* pointer to menu item */
int pflag;			/* indicates parent placeholder or not */
char *cur_dir;			/* current directory */
{
	int changed;		/* indicates if item was changed or not */

	changed = 0;
	if(itemptr->pholder == P_INACTIVE) {
		itemptr->pholder = P_ACTIVE;
		changed = 1;
		log_expr(itemptr, cur_dir, pflag, NO_DEL);
	}

	if(srch_pkg(itemptr) == NOTFOUND) {
		add_pkg(itemptr);
		changed = 1;
	}
	return(changed);
}

static void
add_pkg(itemptr)
struct item_def *itemptr;		/* pointer to menu item */
{
	if ((unsigned int)strlen(itemptr->pkginsts) > 2) {	/* already a list */
		(void) strcat(itemptr->pkginsts, TAB_DELIMIT);
		(void) strcat(itemptr->pkginsts, "#");
		(void) strcat(itemptr->pkginsts, pkginst);
		(void) strcat(itemptr->pkginsts, "#");
	}
	else (void) sprintf(itemptr->pkginsts, "#%s#", pkginst);
}

static int
new_desc(itemptr)
struct item_def *itemptr;		/* pointer to menu item */
{
	struct menu_file *in_file;	/* menu file to change */
	struct menu_line *hdr_line;	/* menu file header line */
	char *pos;			/* position in new_tmp buffer */
	int found;			/* flag if menu line found */

	(void) strcpy(itemptr->mdescr, descr);
	if(srch_pkg(itemptr) != FOUND)
		add_pkg(itemptr);

	/* now make sure to change description in menu file */
	(void) sprintf(new_menu, "%s/%s", new_dir, itemptr->maction);
	(void) strcpy(new_tmp, new_menu);
	pos = strrchr(new_tmp, '/');
	if(pos++ == NULL) {
		inst_err(prog, ERR, -1);
		return(-1);
	}
	*pos = NULL;
	(void) strcat(new_tmp, TMP_NAME);
	
	if((in_file = input_menu(prog, new_menu, new_tmp)) == NULL) 
		return(-1);

	hdr_line = in_file->head;
	found = 0;
	while (!found) {
		if(strncmp(hdr_line->line, MENUHDR, strlen(MENUHDR)) == 0)
			found = 1;
		else if(hdr_line->next != NULL) hdr_line = hdr_line->next;
		else {
			/* NO MENUHDR LINE???? */
			inst_err(prog, ERR, INV_FILE, cur_menu);
			free_menu(in_file);
			return(-1);
		}
	}
	if(found) {
		/* change it, log the change and write the file */
		(void) sprintf(hdr_line->line, "%s%s\n", MENUHDR, descr);
		hdr_line = hdr_line->next;
		if((hdr_line != NULL) && (strncmp(hdr_line->line, HELPHDR,
			strlen(HELPHDR)) == 0)) { /* change help title */
			pos = strrchr(hdr_line->line, TAB_CHAR);
			if(pos++ == NULL) {
				inst_err(prog, ERR, -1);
				free_menu(in_file);
				return(-1);
			}
			*pos = NULL;
			(void) strcat(hdr_line->line, descr);
			(void) strcat(hdr_line->line, "\n");
		}
		if(output_menu(prog, in_file, new_tmp) < 0) {
			free_menu(in_file);
			return(-1);
		}
		log_chgs(new_tmp, new_menu); /* log changes */
	}
	log_chgs(tmp_menu, cur_menu); /* log changes */
	free_menu(in_file);
	return(0); /* we've changed something */
}

static int
merge_menu(itemptr, cur_dir, oflag)
struct item_def *itemptr;		/* pointer to menu item */
char *cur_dir;				/* current directory */
int oflag;				/* online flag */
{
	int filechanged;		/* return value */

	if(strcmp(descr, itemptr->mdescr) == 0)
		filechanged = proc_ph(itemptr, NONPHOLDER, cur_dir);
	else if((srch_pkg(itemptr) == FOUND) || oflag) {
		/* different descr */
		if(new_desc(itemptr))
			return(-1);
		filechanged = 1;
	} else 
		filechanged = proc_ph(itemptr, NONPHOLDER, cur_dir);
	return(filechanged);
}

static int
compare_item(itemptr, name)
struct item_def *itemptr;		/* existing menu entry */
char *name;				/* new name */
{ 
	char *pos;			/* position in itemptr->maction */
	if(((strncmp(itemptr->mname, name, MAXITEMLEN) == 0) 
		|| (strncmp(itemptr->orig_name, name, MAXITEMLEN) == 0))
		&& (strcmp(itemptr->mdescr, descr) == 0)) {
		/* so far the same */
		pos = strrchr(itemptr->maction, '/');
		if(pos++ == NULL)
			return(DIFF);
		if(strcmp(pos, action) == 0) 
			return(SAME);
	}
	return(DIFF);
}

static int
find_mif(p_itemptr, p_previtem, m_item, file_str, cur_dir)
struct item_def	**p_itemptr;
struct item_def **p_previtem;
struct menu_item *m_item;
struct menu_file *file_str;
char	*cur_dir;
{
	struct item_def *newitem;	/* pointer to calloc'd menu item */
	struct item_def	*itemptr;
	struct item_def *previtem;
	int	cp_pkginst, done,
		overwrite,	/* flag if menu entry to be overwritten */
		rename,
		samepkg,	/* pkginst found on line */
		filechanged;
	char	*pt;

	itemptr = *p_itemptr;
	previtem = *p_previtem;
	done = samepkg = overwrite = rename = filechanged = 0;
	do {

		if(itemptr == NULL) 
			done = 1;
		if((done != 1) && (strncmp(m_item->item, itemptr->mname, MAXITEMLEN) == 0)) {
			/* entry already exists - check it */
			/* figure out how to handle collision */
			rename = 1;

			/* 
			 * collisions result in rename of the new
			 * item, an overwrite of the existing
			 * item with the new, or simply ignoring
			 * the new item definition 
			 */
			if(action == NULL) {	/* mif item is menu */
				pt = strrchr(itemptr->maction, '.');
				if((pt != NULL) && (strcmp(pt,MENU_SFX) == 0)) {
					/* existing menu */
					rename = 0;
					filechanged = merge_menu(itemptr,
					   cur_dir, oflag);
					break;
				}
			}
			samepkg = srch_pkg(itemptr);
			if(oflag || (rename && (samepkg == FOUND))) {
				rename = 0;
				overwrite = 1;
			}
			if(rename) {
				newitem = (struct item_def *)
				      calloc(1, sizeof(struct item_def));
				if(newitem == NULL) {
					inst_err(prog, ERR, D_CREAT, path);
					done = (-1);
					break;
				}
				itemptr = find_last(m_item->item, itemptr);
				new_name(newitem->mname,itemptr->mname,
					m_item->item);
				(void) strncpy(newitem->orig_name, 
					m_item->item, MAXITEMLEN);
				*(newitem->orig_name + MAXITEMLEN) = NULL;
				cp_pkginst = 1;
				done = process_item((tflag|oflag),
					newitem, m_item, cp_pkginst);
				if(done) 
					break;
				log_expr(newitem, cur_dir, NONPHOLDER, NO_DEL);
				newitem->next = itemptr->next;
				itemptr->next = newitem;
				filechanged = 1;
				break;
			}
			if(overwrite) {
				if((compare_item(itemptr, m_item->item) == SAME)
					&& (oflag || (samepkg == FOUND))) {
					break;
				}
				log_expr(itemptr, cur_dir, NONPHOLDER, DELETE);
				if(oflag) cp_pkginst = 1;
				else inst_err(prog, WARN, OVERWRITE, 
					itemptr->mname);
				done = process_item((tflag|oflag), itemptr, 
					m_item, cp_pkginst);
				/* done will be set on error */
				if(done) 
					break;
				log_expr(itemptr, cur_dir, NONPHOLDER, NO_DEL);
				filechanged = 1;
				break;
			}
		/*
		 * XXXX if itemptr is null (assumes m_item->item will
		 * not be null - or if item/mname are not equal ...
		 */
		} else if((itemptr == NULL) ||
		    (strcmp(m_item->item, itemptr->mname) > 0)) {
			if((itemptr == NULL) ||
			    (itemptr->next == NULL)) {
				/* set done so it will 
				 * create the new item - at end
				 * of item list 
				 */
				done = 1;
			}
		}  /* end else if */

		if(done || (strncmp(m_item->item, itemptr->mname, strlen(m_item->item)) < 0)) {
			/*
			 * no match found - past possible
			 * matches since file is alphabetical.
			 * create new item & add to linked list
			 * also - flag the fact that the file
			 * has changed so it will be written
			 * to a temp file and logged.
			 */
			newitem = (struct item_def *)
			      calloc(1, sizeof(struct item_def));
			if(newitem == NULL) {
				inst_err(prog, ERR, D_CREAT, path);
				done = (-1);
				break;
			}

			/* copy the name */
			(void) strncpy(newitem->mname, m_item->item,
				MAXITEMLEN);
			*(newitem->mname + MAXITEMLEN) = NULL;
			newitem->orig_name[0] = '\0'; /* null string */ 
			cp_pkginst = 1;
			done = process_item((tflag|oflag), newitem,
				m_item, cp_pkginst);
			if(done) 
				break;

			log_expr(newitem, cur_dir, NONPHOLDER, NO_DEL);
			if(file_str->entries == NULL) {
				/* first item in list */
				file_str->entries = newitem;
				newitem->next = NULL;
			} else {
				/* first item in non-empty list */
				if((itemptr == file_str->entries) &&
				  (strncmp(m_item->item, itemptr->mname,
					strlen(m_item->item)) < 0)){
					newitem->next = itemptr;
					file_str->entries = newitem;
				} else if((itemptr->next == NULL) &&
				  (strncmp(m_item->item, itemptr->mname,
					strlen(m_item->item)) > 0)) {
					newitem->next = itemptr->next;
					itemptr->next = newitem; 
				} else {
					newitem->next = previtem->next;
					previtem->next = newitem;
				}
			}
			done = 1;
			filechanged = 1;

		} /* end if */
		if(!done) {
			previtem = itemptr;
			itemptr = itemptr->next;
		}
	} while(!done);
	*p_previtem = previtem;
	*p_itemptr = itemptr;
	if(done < 0)
		return(-1);
	return(filechanged);
}
