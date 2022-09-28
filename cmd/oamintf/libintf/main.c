/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamintf:libintf/main.c	1.1.2.2"

#include <stdio.h>

char menusfx[] = ".menu";
char t_name[] = "tmp.menu";

extern char *find_menu();
main(argc, argv)
int argc;
char *argv[];
{
	char *dir_path;
	char *menu_name;
	char *item_name;
	char *par_menu;
	char *par_item;
	char path[];


	dir_path= find_menu(argv[1], &menu_name,&item_name,&par_menu,&par_item);
	(void) printf("%s/%s\n", dir_path,item_name);

}
