/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:cmd/lpdata/lpdata.c	1.6.3.1"
# include	<stdio.h>
# include	<ctype.h>
# include	<string.h>
# include	<stdarg.h>
# include	<sys/types.h>
# include	<sys/dir.h>
# include	<lp.h>
# include	<msgs.h>
# include	<class.h>
# include	<form.h>
# include	<filters.h>
# include	<printers.h>

# define	DOWN	0
# define	UP	1
# define	EQUAL(s1, s2)	(strcmp(s1, s2) == 0)
# define	testswitch(s)	(Switchtable[(int)s - (int)'a'] != NULL)
# define	switchval(s)	(Switchtable[(int)s - (int)'a'])

char	Message[MSGMAX];
char	*Switchtable[26];

int	Spooler = DOWN;

FILE	*File;

int	lpsend(int, ...);
void	newfile();
void	getswitches(char *);
char	*Datafile = 0;
int	lprecv(int, ...);

char	*Commands[] =
{
    "list_classes",
    "get_class",

    "list_filters",
    "get_filter",

    "list_forms",
    "get_form",

    "list_printers",
    "get_printer",

    "get_default",
    "check",
    "fix_list",
    "lpsched",
    0
};

void	    list_class(void), get_class(void), list_filter(void), get_filter(void),
	    list_form(void), get_form(void), list_printer(void), get_printer(void),
	    list_default(void), check(void), fix_list(void), chk_sched( void );

void	(*Cmdtab[])(void) =
{
    list_class,
    get_class,
    list_filter,
    get_filter,
    list_form,
    get_form,
    list_printer,
    get_printer,
    list_default,
    check,
    fix_list,
    chk_sched,
    0
};
# define	MAXCMD	12

main(int argc, char * argv[])
{
    char	buff[80];
    char	*cp;
    int		index;
    int		c;
    extern char	*optarg;
    
    while ((c = getopt(argc, argv, "f:")) != EOF)
	switch(c)
	{
	    case 'f':
	    Datafile=optarg;
	    break;

	    default:
	    fprintf(stderr, "Unrecognized option: %c\n", c);
	}

    while (fgets(buff, BUFSIZ, stdin))
    {
	fputs(buff, stderr);
	fflush(stderr);
	
	getswitches(buff);

	for (index = 0; index < MAXCMD; index++)
	    if (strcmp(Commands[index], buff) == 0)
		break;

	if (index >= MAXCMD)
	{
	    printf("Unknown command: %s\n-EOT-\n", buff);
	    fflush(stdout);
	    continue;
	}
	
	(*Cmdtab[index])();
	printf("-EOT-\n");
	fflush(stdout);
    }
}


/*
**	Return a list of classes.
**	/a will show only accepting classes.
**	/r will show only rejecting classes.
**	/sc will separate classes with string <c>.
**	/a and /r are mutually exclusive and the last one found
**	takes precedence.
*/
void
list_class(void)
{
    int		check = 0;
    short	status;
    char	*class;
    short	reject;
    char	*why;
    char	*when;
    char	*sepchar = "\n";
    int		more = 0;
    
    if (testswitch('a')) check |= 1;
    if (testswitch('r')) check |= 2;
    if (testswitch('s')) sepchar = switchval('s');

    if (lpsend(S_INQUIRE_CLASS, "all"))
	return;

    do
    {
	lprecv(R_INQUIRE_CLASS, &status, &class, &reject, &why, &when);
	if (reject && (check & 1))
	    continue;
	if (!reject && (check & 2))
	    continue;

	printf("%s%s", more++ ? sepchar : "", class);
    }
    while (status == MOKMORE);

    if (more)
	printf("\n");
}

/*
**
*/

void
list_printer(void)
{
    int		check = 0;
    short	status;
    char	*printer;
    char	*form;
    char	*cs;
    char	*whydis;
    char	*whyrej;
    short	pstat;
    char	*id;
    char	*edate;
    char	*adate;
    char	*sepchar = "\n";
    int		more = 0;
    
    if (testswitch('a')) check |=  1;
    if (testswitch('d')) check |=  2;
    if (testswitch('e')) check |=  4;
    if (testswitch('r')) check |=  8;
    if (testswitch('m')) check |= 16;
    if (testswitch('s')) sepchar = switchval('s');

    if (lpsend(S_INQUIRE_PRINTER_STATUS, "all"))
	return;

    do
    {
	lprecv(R_INQUIRE_PRINTER_STATUS, &status, &printer,
	       &form, &cs, &whydis, &whyrej,
	       &pstat, &id, &edate, &adate);
	
	        if ((pstat & PS_REJECTED) && (check & 1))
		    continue;
		if (!(pstat & PS_DISABLED) && (check & 2))
		    continue;
	        if ((pstat & PS_DISABLED) && (check & 4))
		    continue;
		if (!(pstat & PS_REJECTED) && (check & 8))
		    continue;
		if (!(*form || *cs) && (check & 16))
		    continue;

	printf("%s%s", more++ ? sepchar : "", printer);
    }
    while (status == MOKMORE);
    if (more)
	printf("\n");
}

void
list_form(void)
{
    DIR			*dp;
    long		addr = -1;
    struct dirent	*dirp;
    char		*p;
    struct stat		stbuf;
    char		*sepchar = "\n";
    int			more = 0;
    
    if ((dp = opendir(Lp_A_Forms)) == 0)
	return;

    while(dirp = Readdir(dp))
    {
	if (EQUAL(dirp->d_name, ".")  || 
	    EQUAL(dirp->d_name, ".."))
	    continue;

	if ((p = makepath(Lp_A_Forms, dirp->d_name, (char *)0)) == NULL)
	    return;

	if (stat(p, &stbuf) == -1)
	{
	    free(p);
	    return;
	}

	free(p);
	
	if ((stbuf.st_mode & S_IFMT) != S_IFDIR)
	    continue;

	printf("%s%s", more++ ? sepchar : "", dirp->d_name);
    }
    if (more)
	printf("\n");
}
void
list_filter(void)
{
    FILTER	*filt;
    int		more = 0;
    char	*sepchar = "\n";
    
    if (testswitch('s')) sepchar = switchval('s');
    
    while (filt = getfilter("all"))
    {
	printf("%s%s", more++ ? sepchar : "", filt->name);
	freefilter(filt);
    }
    if (more)
	printf("\n");
}
void
get_class(void)
{
    CLASS	*cl;
    char	**p;
    
    newfile();

    if (cl = getclass(switchval('n')))
    {
	for (p = cl->members; p[0]; p++)
	    fprintf(File, "%s\n", *p);
	freeclass(cl);
	fclose(File);
    }
}

void
get_printer(void)
{
    PRINTER	*pr;
    void	dostty(FILE *, PRINTER *);
    newfile();
    
    if (pr = getprinter(switchval('n')))
    {
	printlist_setup(0, 0, ",", 0);
	fprintf (File, "banner=%s\n", pr->banner & BAN_OFF ? "off" : "on");
	if (pr->banner & BAN_ALWAYS)
	    fprintf (File, "Always=Yes\n");
	else
	    fprintf (File, "Always=No\n");

	if (pr->cpi.val)
	{
	    fprintf(File, "cpi=");
	    printsdn(File, pr->cpi);
	}
	if (pr->char_sets && *pr->char_sets)
	{
	    fprintf(File, "charsets=");
	    printlist(File, pr->char_sets);
	}
	if (pr->input_types && *pr->input_types)
	{
	    fprintf(File, "input_types=");
	    printlist(File, pr->input_types);
	}
	if (pr->device)
	    fprintf(File, "device=%s\n", pr->device);
	if (pr->dial_info)
	    fprintf(File, "dial_token=%s\n", pr->dial_info);
	if (pr->fault_rec)
	    fprintf(File, "onfault=%s\n", pr->fault_rec);
	if (pr->interface)
	    fprintf(File, "interface=%s\n", pr->interface);
	if (pr->lpi.val)
	{
	    fprintf(File, "lpi=");
	    printsdn(File, pr->lpi);
	}
	if (pr->plen.val)
	{
	    fprintf(File, "length=");
	    printsdn(File, pr->plen);
	}
	if (pr->login & LOG_IN)
	    fprintf (File, "login=yes\n");
	else
	    fprintf (File, "login=no\n");
	if (pr->printer_types && *pr->printer_types)
	{
	    fprintf(File, "prtype=");
	    printlist(File, pr->printer_types);
	}
	if (pr->remote)
	    fprintf(File, "remote=%s\n", pr->remote);
	if (pr->speed)
	    fprintf(File, "baud=%s\n", pr->speed);
	if (pr->pwid.val)
	{
	    fprintf(File, "width=");
	    printsdn(File, pr->pwid);
	}
	dostty(File, pr);
	printlist_unsetup();
	
	freeprinter(pr);
	fclose(File);
    }
}

void dostty(FILE * File, PRINTER * prbuf)
{
    char	**pp;
    int		i;
    int		ppend;
    char	buf[10];
    char	miscbuf[1024] = "";
    
    if ((pp = getlist(prbuf->stty, LP_WS, " 	")) == NULL)
	return;
/*
**
**	Stty options list:
**
**		-<optname>		-echoe
**		<optname>		isig
**		<optname> <arg>		intr ^c
**		<optname><arg>		cr1
**
*/


    for (i = 0; pp[i]; i++)
    {
	if (*pp[i] == '-')
	{
	    fprintf(File, "%s=no\n", pp[i] + 1);

	    /* Check for even or no parity*/
            if ((strcmp(pp[i] + 1, "parodd")) == 0 )
    	    {
   		if (*pp[i + 1] == 'p')
			fprintf(File, "parity=even\n");
		else
			fprintf(File, "parity=none\n");
		
	    }
	    /* Check for stopbits */
            if ((strcmp(pp[i] + 1, "cstopb")) == 0 )
			fprintf(File, "stopbits=1\n");
		
	    continue;
	}
	else
	    fprintf(File, "%s=yes\n", pp[i] );

	/* Check for odd parity*/
   	if (*pp[i] == 'p' && *pp[i + 1] == 'p')
    	{
		fprintf(File, "parity=odd\n");
	}
	/* Check for stopbits */
        if ((strcmp(pp[i], "cstopb")) == 0 )
	    fprintf(File, "stopbits=2\n");

	if (strchr("0123456789E", *pp[i]))
	{
	    fprintf(File, "baud=%s\n", pp[i]);
	    continue;
	}
	ppend = strlen(pp[i]) - 1;
	
	if (strchr("0123456789", pp[i][ppend]))
	{
	    strcpy(buf, pp[i]);
	    buf[ppend] = '\0';
            if ((strcmp(pp[i], "nl1")) == 0 )
	    	fprintf(File, "nldelay=yes\n");
            else if ((strcmp(pp[i], "lp0")) == 0 )
	    	fprintf(File, "nldelay=no\n");
            else if ((strcmp(pp[i], "bs1")) == 0 )
	    	fprintf(File, "bsdelay=yes\n");
            else if ((strcmp(pp[i], "bs0")) == 0 )
	    	fprintf(File, "bsdelay=no\n");
            else if ((strcmp(pp[i], "ff1")) == 0 )
	    	fprintf(File, "ffdelay=yes\n");
            else if ((strcmp(pp[i], "ff0")) == 0 )
	    	fprintf(File, "ffdelay=no\n");
            else if ((strcmp(pp[i], "vt1")) == 0 )
	    	fprintf(File, "vtdelay=yes\n");
            else if ((strcmp(pp[i], "vt0")) == 0 )
	    	fprintf(File, "vtdelay=no\n");
            else if ((strcmp(pp[i], "tab3")) == 0 )
	    	fprintf(File, "tab=expand\n");
	    else 
		fprintf(File, "%s=%s\n", buf, pp[i] + ppend);
	    continue;
	}
	strcat(miscbuf, pp[i]);
	strcat(miscbuf, " ");
    }
    if (miscbuf[0])
	fprintf(File, "misc=%s\n", miscbuf);

    freelist(pp);
}

void
get_filter(void)
{
    FILTER	*fl;
    
    newfile();
    
    if (fl = getfilter(switchval('n')))
    {
	if (fl->name)
	    fprintf(File, "filter=%s\n", fl->name);
	if (fl->command)
	    fprintf(File, "command=%s\n", fl->command);
	if (fl->type == fl_slow)
	    fprintf(File, "type=slow\n");
	else
	    fprintf(File, "type=fast\n");
	printlist_setup(0, 0, ",", 0);
	if (fl->printer_types)
	{
	    fprintf(File, "printer_types=");
	    printlist(File, fl->printer_types);
	}
	if (fl->printers)
	{
	    fprintf(File, "printers=");
	    printlist(File, fl->printers);
	}
	if (fl->input_types)
	{
	    fprintf(File, "input_types=");
	    printlist(File, fl->input_types);
	}
	if (fl->output_types)
	{
	    fprintf(File, "output_types=");
	    printlist(File, fl->output_types);
	}
	if (fl->templates)
	{
	    fprintf(File, "templates=");
	    printlist(File, fl->templates);
	}
        printlist_unsetup();
	freefilter(fl);
    }
    fclose(File);
}
void
get_form(void)
{
    FORM	fr;
    
    newfile();
    
    if (getform(switchval('n'), &fr, NULL, NULL) == 0)
    {
	fprintf(File, "Under development\n");
	if (fr.plen.val)
	{
	    fprintf(File, "plen=");
	    printsdn(File, fr.plen);
	}
	if (fr.pwid.val)
	{
	    fprintf(File, "pwid=");
	    printsdn(File, fr.pwid);
	}
	if (fr.lpi.val)
	{
	    fprintf(File, "lpi=");
	    printsdn(File, fr.lpi);
	}
	if (fr.cpi.val)
	{
	    fprintf(File, "cpi=");
	    printsdn(File, fr.cpi);
	}
	if (fr.chset)
	{
	    fprintf(File, "chset=%s\n", fr.chset);
		free(fr.chset);
	}
	if (fr.mandatory && fr.chset)
	    fprintf(File, "mandatory=mandatory\n");
	else
	    fprintf(File, "mandatory=optional\n");
	if (fr.rcolor)
	{
	    fprintf(File, "rcolor=%s\n", fr.rcolor);
		free(fr.rcolor);
	}
	if (fr.comment)
	{
	    fprintf(File, "comment=%s\n", fr.comment);
		free(fr.comment);
	}
	if (fr.conttype)
	{
	    fprintf(File, "conttype=%s\n", fr.conttype);
		free(fr.conttype);
	}
	if (fr.name)
	{
	    fprintf(File, "name=%s\n", fr.name);
	    free(fr.name);
	}
	
    }
	fclose(File);
}

void
list_default(void)
{
    char	cmd[256];

    sprintf(cmd, "/bin/cat %s/default 2> /dev/null", getspooldir());
    system(cmd);
}

int
lpsend(int mtype, ...)
{
    va_list	arg;

    va_start(arg, mtype);

    if (mopen() && errno != EEXIST)
    {
	Spooler = 0;
	printf("No lpsched\n");
    }
    else
	Spooler = 1;

    if (!Spooler)
	return(-1);
    
    (void) _putmessage(Message, mtype, arg);
    if (msend(Message) == -1 && errno == EIDRM)
    {
	Spooler = 0;
	printf("No lpsched\n");
	(void) mclose();
	return(-1);
    }

    va_end(arg);
    return(0);
}

int
lprecv(int mtype, ...)
{
    va_list	arg;
    
    va_start(arg, mtype);

    if (!Spooler)
	return(-1);

    if (mrecv(Message, MSGMAX) == -1)
    {
	if (errno == EIDRM)
	{
	    Spooler = 0;
	    printf("No\n");
	    (void) mclose();
	}
	return(-1);
    }

    (void) _getmessage(Message, mtype, arg);
        
    va_end(arg);
    return(0);
}

void newfile(void)
{
    unlink(Datafile);
    if ((File = fopen(Datafile, "w")) == NULL)
    {
	fprintf(stderr, "Could not open %s, errno == %d\n", Datafile, errno);
	File = fopen("/dev/null", "w");
    }
}

void getswitches(char * buff)
{
    char	*cp;
    int		index;
    
    memset(Switchtable, 0, sizeof(Switchtable));
    
    if (cp = strchr(buff, '\n'))
	*cp = '\0';
    
    cp = buff;
    while(*cp)
    {
	*cp = tolower(*cp);
	cp++;
    }
    
    cp = buff;

    while ((cp = strchr(cp, '/')) != NULL)
    {
	*cp++ = '\0';
	index = (int) *cp++ - (int)'a';
	Switchtable[index] = cp;
    }
}

char	*Chktab[] =
{
    "class",
    "printer",
    "dest",
    0
};

void check(void)
{
    char	*target;
    char	**cp;
    void	check_stuff(int);
    
    if (!testswitch('t'))
    {
	printf("Missing check subcommand\n");
	return;
    }
    
    target = switchval('t');

    for (cp = Chktab; *cp; cp++)
	if (EQUAL(*cp, target))
	    break;
    if (*cp)
	check_stuff(cp - Chktab + 1);
    else
	printf("Unrecognized check subcommand: %s\n", target);
}

void check_stuff(int type)
{
    int		check = 0;
    int		more = 0;
    short	status;
    char	*class;
    short	reject;
    char	*why;
    char	*when;
    char	*p;
    char	*printer;
    char	*form;
    char	*cs;
    char	*whydis;
    char	*whyrej;
    char	*id;
    char	*edate;
    char	*adate;
    short	pstat;
    int		okclass;
    int		okprinter;
    int		limit = 0;
    int		count = 0;
    int		okunknown = 0;
    
    if (testswitch('a')) check |= 1;
    if (testswitch('d')) check |= 2;
    if (testswitch('e')) check |= 4;
    if (testswitch('r')) check |= 8;
    if (testswitch('m')) check |= 16;
    if (testswitch('l')) limit = atoi(switchval('l'));
    if (testswitch('u')) okunknown = 1;
    

    if (testswitch('n') == 0)
    {
	printf("Missing item list\n");
	return;
    }
    
    for (p = strtok(switchval('n'), ","); p; p = strtok(NULL, ","))
    {
	count++;
	
	okprinter = okclass = 0;

	if (CS_STREQU(p, NAME_ALL) || CS_STREQU(p, NAME_NONE) || CS_STREQU(p, NAME_ANY))
	{
	    if (type != 3)
		printf("%s\"%s\" is a reserved word", more++ ? ", " : "", p);
	    continue;
	}

	if (syn_name(p))
	{
	    printf("%snames are: 1-14 letters, digits, and underscores", more++ ? ", " : "");
	    continue;
	}
	if (lpsend(S_INQUIRE_CLASS, p))
	    return;
	lprecv(R_INQUIRE_CLASS, &status, &class, &reject, &why, &when);
	if (type & 01)
	{
	    if (status != MOK)
		goto printers;

	    okclass = 1;

	    if (reject && (check & 1))
	    {
		printf("%s%s is rejecting", more++ ? ", " : "", p);
		continue;
	    }
	    
	    if (!reject && (check & 2))
	    {
		printf("%s%s is accepting", more++ ? ", " : "", p);
		continue;
	    }
	}
	else
	    if (status == MOK)
	    {
		okclass++;
		printf("%s%s is a class", more++ ? ", " : "", p);
	    }

printers:
	if (lpsend(S_INQUIRE_PRINTER_STATUS, p))
	    return;
	lprecv(R_INQUIRE_PRINTER_STATUS, &status, &printer,
	       &form, &cs, &whydis, &whyrej,
	       &pstat, &id, &edate, &adate);
	if (type & 02)
	{
	    if (status != MOK)
		goto errors;

	    okprinter = 1;
	    
	    if ((pstat & PS_REJECTED) && (check & 1))
	    {
		printf("%s%s is rejecting", more++ ? ", " : "", p);
		continue;
	    }
	    if (!(pstat & PS_DISABLED) && (check & 2))
	    {
		printf("%s%s is enabled", more++ ? ", " : "", p);
		continue;
	    }
	    if ((pstat & PS_DISABLED) && (check & 4))
	    {
		printf("%s%s is disabled", more++ ? ", " : "", p);
		continue;
	    }
	    if (!(pstat & PS_REJECTED) && (check & 8))
	    {
		printf("%s%s is accepting", more++ ? ", " : "", p);
		continue;
	    }
	    if (!(*form || *cs) && (check & 16))
	    {
		printf("%s%s has nothing mounted", more++ ? ", " : "", p);
		continue;
	    }
	}
	else
	    if (status == MOK)
	    {
		okprinter++;
		printf("%s%s is a printer", more++ ? ", " : "", p);
	    }

errors:
	if (!okclass && !okprinter && !okunknown)
	    printf("%s%s is unknown", more++ ? ", " : "", p);
    }

    if (limit && count > limit)
	printf("%stoo many choices", more++ ? ", " : "");
    if (more)
	printf("\n");
    else
	printf("ok\n");
}

void fix_list(void)
{
    char	*cp;
    char	*dst = " ";
    char	*src = ",";
        
    if (testswitch('l') == 0)
	return;

    if (testswitch('s')) src = switchval('s');
    if (testswitch('r')) dst = switchval('r');
    
    for (cp = switchval('l'); *cp; cp++)
    {
	if (*cp == '\\')
	{
	    cp++;
	    continue;
	}
	if (*cp == *src)
	    *cp = *dst;
    }
    printf("%s\n", switchval('l'));
}

void chk_sched(void)
{
    short	s1,
		s2;
    char	*c1,
		*c2,
		*c3;

    if (mopen() && errno != EEXIST)
    {
	printf("No lpsched\n");
	Spooler = 0;
	(void)mclose();
    }
    else
    {
	if (lpsend(S_INQUIRE_CLASS, "none"))
	{
	    Spooler = 0;
	    (void)mclose();
	}
	else
	{
	    if (lprecv(R_INQUIRE_CLASS, &s1, &c1, &s2, &c2, &c3))
	    {
		Spooler = 0;
		(void)mclose();
	    }
	    else
	    {
		Spooler = 1;
		printf("Yes lpsched\n");
	    }
	}
    }
}
