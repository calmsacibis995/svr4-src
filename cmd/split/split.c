/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)split:split.c	1.9"
/*	Portions Copyright (c) 1988, Sun Microsystems, Inc.	*/
/*	All Rights Reserved. 					*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/statvfs.h>

char	*strrchr();
unsigned count = 1000;
int	fnumber;
char	fname[100];
char	head[1024];
char	*ifil;
char	*ofil;
char	*tail;
char	*last;
FILE	*is;
FILE	*os;

main(argc, argv)
char *argv[];
{
	register i, c, f;
	int iflg = 0;
	struct statvfs stbuf;

	for(i=1; i<argc; i++)
		if(argv[i][0] == '-')
			switch(argv[i][1]) {
		
			case '\0':
				iflg = 1;
				continue;
		
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				count = atoi(argv[i]+1);
				continue;
			}
		else if(iflg)
			ofil = argv[i];
		else {
			ifil = argv[i];
			iflg = 2;
		}
	if (count <= 0) {
		fprintf(stderr, "Usage: split [ -# ] [ file [ name ] ]\n");
		exit(1);
	}
	if(iflg != 2)
		is = stdin;
	else
		if((is=fopen(ifil,"r")) == NULL) {
			fprintf(stderr,"cannot open input\n");
			exit(1);
		}
	if(ofil == 0)
		ofil = "x";
	else {
		if ((tail = strrchr(ofil, '/')) == NULL) {
			tail = ofil;
			getcwd(head, sizeof(head));
		}
		else {
			tail++;
			strcpy(head, ofil);
			last = strrchr(head, '/');
			*++last = '\0';
		}
		
		if (statvfs(head, &stbuf) < 0) {
			perror(head);
			exit(1);
		}

		if (strlen(tail) > (stbuf.f_namemax-2) ) {
			fprintf(stderr,"more than %d characters in output file name\n", stbuf.f_namemax-2);
			exit(1);
		}
	}

loop:
	f = 1;
	for(i=0; i<count; i++)
	do {
		c = getc(is);
		if(c == EOF) {
			if(f == 0)
				fclose(os);
			exit(0);
		}
		if(f) {
			for(f=0; ofil[f]; f++)
				fname[f] = ofil[f];
			fname[f++] = fnumber/26 + 'a';
			fname[f++] = fnumber%26 + 'a';
			fname[f] = '\0';
			if(++fnumber > 676) {
				fprintf(stderr,"more than aa-zz output files needed, aborting split\n");
				exit(1); }
			if((os=fopen(fname,"w")) == NULL) {
				fprintf(stderr,"Cannot create output\n");
				exit(1);
			}
			f = 0;
		}
		putc(c, os);
	} while(c != '\n');
	fclose(os);
	goto loop;
}
