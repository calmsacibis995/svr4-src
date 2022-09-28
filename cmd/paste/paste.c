/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)paste:paste.c	1.5"

#
/* paste: concatenate corresponding lines of each file in parallel. Release 1.4 */
/*	(-s option: serial concatenation like old (127's) paste command */
# include <stdio.h>	/* make :  cc paste.c  */
# define MAXOPNF 12  	/* maximal no. of open files (not with -s option) */
# define MAXLINE 512  	/* maximal line length */
#define RUB  '\177'
	char del[MAXLINE] = {"\t"};
  
main(argc, argv)
int argc;
char ** argv;
{
	int i, j, k, eofcount, nfiles, maxline, glue;
	int delcount = { 1 } ;
	int onefile  = { 0 } ;
	register int c ;
	char outbuf[MAXLINE], l, t;
	register char *p;
	FILE *inptr[MAXOPNF];
  
	maxline = MAXLINE -2;
 
	while (argc > 1 && argv[1][0] == '-' && (c = argv[1][1]) != '\0'){
		switch (c) {
			case 's' :  onefile++;
				c = argv[1][2];
				argv[1]++;
				break ;
			case 'd' : argv[1] += 2;
				if((delcount = move(argv[1], &del[0])) == 0) 
					diag("paste: no delimiters\n",NULL);
				break;
			default :
				diag("Usage: paste [-s] [-d<delimiterstring>] file1 file2 ...\n",NULL);
				break;
		}
		--argc;
		++argv;
	} /* end options */
	--argc;
 
	if ( ! onefile) {	/* not -s option: parallel line merging */
		for (i = 0; argc >0 && i < MAXOPNF; i++) {
			if (argv[i + 1][0] == '-') {
				inptr[i] = stdin;
			} else inptr[i] = fopen(argv[i + 1], "r");
			if (inptr[i] == NULL) {
				diag("paste: cannot open %s\n",argv[i + 1]);
			}
		argc--;
		}
		if (argc > 0) diag("paste: too many files- limit %d\n",MAXOPNF);
		nfiles = i;
  
		do {
			p = &outbuf[0];
			eofcount = 0;
			j = k = 0;
			for (i = 0; i < nfiles; i++) {
				while((c = getc(inptr[i])) != '\n' && c != EOF)   {
					if (++j <= maxline) *p++ = c ;
					else {
					diag("paste: line too long\n",NULL);
					}
				}
				if ( (l = del[k]) != RUB) *p++ = l;
				k = (k + 1) % delcount;
				if( c == EOF) eofcount++;
			}
			if (l != RUB) *--p = '\n'; else  *p = '\n';
			*++p = 0;
			if (eofcount < nfiles) fputs(outbuf, stdout);
		}while (eofcount < nfiles);
  
	} else {	/* -s option: serial file pasting (old 127 paste command) */
		p = &outbuf[0];
		glue = 0;
		j = 0;
		k = 0;
		t = 0;
		for (i = 1; i <= argc; i++) {
			if (argv[i][0] == '-') {
				inptr[0] = stdin;
			} else inptr[0] = fopen(argv[i], "r");
			if (inptr[0] == NULL) {
				diag("paste: cannot open %s\n",argv[i]);
			}
	  
			while((c = getc(inptr[0])) != EOF)   {
				if (j >= maxline) {
					t = *--p;
					*++p = 0;
					fputs(outbuf, stdout);
					p = &outbuf[0];
					j = 0;
				}
				if (glue) {
					glue = 0;
					l = del[k];
					if (l != RUB) {
						*p++ = l ;
						t = l ;
						j++;
					}
					k = (k + 1) % delcount;
				}
				if(c != '\n') {
					*p++ = c;
					t = c;
					j++;
				} else glue++;
			}
			if (t != '\n') {
				*p++ = '\n';
				j++;
			}
			if (j > 0) {
				*p = 0;
				fputs(outbuf, stdout);
			}
		}
	}
}
diag(s,arg)
char *s,*arg;
{
	fprintf(stderr, s, arg);
	exit(1);
}
  
move(from, to)
char *from, *to;
{
int c, i;
	i = 0;
	do {
		c = *from++;
		i++;
		if (c != '\\') *to++ = c;
		else { c = *from++;
			switch (c) {
				case '0' : *to++ = RUB;
						break;
				case 't' : *to++ = '\t';
						break;
				case 'n' : *to++ = '\n';
						break;
				default  : *to++ = c;
						break;
			}
		}
	} while (c) ;
return(--i);
}
