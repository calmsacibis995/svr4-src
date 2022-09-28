/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)localedef:chrtbl/chrtbl.c	1.1.5.1"
/*
 * 	chrtbl - generate character class definition table
 */
#include <stdio.h>
#include <ctype.h>
#include <varargs.h>
#include <string.h>
#include <signal.h>

/*	Definitions	*/

#define HEX    			1
#define OCTAL  			2
#define RANGE  			1
#define UL_CONV 		2
#define SIZE			(2 * 257) + 7 + 2
#define START_CSWIDTH		(2 * 257)
#define START_NUMERIC		(2 * 257) + 7
#define	ISUPPER			1
#define ISLOWER			2
#define ISDIGIT			4 
#define ISSPACE			8	
#define ISPUNCT			16
#define ISCNTRL			32
#define ISBLANK			64
#define ISXDIGIT		128
#define UL			256
#define LC_CTYPE		257
#define CSWIDTH 		258
#define	NUMERIC			259
#define	LC_NUMERIC		260
#define DECIMAL_POINT		261
#define THOUSANDS_SEP		262

#define UNDEFINED_CSWIDTH	10

extern	char	*malloc();
extern  void	perror();
extern	void	exit();
extern	int	unlink();
extern	int	atoi();

/*   Internal functions  */

static	void	error();
static	void	init();
static	void	process();
static	void	create1();
static	void	create2();
static	void	create3();
static  void	parse();
static	int	empty_line();
static	void	check_chrclass();
static	void	clean();
static	void	comment1();
static	void	comment2();
static	void	comment3();
static 	int	cswidth();
static 	int	setnumeric();
static	void	error();
static	int	check_digit();
static	unsigned  char	*clean_line();

/*	Static variables	*/

struct  classname	{
	char	*name;
	int	num;
	char	*repres;
}  cln[]  =  {
	"isupper",		ISUPPER,		"_U",
	"islower",		ISLOWER,		"_L",
	"isdigit",		ISDIGIT,		"_N",
	"isspace",		ISSPACE,		"_S",
	"ispunct",		ISPUNCT,		"_P",
	"iscntrl",		ISCNTRL,		"_C",
	"isblank",		ISBLANK,		"_B",
	"isxdigit",		ISXDIGIT,		"_X",
	"ul",			UL,			NULL,
	"LC_CTYPE",		LC_CTYPE,		NULL,
	"cswidth",		CSWIDTH,		NULL,
	"LC_NUMERIC",		LC_NUMERIC,		NULL,
	"decimal_point",	DECIMAL_POINT,		NULL,
	"thousands_sep", 	THOUSANDS_SEP,		NULL,
	NULL,			NULL,			NULL
};

int	readstd;			/* Process the standard input */
unsigned char	linebuf[256];		/* Current line in input file */
unsigned char *p;
int	chrclass = 0;			/* set if LC_CTYPE is specified */
int	lc_numeric;			/* set if LC_NUMERIC is specified */
int	lc_ctype;
char	chrclass_name[20];		/* save current chrclass name */
int	chrclass_num;			/* save current chrclass number */
int	ul_conv = 0;			/* set when left angle bracket
					 * is encountered. 
					 * cleared when right angle bracket
					 * is encountered */
int	cont = 0;			/* set if the description continues
					 * on another line */
int	action = 0;			/*  action = RANGE when the range
					 * character '-' is ncountered.
					 *  action = UL_CONV when it creates
					 * the conversion tables.  */
int	in_range = 0;			/* the first number is found 
					 * make sure that the lower limit
					 * is set  */
int	ctype[SIZE];			/* character class and character
					 * conversion table */
int	range = 0;			/* set when the range character '-'
					 * follows the string */
int	width;				/* set when cswidth is specified */
int	numeric;			/* set when numeric is specified */
char	tablename1[24];			/* save name of the first data file */
char	tablename2[24];			/* save name of the second date file */
char	*cmdname;			/* Save command name */
char	input[256];			/* Save input file name */
char	tokens[] = ",:\0";

/* Error  messages */
/* vprintf() is used to print the error messages */

char	*msg[] = {
/*    0    */ "Usage: chrtbl [ - | file ] ",
/*    1    */ "the name of the output file for \"LC_CTYPE\" is not specified",
/*    2    */ "incorrect character class name \"%s\"",
/*    3    */ "--- %s --- left angle bracket  \"<\" is missing",
/*    4    */ "--- %s --- right angle bracket  \">\" is missing",
/*    5    */ "--- %s --- wrong input specification \"%s\"",
/*    6    */ "--- %s --- number out of range \"%s\"",
/*    7    */ "--- %s --- nonblank character after (\"\\\") on the same line",
/*    8    */ "--- %s --- wrong upper limit \"%s\"",
/*    9    */ "--- %s --- wrong character \"%c\"",
/*   10    */ "--- %s --- number expected",
/*   11    */ "--- %s --- too many range \"-\" characters",
/*   12    */ "--- %s --- wrong specification, %s",
/*   13    */ "malloc error",
/*   14	   */ "--- %s --- wrong specification",
/*   15    */ "the name of the output file for \"LC_NUMERIC\" is not specified",
/*   16    */ "numeric editing information \"numeric\" must be specified",
/*   17    */ "character classification and conversion information must be specified",
/*   18    */ "the same output file name was used in \"LC_CTYPE\" and \"LC_NUMERIC\""
};

static char cswidth_info[] = "\n\nCSWIDTH FORMAT: n1[[:s1][,n2[:s2][,n3[:s3]]]]\n\tn1 byte width (SUP1)\n\tn2 byte width (SUP2)\n\tn3 byte width (SUP3)\n\ts1 screen width (SUP1)\n\ts2 screen width (SUP2)\n\ts3 screen width (SUP3)";


main(argc, argv)
int	argc;
char	**argv;
{

	p = linebuf;
	if (cmdname = strrchr(*argv, '/'))
		++cmdname;
	else
		cmdname = *argv;
	if ( (argc != 1)  && (argc != 2) )
		error(cmdname, msg[0]);
	if ( argc == 1 || strcmp(argv[1], "-") == 0 )
		{
		readstd++;
		(void)strcpy(input, "standard input");
		}
	else
		(void)strcpy(input, argv[1]);
	if (signal(SIGINT, SIG_IGN) == SIG_DFL)
		(void)signal(SIGINT, clean);
	if (!readstd && freopen(argv[1], "r", stdin) == NULL)
		perror(argv[1]), exit(1);
	init();
	process();
	if (!lc_ctype && chrclass)
		error(input, msg[17]);
	if (lc_ctype) {
		if (!chrclass) 
			error(input, msg[1]);
		else {
			create1();
			create2();
		}
	}
	if (lc_numeric && !numeric)
		error(input, msg[16]);
	if (numeric && !lc_numeric)
		error(input, msg[15]);
	if (strcmp(tablename1, tablename2) == NULL)	
		error(input, msg[18]);
	if (lc_numeric && numeric)
		create3();
	exit(0);
}


/* Initialize the ctype array */

static void
init()
{
	register i;
	for(i=0; i<256; i++)
		(ctype + 1)[257 + i] = i;
	ctype[START_CSWIDTH] = ctype[START_CSWIDTH + 3] = ctype[START_CSWIDTH + 6] = 1;
}

/* Read line from the input file and check for correct
 * character class name */

static void
process()
{

	unsigned char	*token();
	register  struct  classname  *cnp;
	register unsigned char *c;
	for (;;) {
		if (fgets((char *)linebuf, sizeof linebuf, stdin) == NULL ) {
			if (ferror(stdin)) {
				perror("chrtbl (stdin)");
				exit(1);	
				}
				break;	
	        }
		p = linebuf;
		/*  comment line   */
		if ( *p == '#' ) 
			continue; 
		/*  empty line  */
		if ( empty_line() )  
			continue; 
		if ( ! cont ) 
			{
			c = token();
			for (cnp = cln; cnp->name != NULL; cnp++) 
				if(strcmp(cnp->name, (char *)c) == NULL) 
					break; 
			}	
		switch(cnp->num) {
		default:
		case NULL:
			error(input, msg[2], c);
		case ISUPPER:
		case ISLOWER:
		case ISDIGIT:
		case ISXDIGIT:
		case ISSPACE:
		case ISPUNCT:
		case ISCNTRL:
		case ISBLANK:
		case UL:
				lc_ctype++;
				(void)strcpy(chrclass_name, cnp->name);
				chrclass_num = cnp->num;
				parse(cnp->num);
				break;
		case LC_CTYPE:
				chrclass++;
				if ( (c = token()) == NULL )
					error(input, msg[1]);
				(void)strcpy(tablename1, "\0");
				(void)strcpy(tablename1, (char *)c);
				if (freopen("ctype.c", "w", stdout) == NULL)
					perror("ctype.c"), exit(1);
				break;
		case LC_NUMERIC:
				lc_numeric++;
				if ( (c = token()) == NULL )
					error(input, msg[15]);
				(void)strcpy(tablename2, "\0");
				(void)strcpy(tablename2, (char *)c);
				break;
		case CSWIDTH:
				width++;
				(void)strcpy(chrclass_name, cnp->name);
				if (! cswidth() )
					error(input, msg[12], chrclass_name, cswidth_info);
				break;
		case DECIMAL_POINT:
		case THOUSANDS_SEP:
				numeric++;
				(void)strcpy(chrclass_name, cnp->name);
				if (! setnumeric(cnp->num) )
					error(input, msg[14], chrclass_name);
				break;
		}
	} /* for loop */
	return;
}

static int
empty_line()
{
	register unsigned char *cp;
	cp = p;
	for (;;) {
		if ( (*cp == ' ') || (*cp == '\t') ) {
				cp++;
				continue; }
		if ( (*cp == '\n') || (*cp == '\0') )
				return(1); 
		else
				return(0);
	}
}

/* 
 * token() performs the parsing of the input line. It is sensitive
 * to space characters and returns a character pointer to the
 * function it was called from. The character string itself
 * is terminated by the NULL character.
 */ 

unsigned char *
token()
{
	register  unsigned char  *cp;
	for (;;) {
	check_chrclass(p);
	switch(*p) {
		case '\0':
		case '\n':
			in_range = 0;
			cont = 0;
			return(NULL);
		case ' ':
		case '\t':
			p++;
			continue;
		case '>':
			if (action == UL)
				error(input, msg[10], chrclass_name);
			ul_conv = 0;
			p++;
			continue;
		case '-':
			if (action == RANGE)
				error(input, msg[11], chrclass_name);
			action = RANGE;
			p++;
			continue;
		case '<':
			if (ul_conv)
				error(input, msg[4], chrclass_name);
			ul_conv++;
			p++;
			continue;
		default:
			cp = p;
			while(*p!=' ' && *p!='\t' && *p!='\n' && *p!='>' && *p!='-')  
				p++;   
			check_chrclass(p);
			if (*p == '>')
				ul_conv = 0;
			if (*p == '-')
				range++;
			*p++ = '\0';
			return(cp);

		}
	}
}


/* conv_num() is the function which converts a hexadecimal or octal
 * string to its equivalent integer represantation. Error checking
 * is performed in the following cases:
 *	1. The input is not in the form 0x<hex-number> or 0<octal-mumber>
 *	2. The input is not a hex or octal number.
 *	3. The number is out of range.
 * In all error cases a descriptive message is printed with the character
 * class and the hex or octal string.
 * The library routine sscanf() is used to perform the conversion.
 */


conv_num(s)
unsigned char	*s;
{
	unsigned char	*cp;
	int	i, j;
	int	num;
	cp = s;
	if ( *cp != '0' ) 
		error(input, msg[5], chrclass_name, s);
	if ( *++cp == 'x' )
		num = HEX;
	else
		num = OCTAL;
	switch (num) {
	case	HEX:
			cp++;
			for (j=0; cp[j] != '\0'; j++) 
				if ((cp[j] < '0' || cp[j] > '9') && (cp[j] < 'a' || cp[j] > 'f'))
					break;
				
				break;
	case   OCTAL:
			for (j=0; cp[j] != '\0'; j++)
				if (cp[j] < '0' || cp[j] > '7')
					break;
			break;
	default:
			error(input, msg[5], chrclass_name, s);
	}
	if ( num == HEX )  { 
		if (cp[j] != '\0' || sscanf((char *)s, "0x%x", &i) != 1)  
			error(input, msg[5], chrclass_name, s);
		if ( i > 0xff ) 
			error(input, msg[6], chrclass_name, s);
		else
			return(i);
	}
	if (cp[j] != '\0' || sscanf((char *)s, "0%o", &i) != 1) 
		error(input, msg[5], chrclass_name, s);
	if ( i > 0377 ) 
		error(input, msg[6], chrclass_name, s);
	else
		return(i);
/*NOTREACHED*/
}

/* parse() gets the next token and based on the character class
 * assigns a value to corresponding table entry.
 * It also handles ranges of consecutive numbers and initializes
 * the uper-to-lower and lower-to-upper conversion tables.
 */

static void
parse(type)
int type;
{
	unsigned char	*c;
	int	ch1 = 0;
	int	ch2;
	int 	lower = 0;
	int	upper;
	while ( (c = token()) != NULL) {
		if ( *c == '\\' ) {
			if ( ! empty_line()  || strlen((char *)c) != 1) 
				error(input, msg[7], chrclass_name);
			cont = 1;
			break;
			}
		switch(action) {
		case	RANGE:
			upper = conv_num(c);
			if ( (!in_range) || (in_range && range) ) 
				error(input, msg[8], chrclass_name, c);
			((ctype + 1)[upper]) |= type;
			if ( upper <= lower ) 
				error(input, msg[8], chrclass_name, c);
			while ( ++lower <= upper ) 
				((ctype + 1)[lower]) |= type;
			action = 0;
			range = 0;
			in_range = 0;
			break;
		case	UL_CONV:
			ch2 = conv_num(c);
			(ctype + 1)[ch1 + 257] = ch2;
			(ctype + 1)[ch2 + 257] = ch1;
			action = 0;
			break;   
		default:
			lower = ch1 = conv_num(c);
			in_range ++;
			if (type == UL) 
				if (ul_conv)
					{
					action = UL_CONV;
					break;
					}
				else
					error(input, msg[3], chrclass_name);
			else
				if (range)
					{
					action = RANGE;
					range = 0;
					}
				else
					;
			
			((ctype + 1)[lower]) |= type;
			break;
		}
	}
	if (action)
		error(input, msg[10], chrclass_name);
}

/* create1() produces a C source file based on the definitions
 * read from the input file. For example for the current ASCII
 * character classification where LC_CTYPE=ascii it produces a C source
 * file named ctype.c.
 */


static void
create1()
{
	struct  field {
		unsigned char  ch[20];
	} out[8];
	unsigned char	*hextostr();
	unsigned char	outbuf[256];
	int	cond = 0;
	int	flag=0;
	int i, j, index1, index2;
	int	line_cnt = 0;
	register struct classname *cnp;
	int 	num;


	comment1();
	(void)sprintf((char *)outbuf,"unsigned char\t_ctype[] =  { 0,");
	(void)printf("%s\n",outbuf);
	
	index1 = 0;
	index2 = 7;
	while (flag <= 1) {
		for (i=0; i<=7; i++)
			(void)strcpy((char *)out[i].ch, "\0");
		for(i=index1; i<=index2; i++) {
			if ( ! ((ctype + 1)[i]) )  {
				(void)strcpy((char *)out[i - index1].ch, "0");
				continue; }
			num = (ctype + 1)[i];
			if (flag) {      
				(void)strcpy((char *)out[i - index1].ch, "0x");  
				(void)strcat((char *)out[i - index1].ch, (char *)hextostr(num));
				continue; }
			while (num)  {
				for(cnp=cln;cnp->num != UL;cnp++) {
					if(!(num & cnp->num))  
						continue; 
					if ( (strlen((char *)out[i - index1].ch))  == NULL)  
						(void)strcat((char *)out[i - index1].ch,cnp->repres);
					else  {
						(void)strcat((char *)out[i - index1].ch,"|");
						(void)strcat((char *)out[i - index1].ch,cnp->repres); }  
				num = num & ~cnp->num;  
					if (!num) 
						break; 
				}  /* end inner for */
			}  /* end while */
		} /* end outer for loop */
		(void)sprintf((char *)outbuf,"\t%s,\t%s,\t%s,\t%s,\t%s,\t%s,\t%s,\t%s,",
		out[0].ch,out[1].ch,out[2].ch,out[3].ch,out[4].ch,out[5].ch,
		out[6].ch,out[7].ch);
		if ( ++line_cnt == 32 ) {
			line_cnt = 0;
			flag++; 
			cond = flag; }
		switch(cond) {
		case	1:
			(void)printf("%s\n", outbuf);
			comment2();
			(void)printf("\t0,\n");
			index1++;
			index2++;
			cond = 0;
			break;
		case	2:
			(void)printf("%s\n", outbuf);
			(void)printf("\n\t/* multiple byte character width information */\n\n");
			for(j=0; j<6; j++) 
				(void)printf("\t%d,", ctype[START_CSWIDTH + j]);
			(void)printf("\t%d\n", ctype[START_CSWIDTH + 6]);
			(void)printf("};\n");
			break;
		default:
			(void)printf("%s\n", outbuf);
			break;
		}
		index1 += 8;
		index2 += 8;
	}  /* end while loop */
	if (width)
		comment3();
}

/* create2() produces a data file containing the ctype array
 * elements. The name of the file is the same as the value
 * of the environment variable LC_CTYPE.
 */


static void
create2()
{
	register   i=0;
	if (freopen(tablename1, "w", stdout) == NULL) {
		perror(tablename1);
		exit(1);
	}
	for (i=0; i< SIZE - 2; i++)
		(void)printf("%c", ctype[i]);
}

static void
create3()
{
	int	length;
	char	*numeric_name;

	if (freopen(tablename2, "w", stdout) == NULL) {
		perror(tablename2);
		exit(1);
	}
	(void)printf("%c%c", ctype[START_NUMERIC], ctype[START_NUMERIC + 1]);
}


/* Convert a hexadecimal number to a string */

unsigned char *
hextostr(num)
int	num;
{
	unsigned char	*idx;
	static unsigned char	buf[64];
	idx = buf + sizeof(buf);
	*--idx = '\0';
	do {
		*--idx = "0123456789abcdef"[num % 16];
		num /= 16;
	  } while (num);
	return(idx);
}

static void
comment1()
{
	(void)printf("#include <ctype.h>\n\n\n");
	(void)printf("\t/*\n");
	(void)printf("\t ************************************************\n");
	(void)printf("\t *		%s  CHARACTER  SET                \n", tablename1);
	(void)printf("\t ************************************************\n");
	(void)printf("\t */\n\n");
	(void)printf("\t/* The first 257 characters are used to determine\n");
	(void)printf("\t * the character class */\n\n");
}

static void
comment2()
{
	(void)printf("\n\n\t/* The next 257 characters are used for \n");
	(void)printf("\t * upper-to-lower and lower-to-upper conversion */\n\n");
}

static void
comment3()
{
	(void)printf("\n\n\t/*  CSWIDTH INFORMATION                           */\n");
	(void)printf("\t/*_____________________________________________   */\n");
	(void)printf("\t/*                    byte width <> screen width  */\n");
	(void)printf("\t/* SUP1	  		     %d    |     %d         */\n",
		ctype[START_CSWIDTH], ctype[START_CSWIDTH + 3]);
	(void)printf("\t/* SUP2			     %d    |     %d         */\n",
		ctype[START_CSWIDTH + 1], ctype[START_CSWIDTH + 4]);
	(void)printf("\t/* SUP3			     %d    |     %d         */\n",
		ctype[START_CSWIDTH + 2], ctype[START_CSWIDTH + 5]);
	(void)printf("\n\t/* MAXIMUM CHARACTER WIDTH        %d               */\n",
		ctype[START_CSWIDTH + 6]);
}

/*VARARGS*/
static	void
error(va_alist)
va_dcl
{
	va_list	args;
	char	*fmt;
	char	*file;

	va_start(args);
	file = va_arg(args, char *);
	(void)fprintf(stderr, "ERROR in %s: ", file);
	fmt = va_arg(args, char *);
	(void)vfprintf(stderr, fmt, args);
	(void)fprintf(stderr, "\n");
	va_end(args);
	clean();
}

static void
check_chrclass(cp)
unsigned char	*cp;
{
	if (chrclass_num != UL)
		if (*cp == '<' || *cp == '>')
			error(input, msg[9], chrclass_name, *cp);
		else
			;
	else
		if (*cp == '-')
			error(input, msg[9], chrclass_name, *cp);
		else
			;
}

static void
clean()
{
	(void)signal(SIGINT, SIG_IGN);
	(void)unlink("ctype.c");
	(void)unlink(tablename1);
	(void)unlink(tablename2);
	exit(1);
}

/*
 *
 * 	n1[:[s1][,n2:[s2][,n3:[s3]]]]
 *      
 *	n1	byte width (supplementary code set 1)
 *	n2	byte width (supplementary code set 2)
 *	n3	byte width (supplementary code set 3)
 *	s1	screen width (supplementary code set 1)
 *	s2	screen width (supplementary code set 2)
 *	s3	screen width (supplementary code set 3)
 *
 */
static int
cswidth()
{
        char *byte_width[3];
	char *screen_width[3];
	char *buf;
	int length;
	unsigned int len;
	int suppl_set = 0;
	unsigned char *cp;

	if (*(p+strlen((char *)p)-1) != '\n') /* terminating newline? */
		return(0);
	p = clean_line(p);
	if (!(length = strlen((char *)p))) /* anything left */
		return(0);
	if (! isdigit((char)*p) || ! isdigit((char)*(p+length-1)) )
		return(0);
        if ((buf = malloc((unsigned)length + 1)) == NULL)
		error(cmdname, msg[13]);
	(void)strcpy(buf, (char *)p);
	ctype[START_CSWIDTH] = ctype[START_CSWIDTH + 6] = 0;
	ctype[START_CSWIDTH + 3] = ctype[START_CSWIDTH + 4] = ctype[START_CSWIDTH + 5] = UNDEFINED_CSWIDTH;
	cp = p;
	while (suppl_set < 3) {
		if ( !(byte_width[suppl_set] = strtok((char *)cp, tokens)) || ! check_digit(byte_width[suppl_set]) ){
			return(0);
		}
		ctype[START_CSWIDTH  + suppl_set] = atoi(byte_width[suppl_set]);
		if ( p + length == (unsigned char *)(byte_width[suppl_set] + 1) )
			break;
		len = (unsigned char *)(byte_width[suppl_set] + 1) - p;
		if (*(buf + len) == ',') {
			cp = (unsigned char *)(byte_width[suppl_set] + 2);
			suppl_set++;
			continue;
		}
		tokens[0] = ',';
		if ( !(screen_width[suppl_set] = strtok((char *)0, tokens)) || ! check_digit(screen_width[suppl_set]) )  {
			return(0);
		}
		ctype[START_CSWIDTH + suppl_set + 3] = atoi(screen_width[suppl_set]);
		if ( p + length == (unsigned char *)(screen_width[suppl_set] + 1) )
			break;
		cp = (unsigned char *)(screen_width[suppl_set] + 2);
		tokens[0] = ':';
		suppl_set++;
	}
	suppl_set = 0;
	while (suppl_set < 3) {
		if ( ctype[START_CSWIDTH + suppl_set + 3] == UNDEFINED_CSWIDTH )
			ctype[START_CSWIDTH + suppl_set + 3] = ctype[START_CSWIDTH + suppl_set];
		suppl_set++;
	}
	ctype[START_CSWIDTH + 6] = ctype[START_CSWIDTH];
	if (ctype[START_CSWIDTH + 1] + 1 > ctype[START_CSWIDTH + 6]) 
		ctype[START_CSWIDTH + 6] = ctype[START_CSWIDTH + 1] + 1;
	if (ctype[START_CSWIDTH + 2] + 1 > ctype[START_CSWIDTH + 6]) 
		ctype[START_CSWIDTH + 6] = ctype[START_CSWIDTH + 2] + 1;
return(1);
}

static unsigned char  *
clean_line(s)
unsigned char *s;
{
	unsigned char  *ns;

	*(s + strlen((char *)s) -1) = (char) 0; /* delete newline */
	if (!strlen((char *)s))
		return(s);
	ns = s + strlen((char *)s) - 1; /* s->start; ns->end */
	while ((ns != s) && (isspace((char)*ns))) {
		*ns = (char)0;	/* delete terminating spaces */
		--ns;
		}
	while (*ns)             /* delete beginning white spaces */
		if (isspace((char)*s))
			++s;
		else
			break;
	return(s);
}

static int
check_digit(s)
char *s;
{
	if (strlen(s) != 1 || ! isdigit(*s))
		return(0);
	else
		return(1);
}

static int
setnumeric(num_category)
int	num_category;
{
	int	len;

	p = clean_line(p);
	if ((len = strlen((char *)p)) == 0) 
		return(1);
	if (len > 1)
		return(0);
	ctype[START_NUMERIC + num_category - DECIMAL_POINT] = (int)p[0];
	return(1);
}
