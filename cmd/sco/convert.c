/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sco:convert.c	1.1"

/*
 *	@(#) convert.c 1.1 86/12/12 
 *
 *	Copyright (C) The Santa Cruz Operation, 1984, 1985, 1986, 1987.
 *	Copyright (C) Microsoft Corporation, 1984, 1985, 1986, 1987.
 *	This Module contains Proprietary Information of
 *	The Santa Cruz Operation, Microsoft Corporation
 *	and AT&T, and should be treated as Confidential.
 */

#include "sys/types.h"
#include "sys/emap.h"
#include "defs.h"

extern bool Dopt;
extern FILE *yyin;		/* the file lex reads */
static char *map_file_name;     /* the name of the input file */
static int line_num;		/* the line number in the input file */
static int value;		/* the returned value from lex */
/*
 * booleans to control multiple or missing
 * input, output, or compose sections 
 */
static bool saw_input, saw_output, saw_compose, saw_beep, saw_null;
/*
 * arrays of booleans to control multiple mappings of
 * input or output characters. 
 */
static bool used_in[256], used_out[256];

/*
 * pointers to the buffer 
 */
static byte *inmap, *outmap;
static byte *buffer; 

/*
 * a linked list of strings.
 * new strings are inserted so to keep the list sorted
 */
static struct string {
	char letter;
	int cnt;
	byte *str;
	struct string *next;
} *first_string;
static int nstrings;		/* how many strings? */
static int len_strbuf;		/* how many string characters in all? */

static byte comp_key;		/* the compose key if any */
/*
 * two linked lists of dead and compose sequences
 */
static struct dc {
	byte seq[3];
	struct dc *next_dc;
} *first_dead, *first_comp;

/*
 * counts of dead and compose keys
 * and the total number of dead/compose seqences
 */
static int ndead, ncomp, ndcseq;

/*
 * functions which return a non-int value
 */
static bool input(), output(), dead(), compose(), fill_the_buffer(),
	    add_seq();	    
/*
 * forward declarations
 */
static void oh_oh(), stuff(), initialize(), count_comp(), add_string(), nl(),
	    skip();
static int syntax(), seqcmp();

/*
 * convert the human readable text in the mapping file to 
 * a structured E_TABSZ character buffer.
 * return TRUE for a correct conversion.
 * if there were any syntax errors print a message and return BAD_MAP_FILE.
 */
int
convert(mfname, buf)
char *mfname, *buf;
{
	/*
	 * take the parameters and make them
	 * available to all functions in this file.
	 */

	map_file_name = mfname;
	buffer = (unsigned char *) buf;
	if ((yyin = fopen(mfname, "r")) == NULL) {
		fprintf(stderr, "\ncannot open %s\n", mfname);
		return(FALSE);
	}

	initialize();
	while (value != LEXEOF) {
		switch(value) {
		case INPUT:
			if(Dopt)
				fprintf(stderr, "\nCalling input()...");
			if (!input())
				return(BAD_MAP_FILE);
			break;
		case OUTPUT:
			if(Dopt)
				fprintf(stderr, "\nCalling output()...");
			if (!output())
				return(BAD_MAP_FILE);
			break;
		case DEAD:
			if(Dopt)
				fprintf(stderr, "\nCalling dead()...");
			if (!dead())
				return(BAD_MAP_FILE);
			break;
		case COMPOSE:
			if(Dopt)
				fprintf(stderr, "\nCalling compose()...");
			if (!compose())
				return(BAD_MAP_FILE);
			break;
		case BEEP:
			saw_beep = TRUE;
			nl();
			break;
		case NULL_KEYWORD:
			if(Dopt)
				fprintf(stderr, "\nSaw a null key-word...");
			saw_null = TRUE;
			nl();
			break;
		default:
			syntax("\nunknown keyword\n");
			return(BAD_MAP_FILE);
		}
	}
	fclose(yyin);
	if (saw_null) {
		if (saw_output || saw_input || saw_compose ||
		    ndead > 0 || saw_beep) {
			syntax("\n'null' must appear alone\n");
			return(BAD_MAP_FILE);
		} else
			return(NULL_MAP_FILE);
	}
	if (!saw_input) {
		syntax("\nmissing input section\n");
		return(BAD_MAP_FILE);
	}
	if (!saw_output) {
		syntax("\nmissing output section\n");
		return(BAD_MAP_FILE);
	}
	count_comp();
	if (Dopt)
		fprintf(stderr, "\n");
	if (fill_the_buffer())
		return(OKAY_MAP_FILE);
	else
		return(BAD_MAP_FILE);
}

/*
 * fill up the buffer with the data that we have gathered
 */
static
bool
fill_the_buffer()
{
	int i, j, prev;
	byte *p_dead, *p_comp, *p_seq, *p_str, *p_strbuf;
	byte *op_seq, *op_strbuf;
	struct dc *pd;
	struct string *ps;

	if (len_strbuf > 255) {
		error("too many characters in strings\n");
		return(FALSE);
	}
	if (ndcseq > 255) {
		error("too many dead/compose sequences\n");
		return(FALSE);
	}
	if (  256 		/* input mapping table */
	    + 256		/* output mapping table */
	    + 1			/* compose key */
	    + 1			/* beep? */
	    + 2			/* ptr to compose table */
	    + 2			/* ptr to dcseq table */
	    + 2			/* ptr to string table */
	    + 2			/* ptr to string buffer */
	    + 2*ndead		/* space for dead key table */ 
	    + 2*ncomp		/* space for comp key table */
	    + ((ndead+ncomp)? 2: 0) /* perhaps space for the last entry */
	    + 2*ndcseq		/* space for dead/compose sequences */
	    + 2*nstrings	/* space for string table */
	    + ((nstrings)? 2: 0)  /* perhaps space for the last entry */
	    + len_strbuf	/* space for string sequences */ 
	    > E_TABSZ ) {		/* total buffer size */
	 	error("there is no room to hold all of the data\n");
		error("for the strings, dead and compose keys\n");
		return(FALSE);
	}
	i = 256+256;
	buffer[i] = comp_key;
	buffer[i+1] = saw_beep;

	i += 1+1+4*2;
	p_dead = buffer+i;

	/*
	 * calculate the various indices and stuff them
	 * into the buffer at this point.
	 */
	i += 2*ndead;
	j = 256+256+1+1;
	stuff(j, i);
	p_comp = buffer+i;

	j += 2;
	i += ncomp*2;
	if (ndead+ncomp)
		i += 2;
	stuff(j, i);
	p_seq = buffer+i;
	op_seq = p_seq;	/* save for later comparison */

	j += 2;
	i += ndcseq*2;
	stuff(j, i);
	p_str = buffer+i;

	j += 2;
	i += nstrings*2;
	if (nstrings)
		i += 2;
	stuff(j, i);
	p_strbuf = buffer+i;
	op_strbuf = p_strbuf;

	/*
	 * now we have pointers to all the places we need
	 */
	prev = 0;
	i = 0;
	for (pd = first_dead; pd; pd = pd->next_dc) {
		if (pd->seq[0] != prev) {
			prev = *p_dead++ = pd->seq[0];
			*p_dead++ = i;
		}
		if (!inmap[pd->seq[1]]) {
			error("dead/compose keys cannot be used inside a dead key sequence\n");
			error("0x%02x 0x%02x 0x%02x\n",
			      pd->seq[0],
			      pd->seq[1],
			      pd->seq[2]);
			return(FALSE);
		}
		*p_seq++ = pd->seq[1];
		*p_seq++ = pd->seq[2];
		++i;
	}
	/*
	 * a sanity check.
	 * if we calculated everything correctly,
	 * at this point p_dead should be just up to p_comp.
	 */
	if (p_dead != p_comp)
		oh_oh("dead != comp diff = %d\n", p_comp-p_dead);
	prev = 0;
	for (pd = first_comp; pd; pd = pd->next_dc) {
		if (pd->seq[0] == comp_key || pd->seq[1] == comp_key) {
			error("the compose key cannot be used within a compose key sequence\n");
			error("0x%02x 0x%02x 0x%02x\n",
			      pd->seq[0],
			      pd->seq[1],
			      pd->seq[2]);
			return(FALSE);
		}
		if (pd->seq[0] != prev) {
			prev = *p_comp++ = pd->seq[0];
			*p_comp++ = i;
		}
		*p_seq++ = pd->seq[1];
		*p_seq++ = pd->seq[2];
		++i;
	}
	/*
	 * if there were any dead or compose sequences
	 * add a terminating entry in the table.
	 * if there were none do not.
	 */
	if (ndead+ncomp) {
		*p_comp++ = BNULL;	/* does not matter what it is */
				    /* but it should be the same each time */
		*p_comp++ = i;
	}
	/*
	 * some more sanity checks 
	 */
	if (p_comp != op_seq)
		oh_oh("comp != seq diff = %d\n", op_seq - p_comp);
	if (p_seq != p_str)
		oh_oh("seq != str diff = %d\n", p_str - p_seq);
	i = 0;
	for (ps = first_string; ps; ps = ps->next) {
		*p_str++ = ps->letter;
		*p_str++ = i;
		for (j = 0; j < ps->cnt; ++j) {
			*p_strbuf++ = ps->str[j];
			++i;
		}
	}
	if (nstrings) {
		*p_str++ = BNULL;	/* again, it has no meaning */
				   /* but it should be the same each time */
		*p_str++ = i;
	}
	if (p_str != op_strbuf)
		oh_oh("str != strbuf diff = %d\n", op_strbuf-p_str);
	/*
	 * fill the rest of the buffer with NULLS
	 * necessary so we can simply compare two buffers bytewise
	 * to see if they are identical.
	 */
	while (p_strbuf < buffer+E_TABSZ)
		*p_strbuf++ = BNULL;
	return(TRUE);
}

/*
 * something happened while the buffer was being filled
 * that indicated there was some internal inconsistency.
 * print a serious looking error message and quit.
 */
static void
oh_oh(fmt, args)
char *fmt, *args;
{
	error("mapchan: INTERNAL ERROR: ");
	fprintf(stderr, fmt, &args);
	exit(1);
}
		
/*
 * in the buffer stick the low and high bytes of an integer
 */
static void
stuff(j, i)
int j, i;
{
	buffer[j]   = (i & 0xff);
	buffer[j+1] = (i >> 8);
}

/*
 * get everything ready
 */
static void
initialize()
{
	int i;

	inmap = buffer;
	outmap = buffer+256;
	for (i = 0; i < 256; ++i) {
		inmap[i] = i;
		used_in[i] = FALSE;
		outmap[i] = i;
		used_out[i] = FALSE;
	}
	saw_input = FALSE;
	saw_output = FALSE;
	saw_compose = FALSE;
	saw_beep = FALSE;
	saw_null = FALSE;
	first_string = NULL;
	first_dead = NULL;
	comp_key = BNULL;
	first_comp = NULL;
	ndead = 0;
	ndcseq = 0;
	nstrings= 0;
	len_strbuf = 0;
	line_num = 1;
	skip();		/* a call to get the first real token */
}

/*
 * report a syntax error in the file
 * giving the file name and line number.
 */
static
syntax(fmt, args)
char *fmt;
int args;
{
	fprintf(stderr, "%s: %d: ", map_file_name, line_num);
	fprintf(stderr, fmt, &args);
	return(FALSE);
}

/*
 * how many different initial compose keys were there?
 */
static void
count_comp()
{
	byte prev;
	struct dc *p;

	ncomp= 0;
	prev = 0;
	for (p = first_comp; p; p = p->next_dc)
		if (prev != p->seq[0]) {
			++ncomp;
			prev = p->seq[0];
		}
}

/*
 * process the input section
 */
static
bool
input()
{
	int a, b;

	if (saw_input)
		return(syntax("input already seen\n"));
	saw_input = TRUE;
	nl();
	while (value >= 0) {
		a = value;
		if (a == 0)
			return(syntax("cannot map null\n"));
		if (used_in[a])
			return(syntax(
			     "value 0x%02x already used for input mapping\n", a));
		used_in[a] = TRUE;
		b = token();
		if (b < 0)
			return(syntax("expected char value\n"));
		else if (b == 0)
			return(syntax("cannot map to a null\n"));
		inmap[a] = b;
		nl();
	}
	if (Dopt)
		fprintf(stderr, "Returning from input()...");
	return(TRUE);
}

/* 
 * the output section including strings
 */
static
bool
output()
{
	int a, b, count, i;
	byte buf[80];
	struct string *p;

	if (saw_output)
		return(syntax("output already seen\n"));
	saw_output = TRUE;
	nl();
	while (value >= 0) {
		a = value;
		if (a == 0)
			return(syntax("cannot map null\n"));
		if (used_out[a])
			return(syntax(
			      "value 0x%02x already used for output mapping\n", a));
		used_out[a] = TRUE;
		b = token();
		if (b < 0)
			return(syntax("expected char value\n"));
		value = token();
		if (value == NEWLINE) {
			if (b == 0)
				return(syntax("cannot map to a null\n"));
			++line_num;
			outmap[a] = b;
			skip();
		} else if (value >= 0) {
			outmap[a] = 0;
			count = 2;
			buf[0] = b;	/* null is okay here */
			buf[1] = value;
			while ((value = token()) >= 0) 
				buf[count++] = value;
			len_strbuf += count;
			if (value != NEWLINE)
				return(syntax("NEWLINE expected\n"));
			++line_num;
			skip();
			/*
			 * get space to store the string and
			 * add it to the linked list.
			 */
			p = (struct string *) malloc(sizeof(struct string));
			if (!p)
				oops("out of memory\n");
			p->letter = a;
			p->cnt = count;
			p->str = (byte *) malloc(count*sizeof(char));
			if (!p)
				oops("out of memory\n");
			for (i = 0; i < count; ++i)
				(p->str)[i] = buf[i];
			add_string(p);
		}
	}
	if (Dopt)
		fprintf(stderr, "Returning from output()...");
	return(TRUE);
}

/*
 * add the string structure to the linked list
 * keeping things sorted
 */
static void
add_string(p)
struct string *p;
{
	struct string *q, *prev;

	++nstrings;
	if (!first_string) {
		p->next = NULL;
		first_string = p;
	} else if (p->letter < first_string->letter) { 
		p->next = first_string;
		first_string = p;
	} else {
		q = first_string;
		while (q->letter < p->letter && q->next) {
			prev = q;
			q = q->next;
		}
		if (q->letter >= p->letter) {
			prev->next = p;
			p->next = q;
		} else {
			q->next = p;
			p->next = NULL;
		}
	}
}

/*
 * the dead section
 */
static
bool
dead()
{
	int d, a, b;

	if ((d = token()) < 0)
		return(syntax("expected char value for dead key\n"));
	if (d == 0)
		return(syntax("dead key cannot be null\n"));
	if (used_in[d])
		return(syntax("value 0x%02x already used for input mapping\n", d));
	used_in[d] = TRUE;
	inmap[d] = 0;
	nl();
	/*
	 * are there any dead sequences following?
	 * if not, don't bump ndead.
	 */
	if (value >= 0)
		++ndead;
	while (value >= 0) {
		a = value;
		if (a == 0)
			return(syntax("cannot use null in dead key sequence\n"));
		if ((b = token()) < 0)
			return(syntax("expected char value\n"));
		if (!add_seq(&first_dead, d, a, b))
			return(syntax("duplicate dead key sequence: 0x%02x 0x%2x\n", d, a));
		nl();
	}
	if (Dopt)
		fprintf(stderr, "Returning from dead()...");
	return(TRUE);
}

/*
 * the compose key section
 */
static
bool
compose()
{
	int a, b, c;

	if (saw_compose)
		return(syntax("already saw compose\n"));
	saw_compose = TRUE;
	if ((a = token()) < 0)
		return(syntax("missing char value for compose key\n"));
	if (a == 0)
		return(syntax("compose key cannot be null\n"));
	comp_key = a;
	if (used_in[comp_key])
		return(syntax("value 0x%02x already used for input mapping\n",
			      comp_key));
	used_in[comp_key] = TRUE;
	inmap[comp_key] = 0;
	nl();
	while (value >= 0) {
		a = value;
		if ((b = token()) < 0)
			return(syntax("expected char value\n"));
		if (a == 0 || b == 0)
			return(syntax("cannot use nulls in compose key sequences\n"));
		if ((c = token()) < 0)
			return(syntax("expected char value\n"));
		if (!add_seq(&first_comp, a, b, c))
			return(syntax("duplicate compose key sequence: 0x%02x 0x%02x\n", a, b));
		nl();
	}
	if (Dopt)
		fprintf(stderr, "Returning from compose()...");
	return(TRUE);
}

/*
 * add the dead/compose key sequence to the specified list
 * keeping things sorted.
 */
static
bool
add_seq(pfirst, d, a, b)
struct dc **pfirst;
int d, a, b;
{
	int rc;
	struct dc *first;
	struct dc *p, *q, *prev;

	++ndcseq;
	p = (struct dc *) malloc(sizeof(struct dc));
	if (!p)
		oops("out of memory\n");
	p->seq[0] = d;
	p->seq[1] = a;
	p->seq[2] = b;
	first = *pfirst;
	if (!first) {
		p->next_dc = NULL;
		*pfirst = p;
	} else if (seqcmp(p, first) < 0) {
		p->next_dc = first;
		*pfirst = p;
	} else {
		q = first;
		while ((rc = seqcmp(q, p)) < 0 && q->next_dc) {
			prev = q;
			q = q->next_dc;
		}
		if (rc == 0)
			return(FALSE);		/* duplicate sequence */
		else if (rc < 0) {		/* add p after q */
			q->next_dc = p;
			p->next_dc = NULL;
		} else {			/* add p before q */
			prev->next_dc = p;
			p->next_dc = q;
		}
	}
	return(TRUE);
}

/*
 * the compare function for the sort.
 * there are two fields the sort is done on.
 */
static
int
seqcmp(p, q)
struct dc *p, *q;
{
	if (p->seq[0] < q->seq[0])
		return(-1);
	if (p->seq[0] > q->seq[0])
		return(1);
	return(p->seq[1] - q->seq[1]);
}

/*
 * the next token had better be a newline.
 * then skip to the next non-newline.
 */
static void
nl()
{
	if (token() != NEWLINE) 
		syntax("NEWLINE expected\n");
	else {	
		++line_num;
		skip();		/* we got end of line, skip additional ones */
	}
}

/*
 * skip to the next non-newline token.
 */
static void
skip()
{
	while ((value = token()) == NEWLINE)
		++line_num;
}
