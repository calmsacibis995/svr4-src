/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamintf:intf_remove/remove.c	1.3.1.2"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "intf.h"
#include "rm_err.h"

#define WRITEIT		1
#define REMOVELN	0

/*
 * remove - removes all entries with the package instance stamp indicated
 *		by the environment variable PKGINST from the file name
 * 		given as an argument.
 */

struct menu_line in_line;		/* input line structure */
struct menu_line nxt_line;		/* another input line structure */
struct menu_line *ptr_line = &in_line;	/* pointer to input line structure */
struct menu_line *ptr_nxt = &nxt_line;	/* pointer: another input line struct */
struct menu_line s_line;		/* save line */
struct menu_line *ptr_s_line = &s_line;	/* pointer to save line */

int notuniq;				/* flag if nonunique expr line */
int pholderln;				/* flag if expr line is pholder */

char tmp_name[LNSZ];			/* temp file name */
char action[LNSZ];			/* saves action for pholder swap */
char nonuniqpkgs[LNSZ];			/* saves pkgs on nonunique expr line */

char tmp_filenam[] = "tmp.remove";	/* temp menu file name */
char tmp_exprnam[] = "tmp.expr";	/* temp express file name */
char dir_delimit[] = DIR_DELIMIT;	/* directory delimiter */

char *cmd_name;				/* name of command from argv[0] */

char *getenv();
char *read_item();
void deleteit();
void process_expr();

main(argc, argv)
int argc;
char **argv;
{
	int ret;		/* return from process_menu */
	int inptr;		/* pointer to position in input file name */
	int perm_file;		/* set if file cannot be removed */
	char *pkginst;		/* package instance */
	char *infilename;	/* input file name */
	char *line;		/* input line */
	char *nextln;		/* next input line */
	FILE *fileptr;		/* file pointer of input file name */
	FILE *tmpptr;		/* file pointer of tmp file */

	cmd_name = argv[0];

	ptr_line->next = NULL;
	ptr_nxt->next = NULL;

	if(argc != 2) {
		rm_err(ERR, cmd_name, USAGE);
		return(ERR_RET);
	}
	infilename = *(argv+1);
	pkginst = getenv("PKGINST");

	inptr = 0;
	/* find end of input file name */
	while(*(infilename+inptr) != NULL) inptr++;
	/* now backup over filename */
	while(*(infilename+inptr) != '/') inptr--;
	inptr++;	/* pick up '/' */
	(void) strncpy(tmp_name, infilename, inptr);
	*(tmp_name + inptr) = NULL;

	/* now open infilename */

	if((fileptr = fopen(infilename, "r")) == NULL) {
		if(access(infilename, 00) < 0) return(0);
		else {
			rm_err(ERR, cmd_name, FILE_OPN, infilename);
			return(ERR_RET);
		}
	}

	if((line = read_item(ptr_line, fileptr,1)) == NULL) {
		rm_err(ERR, cmd_name, FILE_RD, infilename);
		return(ERR_RET);
	}
	/* read second line */
	if((nextln = read_item(ptr_nxt, fileptr,1)) == NULL) {
		rm_err(ERR, cmd_name, FILE_RD, infilename);
		return(ERR_RET);
	}


	if(strncmp(line, IDENT, sizeof(IDENT)-1) == 0) {
		perm_file = 1;
		line = nextln;
	}

	if(strncmp(line, MENUHDR, sizeof(MENUHDR)-1) == 0) {

		/* open temp file for menu */

		(void) strcat(tmp_name, tmp_filenam);

		if((tmpptr = fopen(tmp_name, "a")) == NULL) {
			rm_err(ERR, cmd_name, FILE_OPN, tmp_name);
			return(ERR_RET);
		}

		if(perm_file) (void) write_item(ptr_line, tmpptr);
		(void) write_item(ptr_nxt, tmpptr);

		ret = process_menu(fileptr, tmpptr, pkginst);
		(void) fclose(fileptr);
		(void) fclose(tmpptr);
		if(ret == 0) { /* 0 lines written to tmp file */
			if(!perm_file) {
				(void) unlink(infilename);
				(void) unlink(tmp_name);
			}
			else (void) move_tmp(tmp_name, infilename);
		}
		else if(ret > 0) (void) move_tmp(tmp_name, infilename);
	}
	else {
		/* open temp file for express file */

		(void) strcat(tmp_name, tmp_exprnam);

		if((tmpptr = fopen(tmp_name, "w+")) == NULL) {
			rm_err(ERR, cmd_name, FILE_OPN, tmp_name);
			return(ERR_RET);
		}

		if(perm_file) (void) write_item(ptr_line, tmpptr);
		ptr_line = ptr_nxt;
		process_expr(fileptr, tmpptr, pkginst);
		(void) fclose(fileptr);
		(void) fclose(tmpptr);
		(void) move_tmp(tmp_name, infilename);
		return(SUCCESS_RET);
	}
	return(SUCCESS_RET);
}


int
process_menu(fileptr, tmpptr, pkginst)
FILE *fileptr;		/* file pointer for input file */
FILE *tmpptr;		/* file pointer for tmp file */
char *pkginst;		/* package instance being removed */
{
	int done;		/* flag to test read loop */
	int count_items;	/* count of menu items written to tmp file */
	char *inline;		/* input line */

	/*
	 * process_menu - assume that #menu# line has already been read
	 *		and written.  
	 */


	/* read & write help line and any following blank lines */
	done = 0;
	while(!done) {
		if((inline = read_item(ptr_line, fileptr,1)) == NULL) 
			return(0);
		else {
			if((strncmp(inline, HELPHDR, sizeof(HELPHDR)-1) == 0)) {
#ifdef OBSOLETE
				(void) write_item(ptr_line, tmpptr);
#endif
			}
			if((*inline == '\n')) {
				(void) write_item(ptr_line, tmpptr);
			}
			else done = 1;
		}
	}

	/* now we have the first real line read & ready for processing */
	count_items = 0;
				
	done = 0;
	while(!done) {
		if(del_pkg(ptr_line, pkginst, MENU) == WRITEIT) {
			(void) write_item(ptr_line, tmpptr);
			count_items++;
		}
		if((inline = read_item(ptr_line, fileptr,1)) == NULL)
			done = 1;
	}
		
	return(count_items);
}

int
del_pkg(p_line, pkginst, type)
struct menu_line *p_line;	/* menu line */
char *pkginst;			/* package instance to remove from line */
int type;			/* type of line: MENU or EXPRESS */
{

	int ispholder;		/* flag if placeholder needs to be swapped */
	int linebegin;		/* set it at beginning of line */
	char *aptr;		/* used to copy action during pholder swap */
	char *aposition;	/* used to save beginning position of action */
	char *aposptr;		/* used to copy action during pholder swap */
	char *line;		/* pointer to actual line */
	struct menu_line *mnu_ptr;	/* pointer to menu line structure */
	struct menu_line *prev;		/* pointer to prev menu line struct */

	/* if no pkginst identifiers on line, then write line because it's
	 * part of base OAM */
	if(cnt_pkgs(p_line, type) == 0) return(WRITEIT);

	/* if pkginst isn't on line, then write it */
	if(search_pkg(p_line, pkginst, type) == NOTFOUND) {
		return(WRITEIT);
	}

	/* at this point pkginst IS on line - remove it */

	mnu_ptr = p_line;
	line = p_line->line;

	while(*line++ != '^');	/* skip to descr beginning */
	while(*line++ != '^'); 	/* skip to action */

	/* save action position on line */
	aposition = line;

	/* skip to what's beyond action */
	if(!notuniq) {
		while((*line != '^') && (*line != NULL)
			&& (*line != '\\') && (*line != '\n')) line++;
	}

	ispholder = 0;
	if((type == MENU) && (*line == '^')) {	
		/* could be placeholder OR could be pkginst */
		/* check if placeholder. if so, note it and skip over */
		line++;
		if(strncmp(line, PHOLDER, strlen(PHOLDER)) == 0) ispholder = 1;
			/* save placeholder position on line */
		if(ispholder || (*line == '[')) { /* could be rename field */
			/* skip to what's beyond */
			while((*line != '^') && (*line != NULL)
				&& (*line != '\\') && (*line != '\n')) line++;
		}
	}

	for(;;) {
		linebegin = 0;
		/* now find pkginst & delete it from line */
		if((*line == '\n') || (*line == NULL)) return(WRITEIT);
		if(*line == '\\') {
			prev = mnu_ptr;
			mnu_ptr = mnu_ptr->next;
			if(mnu_ptr == NULL) return(WRITEIT);
			line = mnu_ptr->line;
			linebegin = 1;
		}
		if(*line == '^') line++;
		if(*line == '#') {
			line++;
			if(strncmp(line, pkginst, strlen(pkginst)) 
				== 0) {
				/* found it - now delete it */
				if(linebegin) line = mnu_ptr->line;
				else line-=2; /* line points to '#' */

				deleteit(line, mnu_ptr, prev);

				/* that's it - pkginst deleted so
				 * break out of loop */
				break;
			}
		}
		/* skip to next */
		while((*line != '^') && (*line != NULL)
			&& (*line != '\\') && (*line != '\n')) line++;
	}
	if(cnt_pkgs(p_line, type) == 0) {
		if((type == MENU) && (!ispholder)) return(REMOVELN);
		else if(type == MENU) {
			/* swap placeholder back */
			/* save action in action array because 'placeholder'
			 * would otherwise overwrite it */
			aptr = action;
			aposptr = aposition;
			while((*aptr++ = *aposptr++) != '^');
			*(aptr-1) = NULL;
			(void) strcpy(aposition, PHOLDER);
			(void) strcat(aposition, TAB_DELIMIT);
			(void) strncpy((aposition+strlen(PHOLDER)+1), action, 
				strlen(action));
		}
		else {	/* type == EXPRESS */
			return(REMOVELN);
		}
	}

	return(WRITEIT);
}

void
process_expr(fileptr, tmpptr, pkginst)
FILE *fileptr;		/* input file pointer */
FILE *tmpptr;		/* tmp file file pointer */
char *pkginst;		/* package instance identifier to remove */
{

	/* 
	 * process express mode lookup file - first line of file
	 * (after #ident line) has been read and is pointed to by
	 * ptr_line.
	 */

	char *inline;	/* pointer to input line */
	int done;	/* flag when finished */
	int moresvs;	/* flag while more save lines */
	int cnt_saves;	/* count of # of save lines following a line */
	int delete;	/* flag if line deleted */
	int nonunique;	/* flag if line is nonunique entry */
	long offset;	/* offset into tmp file */

	inline = ptr_line->line;
	done = 0;
	offset = (long) ftell(tmpptr);
	while(!done) {
		delete = 0;
		if(strncmp(inline, SAVHDR, sizeof(SAVHDR)-1) == 0) {
			/* it's a save line - write it out */
			(void) write_item(ptr_line, tmpptr);
		}
		else {
			offset = (long) ftell(tmpptr);
			if(del_pkg(ptr_line, pkginst, EXPRESS) == 
				WRITEIT) {
				(void) write_item(ptr_line, tmpptr);
			}
			/* del_pkg returned REMOVELN - remove line */
			else delete = 1;
			nonunique = notuniq;

			/* check all possible save lines */
			/* now check #save# lines to see if any match */
			moresvs = 1;
			cnt_saves = 0;
			while(moresvs) {
				if((inline = read_item(ptr_line, fileptr,1)) 
					== NULL) {
					done = 1;
					moresvs = 0;
				}
				if(strncmp(inline,SAVHDR,sizeof(SAVHDR)-1)!=0) {
					moresvs = 0;
				}
				else {
					if(del_pkg(ptr_line, pkginst, EXPRESS) 
						== WRITEIT) {
						(void) write_item(ptr_line, tmpptr);
						cnt_saves++;
					}
				}
			}
			if((cnt_saves == 1) && (nonunique || delete)) {
				/* it's no longer nonunique or save line */
				if(fseek(tmpptr, offset, 0) != 0) {
					rm_err(ERR, cmd_name, RM_ERR, inline);
				}
				/* read the first entry */
				(void) read_item(ptr_s_line, tmpptr,1);

				/* read the one save line */
				if(nonunique) 
					(void) read_item(ptr_s_line,tmpptr,1);

				/* now delete the nonunique entry - first seek*/
				if(fseek(tmpptr, offset, 0) != 0) {
					if(notuniq) {
						rm_err(ERR, cmd_name, RM_ERR, 
							inline);
					}
					else if(delete) {
						rm_err(ERR, cmd_name, RM_ERR, 
							inline);
					}
				}

				/* write the save line again */
				/* first remove the SAVHDR */
				(void) strcpy(ptr_s_line->line, ptr_s_line->line+
					sizeof(SAVHDR)-1);
				(void) write_item(ptr_s_line, tmpptr);
			}
		}
	}
}

int
cnt_pkgs(p_line, type)
struct menu_line *p_line;	/* line to count pkginst's on */
int type;			/* type of line - MENU or EXPRESS */
{
	int count;			/* count of pkginst's on line */
	char *line;			/* pointer to line */
	struct menu_line *mnu_ptr;	/* pointer to menu item */

	notuniq = 0;
	pholderln = 0;
	mnu_ptr = p_line;
	line = p_line->line;

	while(*line++ != '^');	/* skip to descr beginning */
	if(type == EXPRESS) {
		/* check and flag if it's nonunique */
		if(strncmp(line, NONUNIQUE, sizeof(NONUNIQUE) -1) == 0)
			notuniq = 1;
		else if(strncmp(line, PHOLDER, sizeof(PHOLDER) -1) == 0) {
			pholderln = 1;
			return(0);	/* 0 pkginst's on placeholder line */
		}
	}
	while(*line++ != '^'); 	/* skip to action */
	if(notuniq) (void) strcpy(nonuniqpkgs, line);

	/* else skip to what's beyond action */
	else while((*line != '^') && (*line != NULL)
		&& (*line != '\\') && (*line != '\n')) line++;

	if((type == MENU) && (*line == '^')) {	
		/* could be placeholder OR could be pkginst */
		/* OR could be rename field */
		/* check if placeholder. if so, skip over */
		line++;
		if((*line == '[') || (strncmp(line, PHOLDER, strlen(PHOLDER)) 
			== 0)) {
			/* skip to what's beyond */
			while((*line != '^') && (*line != NULL)
				&& (*line != '\\') && (*line != '\n')) line++;
		}
	}

	count = 0;
	for(;;) {
		/* now count pkginst's on line */
		if((*line == '\n') || (*line == NULL)) return(count);
		if(*line == '\\') {
			mnu_ptr = mnu_ptr->next;
			if(mnu_ptr == NULL) return(count);
			line = mnu_ptr->line;
		}
		if(*line == '^') line++;
		if(*line == '#') {
			count++;
		}
		/* skip to next */
		while((*line != '^') && (*line != NULL)
			&& (*line != '\\') && (*line != '\n')) line++;
	}
}

void
deleteit(line, mnu_ptr, prev)
char *line;		/* position of pkg to delete */
struct menu_line *mnu_ptr;	/* current line of item */
struct menu_line *prev;	/* previous line of item in case need to add '\' */
{
	int i;		/* save position in line */
	/* find end of pkginst */
	/* start at second char of 'line' because first is '^' */
	i = 1;
	while((*(line+i) != '^') && 
		(*(line+i) != NULL) &&
		(*(line+i) != '\\') &&
		(*(line+i) != '\n')) i++;
	(void) strcpy(line, line+i);
	/* if nothing is left on line, delete */
	line = mnu_ptr->line;
	if(strlen(line) < 4) { /* 2 #'s, 1 nl,
			      * & 1 other */
		/* nothing left on line */
		/* null it out and delete continuation from prev */
		*line = NULL;
		line = prev->line;
		while(*line != '\n') line++;
		*(line-1) = '\n';
		*line = NULL;
		/* don't worry about freeing -
		 * (mnu_ptr)
		 * that's taken care of when
		 * the next read_item occurs */
	}
}
