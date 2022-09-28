/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sccs:lib/cassi/filehand.c	6.3"

/* EMACS_MODES: c !fill tabstop=4 */

/*
 *	filehand -- Get a handle on file operations.
 *
 *
 *
 *	This file contains a number of routines which are useful in handling
 *	files.  The operationd performed are defined in the filehand.h file,
 *	as are the return codes.
 */

# include <stdio.h>
# include "../../hdr/filehand.h"

/* Debugging options. */

# ifdef TRACE
extern FILE		*trace;
# define TR(W,X,Y,Z) fprintf (trace, W, X, Y, Z)
# else
# define TR(W,X,Y,Z) /* W X Y Z */
# endif

# define READ "r"
# define APP "a"
# define BUFSIZE 1024
# define SAME 0							/* Same as strcmp. */
# define BEFORE 1						/* Almost same as strcmp. */
# define AFTER (-1)						/* Ditto. */
# define ERROR (-1)
# define OFILE1 1
# define OFILE2 2
# define ALLOC1 4
# define ALLOC2 8
# define ALLOC3 16

# ifdef XALLOC
/* Use this for places where xalloc and xfree are used in the code which
 * calls filehand routines. */
USXALLOC();
# endif

static int	copyrest();

/*
 *	ftrans -- Handle routine file transfer operations.
 */

static int
ftrans (opcode, file1, file2, area, size)
int	opcode;				/* Choices are in filehand.h. */
char	*file1, *file2,			/* The files to handle. */
	*area;				/* Buffer area pointer. */
int	size;				/* Of buffer area. */
{
	FILE	*fp1, *fp2;
	int		retcode, unlink();

	TR("Ftrans: %d (%s) (%s)\n", opcode, file1, file2);
	TR("Ftrans: area=%d size=%d\n", area, size, EMPTY);

	if ((opcode == RENAME || opcode == CPY) &&
	  (fp2 = fopen (file2, READ)) != NULL) {
		if (fclose (fp2) == ERROR)
			return (RECURSE + DESTEXISTS);
		return (DESTEXISTS);
		}
	
	if ((opcode == MOVE || opcode == COPYOVER)
	  && (fp2 = fopen (file2, READ)) != NULL) {
		if (fclose (fp2) == ERROR)
			return (RECURSE + LIVEDEST);
		if (unlink (file2) == ERROR)
			return (LIVEDEST);
		}

	if ((fp1 = fopen (file1, READ)) == NULL)
		return (NOSOURCE);

	if ((fp2 = fopen (file2, APP)) == NULL) {
		if (fclose (fp1) == ERROR)
			return (RECURSE + NODEST);
		return (NODEST);
		}

	retcode = copyrest (fp1, fp2, area, size);
	if (fclose (fp1) == ERROR)
		retcode = LIVESRC;
	if (fclose (fp2) == ERROR)
		retcode = NOCLOSE;
	if (retcode != DONE)
		return (retcode);
	if ((opcode == MOVE || opcode == RENAME) && unlink (file1) == ERROR)
		return (LIVESRC);
	TR("Ftrans: normal return\n", EMPTY, EMPTY, EMPTY);
	return (DONE);
}

static int
copyrest (fp1, fp2, place, size)		/* Copy the rest of a file. */
FILE	*fp1, *fp2;				/* Use these file pointers. */
char	*place;					/* A buffer to use. */
int	size;					/* Size of buffer. */
{
	void	free();
	char	*space;				/* Area being used. */
	unsigned count;						/* Of bytes read. */
	int		retcode = DONE;

	TR("Copyrest: fp1=%d fp2=%d place=%d ", fp1, fp2, place);
	TR("size=%d\n", size, EMPTY, EMPTY);

	if (place == EMPTY) {
		if (size <= 0)
			size = BUFSIZE;
		if ((space = (char *) malloc ((unsigned) size)) == EMPTY) {
			TR("Copyrest: no space\n", EMPTY, EMPTY, EMPTY);
			return (NOSPACE);
			}
		}
	else {
		if (size <= 0)
			return (BADSIZE);
		space = place;
		}

	while ((count = fread (space, sizeof (char), (unsigned) size, fp1)) >
	  (unsigned) 0)
		if (fwrite (space, sizeof (char), count, fp2) != count) {
			retcode = COPYERROR;
			break;
			}

	if (place == EMPTY)
		(void) free (space);
	TR("Copyrest: returns %d\n", retcode, EMPTY, EMPTY);
	return (retcode);
}

static int
getrec (file, buffer, recsep, rectype, maxlen)
FILE	*file;					/* The open file to read from. */
char	buffer[],				/* The buffer to read into. */
	recsep;					/* The character to break on. */
int	rectype,				/* FIXED or VARYING length. */
	maxlen;					/* Maximum record length. */
{
	int		count;						/* Of characters read. */
	int	lc;		/* must be int to hold EOF */

	TR("Getrec: file=%d recsep=(%c) maxlen=%d", file, recsep, maxlen);
	TR(" rectype=%d buffer=%d\n", rectype, buffer, EMPTY);

	if (rectype == FIXED) {
		TR("Getrec: FIXED record size\n", EMPTY, EMPTY, EMPTY);
		count = fread (buffer, sizeof (char), (unsigned) maxlen, file);
		if (count == 0)
			return (FILE1EOF);
		else if (count == maxlen)
			return (DONE);
		else
			return (SHORTREC);
		}
	else if (rectype == VARIED) {
		TR("Getrec: VARIED record size\n", EMPTY, EMPTY, EMPTY);
		count = 0;
		while ((lc = fgetc (file)) != EOF) {
			if ((char) lc == recsep) {
				buffer[count] = NULL;
				TR("Getrec: returns buffer=(%s)\n", buffer, EMPTY, EMPTY);
				return (DONE);
				}
			else {
				buffer[count++] = (char) lc;
				if (count >= maxlen)
					return (BIGREC);
				}
			}
		TR("Getrec: end of file\n", EMPTY, EMPTY, EMPTY);
		buffer[count] = NULL;
		return (FILE1EOF);
		}
	else
		return (BADTYPE);				/* Unknown record type. */
}

static int
rec_cmp (args, fmatch, recsep)			/* Compare a record with fmatch. */
char	*args[], *fmatch[], recsep;
{
	int		i,							/* For looping. */
			cmpstat;					/* Comparison value. */
	int	strcmp();

	cmpstat = SAME;
	for (i = 0; fmatch[i] != EMPTY; i++) {
		cmpstat = strcmp (fmatch[i], args[i]);
		if (fmatch[i][0] == recsep || cmpstat == 0)
			cmpstat = SAME;
		else if (cmpstat > 0) {
			cmpstat = BEFORE;			/* fmatch must follow args. */
			/* That is, we could still match. */
			break;
			}
		else {
			cmpstat = AFTER;
			break;						/* The for loop. */
			}
		}
	TR("Rec_cmp: returns %d\n", cmpstat, EMPTY, EMPTY);
	return (cmpstat);
}

static int
argchop (s, fldsep, sargs)		/* Seperate s into its fields. */
char	*s,				/* The string to seperate. */
	fldsep,				/* Seperates the fields. */
	**sargs;			/* Pointers to the fields. */
{
	int		i = 0,						/* For looping. */
			subs = 0;					/* Counter of fields. */

	TR("Argchop: s=(%s) fldsep=(%c) sargs=%d\n", s, fldsep, sargs);

	while (s[i] == fldsep || (fldsep == WHITE && (s[i] == ' '
	  || s[i] == '\t')))
		s[i++] = NULL;					/* Skip initial blank fields. */

	while (s[i] != NULL) {
		if (s[i] == fldsep || (fldsep == WHITE && (s[i] == ' '
		  || s[i] == '\t')))
			s[i] = NULL;				/* Insert a string terminater. */
		else if ((i > 0 && s[i - 1] == NULL) || i == 0)
			sargs[subs++] = &s[i];		/* An argument begins here. */
		i++;
		}
	sargs[subs] = EMPTY;
# ifdef TRACE							/* Echo the fields. */
	for (i = 0; sargs[i] != EMPTY; i++)
		TR(".. %d (%s)\n", i, sargs[i], EMPTY);
# endif
	TR("Argchop: normal return\n", EMPTY, EMPTY, EMPTY);
	return (DONE);
}

/*
 *	sweep -- One shot file modification.  Execute the request and exit.
 */

sweep (opcode, file1, file2, recsep, fldsep, maxlen, fmatch, usrbuf, usrmatch,
  chop, compare)
int		opcode;							/* The operation to perform. */
char	*file1,							/* The search/source file. */
		*file2,							/* The scratch file.  (Required.) */
		recsep,							/* Record seperator. */
		fldsep;							/* Field seperator. */
int		maxlen;							/* Maximum length of record. */
char	*fmatch[],						/* Fields to match. */
		*usrbuf,						/* User's copy of matching record. */
		*usrmatch[];					/* Pointers to arguments. */
int		(*chop)(),						/* Routine to seperate the fields. */
		(*compare)();					/* Routine to compare records. */
{
	char	**arg,						/* Pointers to the fields. */
			*realrec,					/* A copy of what was actually read. */
			*hackrec;					/* A place to keep the fields. */

	int		cmpstat,					/* Comparison status. */
			retcode,					/* Return code. */
			action = 0,					/* Record of what we have done. */
			rectype,					/* FIXED or VARIED record size. */
			i, j;						/* Looping variable. */

	FILE	*fp1, *fp2;					/* Pointers to file1 and file2. */

	void	free();
	char 	*strncpy();				/* String copy in lPW. */

	TR("Sweep: opcode=%d file1=(%s) file2=(%s)\n", opcode, file1, file2);
	TR(".. recsep=(%c) fldsep=(%c) maxlen=%d ", recsep, fldsep, maxlen);
	TR("usrbuf=%d usrmatch=%d chop=%d\n", usrbuf, usrmatch, chop);
	TR(".. usrbuf=(%s) compare=%d\n", usrbuf, compare, EMPTY);
#ifdef TRACE
	TR(".. fmatch ..\n", EMPTY, EMPTY, EMPTY);
	for (i = 0; fmatch[i] != EMPTY; i++)
		TR("%d (%s)\n", i, fmatch[i], EMPTY);
#endif

	if ((fp1 = fopen (file1, READ)) == NULL) {
		if (opcode == PUTNOW || (opcode & INSERT) > 0) {
			if ((fp1 = fopen (file1, APP)) == NULL)
				return (NOSOURCE);
			if (fclose (fp1) == ERROR)
				return (NOSOURCE);
			if ((fp1 = fopen (file1, READ)) == NULL)
				return (NOSOURCE);
			TR("Sweep: made a blank file1\n", EMPTY, EMPTY, EMPTY);
			opcode = PUTNOW;			/* Since buffers are unneeded. */
			}
		else
			return (NOSOURCE);
		}

	if ((opcode & VERIFY) == 0) {
		if ((fp2 = fopen (file2, READ)) != NULL) {
			if (fclose (fp2) == ERROR || unlink (file2) == ERROR) {
				if (fclose (fp1) == ERROR)
					return (RECURSE + LIVEDEST);
				return (LIVEDEST);
				}
			}
		if ((fp2 = fopen (file2, APP)) == NULL) {
			if (fclose (fp1) == ERROR)
				return (RECURSE + NODEST);
			return (NODEST);
			}
		action += OFILE2;				/* Flag file2 as open. */
		}

	action += OFILE1;					/* Flag file1 as open. */

	TR("Sweep: file1%s open\n", ((action & OFILE2) > 0) ? " and file2" : "",
	  EMPTY, EMPTY);

	if (opcode == PUTNOW)
		goto begin;						/* Because we need no allocations. */

	/* At this point, we begin allocating the three areas that  this routine
	 * needs.  realrec will contain a copy of what was really read.  hackrec
	 * will provide space to store its individual fields.  arg will  provide
	 * space for the pointers the those fields.  The general scheme used for
	 * allocation and use of these areas is as follows:
	 *
	 *			Given:	usrbuf		--			usrbuf		--
	 *					usrmatch	usrmatch	--			--
	 *			---------------------------------------------------
	 *			arg		usrmatch/1	usrmatch/2	ALLOC3		ALLOC3
	 *			---------------------------------------------------
	 *			realrec	ALLOC1		ALLOC1		usrbuf/3	ALLOC1
	 *			---------------------------------------------------
	 *			hackrec	usrbuf/4	ALLOC2		ALLOC2		ALLOC2
	 *			---------------------------------------------------
	 *
	 * Note 1: On DELETE this  array will hold pointers to the actual fields
	 *  of the DELETED record.  On INSERT and REPLACE, it will hold pointers
	 *  into usrbuf which will be MEANINGLESS.  On VERIFY, it will also hold
	 *	pointers, but they are to the matched record.  On PUTNOW, this array
	 *	is unaffected.
	 *
	 * Note 2: On all operations, the pointers in this array will be useless
	 *	upon return.
	 *
	 * Note 3: On DELETE and VERIFY, this array will hold the actual  record
	 *	deleted or matched.  On INSERT or REPLACE, this record will hold the
	 *	actual  record  inserted  or  replaced (and is not used internally).
	 *	PUTNOW has no effect.
	 *
	 * Note 4: On DELETE and VERIFY, this array will hold the actual  record
	 *	deleted or matched.  On INSERT and REPLACE, this array will hold the
	 *	actual record inserted or replaced (in which case, the specification
	 *	of usrmatch will put only junk into that array, as in note 1).  This
	 *	area is not used internally for an INSERT or REPLACE.  This array is
	 *	not affected by PUTNOW.
	 */

	if (maxlen <= 0) {
		retcode = BADSIZE;
		goto bye;
		}

	retcode = NOSPACE;					/* All errors will be for this. */

	if (usrbuf == EMPTY) {				/* We must allocate a buffer. */
		TR("Sweep: no usrbuf\n", EMPTY, EMPTY, EMPTY);
		if ((realrec = (char *) malloc ((unsigned) maxlen)) == EMPTY)
			goto bye;
		action += ALLOC1;				/* A buffer was allocated. */
		TR("Sweep: allocated (1) realrec=%d\n", realrec, EMPTY, EMPTY);
		if ((hackrec = (char *) malloc ((unsigned) maxlen)) == EMPTY)
			goto bye;
		action += ALLOC2;
		TR("Sweep: allocated (2) hackrec=%d\n", hackrec, EMPTY, EMPTY);
		if (usrmatch == (char**) NULL) { /* User provided no field pointers. */
			j = (maxlen / 2 + 1) * sizeof (char*);
			if ((arg = (char**) malloc ((unsigned) j)) == (char**) NULL)
				goto bye;
			action += ALLOC3;			/* Pointer space was allocated. */
			TR("Sweep: allocated (3) arg=%d\n", arg, EMPTY, EMPTY);
			}
		else
			arg = usrmatch;
		}
	
	else {								/* A usrbuf area was specified */
		TR("Sweep: usrbuf given\n", EMPTY, EMPTY, EMPTY);
		if (usrmatch == (char**) NULL) {
			TR("Sweep: no usrmatch\n", EMPTY, EMPTY, EMPTY);
			if ((opcode & REPLACE) > 0 || (opcode & INSERT) > 0) {
				/* Alas, we cannot use the area. */
				if ((realrec = (char *) malloc ((unsigned) maxlen)) == EMPTY)
					goto bye;
				action += ALLOC1;
				TR("Sweep: allocated (4) realrec=%d\n", realrec, EMPTY, EMPTY);
				}
			else
				realrec = usrbuf;
			if ((hackrec = (char *) malloc ((unsigned) maxlen)) == EMPTY)
				goto bye;
			action += ALLOC2;
			TR("Sweep: allocated (5) hackrec=%d\n", hackrec, EMPTY, EMPTY);
			j = (maxlen / 2 + 1) * sizeof (char*);
			if ((arg = (char**) malloc ((unsigned) j)) == (char**) NULL)
				goto bye;
			action += ALLOC3;
			TR("Sweep: allocated (6) arg=%d\n", arg, EMPTY, EMPTY);
			}
		else {							/* We have usrbuf and usrmatch. */
			if ((opcode & REPLACE) > 0 || (opcode & INSERT) > 0) {
				if ((hackrec = (char *) malloc ((unsigned) maxlen)) == EMPTY)
					goto bye;
				action += ALLOC2;
				TR("Sweep: allocated (7) hackrec=%d\n", hackrec, EMPTY, EMPTY);
				}
			else
				hackrec = usrbuf;
			arg = usrmatch;
			if ((realrec = (char *) malloc ((unsigned) maxlen)) == EMPTY)
				goto bye;
			action += ALLOC1;
			TR("Sweep: allocated (8) realrec=%d\n", realrec, EMPTY, EMPTY);
			}
		}

begin:

	if (opcode == PUTNOW) {				/* Insert a message at beginning. */
		if ((action & OFILE2) > 0) {
			fprintf (fp2, "%s%c", usrbuf, recsep);
			retcode = DONE;
			goto bye;
			}
		else {
			retcode = NOTPUT;
			goto bye;
			}
		}

	if (recsep == NULL)
		rectype = FIXED;
	else
		rectype = VARIED;

	while ((i = getrec (fp1, realrec, recsep, rectype, maxlen)) == DONE) {

		/* Copy the buffer */
		strncpy (hackrec, realrec, (unsigned) maxlen);

		/* Seperate the command into arguments. */
		if (chop == (int (*)()) NULL)
			i = argchop (hackrec, fldsep, arg);
		else
			i = (*chop) (hackrec, fldsep, arg);
		if (i != DONE)
			break;						/* Something funny in record. */

		/* Now, determine the alphabetic status of this record. */
		if (compare == (int (*)()) NULL)
			cmpstat = rec_cmp (arg, fmatch, recsep);
		else
			cmpstat = (*compare) (arg, fmatch, recsep);

		/* Now, decide to continue or not. */
		if ((opcode & SEQUENTIAL) == 0 && cmpstat != SAME)
			continue;
			/* We continue if the search is not sequential and the record
			 * did not match. */

		if (cmpstat == SAME) {
			TR("Sweep: Enter SAME section\n", EMPTY, EMPTY, EMPTY);
			if ((opcode & INSERT) > 0)
				retcode = NOTNEW;
			else if ((opcode & VERIFY) > 0)
				retcode = FOUND;
			else {
				if ((opcode & REPLACE) > 0 && usrbuf != EMPTY)
					fprintf (fp2, "%s%c", usrbuf, recsep);
					/* And if this was a DELETE, nothing is printed. */
				retcode = DONE;
				}
			goto bye;
			}

		else if (cmpstat == BEFORE) {
			TR("Sweep: Enter BEFORE section\n", EMPTY, EMPTY, EMPTY);
			/* Put the record into the "scratch" file. */
			if ((opcode & VERIFY) == 0)
				fprintf (fp2, "%s%c", realrec, recsep);
			}

		else {							/* cmpstat == AFTER. */
			TR("Sweep: enter AFTER section\n", EMPTY, EMPTY, EMPTY);
			/* Match goes (or should have come) before this one. */
			if (opcode == SEQINSERT) {
				if (usrbuf != EMPTY)
					fprintf (fp2, "%s%c", usrbuf, recsep);
				/* Put in the INSERTed one, then replace the old one. */
				fprintf (fp2, "%s%c", realrec, recsep);
				retcode = DONE;
				}
			else
				/* We never get here on non-sequential searches. */
				retcode = NOTFOUND;
			goto bye;
			}
		}								/* End of while loop. */
	TR("Sweep: end of record loop\n", EMPTY, EMPTY, EMPTY);

	if (i != FILE1EOF) {
		TR("Sweep: funny death\n", EMPTY, EMPTY, EMPTY);
		retcode = i;					/* So user knows about bad data. */
		goto bye;
		}

	if (fclose (fp1) == ERROR)
		retcode = NOCLOSE;
	action -= OFILE1;

	if (((opcode & VERIFY) | (opcode & REPLACE) | (opcode & DELETE)) > 0) {
		TR("Sweep: not there\n", EMPTY, EMPTY, EMPTY);
		if ((opcode & VERIFY) == 0) {
			if (fclose (fp2) == ERROR)
				retcode = RESETERR;
			action -= OFILE2;
			if (unlink (file2) == ERROR) {
				retcode = RESETERR;		/* Should never happen? */
				goto bye;
				}
			}
		retcode = NOTFOUND;
		goto bye;
		}

	if ((opcode & INSERT) > 0) {
		TR("Sweep: insert tail record\n", EMPTY, EMPTY, EMPTY);
		if (usrbuf != EMPTY)
			fprintf (fp2, "%s%c", usrbuf, recsep);
		retcode = DONE;
		}
	else
		retcode = ABEND;				/* Should never happen? */

bye:

	TR("Sweep: closing retcode=%d action=%d\n", retcode, action, EMPTY);
	if (retcode == DONE) {				/* Successful on a file update. */
		if ((action & OFILE1) > 0 && (action & OFILE2) > 0)
			retcode = copyrest (fp1, fp2, EMPTY, BUFSIZE);
		}

	if ((action & OFILE1) > 0) {
		if (fclose (fp1) == ERROR)
			retcode = NOCLOSE;
		TR("Sweep: file1 closed\n", EMPTY, EMPTY, EMPTY);
		}
	if ((action & OFILE2) > 0) {
		if (fclose (fp2) == ERROR)
			retcode = NOCLOSE;
		TR("Sweep: file2 closed\n", EMPTY, EMPTY, EMPTY);
		}

	/* With the files closed, we now replace file1 with file2 (if needed). */
	if (retcode == DONE) {
		if ((retcode = ftrans (MOVE, file2, file1, EMPTY, BUFSIZE)) != DONE)
			retcode += RECURSE;
		TR("Sweep: final transfer\n", EMPTY, EMPTY, EMPTY);
		}
	if ((action & ALLOC1) > 0) {
		TR("Sweep: free realrec\n", EMPTY, EMPTY, EMPTY);
		free (realrec);
		}
	if ((action & ALLOC2) > 0) {
		TR("Sweep: free hackrec\n", EMPTY, EMPTY, EMPTY);
		free (hackrec);
		}
	if ((action & ALLOC3) > 0) {
		TR("Sweep: free arg\n", EMPTY, EMPTY, EMPTY);
		free ((char*) arg);
		}
	return (retcode);
}

