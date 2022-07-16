/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)boot:boot/at386/tool/setfilter.c	1.1.4.1"
#include <stdio.h>

/*
 *	Maximum number of search patterns.
 */
#define	MAX_LIST	100

/*
 *	Total number of characters + attribute characters in search patterns.
 */
#define	MAX_CHARS	2000

/*
 *	Input buffer (read in from file) size
 */
#define	MAX_BUF		80

#define	IS_WHITE(c)	((c) == '\t' || (c) == ' ')

/*
 *	Each list structure refers to a single pattern to be searched.
 */
struct	list	{
		struct list *prev, *next;	/* doubly linked list - ease in */
						/* deletion */
		char *ptr;			/* pointer to pattern string */
		int len;			/* pattern string length */
						/* Each pattern string should be */
						/* embedded in double quotes */
		}	list[MAX_LIST];

char	buffers[MAX_CHARS];			/* Buffer for pattern string */

char	ibufp[MAX_BUF];				/* The input buffer */

int	done = 0;				/* All patterns processed flag */

FILE	*ifp = (FILE *) NULL,
	*efp = (FILE *) NULL;			/* Extraction file pointer */

extern	char *fgets();				/* unfortunately we use this */

char	*efilename = (char *) NULL;		/* Extraction file name */

char	target[] = "\t.string\t";		/* The target string to search */
int	target_len;				/* And its length to be computed */

/*
 *	Macro to scan the pattern (source) file and buffer in core the pattern
 *	strings.
 *	Each pattern has a pattern attribute followed by the pattern string
 *	embedded within double quotes, e.g.,
 *
 *	Attribute	Pattern-String
 *	0		"hdp_ncyl"
 *
 *	For now pattern type 0 is supported.
 */

#define	scan_input(ibufp, obufp, n)	{					\
	register int i, j;							\
	register int limit = MAX_BUF - 2;					\
										\
	for (i = 0; i < limit && IS_WHITE(ibufp[i]); i++);			\
	if (i == limit) error("missing string");				\
	j = i;									\
	*obufp++ = ibufp[i++];							\
	for ( ; i < limit && ibufp[i] != 10; i++);				\
	ibufp[i]='\0';							\
	list[n].len = (i - j)  ;						\
	list[n].ptr = obufp;							\
	strcpy(obufp, ibufp + j);						\
	obufp += (i - j)+1 ;							\
	if (n == 0)								\
		list[0].prev = list[0].next = &list[0];				\
	else {									\
		list[n].prev = &list[n - 1];					\
		list[n].next = &list[0];					\
		list[n-1].next = list[0].prev = &list[n];			\
	}									\
}

main(argc, argv)
	int	argc;
	char	*argv[];
{
	int		n = 0;				/* number of patterns */
	char		*obufp = (char *) NULL;		/* buffer address where */
							/* patterns are stored in */
							/* memory */

	if ((efilename=argv[1]) == NULL)
		error("Extract file open error");

	ifp=stdin;

again:
	n = 0;				/* Start fresh - number of patterns */
	if (obufp) {			/* We did not have enough memory to */
					/* process all patterns.	    */
					/* Process the pattern remaining from */
					/* the previous scan */
		obufp = buffers;
		scan_input(ibufp, obufp, n);
		n++;
	}
	else	obufp = buffers;

	/*
	 *	Process all patterns - until no more memory or end of input.
	 */
	for (; (n < MAX_LIST) && ((buffers + MAX_CHARS - obufp) > MAX_BUF); n++) {
		if (fgets(ibufp, MAX_BUF, ifp) == (char *) NULL) {
			done = 1;
			fclose(ifp);
			goto out;
		}
#ifdef DEBUG
		printf("input read: %s\n",ibufp);
#endif
		scan_input(ibufp, obufp, n);
	}
out:
	do_extract(n);
	if (done == 0)			/* More patterns left due to insufficent */
		goto again;		/* memory? - Process remaining patterns  */
	exit(0);			/* Success! */
}

do_extract(n)
	int n;
{
	register	int i;
	struct list	*list_head, *p;
	char		*extract_0_ptr, *extract_0();
	char		ostring[MAX_BUF];
	char		*ostringp, *tp;
	target_len = strlen(target);	/* The target string length */

#ifdef DEBUG
	printf("extraction process\n");
	printf("------------------------ List structures ------------------------\n");
	printf("index \t string \t\t\t len \t prev \t next \t attr\n");
	for (i = 0; i < n; i++)
		printf("%d \t >%s< \t\t\t %d \t %d \t %d \t %c\n",
			i, list[i].ptr, list[i].len, (int) (list[i].prev - &list[0]),
			(int) (list[i].next - &list[0]), *(list[i].ptr - 1));
	printf("----------------------------------------------------------------\n");
	printf("target string length = %d target string = >%s<\n",
		target_len,target);
#endif

	/*
	 *	Open the extraction file.
	 */
	if ((efp = fopen(efilename,"r")) == (FILE *) NULL)
		error("extraction file open error");


	list_head = &list[0];			/* Pattern list head pointer */
	for (i = 0; i <= target_len; ibufp[i] = '\0', i++);	/* Flush buffer */

	/*
	 *	Process each line in the extraction file.
	 */
	while (fgets(ibufp, MAX_BUF, efp) != (char *) NULL) {
#ifdef DEBUG
		printf("extract: %s\n",ibufp);
#endif
		/*
		 *	Found a target string: "\t.string\t".
		 */

		if (strncmp(ibufp, target, target_len) != 0) 
			goto next;

		/*
		 *	Scan the patern list for a match.
		 */
		for (p = list_head, i = 0; i < n; i++, p = p->next) {
			/* String matches? */
			ostringp=ibufp+target_len+1;
			if (strncmp(ostringp,p->ptr, p->len) != 0)
				continue;
#ifdef DEBUG
			printf("match for: %s\n",p->ptr);
			printf("ibuf = %s\n",ibufp);
#endif
			tp=ostring;
			while (*ostringp != '\"')
				*tp++=*ostringp++;
			*tp='\0';
			do {
				fgets(ibufp,MAX_BUF,efp);
			} while (strlen(ibufp) < 40);
			ibufp[MAX_BUF - 1] = '\0';
#ifdef DEBUG
			printf("%s len = %d %s\n",p->ptr, strlen(ibufp), ibufp);
#endif
			/* So a printf will work */
#if 0
			*(p->ptr + p->len + 1) = '\0';
#endif
			/* get the string to be extracted */

			extract_0_ptr = extract_0(ibufp);
			printf("\t.set	%s,	%s\n", ostring, extract_0_ptr);

#if 0
			/* Remove pattern from Linked List - already done */
			p->next->prev = p->prev;
			p->prev->next = p->next;

			/* Fix head of list - if this is the one removed */
			if (p == list_head) list_head = list_head->next;

			/* One less pattern to be extracted */
			n--;
#endif
			goto next;
		}
next:
		/* Clear buffer */
		for (i = 0; i < target_len; ibufp[i] = '\0', i++);
	}
	if (efp)
		fclose(efp);
	return(n);		/* Must be zero - else not all patterns extracted */
}

usage()
{
	fprintf(stderr,"\nextract -s sourcefile -e extractfile\n");
	exit(1);
}

error(s)
	char *s;
{
	if (done == 0 && ifp)
		fclose(ifp);
	if (done == 0 && efp)
		fclose(efp);
	fprintf(stderr,"error: %s\n",s);
	exit(1);
}

char *
extract_0(s)
	char *s;
{
	char *p, *ret;

	for (p = s; p && *p; p++) {
		if (*p == '.' && ++p && *p && strncmp(p,"4byte",5) == 0) {
			p += 5;
			ret = p + 1;
			if (!p || !*p || *p != ' ')
				error("invalid format in Extraction file");
			for (++p; p && *p && *p != ';'; p++)
				;
			if (!p || !*p)
				error("invalid format in the Extraction file");
			*p = '\0';
			goto found_0;
		}
	}
	error("invalid format in extraction file");
found_0:
	return(ret);
}
