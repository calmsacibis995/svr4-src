/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamintf:interface/sysadm.c	1.12.3.2"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <pwd.h>
#include <errno.h>
#include <limits.h>
#include "intf.h"
#include "print_err.h"
#include "errors.h"
#include "userdefs.h"
#include "sysadm.h"
#include "menu_io.h"
#include "menutrace.h"
#include "../intf_reloc/oldmenu.h"

extern pid_t	getppid();
extern uid_t	geteuid();
extern TASKDESC *find_node();
extern struct passwd 
		*getpwnam();
extern char	*getenv(),
		*read_item();
extern int	execv(), 
		execve(),
		execle(),
		match_cnt(),
		putenv(),
		ismenu(),
		getopt(),
		find_expr();
extern void	print_err(),
		clr_marks(),
		lineage(),
		exit(),
		free_menu();
extern struct menu_item 
		*find_menu();
extern struct menu_file
		*input_menu();

struct passwd *userp;	/* pointer to password structure */
FILE *exp_file;		/* file pointer to express lookup file */

char inbuf[BUFSIZ/2];		/* input buffer */
char object[BUFSIZ];		/* initial FMLI object to bring up */
char oaminit[BUFSIZ];		/* oam.init file from OAMBASE */
char oamcmd[BUFSIZ];		/* oam.cmd file from OAMBASE */
char command[BUFSIZ];		/* used to build arguments in express() */
				/* and for shell invocation */
char i_exprfile[PATH_MAX];		/* full path to express file */
char path[PATH_MAX];		/* path array to set in environment */
char oambase[PATH_MAX];		/* oambase to set in environment */
char intfbase[PATH_MAX];		/* intfbase to set in environment */
char shell[] = STD_SHL;		/* shell to set in environment */
char homedir[PATH_MAX];		/* home directory to set in environment */
char sysdir[PATH_MAX];		/* SYSDIR in environ. - initial invocation */
char sysstart[PATH_MAX];	/* SYSSTART in environ. - initial invocation */
char pinfo[] = STD_PATH;		/* basic path information */
char i_expr_name[] = I_EXPR_PTH;	/* express file name */
char bin_name[] = BIN_PTH;	/* bin name */
char menu_nam[] = MAIN_PTH;	/* std intf. directory under OAMBASE */
char add_ons[] = PKG_PTH;	/* add-ons directory under OAMBASE */
char p_str[] = PATH_VAR;	/* path variable */
char oam_str[] = OAM_VAR;	/* oambase variable */
char intf_str[] = INTF_VAR;	/* intfbase variable */
char home_str[] = HOME_VAR;	/* home directory variable */
char sdir_str[] = SDIR_VAR;	/* sysdir variable */
char sstart_str[] = START_VAR;	/*sysstart variable */

char envarray[BUFSIZ];		/* environment array space */
char *vars[] = {
	"TERM",		/* term must always be first in this list */
	"LOGNAME",
	"CFTIME",
	"LOADPFK",
	"LD_BIND_NOW",
	"CHRCLASS",
	"LANGUAGE",
	"TRACE",
	"TZ",
	"LOGTTY",
	"LINES",
	"COLUMNS",
	"OAMCOLS",
	"HOME",
	NULL
};

char *nenvp[12];		/* "new" environment array */
char *nargv[12];		/* "new" argument array */
char *shlargv[12];		/* "new" argument array for shell scripts */

char *base;			/* oam interface base directory */
char *item;			/* item from express lookup file */
char *pkgi;			/* pkginst for express entry */
char *subp;			/* sub-dir ptr from OAMBASE for express entry*/
char *descp1;			/* point to description */
char *actp1;			/* point within act_path */
int obj_type;			/* Form = 1, Text = 2, Menu = 3 */

#define PL_LEN (sizeof(PHOLDER)-1)
#define TMP_NM "/tmp/tmp.menu"

int task_id = 0;
TASKDESC *root = (TASKDESC *)0;
TASKDESC *exec_list = (TASKDESC *)0;
TASKDESC *rn_list = (TASKDESC *)0;
TASKDESC *ph_list = (TASKDESC *)0;
TASKDESC *thread_strt = (TASKDESC *)0;
TASKDESC **thread_end = &thread_strt;
char path_lineage[PATH_MAX];	/* path to an action */
char act_path[PATH_MAX];	/* path to an action for tmp menu */
char action_dir[PATH_MAX];	/* path to action directory */

struct menu_file *file_str;	/* return from input_menu() */
struct menu_item *m_itemp;
int mk_mt_node();

static int	iexpr_proc(),
		iexpr_read(),
		envalt(),
		express(),
		menu_thread(),
		tmp_menu();
static char	*get_term();
static void	old_entry();
static struct item_def *get_task();
static TASKDESC * find_first();

main(argc, argv)
int argc;
char **argv;
{
	register char **argptr;	/* pointer to 'new' argv array */
	char **sargptr;		/* pointer to shell argv array */
	char *namep;	/* simple name of command: sysadm, setup, etc. */
	char *nnamep;	/* simple name of command */
	char *init_obj;	/* initial fmli object to invoke */
	char *cmd;	/* cmd to execute ifnot menu or fmli obj */
	char *cmdptr;	/* pointer into command array */
	char *space;	/* pointer to space between arguments, ifany */
	int ret;	/* ret from express() */

	argptr = nargv;

	/* WITHOUT TFM: */
	/* get simple name of argv[0] */

	namep = strrchr(*(argv+0), (int) '/');
	if(namep == NULL) namep = *(argv+0);
	else namep++;

	if(*namep == '-') {
		++namep;
		nnamep = namep;
	}
	else nnamep = namep;

	if(((argc > 2) || (*argv[1] == '-')) && (strcmp(nnamep, SYSADM) == 0)) {
		print_err(S_USAGE, nnamep);
		return(USAGE_ERR);
	}
	else if((argc > 2) && (strcmp(nnamep, POWER) == 0)) {
		print_err(P_USAGE, nnamep);
		return(USAGE_ERR);
	}
	else if((argc > 3) && (strcmp(nnamep, MOUNT) == 0)) {
		print_err(M_USAGE, nnamep);
		return(USAGE_ERR);
	}
	else if((argc > 2) && (strcmp(nnamep, UMOUNT) == 0)) {
		print_err(U_USAGE, nnamep);
		return(USAGE_ERR);
	}
	else if((argc > 1) && ((strcmp(nnamep, SETUP) == 0)
		|| (strcmp(nnamep, CHECK) == 0)
		|| (strcmp(nnamep, MAKE) == 0))) {
		print_err(O_USAGE, nnamep, nnamep);
		return(USAGE_ERR);
	}

	/* now get info about simple name "user" from /etc/passwd */
	if((userp = getpwnam(nnamep)) == NULL) {
		print_err(INVNAME, SYSADM, nnamep);
		return(INV_ERR);
	}

	/* 
	 * If the name from the password file is "sysadm", check to see
	 * ifan argument was supplied.  If so, pass it since this is  
	 * valid for the "sysadm" function.
	 * *MODIFIED* FROM SHADE:
	 */

	*argptr++ = "/usr/bin/su";
	*argptr++ = userp->pw_name;

	if((strcmp(userp->pw_name, SYSADM) == 0) 
		|| (strcmp(userp->pw_name, POWER) == 0)
		|| (strcmp(userp->pw_name, MOUNT) == 0)
		|| (strcmp(userp->pw_name, UMOUNT) == 0)) {
		if(getppid() == 1)	/* called from login level */
			*argptr++ = getenv("L0");
		else if(argv[1]) {	/* called from shell level */
			*argptr++ = argv[1];
			if((strcmp(userp->pw_name, MOUNT) == 0)
				&& argv[2]) {
				*argptr++ = argv[2];
			}
		}
		*argptr = NULL;	/* NULL terminate */
	}

	/*
	 * If the user's effective uid and the uid from /etc/passwd
	 * don't match, exec "su -" with name from password file.  This
	 * will handle the case when called by user at shell level.
	 * FROM SHADE:
	 */

	if(geteuid() != userp->pw_uid) {
		(void) execv("/usr/bin/su", nargv);
		/* should normally not get here */
		(void) fprintf(stderr, "Exec of su failed.\n");
		exit(1);
	}

	/*
	 * set OAMBASE variable value
	 */
	base = OAM_PATH;

	/* set oambase and intfbase in arrays */
	(void) sprintf(oambase, "%s%s", oam_str, base);
	(void) sprintf(intfbase, "%s%s%s%s", intf_str, pkgi, base, menu_nam);
	(void) putenv(oambase);

	/* set path in path array */
	(void) sprintf(path, "%s%s:%s%s", p_str, pinfo, base, bin_name);

/*	(void) sprintf(homedir, "%s%s", home_str, userp->pw_dir);	*/

	/* alter environment */
	if((ret = envalt()) != SUCCESS)
		return(ret);
	if((ret = express(nnamep, argc, argv)) != 0) 
		return(ret);

	/*
	 * Build the command string that "exec" will execute when called.
	 */
	if(obj_type == FML_FORM) init_obj = FORM_OBJ;
	else if(obj_type == FML_TEXT) init_obj = TEXT_OBJ;
	else if((obj_type == FML_MENU) || (obj_type == IS_MENU))
		init_obj = MENU_OBJ;

	if(obj_type != OTHER) {
		(void) sprintf(object, "%s%s/%s", base, menu_nam, init_obj);
		(void) sprintf(oaminit, "%s%s/%s", base, menu_nam, INIT_OBJ);
		(void) sprintf(oamcmd, "%s%s/%s", base, menu_nam, CMD_OBJ);

		if((ret = execle(FMLI, FMLI, "-i", oaminit, "-c", oamcmd, 
			object, 0, nenvp)) < 0) {
			print_err(NOFMLI, nnamep);
			return(NO_FMLI_ERR);
		}
/*
	(void) printf("now execle test program\n");
	execle("/usr2/cme/interface/test","/usr2/cme/interface/test", 0, nenvp);
*/
	}
	else {	/* it's an executable - execute directly */
		if(strncmp(item, LOC_IDENT, strlen(LOC_IDENT)) == 0) {
			/* translate pkginst */
			item += strlen(LOC_IDENT);	/* skip *LOC* */
			if(pkgi == NULL) {
				print_err(INV_ENT, nnamep, item);
				return(ENV_EXP);
			}
			(void) sprintf(oamcmd, "%s%s/%s%s", base, add_ons, 
					pkgi, item);
			cmd = oamcmd;
		}
		else cmd = item;
		
		argptr = nargv;
		cmdptr = cmd;
		for(;;) {
			*argptr = cmdptr;
			if(*cmdptr == '\"') {
				*argptr += 1;
				/* pass quoted string */
				if((space = strchr(cmdptr+1, '\"'))
				  == NULL) {
					/* unbalanced string */
					break;
				}
				*space = NULL;
				cmdptr = space + 1; /* next arg */
			}
			space = strchr(cmdptr, (int) ' ');
			argptr++;
			if(space == NULL) {	/* no more arguments */
				*argptr++ = NULL;
				break;
			}
			else {
				cmdptr = space + 1;
				*space = NULL;
			}
		}
#ifdef EBUG
		(void) printf("run command: %s \n", cmd);
		for(ii = 0; nargv[ii] != NULL; ++ii) {
			(void) printf("%s \n", nargv[ii]);
		}
		(void) printf("\n");
#endif
		if((ret = execve(cmd, nargv, nenvp)) < 0) {
			/* printf("re-run command: %s\n", cmd); */
			if(errno == ENOEXEC) {	/* attempt shell script */
				argptr = nargv;
				sargptr = shlargv;
				*sargptr++ = "/sbin/sh";
				while(*sargptr++ = *argptr++);
				if((ret = execve("/sbin/sh", shlargv, nenvp)) 
					< 0) {	/* still can't execute */
					print_err(NO_COMND, nnamep, cmd);
					return(NO_FMLI_ERR);
				}
			}
		}
	}

	return(SUCCESS);
}


static int
envalt()
{

	register char **envptr;		/* envp pointer */
	char *envp;			/* pointer into envarray */
	char *varptr;			/* pointer into vars array */
	char *valptr;			/* pointer to variable value */
	int i;				/* misc loop counter */
	int setterm;

	envptr = nenvp;
	envp = envarray;
	*envptr++ = sysdir;
	*envptr++ = sysstart;
	*envptr++ = oambase;
	*envptr++ = intfbase;
	*envptr++ = shell;
	*envptr++ = homedir;
	*envptr++ = path;

	/* now copy rest of environment that we're interested in */
	varptr = vars[0];
	setterm = i = 0;

	while(varptr != NULL) {
		valptr = getenv(varptr);
		if((i == 0) && ((valptr == NULL) || (strlen(valptr) < 1) ||
		    (strcmp(valptr, "unknown") == 0))) {
			/* term variable is not set up properly... */
			/* get it */
			setterm = 1;
			if((valptr = get_term()) == NULL)
				return(INV_TERM);
		}
		if(valptr != NULL) {
			(void) sprintf(envp, "%s=%s", varptr, valptr);
			*envptr++ = envp;
		}
		if((i == 0) && (setterm != 0)) {
			(void) putenv(envp);
			(void) printf("Running the UNIX command \"tput init\"\n");
			(void) system("tput init");
		}
		if(valptr != NULL) {
			envp += (strlen(varptr) + strlen(valptr) + 2);
		}
		i++;
		varptr = vars[i];
	}
	return(SUCCESS);
}

static char *
get_term()
{
	(void) printf("Please enter your terminal type:  ");
	return(gets(inbuf));
}


static int
express(cmd, argc, argv)
char *cmd;			/* simple command name */
int argc;			/* argument count */
char **argv;			/* argv array from main */
{
	extern char *optarg;		/* used by getopt */
	extern int optind, opterr;	/* used by getopt */
	char *lookup;	/* name to lookup in express lookup file */
	char *dot;	/* position of dot in file name */
	int found;	/* flag gets set iflookup found in file */
	int cmd_code;	/* code for command - powerdown, mount, umount, other */
	int ch;		/* return from getopt */
	int errflg;	/* error flag */

	opterr = 0;			/* no print errors from getopt */

	errflg = 0;		/* init*/


	if(strcmp(cmd, SYSADM) != 0) { /* not sysadm command, could be args */
		(void) strcpy(command, cmd);
		lookup = command;
		cmd_code = NOOPTS;
		if(strcmp(cmd, POWER) == 0) cmd_code = PWR_CMD;
		else if(strcmp(cmd, MOUNT) == 0) cmd_code = MNT_CMD;
		else if(strcmp(cmd, UMOUNT) == 0) cmd_code = UMNT_CMD;
		else if(strcmp(cmd, SETUP) == 0) strcpy( command, "syssetup");
		if(cmd_code != NOOPTS) {
			while((ch = getopt(argc, argv, "yYr")) != EOF) {
				switch(ch) {
					case 'y':
						(void) strcat(command, " -y");
						break;
					case 'Y':
						if(cmd_code == PWR_CMD)
							(void) strcat(command, " -Y");
						else 
							errflg++;
						break;
					case 'r':
						if(cmd_code == MNT_CMD)
							(void) strcat(command, " -r");
						else
							errflg++;
						break;
					case '?':
						errflg++;
				}

				if(*optarg == '-') errflg++;
			}

			if(errflg || (optind > argc)) {
				if(cmd_code == PWR_CMD)
					print_err(P_USAGE, cmd);
				else if(cmd_code == MNT_CMD)
					print_err(M_USAGE, cmd);
				else if(cmd_code == UMNT_CMD)
					print_err(U_USAGE, cmd);
	
				return(USAGE_ERR);
			}
		}
	} else if(argc == 2)	/* sysadm command with 1 arg */
		lookup = *(argv+1);
	
	else { /* sysadm command with no args */
		/* set sysdir and sysstart in arrays */
		(void) sprintf(sysdir, "%s%s%s", sdir_str, base, menu_nam);
		(void) sprintf(sysstart, "%s%s", sstart_str, MAIN_NAME);
		obj_type = IS_MENU;
		return(0);
	}
	
	(void) sprintf(i_exprfile, "%s%s%s", base, menu_nam, i_expr_name);
	if((exp_file = fopen(i_exprfile, "r")) == NULL) {
		/* can't open express mode lookup file */
		/* try to create and then re-open */
		(void) fprintf(stderr, "building express data base ... please wait\n");
		if(system(I_EXPR_BLD) != 0) return(NOEXPR);
		if((exp_file = fopen(i_exprfile, "r")) == NULL) {
			/* still can't open express mode lookup file */
			print_err(NO_EXPR, cmd);
			return(NOEXPR);
		}
	}
	found = 0;

	/* re-create internal express mode file "i_expr" */
	root = (TASKDESC *)0;
	(void) mk_mt_node(&root, (TASKDESC *)0, "main");
	root->action = "main.menu";
	root->ident = 0;
	if(iexpr_read(root) != 0) exit(1);
	(void) fclose(exp_file);
	found = iexpr_proc( lookup );

	if(found) {

		if(strncmp(item, FORM_PFX, strlen(FORM_PFX)) == 0)
			obj_type = FML_FORM;
		else if(strncmp(item, TEXT_PFX, strlen(TEXT_PFX)) == 0)
			obj_type = FML_TEXT;
		else if(strncmp(item, MENU_PFX, strlen(MENU_PFX)) == 0)
			obj_type = FML_MENU;
		else if(((dot = strrchr(item, (int) '.')) != NULL)
			&& (strncmp(dot,MENU_SFX, strlen(MENU_SFX)) == 0))
				obj_type = IS_MENU;
		else obj_type = OTHER;

		/* set sysdir and sysstart in arrays */
		if(((obj_type == FML_FORM) || (obj_type == FML_TEXT)
			|| (obj_type == FML_MENU)) && (pkgi != NULL))
			(void) sprintf(sysdir, "%s%s%s/%s/%s", sdir_str, base, 
				add_ons, pkgi, action_dir);
		else {
			if(action_dir[0] == '-') action_dir[0] = '\0';
			(void) sprintf(sysdir, "%s%s/%s", sdir_str, base,
				action_dir);
		}
		(void) sprintf(sysstart, "%s%s", sstart_str, item);

		return(0);
	}
	else {
		print_err(NOT_FOUND, SYSADM, lookup);
		return(N_EXIST_ERR);
	}
}

/* find express mode string in hierarchy and run action */
static int
iexpr_proc(strp)
char *strp;	/* pattern to match */
{
	int y, new, old;

	/* first, look at exec and rename lists */
	thread_strt = (TASKDESC *)0;
	thread_end = &thread_strt;
	if(find_expr(exec_list, strp, 0) != 0) {
		(void) find_first(exec_list);
	} else {
		if ( find_expr(rn_list, strp, 2) == 0 ) {
			/* only look at hierarchy if "link" not found */
			(void) find_expr(root, strp, 0);
		}
		if((y = match_cnt(&new, &old)) == 0) {
			/* no match: error */
			/* print_err(NOTUNIQ, "sysadm", strp); */
			return(0);
		}
		if(y == 1) {
			/* a single match pt-ed to by thread_strt */
			(void) menu_thread(thread_strt);
			if(file_str != (struct menu_file *)0) {
				free_menu(file_str);
				file_str = (struct menu_file *)0;
			}
		} else {
			/* ifnot a unique match, build a menu */
			if(tmp_menu(strp) == -1) return(0);
		}
	}
	return(1);
}


/* build a temporary menu that contains all selections for user choice */
static int
tmp_menu(sp)
char	*sp;
{
	TASKDESC *thrp;
	FILE	*tmpfp;
	char	*cp;		/* tmp char ptr */
	char	fn[PATH_MAX];	/* tmp file name */

	/* open tmp menu file for writing */
	(void) strcpy(action_dir, menu_nam);
	item = "exp.menu";
	(void) sprintf(fn, "%s/%s/%s", base, menu_nam, item);
	if((tmpfp = fopen(fn, "w")) == (FILE *)0) {
		print_err(NOT_OPEN, "sysadm", TMP_NM);
		return(-1);
	}
	/* print header info into file */
	(void) fprintf(tmpfp, "%sExpress Mode Menu for Path = %s\n", 
		MENUHDR, sp);
	(void) fprintf(tmpfp, "%spermanent\n\n", LIFEHDR);
	/* create a .menu line for each possible selection */
	for (thrp = thread_strt; thrp != (TASKDESC *)0; thrp = thrp->thread) {
		if(menu_thread(thrp)) {
			for (cp = path_lineage; *cp != '\0'; ++cp)
				if(*cp == ':') *cp = '/';
			cp = path_lineage;
			if(strncmp(cp, "main/", sizeof("main/") - 1) == 0) {
				cp += sizeof("main/") - 1;
			}
			(void) fprintf(tmpfp, "%s", cp);
			(void) fprintf(tmpfp, "^%s^%s",
				descp1, actp1);
			(void) fprintf(tmpfp, "\n");
		}
		if(file_str != (struct menu_file *)0) {
			free_menu(file_str);
			file_str = (struct menu_file *)0;
		}
	}
	(void) fclose(tmpfp);
	(void) strcpy(action_dir, "menu");
	item = "exp.menu";
	return(0);
}

/* find .menu entry for the leaf node pointer */
static int
menu_thread(leafp)
TASKDESC  *leafp;		/* point to leaf node to find menu entry for */
{
	char	bfr[PATH_MAX], *cp, *sp1;
	struct menu_item *m_item;		/* return from find_menu() */
	struct item_def *def;

	if(*(leafp->action) == 'R') {
		(void) fprintf(stderr, "assertion check\n");
		exit(1);
	}

	/* ifold_sysadm, route through old_entry() */
	if(*(leafp->action) == 'O') {
		old_entry(leafp);
		return(1);
	}

	path_lineage[0] = '\0';
	lineage(leafp, path_lineage);
	if((cp = strrchr(path_lineage, ':')) != (char *)0) {
		/* get rid of last ':' */
		if(*(cp + 1) == '\0') *cp = '\0';
	}
	m_itemp = m_item = find_menu("sysadm", strdup(path_lineage));
	if(m_item == (struct menu_item *)0) {
		print_err(NOT_FOUND, "sysadm", path_lineage);
		return(0);
	}

	/* build path name to temp file in cur_  */
	if(strncmp(m_item->path, OAMBASE, sizeof(OAMBASE) - 1) == 0) {
		/* skip "OAMBASE" in cur_dir string */
		m_item->path += sizeof(OAMBASE);
	}

	/* path to menu file containing the item */
	(void) sprintf(bfr, "%s/%s/%s", OAM_PATH, m_item->path, 
		m_item->menu_name);

	if((file_str = input_menu("sysadm", bfr, (char *)0)) == NULL)
		return(0);
	def = get_task(leafp->tname);
	
	/* calculate path to action from OAMBASE */
	if(strncmp(m_itemp->path, "menu", sizeof("menu") - 1) == 0) {
		m_itemp->path += sizeof("menu") - 1;
		if(*m_itemp->path == '/') ++(m_itemp->path);
	}
	pkgi = def->pkginsts;
	if( ismenu(def->maction)) *pkgi = '\0';
	if(*pkgi != '\0') {
		subp = "";
		sp1 = "../add-ons/";
		/* isolate last pkg instance ifmore than one */
		if((cp = strrchr(def->pkginsts, '#')) != (char *)0)
			*cp = '\0';
		if((pkgi = strrchr(def->pkginsts, '#')) != (char *)0)
			++pkgi;
	} else {
		subp = MAIN_PTH;
		sp1 = "";
		pkgi = (char *)0;
	}

	descp1 = m_itemp->m_desc;

	/* old sysadm to be invoked? */
	if(strncmp(def->maction, OLD_SYSADM, sizeof(OLD_SYSADM) - 1) == 0) {
		(void) strcpy(act_path,   def->maction);
		(void) strcpy(action_dir, def->maction);
		actp1 = item = act_path;
		return(1);
	}

	/* absolute path to action? */
	if((*def->maction == '/') ||  (*def->maction == '*')) {
		sp1 = NULL;
		m_itemp->path = NULL;
		subp = NULL;
	}

	(void) sprintf(act_path, "%s%s/%s/%s", sp1, pkgi, m_itemp->path, def->maction);
	actp1 = act_path;
	if(act_path[0] == '/') ++actp1;

	(void) sprintf(action_dir, "%s/%s/%s", subp, m_itemp->path, def->maction);

	/* get rid of any trailing '/' at end of path name */
	if((cp = strrchr(action_dir, '/')) != (char *)0) {
		item = cp + 1;
		*cp = '\0';
	}

	return(1);
}

/* get description from menu and put into temp file */
static struct item_def *
get_task(namep)
char	*namep;		/* comparison */
{
	struct item_def *itemptr;	/* pointer to menu item */

	/* search for entry */
	for (itemptr = file_str->entries; itemptr != (struct item_def *)0; itemptr = itemptr->next) {
		if(strncmp(namep, itemptr->mname, MAXITEMLEN) == 0) {
			/* menu item found */
			return(itemptr);
		}
	}
	return((struct item_def *)0);
}


/* find match in list at one level */
static TASKDESC *
find_first(p)
TASKDESC *p;
{
	TASKDESC *tmpp;

	for (tmpp = p; tmpp != (TASKDESC *)0; tmpp = tmpp->next) {
		if(tmpp->mark != 0) {
			/* match */
			return(tmpp);
		}
	}
	return((TASKDESC *)0);
}

/* read in internal express DB into a hierarchical structure */
static int
iexpr_read(rootp)
TASKDESC *rootp;
{
	TASKDESC *nodep;
	char *eof, inbfr[200];
	char *nodeid, *nodename, *parentid, *cmdstr;
	int   nodedig, parentdig;

	exec_list = (TASKDESC *)0;
	rn_list = (TASKDESC *)0;
	while ((eof = fgets(inbfr, 200, exp_file)) != (char *)0) {
		if((eof = strrchr(inbfr, '\n')) != (char *)0) *eof = '\0';
		if((nodeid = strtok(inbfr, "\t")) == NULL) continue;
		if((parentid = strtok(NULL, "\t")) == NULL)continue;
		if((nodename = strtok(NULL, "\t")) == NULL)continue;
		cmdstr = strtok(NULL, "\t"); /*optional*/

		if(sscanf(parentid, "%d", &parentdig) != 1) {
			/* conversion error */
			(void) fprintf(stderr, "UX:sysadm:ERROR:i_expr format error at %s\n",
				nodename);
		}
		if((nodep = find_node(rootp, parentdig)) == (TASKDESC *)0) {
			/* error -- cannot find parent node */
			(void) fprintf(stderr, "UX:sysadm:ERROR:i_expr error at %s\n", nodename);
		}

		nodedig = 0;
		if(*nodeid == 'e') {
			if(mk_mt_node(&exec_list, (TASKDESC *)0, nodename) != 0) return(-1);
			exec_list->action = strdup(cmdstr);
		} else if(*nodeid == 'r') {
			if(mk_mt_node(&rn_list, (TASKDESC *)0, nodename) != 0) return(-1);
			rn_list->child = nodep;
			rn_list->action = "R";
		} else if(*nodeid == 'p') {
			if(mk_mt_node(&ph_list, (TASKDESC *)0, nodename) != 0) return(-1);
		} else {
			/* numeric */
			if(sscanf(nodeid, "%d", &nodedig) != 1) {
				/* conversion error */
				(void) fprintf(stderr,
					"UX:sysadm:ERROR:i_expr format error at %s\n",
					nodename);
			}
			/* place child node under parent */
			if(mk_mt_node(&(nodep->child), nodep, nodename) != 0) return(-1);
			nodep->child->ident = nodedig;
			if(strcmp(cmdstr, "O") == 0) {
				/* OLD_SYSADM branch */
				nodep->child->action = "O";
			}
		}
	}
	return(0);
}


/* handle old_sysadm entry */
static void
old_entry(p)
TASKDESC  *p;	/* ptr to leaf node */
{
	char	*cp;
	char	*ss;
	int	len;

	/* double check that this is old entry */
	if(*(p->action) != 'O') return;

	path_lineage[0] = '\0';
	lineage(p, path_lineage);
	for (cp = path_lineage; *cp != '\0'; ++cp)
		if(*cp == ':') *cp = '/';
	--cp;
	if(*cp == '/') *cp = '\0';

	cp = path_lineage;
	ss = "main/applications/";
	len = strlen( ss );
	if(strncmp(cp, ss, len) == 0) {
		cp += len;
	}
	ss = "main/";
	len = strlen( ss );
	if(strncmp(cp, ss, len) == 0) {
		cp += len;
	}
	ss = OLD_SYS;
	len = strlen( ss );
	if(strncmp(cp, ss, len) == 0) {
		cp += len;
	}
	if(*cp == '/') ++cp;
	action_dir[0] = '\0';
	item = act_path;
	(void) sprintf(act_path, "%s %s", OLD_SYSADM, cp);
	actp1 = act_path;
	descp1 = "old sysadm path";
	return;
}
