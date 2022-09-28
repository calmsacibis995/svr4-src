/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamintf:intf_include/intf.h	1.6.5.1"

#define LNSZ 		256
#define MENULEN 	20
#define MAXITEMLEN 	16
#define NAMELEN		16
#define DESCLEN		58
#define ACTLEN		128
#define HELPLEN		15
#define PKGILEN		100
/*
 * Menu name length really determines MAXDIR. Since we use the
 * menu name to build the directory path, and we also limit the
 * menu name to NAMELEN a'la Appendix D, p. 15 of the SAG,
 * limit directory sizes accordingly. If the directory
 * name is longer than the s5 fs's limit of 14 characters - that's
 * ok; leave it up to the s5 fs to handle the truncation.
 *
 * HOWEVER, if we are talking about a menu file (foo.menu), then
 * we must respect the s5 fs's fondness for truncation. Since scripts
 * all over try to do pattern matches like '*.menu', we must
 * limit menu file names to a total of 14 chars, INCLUDING the .menu
 * suffix. This scheme will work on UFS as well as S5 FS, and pkgs
 * made on either will be installable/useable on either type.
 * Thus, MAXNAM can ONLY be 14 characters long.
 */
#define MAXDIR		256	/* arbitrary: FS will enforce it */
#define MAXSVDIR	14	/* S5 FS */
#define MAXNAM		MAXSVDIR-sizeof(MENU_SFX)+1
#define FULL_LINE	1
#define FIRST_ONLY	0
#define FOUND    	1
#define NOTFOUND 	0
#define DELETE		1
#define NO_DEL		0
#define TRUE		1
#define FALSE		0
#define SAME		0
#define DIFF		1
#define ISPHOLDER 	1
#define NONPHOLDER 	0
#define MENU 		0
#define EXPRESS		1	
#define FML_MENU	0
#define FML_FORM	1
#define FML_TEXT	2
#define IS_MENU		3
#define OTHER		4

#define DIRINFO " x 0755 root sys"	/* directory info for instalf */
#define FILEINFO " e 0644 root sys"	/* file info for instalf */
#define INTFCLASS "OAMintf"		/* oam interface class for instalf */

#define EXPR_LOG 	"I_expr_log"
#define INST_LOG 	"I_inst_log"
#define TAB_CHAR	'^'
#define TAB_DELIMIT 	"^"
#define DIR_DELIMIT	"/"
#define PATH_DELIMIT	":"
#define TMP_EXPR	"tmp.express"
#define EXP_FILE	"express"
#define INTFBASE	"INTFBASE"
#define OAMBASE		"OAMBASE"
#define OAM_PATH	"/usr/sadm/sysadm"
#define TESTBASE	"TESTBASE"
#define NONUNIQUE	"nonunique"
#define PHOLDER		"placeholder"
#define PHOLDERSZ	sizeof(PHOLDER)
#define TMP_NAME	"tmp.menu"
#define NTMP_NAME	"ntmp.menu"
#define MAIN_NAME	"main.menu"
#define MAIN		"main"
#define MAIN_PATH	"menu/"
#define BIN_PTH		"/bin"
#define MAIN_PTH	"/menu"
#define PKGBASE		"/add-ons/"
#define PKG_PTH		"/add-ons"
#define EXPR_PTH	"/express"
#define I_EXPR_PTH	"/i_expr"
#define I_EXPR_BLD	"/usr/sadm/install/bin/ie_build"

#define STD_PATH	"/sbin:/usr/bin:/usr/lbin:/etc:/usr/sadm/bkup/bin:/sbin:/usr/sbin:/usr/sadm/bin"
#define STD_SHL		"SHELL=/sbin/sh"

#define PATH_VAR	"PATH="
#define OAM_VAR		"OAMBASE="
#define INTF_VAR	"INTFBASE="
#define HOME_VAR	"HOME="
#define SDIR_VAR	"SYSDIR="
#define START_VAR	"SYSSTART="

#define LOC_IDENT	"*LOC*"

#define SAVHDR		"#save#"
#define MENUHDR		"#menu#"
#define HELPHDR		"#help#"
#define LIFEHDR		"#life#"
#define IDENT		"#ident"

#define LIFELINE	"#life#permanent\n"

#define HELP_SFX	".help"
#define MENU_SFX	".menu"
#define MENU_PFX	"Menu."
#define TEXT_PFX	"Text."
#define FORM_PFX	"Form."

#define PKGINST		"PKGINST"
#define DEL_STR		"#DELETE#"
#define VOID_STR	"#VOID#"

struct menu_item {	/* struct of menu item info - returned from find_menu */
	char *path;	/* physical path to menu containing item */
	char *menu_name;	/* name of menu containing item */
	int exists;		/* flag if item already exists */
	char *item;		/* menu item */
	char *par_menu;	/* parent menu - NULL if not placeholder */
	char *par_item;	/* menu item within parent - NULL if not placeholder */
  	char *leftover;	/* left over items not matched from input path */
  	char *pkginsts;	/* any package instances found on the menu line */
	char *m_desc;	/* descr line for parent menu item */
};

struct menu_line {	/* input structure for continued lines of input */
char line[LNSZ];	/* line of input from menu file */
struct menu_line *next;	/* pointer to next line of input */
};
