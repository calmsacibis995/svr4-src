/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)dbfconv:dbfconv.c	1.9.2.1"

#include <stdio.h>
#include <sys/param.h>
#include <string.h>
#include "local.h"

extern	char	*init[];
char	*nexttok();
char	*cvt_modules();

char	*Command;
char	*Outfile;	/* save output file name for error() */
char	xflag[] = "x";
char	Scratch[BUFSIZ];
char	TTYaddr[DBFLINESZ];/* address for terminal service (svc code 1) */
FILE	*Ifp;		/* input file pointer */
FILE	*Ofp;		/* output file pointer */


main(argc, argv)
int argc;
char *argv[];
{
	int sversion;	/* starting version # */

	Command = argv[0];

	if (argc != 3) {
		sprintf(Scratch, "Usage: %s input_file output_file", Command);
		error(Scratch);
	}

	Ifp = fopen(argv[1], "r");
	if (Ifp == NULL) {
		sprintf(Scratch, "Cannot open <%s>", argv[1]);
		error(Scratch);
	}

	sversion = find_version();
	if (sversion == VERSION)
		exit(0);	/* nothing to do */
	else if (sversion > VERSION)
		error("Can not convert to previous version");

	rewind(Ifp);
	Ofp = fopen(argv[2], "w");
	if (Ofp == NULL) {
		sprintf(Scratch, "Cannot open <%s>", argv[2]);
		error(Scratch);
	}
	Outfile = argv[2];

	initialize(argv[1], sversion);
	convert(sversion);
	fclose(Ifp);
	fclose(Ofp);
	exit(0);
}


/*
 * find_version:	find the version number of the current database file
 */

find_version()
{
	char *line, *p;
	int version;

	if ((line = (char *) malloc(DBFLINESZ)) == NULL)
		error("malloc failed");
	p = line;
	while (fgets(p, DBFLINESZ, Ifp)) {
		if (!strncmp(p, VERSIONSTR, strlen(VERSIONSTR))) {
			p += strlen(VERSIONSTR);
			if (*p)
				version = atoi(p);
			else
				error("database file is corrupt");
			free(line);
			return(version);
		}
		p = line;
	}
	error("database file is corrupt");
	return(-1);
}


/*
 * convert:	convert a database file to the current version number
 *		Note: as the versions change, it is necessary to update
 *		      this routine to reflect the new format
 */

convert(start)
int start;
{
	char buf[DBFLINESZ];
	register char *p = buf;
	int lineno = 0;
	struct dbf_v1 v1;
	register struct dbf_v1 *v1p = &v1;
	struct dbf_v2 v2;
	register struct dbf_v2 *v2p = &v2;
	struct dbf_v3 v3;
	register struct dbf_v3 *v3p = &v3;
	struct dbf_v4 v4;
	register struct dbf_v4 *v4p = &v4;

	while (fgets(p, DBFLINESZ, Ifp)) {
		lineno++;
		/* check for comment only line or the unfortunate blank line
		   that is in the database */
		if ((*p == DBFCOMMENT) || !strcmp(p, " \n"))
			continue;
		switch (start) {
		/* Note: version 0 and version 1 are identical */
		case	0:
		case	1:
			scanner(start, lineno, p, v1p);
			v4p->dbf_svccode = v1p->dbf_svccode;
			v4p->dbf_flags = v1p->dbf_flags;
			v4p->dbf_modules = cvt_modules(v1p->dbf_modules);
			v4p->dbf_command = v1p->dbf_command;
			v4p->dbf_comment = v1p->dbf_comment;
			v4p->dbf_id = DEFAULTID;
			v4p->dbf_res1 = "reserved";
			v4p->dbf_res2 = "reserved";
			v4p->dbf_res3 = "reserved";
			v4p->dbf_addr = "";
			v4p->dbf_rpcinfo = "";
			v4p->dbf_lflags = DEFAULTTYPE;
			break;
		case	2:
			scanner(start, lineno, p, v2p);
			v4p->dbf_svccode = v2p->dbf_svccode;
			v4p->dbf_flags = v2p->dbf_flags;
			v4p->dbf_modules = cvt_modules(v2p->dbf_modules);
			v4p->dbf_command = v2p->dbf_command;
			v4p->dbf_comment = v2p->dbf_comment;
			v4p->dbf_id = v2p->dbf_id;
			v4p->dbf_res1 = "reserved";
			v4p->dbf_res2 = "reserved";
			v4p->dbf_res3 = "reserved";
			if (strcmp(v4p->dbf_svccode, TTY_SVCCODE) == 0)
				v4p->dbf_addr = TTYaddr;
			else
				v4p->dbf_addr = "";
			v4p->dbf_rpcinfo = "";
			v4p->dbf_lflags = DEFAULTTYPE;
			break;
		case	3:
			scanner(start, lineno, p, v3p);
			v4p->dbf_svccode = v3p->dbf_svccode;
			v4p->dbf_flags = v3p->dbf_flags;
			v4p->dbf_modules = v3p->dbf_modules;
			v4p->dbf_command = v3p->dbf_command;
			v4p->dbf_comment = v3p->dbf_comment;
			v4p->dbf_id = v3p->dbf_id;
			v4p->dbf_res1 = "reserved";
			v4p->dbf_res2 = "reserved";
			v4p->dbf_res3 = "reserved";
			v4p->dbf_addr = v3p->dbf_addr;
			v4p->dbf_rpcinfo = "";
			v4p->dbf_lflags = DEFAULTTYPE;
			break;
		default:
			sprintf(Scratch, "unknown version number <%d>", start);
			error(Scratch);
		}

		/* 
		 * print out data in CURRENT version format.  
		 */

		fprintf(Ofp, "%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s#%s\n",
			v4p->dbf_svccode, v4p->dbf_flags, v4p->dbf_id,
			v4p->dbf_res1, v4p->dbf_res2, v4p->dbf_res3,
			v4p->dbf_addr, v4p->dbf_rpcinfo, v4p->dbf_lflags,
			v4p->dbf_modules, v4p->dbf_command, v4p->dbf_comment);
	}
	if (feof(Ifp))
		return;
	if (ferror(Ifp))
		error("error reading input file");
}


/*
 * scanner:	break up an input line into the appropriate data structure
 *		Note: as the versions change, it is nessary to update this
 *		      routine to handle the new inputs
 */

scanner(version, lineno, inp, p)
int version;
int lineno;
char *inp;
char *p;
{
	register struct dbf_v1 *v1;
	register struct dbf_v2 *v2;
	register struct dbf_v3 *v3;
	char *tp;

	switch (version) {
	case	0:
	case	1:
		v1 = (struct dbf_v1 *) p;
		if (*(inp + strlen(inp) - 1) != '\n') {
			sprintf(Scratch, "missing newline on line %d", lineno);
			error(Scratch);
		}

		if ((tp = nexttok(inp, DBFTOKENS)) == NULL) {
			sprintf(Scratch, "illegal entry on line %d", lineno);
			error(Scratch);
		}
		v1->dbf_svccode = tp;

		if ((tp = nexttok(NULL, DBFTOKENS)) == NULL) {
			sprintf(Scratch, "illegal entry on line %d", lineno);
			error(Scratch);
		}
		/* get rid of flags that are unused in version 3 	*/
		/* 'x' is the only flag we need to transfer		*/
		if (strchr(tp, 'x') != NULL) 
			v1->dbf_flags = xflag;
		else
			v1->dbf_flags = NULL;

		if ((tp = nexttok(NULL, DBFTOKENS)) == NULL) {
			sprintf(Scratch, "illegal entry on line %d", lineno);
			error(Scratch);
		}
		v1->dbf_modules = tp;

		if ((tp = nexttok(NULL, DBFTOKENS)) == NULL) {
			sprintf(Scratch, "illegal entry on line %d", lineno);
			error(Scratch);
		}
		v1->dbf_command = tp;

		if ((tp = nexttok(NULL, DBFTOKENS)) == NULL) {
			sprintf(Scratch, "illegal entry on line %d", lineno);
			error(Scratch);
		}
		v1->dbf_comment = tp;

		return;
	case	2:
		v2 = (struct dbf_v2 *) p;
		if (*(inp + strlen(inp) - 1) != '\n') {
			sprintf(Scratch, "missing newline on line %d", lineno);
			error(Scratch);
		}

		if ((tp = nexttok(inp, DBFTOKENS)) == NULL) {
			sprintf(Scratch, "illegal entry on line %d", lineno);
			error(Scratch);
		}
		v2->dbf_svccode = tp;

		if ((tp = nexttok(NULL, DBFTOKENS)) == NULL) {
			sprintf(Scratch, "illegal entry on line %d", lineno);
			error(Scratch);
		}
		/* get rid of flags that are unused in version 3 	*/
		/* 'x' is the only flag we need to transfer		*/
		if (strchr(tp, 'x') != NULL) 
			v2->dbf_flags = xflag;
		else
			v2->dbf_flags = NULL;

		if ((tp = nexttok(NULL, DBFTOKENS)) == NULL) {
			sprintf(Scratch, "illegal entry on line %d", lineno);
			error(Scratch);
		}
		v2->dbf_id = tp;

		if ((tp = nexttok(NULL, DBFTOKENS)) == NULL) {
			sprintf(Scratch, "illegal entry on line %d", lineno);
			error(Scratch);
		}
		v2->dbf_reserved = tp;

		if ((tp = nexttok(NULL, DBFTOKENS)) == NULL) {
			sprintf(Scratch, "illegal entry on line %d", lineno);
			error(Scratch);
		}
		v2->dbf_modules = tp;

		if ((tp = nexttok(NULL, DBFTOKENS)) == NULL) {
			sprintf(Scratch, "illegal entry on line %d", lineno);
			error(Scratch);
		}
		v2->dbf_command = tp;

		if ((tp = nexttok(NULL, DBFTOKENS)) == NULL) {
			sprintf(Scratch, "illegal entry on line %d", lineno);
			error(Scratch);
		}
		v2->dbf_comment = tp;

		return;
	case	3:
		v3 = (struct dbf_v3 *) p;
		if (*(inp + strlen(inp) - 1) != '\n') {
			sprintf(Scratch, "missing newline on line %d", lineno);
			error(Scratch);
		}

		if ((tp = nexttok(inp, DBFTOKENS)) == NULL) {
			sprintf(Scratch, "illegal entry on line %d", lineno);
			error(Scratch);
		}
		v3->dbf_svccode = tp;

		if ((tp = nexttok(NULL, DBFTOKENS)) == NULL) {
			sprintf(Scratch, "illegal entry on line %d", lineno);
			error(Scratch);
		}
		v3->dbf_flags = tp;

		if ((tp = nexttok(NULL, DBFTOKENS)) == NULL) {
			sprintf(Scratch, "illegal entry on line %d", lineno);
			error(Scratch);
		}
		v3->dbf_id = tp;

		if ((tp = nexttok(NULL, DBFTOKENS)) == NULL) {
			sprintf(Scratch, "illegal entry on line %d", lineno);
			error(Scratch);
		}
		v3->dbf_addr = tp;

		if ((tp = nexttok(NULL, DBFTOKENS)) == NULL) {
			sprintf(Scratch, "illegal entry on line %d", lineno);
			error(Scratch);
		}
		v3->dbf_type = tp;

		if ((tp = nexttok(NULL, DBFTOKENS)) == NULL) {
			sprintf(Scratch, "illegal entry on line %d", lineno);
			error(Scratch);
		}
		v3->dbf_modules = tp;

		if ((tp = nexttok(NULL, DBFTOKENS)) == NULL) {
			sprintf(Scratch, "illegal entry on line %d", lineno);
			error(Scratch);
		}
		v3->dbf_command = tp;

		if ((tp = nexttok(NULL, DBFTOKENS)) == NULL) {
			sprintf(Scratch, "illegal entry on line %d", lineno);
			error(Scratch);
		}
		v3->dbf_comment = tp;

		return;
	default:
		error("Internal error in scanner");
	}
}


error(s)
char *s;
{
	fprintf(stderr, "%s: %s\n", Command, s);
	fclose(Ifp);
	unlink(Outfile);	/* don't leave droppings around */
	fclose(Ofp);
	exit(2);
}

/*
 * cvt_modules:	Convert the modules fields from the version 0, 1, and 2
 * 		format to the version 3 format.  Version 3 no longer 
 *		requires the NULL, or the trailing , in this field.
 */

char	modbuf[DBFLINESZ];

char	*
cvt_modules(buf)
char	*buf;
{
	char	*mod_p;
	char	*tokstr_p;
	char	*modbuf_p = modbuf;

	modbuf[0] = NULL;
	tokstr_p = buf;
	while ((mod_p = nexttok(tokstr_p, ",")) != NULL) {
		tokstr_p = NULL;
		if (*mod_p == NULL) {
			/* 
			 * hit end case -- old format required trailing
			 * comma after module list, new format doesn't.
			 */
			return(modbuf_p);
		}
		if (strcmp(mod_p, "NULL") == 0) 
			continue;
		strcat(modbuf_p, mod_p);
		strcat(modbuf_p, ",");
	}

	/* eliminate trailing comma if necessary */
	if (modbuf[strlen(modbuf_p)-1] == ',')
		modbuf[strlen(modbuf_p)-1] = NULL;

	return(modbuf_p);
}

/*
 * initialize:	Add initial entries to new file.
 */

initialize(filename, sversion)
char	*filename;
int sversion;
{	
	char	addrfile[DBFLINESZ];
	char	addrbuf[DBFLINESZ];
	char	*ptr;
	FILE	*addrfp;
	int	i;

	/* add version string */
	fprintf(Ofp, "%s%d\n", VERSIONSTR, VERSION);

	/* print initial blurb */
	for (i = 0; init[i]; i++)
		fprintf(Ofp, "%s\n", init[i]);

	/* 
	 * read addr file and add entry for nlps_server if converting
	 * to version 3+ from a version 2 or earlier.  Addr file is assumed
	 * to be in the same directory as the input dbf file
	 */
	
	if (sversion > 2)
		return;
	addrbuf[0] = '\0'; /* make sure 1st byte is initialized for later */
	strcpy(addrfile, filename);
	if ((ptr = strrchr(addrfile, '/')) != NULL) {
		*++ptr = NULL;
		strcat(addrfile, ADDRFILE);
	}
	else
		strcpy(addrfile, ADDRFILE);

	if ((addrfp = fopen(addrfile, "r")) == NULL) {
		fprintf(stderr, "%s: warning: could not open %s\n", 
			Command, addrfile);
	}

	/* first entry in addr file is general listen address */
	/* read it and add line for nlps_server in database   */
	if (addrfp) {
		if (fgets(addrbuf, DBFLINESZ, addrfp) == NULL) {
			fprintf(stderr, "%s: warning: error in reading %s\n",
				Command, addrfile);
		}
	}
	if (*addrbuf != NULL) 
		/* throw away carriage return */
		addrbuf[strlen(addrbuf)-1] = NULL;

	fprintf(Ofp, "%s:%s:%s:reserved:reserved:reserved:%s::%s:%s:%s\t#%s\n",
		NLPS_SVCCODE, NLPS_FLAGS, NLPS_ID, (*addrbuf ? addrbuf : ""),
		NLPS_TYPE, NLPS_MODULES, NLPS_COMMAND, NLPS_COMMENT);

	/* now read TTYaddr.  if it doesn't exist, don't worry.		*/
	/* we can't add the entry here, because we have to preserve any	*/
	/* old info for the tty server.  we will look for service code	*/
	/* 1 when doing the database conversion, and add this address	*/
	/* there.							*/

	if (addrfp) {
		fgets(TTYaddr, DBFLINESZ, addrfp);
		if (*TTYaddr != NULL) 
			/* throw away carriage return */
			TTYaddr[strlen(TTYaddr)-1] = NULL;
	}
	return;
}


/*
 * nexttok - return next token, essentially a strtok, but it can
 *	deal with null fields and strtok can not
 *
 *	args:	str - the string to be examined, NULL if we should
 *		      examine the remembered string
 *		delim - the list of valid delimiters
 */


char *
nexttok(str, delim)
char *str;
register char *delim;
{
	static char *savep;	/* the remembered string */
	register char *p;	/* pointer to start of token */
	register char *ep;	/* pointer to end of token */

	p = (str == NULL) ? savep : str ;
	if ((p == NULL) || (*p == '\0'))
		return(NULL);
	ep = strpbrk(p, delim);
	if (ep == NULL) {
		savep = NULL;
		return(p);
	}
	savep = ep + 1;
	*ep = '\0';
	return(p);
}
