/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)localedef:montbl/montbl.c	1.2.4.1"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <locale.h>
#include <limits.h>

#define MONSIZ	16
#define NUMSTR	8
#define NUMCHAR	8

extern char	*optarg;
extern int	optind;

main(argc, argv)
int argc;
char **argv;
{
	FILE	*infile, *outfile;
	char	line[BUFSIZ];
	char	buf[BUFSIZ];
	int	i, j;
	char	*ptr, *nptr;
	char	*monstrs[NUMSTR];
	char	monchars[NUMCHAR];
	int	lineno;
	int	offset;
	int	c;
	int	errflag = 0;
	char	*outname = NULL;
	struct lconv	mon;

	/* Process command line */
	while ((c = getopt(argc, argv, "o:")) != -1) {
		switch(c) {
		case 'o':
			outname = optarg;
			break;
		case '?':
			errflag++;
		}
	}
	if (errflag || optind != argc - 1) {
		fprintf(stderr, "usage: montbl [-o outfile] infile\n");
		exit(1);
	}
	if (outname == NULL)
		outname = "LC_MONETARY";
	if ((infile = fopen(argv[optind], "r")) == NULL) {
		fprintf(stderr, "Cannot open input: %s\n", argv[1]);
		exit(1);
	}

	/* Process input file */
	i = 0;
	lineno = 1;
	while (fgets(line, BUFSIZ, infile) != NULL) {
		lineno++;
		if (strlen(line) == BUFSIZ-1 && line[BUFSIZ-1] != '\n') {
			fprintf(stderr,"line %d: line too long\n", lineno);
			exit(1);
		}
		if (line[0] == '#')	/* comment */
			continue;
		ptr = line;
		j = 0;
		while (*ptr != '\n' && *ptr != '\0') {
			if (*ptr == '\\') {
				switch (ptr[1]) {
				case '"': buf[j++] = '"'; ptr++; break;
				case 'n': buf[j++] = '\n'; ptr++; break;
				case 't': buf[j++] = '\t'; ptr++; break;
				case 'f': buf[j++] = '\f'; ptr++; break;
				case 'r': buf[j++] = '\r'; ptr++; break;
				case 'b': buf[j++] = '\b'; ptr++; break;
				case 'v': buf[j++] = '\v'; ptr++; break;
				case 'a': buf[j++] = '\7'; ptr++; break;
				case '\\': buf[j++] = '\\'; ptr++; break;
				default:
					if (ptr[1] == 'x') {
						ptr += 2;
						buf[j++] = strtol(ptr, &nptr, 16);
						if (nptr != ptr) {
							ptr = nptr;
							continue;
						} else
							buf[j-1] = 'x';
					} else if (isdigit(ptr[1])) {
						ptr++;
						buf[j++] = strtol(ptr, &nptr, 8);
						if (nptr != ptr) {
							ptr = nptr;
							continue;
						} else
							buf[j-1] = *ptr;
					} else
						buf[j++] = ptr[1];
				}
			} else
				buf[j++] = *ptr;
			ptr++;
		}
		buf[j] = '\0';

		if (i < NUMSTR) {
			if ((monstrs[i++] = strdup(buf)) == NULL) {
				fprintf(stderr, "Out of space\n");
				exit(1);
			}
		} else if (i >= MONSIZ) {
			fprintf(stderr, "Incorrect number of fields specified, %d\n", i);
			exit(1);
		} else {
			monchars[i - NUMSTR] = strtol(buf, &ptr, 0);
			if (monchars[i - NUMSTR] > CHAR_MAX || *ptr != '\0')
				fprintf(stderr, "line %d: bad input\n", lineno);
			i++;
		}
	}
	fclose(infile);

	/* initialize data structures for output */
	i = 0;
	offset = 0;
	mon.decimal_point = NULL;
	mon.thousands_sep = NULL;
	mon.grouping = (char *)offset;
	offset += strlen(monstrs[i++]) + 1;
	mon.int_curr_symbol = (char *)offset;
	offset += strlen(monstrs[i++]) + 1;
	mon.currency_symbol = (char *)offset;
	offset += strlen(monstrs[i++]) + 1;
	mon.mon_decimal_point = (char *)offset;
	offset += strlen(monstrs[i++]) + 1;
	mon.mon_thousands_sep = (char *)offset;
	offset += strlen(monstrs[i++]) + 1;
	mon.mon_grouping = (char *)offset;
	offset += strlen(monstrs[i++]) + 1;
	mon.positive_sign = (char *)offset;
	offset += strlen(monstrs[i++]) + 1;
	mon.negative_sign = (char *)offset;
	offset += strlen(monstrs[i]) + 1;

	i = 0;
	mon.int_frac_digits = monchars[i++];
	mon.frac_digits = monchars[i++];
	mon.p_cs_precedes = monchars[i++];
	if (mon.p_cs_precedes > 1 && mon.p_cs_precedes < CHAR_MAX)
		fprintf(stderr, "Bad value for p_cs_precedes\n");
	mon.p_sep_by_space = monchars[i++];
	if (mon.p_sep_by_space > 1 && mon.p_sep_by_space < CHAR_MAX)
		fprintf(stderr, "Bad value for p_sep_by_space\n");
	mon.n_cs_precedes = monchars[i++];
	if (mon.n_cs_precedes > 1 && mon.n_cs_precedes < CHAR_MAX)
		fprintf(stderr, "Bad value for n_cs_precedes\n");
	mon.n_sep_by_space = monchars[i++];
	if (mon.n_sep_by_space > 1 && mon.n_sep_by_space < CHAR_MAX)
		fprintf(stderr, "Bad value for n_sep_by_space\n");
	mon.p_sign_posn = monchars[i++];
	if (mon.p_sign_posn > 4 && mon.p_sign_posn < CHAR_MAX)
		fprintf(stderr, "Bad value for p_sign_posn\n");
	mon.n_sign_posn = monchars[i];
	if (mon.n_sign_posn > 4 && mon.n_sign_posn < CHAR_MAX)
		fprintf(stderr, "Bad value for n_sign_posn\n");

	/* write out data file */
	if ((outfile = fopen(outname, "w")) == NULL) {
		fprintf(stderr, "Cannot open output file\n");
		exit(1);
	}
	if (fwrite(&mon, sizeof(struct lconv), 1, outfile) != 1) {
		fprintf(stderr, "Cannot write to output file, %s\n", outname);
		exit(1);
	}
	for (i=0; i < NUMSTR; i++) {
		if (fwrite(monstrs[i],strlen(monstrs[i])+1, 1, outfile) != 1) { 
			fprintf(stderr, "Cannot write to output file, %s\n", outname);
			exit(1);
		}
	}
	exit(0);
}
