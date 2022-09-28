/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)iconv:iconv.c	1.2.3.1"

/*
 * iconv.c	code set conversion
 */

#include <stdio.h>
#include <sys/errno.h>
#include <dirent.h>
#include <string.h>
#include <iconv.h>

#define DIR_DATABASE 		"/usr/lib/iconv"	/* default database */
#define FILE_DATABASE 		"iconv_data"	/* default database */
#define MAXLINE 	1282			/* max chars in database line */
#define MINFLDS 	4			/* min fields in database */
#define FLDSZ 		257			/* max field size in database */

extern int errno;
extern int optind;
extern int opterr;
extern char *optarg;

extern struct kbd_tab *gettab();

main(argc, argv)
int argc;
char **argv;
{
register int c;
char *fcode;
char *tcode;
char *d_data_base = DIR_DATABASE;
char *f_data_base = FILE_DATABASE;
char from[FLDSZ];
char to[FLDSZ];
char table[FLDSZ];
char file[FLDSZ];
struct kbd_tab *t;
int fd;


	fcode = (char*)NULL;
	tcode = (char*)NULL;
	c = 0;

	/*
	 * what about files
	 */
	while ((c = getopt(argc, argv, "f:t:")) != EOF) {

		switch (c) {

			case 'f':
				fcode = optarg;
				break;	

			case 't':
				tcode = optarg;
				break;

			default:
				usage_iconv(1);
		}

	}

	/* required arguments */
	if (!fcode || !tcode)
		usage_iconv(2);

	if (optind < argc) {

		/*
		 * there is a file
		 */
		fd = open(argv[optind],0);
		if (fd < 0) {
			fprintf(stderr,"Can't open %s\n",argv[optind]);
			exit(1);
		}
	} else
		fd = 0;

	if (search_dbase(file,table,d_data_base,f_data_base,(char*)0,fcode,tcode)) {		

		/*
		 * got it so set up tables
		 */
		t = gettab(file,table,d_data_base,f_data_base,0);
		if (!t) {
		    fprintf(stderr,"Cannot access conversion table (%s) (%d)\n",
                                	table,errno);
			exit(1);
		}
		process(t,fd,0);

	} else {

		fprintf(stderr,"Not supported %s to %s\n",fcode,tcode);
		exit(1);
	}
		
	exit(0);
}


usage_iconv(x)
{
	fprintf(stderr, "Usage: iconv -f fromcode -t tocode [file] (%d)\n", x);
	exit(1);
}


search_dbase(o_file, o_table, d_data_base, f_data_base, this_table, fcode, tcode)
char        *o_file,*o_table,*d_data_base,*f_data_base,*this_table,*fcode,*tcode;
{
int fields;
int row;
char buff[MAXLINE];
FILE *dbfp;
char from[FLDSZ];
char to[FLDSZ];
char data_base[MAXNAMLEN];

	fields = 0;

	from[FLDSZ-1] = '\0';
	to[FLDSZ-1] = '\0';
	o_table[FLDSZ-1] = '\0';
	o_file[FLDSZ-1] =  '\0';
	buff[MAXLINE-2] = '\0';

	sprintf(data_base,"%s/%s",d_data_base,f_data_base);

	/* open database for reading */
	if ((dbfp = fopen(data_base, "r")) == NULL) {
		fprintf(stderr,"Cannot access data base %s (%d)\n",
                                	data_base, errno);
			exit(1);
	}

	/* start the search */

	for (row=1; fgets(buff, MAXLINE, dbfp) != NULL ; row++) {

		if (buff[MAXLINE-2] != NULL) {
			fprintf(stderr, "Database Error : row %d has more than %d characters\n",
					row, MAXLINE-2);
			exit(1);
		}

		fields = sscanf(buff, "%s %s %s %s", from, to, o_table, o_file);
		if (fields < MINFLDS) {
			fprintf(stderr, "Database Error : row %d cannot retrieve required %d fields\n",
					row, MINFLDS);
			exit(1);
		}

		if ( (from[FLDSZ-1] != NULL) || (to[FLDSZ-1] != NULL) ||
		     (o_table[FLDSZ-1] != NULL) || (o_file[FLDSZ-1] != NULL) ) {
			fprintf(stderr, "Database Error : row %d has a field with more than %d characters\n",
					row, FLDSZ-1);
			exit(1);
		}

		if (this_table) {

			if (strncmp(this_table,o_table,KBDNL) == 0) {

				fclose(dbfp);
				return 1;

			}
		} else
		if (strcmp(fcode, from) == 0 && strcmp(tcode, to) == 0) {

			fclose(dbfp);
			return 1;
		}
	}

	fclose(dbfp);
	return 0;
}

