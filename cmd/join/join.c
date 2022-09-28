/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)join:join.c	1.6"
/*	join F1 F2 on stuff */

#include	<stdio.h>
#define F1 0
#define F2 1
#define	NFLD	20	/* max field per line */
#define comp() cmp(ppi[F1][j1],ppi[F2][j2])
#define putfield(string) if(*string == NULL) (void) fputs(null, stdout); \
			else (void) fputs(string, stdout)

FILE *f[2];
char buf[2][BUFSIZ];	/*input lines */
char *ppi[2][NFLD];	/* pointers to fields in lines */
int	j1	= 1;	/* join of this field of file 1 */
int	j2	= 1;	/* join of this field of file 2 */
int	olist[2*NFLD];	/* output these fields */
int	olistf[2*NFLD];	/* from these files */
int	no;	/* number of entries in olist */
int	sep1	= ' ';	/* default field separator */
int	sep2	= '\t';
char*	null	= "";
int	aflg;

main(argc, argv)
char *argv[];
{
	int i;
	int n1, n2;
	long top2, bot2;
	long ftell();

	while (argc > 1 && argv[1][0] == '-') {
		if (argv[1][1] == '\0')
			break;
		switch (argv[1][1]) {
		case 'a':
			switch(argv[1][2]) {
			case '1':
				aflg |= 1;
				break;
			case '2':
				aflg |= 2;
				break;
			default:
				aflg |= 3;
			}
			break;
		case 'e':
			null = argv[2];
			argv++;
			argc--;
			break;
		case 't':
			sep1 = sep2 = argv[1][2];
			break;
		case 'o':
			for (no = 0; no < 2*NFLD; no++) {
				if (argv[2][0] == '1' && argv[2][1] == '.') {
					olistf[no] = F1;
					olist[no] = atoi(&argv[2][2]);
				} else if (argv[2][0] == '2' && argv[2][1] == '.') {
					olist[no] = atoi(&argv[2][2]);
					olistf[no] = F2;
				} else
					break;
				argc--;
				argv++;
			}
			break;
		case 'j':
			if (argv[1][2] == '1')
				j1 = atoi(argv[2]);
			else if (argv[1][2] == '2')
				j2 = atoi(argv[2]);
			else
				j1 = j2 = atoi(argv[2]);
			argc--;
			argv++;
			break;
		}
		argc--;
		argv++;
	}
	for (i = 0; i < no; i++)
		olist[i]--;	/* 0 origin */
	if (argc != 3)
		error("usage: join [-an] [-e s] [-jn m] [-tc] [-o list] file1 file2");
	j1--;
	j2--;	/* everyone else believes in 0 origin */
	if (argv[1][0] == '-')
		f[F1] = stdin;
	else if ((f[F1] = fopen(argv[1], "r")) == NULL)
		error("can't open %s", argv[1]);
	if ((f[F2] = fopen(argv[2], "r")) == NULL)
		error("can't open %s", argv[2]);

#define get1() n1=input(F1)
#define get2() n2=input(F2)
	get1();
	bot2 = ftell(f[F2]);
	get2();
	while(n1>0 && n2>0 || aflg!=0 && n1+n2>0) {
		if(n1>0 && n2>0 && comp()>0 || n1==0) {
			if(aflg&2) output(0, n2);
			bot2 = ftell(f[F2]);
			get2();
		} else if(n1>0 && n2>0 && comp()<0 || n2==0) {
			if(aflg&1) output(n1, 0);
			get1();
		} else /*(n1>0 && n2>0 && comp()==0)*/ {
			while(n2>0 && comp()==0) {
				output(n1, n2);
				top2 = ftell(f[F2]);
				get2();
			}
			(void) fseek(f[F2], bot2, 0);
			get2();
			get1();
			for(;;) {
				if(n1>0 && n2>0 && comp()==0) {
					output(n1, n2);
					get2();
				} else if(n1>0 && n2>0 && comp()<0 || n2==0) {
					(void) fseek(f[F2], bot2, 0);
					get2();
					get1();
				} else /*(n1>0 && n2>0 && comp()>0 || n1==0)*/{
					(void) fseek(f[F2], top2, 0);
					bot2 = top2;
					get2();
					break;
				}
			}
		}
	}
	return(0);
}

input(n)		/* get input line and split into fields */
{
	register int i, c;
	char *bp;
	char **pp;

	bp = buf[n];
	pp = ppi[n];
	if (fgets(bp, BUFSIZ, f[n]) == NULL)
		return(0);
	i = 0;
	do {
		i++;
		if (sep1 == ' ')	/* strip multiples */
			while ((c = *bp) == sep1 || c == sep2)
				bp++;	/* skip blanks */
		else
			c = *bp;
		*pp++ = bp;	/* record beginning */
		while ((c = *bp) != sep1 && c != '\n' && c != sep2 && c != '\0')
			bp++;
		*bp++ = '\0';	/* mark end by overwriting blank */
			/* fails badly if string doesn't have \n at end */
	} while (c != '\n' && c != '\0');

	*pp = 0;
	return(i);
}

output(on1, on2)	/* print items from olist */
int on1, on2;
{
	int i;

	if (no <= 0) {	/* default case */
		if (on1)
			putfield(ppi[F1][j1]);
		else
			putfield(ppi[F2][j2]);
		for (i = 0; i < on1; i++)
			if (i != j1) {
				(void) putchar(sep1);
				putfield(ppi[F1][i]);
			}
		for (i = 0; i < on2; i++)
			if (i != j2) {
				(void) putchar(sep1);
				putfield(ppi[F2][i]);
			}
		(void) putchar('\n');
	} else {
		for (i = 0; i < no; i++) {
			if(olistf[i]==F1 && on1<=olist[i] ||
			   olistf[i]==F2 && on2<=olist[i])
				(void) fputs(null, stdout);
			else
				putfield(ppi[olistf[i]][olist[i]]);
			if (i < no - 1)
				(void) printf("%c", sep1);
			else
				(void) putchar('\n');
		}
	}
}

/*VARARGS*/
error(s1, s2, s3, s4, s5)
char *s1;
{
	(void) fprintf(stderr, "join: ");
	(void) fprintf(stderr, s1, s2, s3, s4, s5);
	(void) fprintf(stderr, "\n");
	exit(1);
}

cmp(s1, s2)
char *s1, *s2;
{
	if (s1 == NULL) {
		if (s2 == NULL)
			return(0);
		else
			return(-1);
	} else if (s2 == NULL)
		return(1);
	return(strcmp(s1, s2));
}
