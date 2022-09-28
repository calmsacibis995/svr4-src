/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)lxprof:bblk/parse.c	1.9.2.10"

#include "hidelibc.h"		/* uses "_" to hide libc functions */
#include "hidelibelf.h"		/* uses "_" to hide libelf functions */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "comdef.h"
#include "comext.h"
#include "macdef.h"
#include "sgs.h"	/* SGS specific information for version stamping */
#include "dwarf.h"	/* debugger-specific info, for .debug data scanning */


/* * * * * *
 * # defines.
 */



/* * * * * *
 * define: BBLK_DEBUG_LEVEL -- set to 0, 1 or 2. 
 * 
 * Three levels of debugging aid which can be compiled
 * into basicblk: 0 (none), 1 (assertion checking), and
 * 2 (assertion checking plus debug/status output).
 * 
 * The level of info can be selected by defining this
 * identifier on the cc command line,.. or else below.
 * 
 * 
 * The features are turned off and on, by defining (or not)
 * the following #define identifiers:
 * 
 * un-defining NDEBUG enables 'assertion checking'.
 * This does sanity checks as we process things,
 * and only generates output when an abnormal situation
 * has been detected.  This may be useful for tracking
 * down the cause for abends.
 * 
 * un-defining BBLKDEB_OFF enables debug output, which
 * gives a heuristic report of basicblk's processing.
 */


	/* set to 0 (off), or 1 or 2 as above. */
#ifndef	BBLK_DEBUG_LEVEL
# define BBLK_DEBUG_LEVEL 0	/* default level is ``No debug help''. */
#endif



	/* * * * * *
	 * set flags according to debug level.
	 */
#if	BBLK_DEBUG_LEVEL == 0
# define BBLKDEB_OFF 1
# define NDEBUG 1

#else
# if	BBLK_DEBUG_LEVEL == 1
# define BBLKDEB_OFF 1
# undef NDEBUG

# else
#  undef BBLKDEB_OFF
#  undef NDEBUG
# endif

#endif


	/* * * * * *
	 * #include stuff and #define macros, based on above flags.
	 */
#include <assert.h>	/* for debugging and validation. */

#if (BBLKDEB_OFF || NDEBUG)
#	define debug(s)
#else
#	define debug(s)		s
#endif



/* * * * * *
 * #bytes to expand the line-buffer (line).
 * also, initial size.
 */
# define LINEBLKSIZE 2048


/* * * * * *
 * macros for advancing 'linptr' to/past white space.
 */

#define SKIP_WHITE_SPACE_AT(s)	for(; s!=NULL && (*s)!='\0' &&  isspace(*s); s++) \
						{/* null action */}

#define FIND_WHITE_SPACE_AT(s)	for(; s!=NULL && (*s)!='\0' && !isspace(*s); s++) \
						{/* null action */}


/* * * * * *
 * values for pseudo-ops.
 */

enum Pseudo_op_values
{
	ALIGN=1, BYTE, STRING, WORD, UA4BYTE, HALF, UA2BYTE, RESERVE,
	DATA, BSS, TEXT, SECTION, PREVIOUS,
	PUSHSECTN, POPSECTN,
	SET, FIL, TV, GLOBL,
		/* value returned if not one of the above */
	POTHER
}; 



/* * * * * *
 * labels which start with this prefix, by convention,
 * are not the target of a branch (convention observed by
 * the front-end and the optimizer); therefore, this type
 * of label need not signal the beginning of a new basic block.
 */
#define NONBRANCH_PREFIX	".."



/* * * * * *
 * these defines are used with error_alert.
 */
	/* level values */
#define LVL_ERROR	"ERROR"
#define LVL_WARNING	"WARNING"

	/* who values */
#define WHO_YYLEX	"yylex()"
#define WHO_BBLK	"basicblk"
#define WHO_CAOPEN	"CAopen"


/* * * * * *
 * these sections are used with sectn_change() et al.
 */

enum Section_action
{
	ACTN_GOTO, ACTN_NUKE, ACTN_PUSH, ACTN_POP, ACTN_PRIOR
};







	/* define macro to set a pointer (ptr) pointing at either */
	/* the next occurrence of any of the chars in 'set', or */
	/* the end of the string, whichever comes first! */
	/* NB requires terminating semicolon. */
# define SEEKCHARSorEOS(ptr,str,set)	ptr = strpbrk(str, set); \
					if (!ptr) \
						ptr = strchr(str,'\0')


/* * * * * *
 * structures and global variables.
 */


	/* anchor for current line buffer */
char * line = 0;
	/* size of buffer allocated at ``line'' */
int    LineSize=0;

	/* ``cur position'' pointer in current line */
char * linptr;


	/* line counter */
debug(int linct = 0;)



char *function_name=0;			/* name of (last) function */

#if MACH_IS_m32
static BOOL cc_trouble = FALSE;		/* TRUE=> save condition codes */
#endif



/* * * * * *
 * With ELF, the assembler maintains a stack of sections,
 * so that the current one can be pushed and a new one
 * asserted, restoring the later one with a pop.
 * We'll have to simulate this, to keep track of which
 * section we're in, when looking for .debug and .line
 * information.. in addition to knowing how to figure out
 * when we get back to the text and data sections.
 */

enum Section_name
{
		/* the sections we care about */
	SCN_BSS=1, SCN_DATA, SCN_TEXT, SCN_DEBUG, SCN_LINE,
		/* all other sections */
	SCN_OTHER
};


/* stack of sections, cur posn */
enum Section_name *Scn_stack = 0;
enum Section_name *Scn_p = 0;


int Scn_capacity = 0;		/* number of entries there, altogether */


 /* * * * * *
  * flag, True after a section change occurs, until
  * after a pseudo-op which defines data for that
  * section has been processed.
  */
int No_data_since_section_change = TRUE;




/* * * * * *
 * tests for the current section.
 */

#define CUR_SCN		( *Scn_p )
#define IN_SCN1(s)	( CUR_SCN == s )

#define IN_DATA		( IN_SCN1( SCN_DATA ) )
#define IN_TEXT		( IN_SCN1( SCN_TEXT ) )
#define IN_DEBUG	( IN_SCN1( SCN_DEBUG ) )
#define IN_LINE		( IN_SCN1( SCN_LINE ) )
#define IN_OTHER	( IN_SCN1( SCN_OTHER ) )



static BOOL savecc = FALSE;		/* has savecc allocation been done? */
static BOOL usecc = FALSE;			/* savecc code needed */



/* * * * * *
 * TRUE ==	the next line which appears, defines
 * 		the start of the next basic block.
 * 
 * hence, when the next line shows up, and this
 * is true, we should take appropriate action to
 * start a new basic block.
 * 
 * NB there are, in a sense, two kinds of basic blocks
 * in the world of basicblk: an 'official' basic block,
 * and an 'intuitive' basic block.
 * 
 * intuitive - a code sequence where all is executed if
 * 	any is, and which is punctuated by label-entries
 * 	and branch exits.
 * 
 * official - an intuitive basic block which is ALSO
 * 	punctuated by a 'source line' indication.
 * 
 * This is trying to imply that we define no more basic
 * blocks than there are .line section entries; there may
 * in fact be fewer (that is the hope, and in fact what we see
 * very often).
 * 
 * Another way of saying this, is that the granularity
 * of basic blocks is in source lines, not in actual
 * assembler instructions.  hence, several intuitive
 * basic blocks may be included in an official basic block,
 * because they all belong to the same line of source code
 * (e.g. a "?:" expression may be several intuitive bblks).
 * 
 * This flag is set when we can conclude that the next source line will
 * start a new ('official', from here on out) basic block.  This conclusion
 * is drawn under the following conditions:
 * 	at the beginning of a function
 * 	when we see a branch instruction
 * 	when we see a (non-ignorable) label
 * We turn off the flag unconditionally, under the following circumstances:
 * 	once we see a line-notation and define the expected block!
 * 	at the end of a function.
 * 
 * NB when COFF basicblk saw the .ln NN where NN matched the line number
 * mentioned in the .ef (i.e. the last line of the function), it would
 * turn the equivalent of this flag, off.  i don't have the same 'forsight'
 * (i.e. i don't know ahead of time what the last line is) - for now,
 * bblk does not try to carry over this checking.  maybe it should!
 * 	18oct88 rjp
 */
static BOOL NextLineStartsBblk = FALSE;

	/* macro to turn flag ON */
#define NEXTLINESTARTSBBLK_YES	do {  /* do this once */ \
					NextLineStartsBblk = TRUE; \
					debug(fprintf(stderr,"NextLine TRUE\n");) \
				} while(0);


	/* macro to turn flag OFF */
#define NEXTLINESTARTSBBLK_NO	do {  /* do this once */  \
					NextLineStartsBblk = FALSE; \
					debug(fprintf(stderr,"NextLine FALSE\n");) \
				} while(0);
	


/* * * * * *
 * TRUE ==	next time we see some executable code,
 * 		we should First generate some coverage instructions.
 */
static BOOL insertcov = FALSE;		/* insert coverage instruction(s) */


/* * * * * *
 * TRUE ==	next time we even transition to .text, put out
 * 		coverage instructions, right away.
 * 
 * this flag is set when we see a new .line entry, but wanted to
 * generate coverage instructions for the Prior (new) basic block,
 * and never had the opportunity.. because, for example, no code
 * was generated for that basic block (or it was optimized away..).
 * 
 * this is essentially 'insertcov, the first opportunity you get'!
 */
static BOOL insertCovSinceWasOmitted = FALSE;



/* * * * * *
 * the expected structure of the ELF assembler code for a function,
 * is as follows:
 *
 *  (a)	.debug info defining a subroutine or global_function
 *  (b)	the label for the function
 *	[the function's code]
 *  (c)	the end label for the function (pchi in leading .debug info)
 *
 * the action we take in each of these 'states', is:
 *
 *	(pre-a) ACTION:		none
 *		REACTION:	expect (a) for a function
 *
 *	(a)	ACTION:		record fcn name, hipc label
 *		REACTION:	expect the label; be ready to accept
 *				.line section information
 *
 *	(b)	ACTION:		none
 *		REACTION:	start recognizing and processing
 *				basic blocks; continue to see and
 *				process .line section info.
 *
 *	(c)	ACTION:		output coverage structure definition,
 *				finish internally for this function.
 *		REACTION:	expect (a)!
 *	
 *
 */

/* * * * * *
 * These flags track what we have seen, and what we expect
 * to see, for the function being scanned/to be scanned next.
 * They indicate (a) when we've come across the .debug entry
 * declaring to the world that a function is about to be defined,
 * (b) when we've seen the function's label, and (c) when we
 * come across the label that defines the 'hipc', or highest
 * address, hence implying that we're done with the function.
 *
 */

enum Function_Structure_state
	{
		STATE_SAW_NADA,
		STATE_SAW_FCN_DEBUG_DATA,
		STATE_SAW_START_LABEL,
		STATE_SAW_END_LABEL
	}
	cur_structure_state = STATE_SAW_NADA ;


	/* primitive state-access macros, used below */
#define CHK1STATE(s1)		( cur_structure_state == s1 )
#define NOTE_STATE(s)		cur_structure_state = s


	/* tests for the current state, in terms of what we have seen last */
#define SAW_NADA		CHK1STATE(STATE_SAW_NADA)
#define SAW_FCN_DEBUG_DATA	CHK1STATE(STATE_SAW_FCN_DEBUG_DATA)
#define SAW_START_LABEL		CHK1STATE(STATE_SAW_START_LABEL)
#define SAW_END_LABEL		CHK1STATE(STATE_SAW_END_LABEL)

	/* tests for the current state, in terms of what we expect next */
#define EXPECT_FCN_DEBUG_DATA	( SAW_NADA || SAW_END_LABEL )
#define EXPECT_START_LABEL	SAW_FCN_DEBUG_DATA
#define EXPECT_END_LABEL	SAW_START_LABEL


		/* state update macros, with error checking */
#define NOTE_FCN_DEBUG_DATA	if ( ! EXPECT_FCN_DEBUG_DATA ) { \
					error_alert(LVL_ERROR,WHO_YYLEX,err_msg[SR103]); \
				} else { \
					NOTE_STATE(STATE_SAW_FCN_DEBUG_DATA); \
				}

#define NOTE_START_LABEL	if ( ! EXPECT_START_LABEL ) { \
					error_alert(LVL_ERROR,WHO_YYLEX,err_msg[SR104]); \
				} else { \
					NOTE_STATE(STATE_SAW_START_LABEL); \
				}

#define NOTE_END_LABEL		if ( ! EXPECT_END_LABEL ) { \
					error_alert(LVL_ERROR,WHO_YYLEX,err_msg[SR103]); \
				} else { \
					NOTE_STATE(STATE_SAW_END_LABEL); \
				}


/* * * * * *
 * these flags are used to convey the current state of
 * the data accumulated for the .debug section.
 * 
 * the first thing we expect to see is the length of the tag.
 * 
 * NB! it is assumed that EACH transition to the .debug
 * section defines one (or 0) .debug entries!  this allows
 * us to skip over stuff we're not interested in.
 */

enum debugState{
	EXPECT_DEBUG_ENTRY_LENGTH, EXPECT_TAG_TYPE,
	EXPECT_ATTR_TYPE, EXPECT_ATTR_VALUE,
	EXPECT_TO_FLUSH_DEBUG
};

static enum debugState curDebState = EXPECT_DEBUG_ENTRY_LENGTH;


/* * * * * *
 * these flags are used to convey the current state of
 * the data accumulated for the .line section.
 * 
 * the first thing we expect to see is a header;
 * after that, we expect Only entries.
 * Since we really need to process all the .line
 * entries (even if only to catch the line number in each),
 * therefore we don't particularly care how the transitions
 * are made from/to this section.
 */

enum lineState{
		/* these two states are the initial two; then nevermore. */
	EXPECT_LINE_HEADER_WD1, EXPECT_LINE_HEADER_WD2,
		/* we cycle thru these states, one per field in entry. */ 
	EXPECT_LINE_NUMBER, EXPECT_STMTOFF, EXPECT_LINE_ENTRY_ADDR
};

static enum lineState curLineState = EXPECT_LINE_HEADER_WD1;



/* * * * * *
 * the type of a line number.
 */
typedef long int LINE_NUM;

static LINE_NUM curnum = 0;		/* last line number seen */



/* * * * * *
 * Note Bene - firstnum and lastnum are not valid when EXPECT_FCN_DEBUG_DATA,
 * since there are no .line entries between functions!, and curnum will only
 * be valid (in all likelihood) while EXPECT_END_LABEL.
 */

static LINE_NUM firstnum;		/* file-ABSOLUTE line# where fcn STARTs */
static LINE_NUM lastnum;		/* file-ABSOLUTE line# where fcn ENDs */
static char num_buf[16];		/* buffer for numbers: big enough! */


/* * * * * *
 * when this is a ptr (i.e. nonzero), this is used
 * to tell yylex to just scan and echo until it finds
 * the specified label.  the label (the actual line of
 * text to seek, including : and newline) are expected
 * to be in an allocated string (using say bbMalloc or strdup)
 * pointed at by an anchor.  This cell Here is to be
 * pointed at the anchor; when the label is found, the
 * string is freed and the anchor is zeroed (so the
 * caller knows it found it's label!).
 */
char **FlushSeekingLabelGivenAnch = 0;


/* * * * * *
 * these two 'anchors' are pointed at bbMalloc-ed
 * strings, which represent label-lines for which
 * to search.  the text of the string must be of
 * the form "<string>:", with no leading spaces
 * nor trailing spaces nor newline.
 */

char *function_end_label=0;		/* 0 or addr of endoffcn label str */

char *debug_end_label=0;		/* 0 or addr of endoffcn label str */





extern void exit();		/* standard exit function */


/* * * * * *
 * bbMalloc - safe malloc.
 * bbRealloc - safe realloc.
 * these routines do malloc/realloc for the specified amount;
 * if the alloction fails, the routine will print an error msg
 * and exit!
 */
char *
bbMalloc(size)
int size;
{
	char *mp = (char *) malloc (size);

	if (!mp)
		error_alert(LVL_ERROR, WHO_BBLK, "Insufficient Storage");
	
	return(mp);
}

char *
bbRealloc(mp, size)
char *mp;
int size;
{
	mp = (char *) realloc (mp, size);

	if (!mp)
		error_alert(LVL_ERROR, WHO_BBLK, "Insufficient Storage");
	
	return(mp);
}




/* * * * * *
 * this function finds the last whitespace char
 * in the string (if there is one), and zaps it
 * to NULL, effectively truncating trailing whitespaces.
 */
void
zap_trailing_whitespace(s)
char *s;
{
	char *t;


	if ( *s == '\0' )
		return;

	t = strchr(s, '\0')-1;

	while(t >= s && (*t == '\n' || isspace(*t)) )
		*t-- = '\0';		/* assumes there won't be many */
}



/* * * * * *
 * lineLoad - load 'line' with the next complete input line.
 * 
 * this routine gets the next line, and puts it in 'line'.
 * if line is not big enough, then it is extended.  the way
 * we determine whether or not it is big enough, is by
 * checking for a trailing newline - fgets will put one
 * there if we got the whole line.
 * 
 * if we did NOT get a whole line the first time, and
 * have more hanging out there, then we keep extending
 * the buffer and trying to read more.
 * we keep trying until we see a newline, or until
 * we hit EOF on reading from stdin.
 */
char *
lineLoad()
{
	char *tmp1ST, *tmpnTH;


	/* * * * * *
	 * if got no buffer yet, then get one first!
	 */
	if(!line)
	{
		LineSize = LINEBLKSIZE;
		line = bbMalloc(LineSize);
	}


	/* * * * * *
	 * please at least TRY to get the line
	 * in one shot!
	 */
	tmp1ST = fgets(line,LineSize,stdin);


	/* * * * * *
	 * while ( last read did not get a NL, & hence was too small )
	 * 	bump buffer size.
	 * 	extend the buffer itself.
	 * 	locate where we left off with the Last
	 * 	  line read in, and try to read the balance
	 * 	  pending, starting there!
	 */
	tmpnTH = tmp1ST;

	while ( tmpnTH!=NULL && tmpnTH[strlen(tmpnTH)-1]!='\n' )
	{
		LineSize += LINEBLKSIZE;
		line = bbRealloc(line, LineSize);
		tmpnTH = strchr(line,'\0');
		tmpnTH = fgets( tmpnTH, LineSize-(tmpnTH-line), stdin);
	}


	/* * * * * *
	 * if at least the FIRST read succeeded, then
	 * we've got SUMPIN to offer them.  hand that
	 * back.  if the first one failed, then by
	 * golly this will be zero...
	 * 
	 * NB IF we givem a line, first make sure
	 * that it's got no trailing blanks.
	 */
	if (tmp1ST)
		zap_trailing_whitespace(tmp1ST);

	return(tmp1ST);
}






static LINE_NUM bkcap = 0;	/* capacity of the queue. */
static LINE_NUM *bkqueue = 0;	/* the queue */

void
startline_add()
{
/* * * * * *
 * add a start-line-number, for a basicblk, to the list 
 * of start-line-numbers-for-bblks that we've accumulated
 * so far.  the number to add is curnum.
 * 
 * a side effect is to bump the number of basic blocks!
 * 
 * NB the storage associated with the current list can be
 * used for the next list, if bkcnt is reset to zero
 * before calling.  be sure startline_free is called
 * when all done, to clean things up.
 */
	int increm = 64;

	/* * * * * *
	 * if exceed current capacity, then expand it.
	 */
	if(++bkcnt>bkcap)
	{
		bkcap += increm;
		if(!bkqueue)
			bkqueue = (LINE_NUM *)
			  bbMalloc(bkcap * sizeof(LINE_NUM));
		else
			bkqueue = (LINE_NUM *)
			  bbRealloc( (char *) bkqueue, bkcap * sizeof(LINE_NUM));
	}

	bkqueue[bkcnt-1] = curnum;
}


/* * * * * *
 * free the queue datastructure.
 */
void
startline_free()
{
	/* * * * * *
	 * a NULL bkqueue is possible, when the .s code was
	 * generated manually, and contains No ``.line'' section
	 * data (with -ql the compiler always generates some, otherwise).
	 */
	if (bkqueue)
		free(bkqueue);
	bkqueue = (LINE_NUM *) 0; bkcap = 0;
}


void
alloc_array()
{

	/* sanity check on the size of the coverage array */
	if (bkcnt == ZERO) {
		fprintf(stderr,"ERROR - yylex(): %s\n", err_msg[SR105]);
		exit(FATAL);
	}

	/* * * * * *
	 * Coverage Structure definition.
	 * 
	 * first, change to data section.
	 * 
	 * then, call CAsize to put out the structure, along with
	 * a label which contains the function number.
	 * 
	 * then, put out a definition of our std label,
	 * and finally generate the definition for
	 * a place to save the condition code, if that's necessary.
	 */

		/* switch to data section of ELF file */
	if (fprintf(stdout,_DATA) <= PNT_FAIL) {
		fprintf(stderr,"ERROR - yylex(): %s\n", err_msg[SR109]);
		exit(FATAL);
	}

	/* * * * * *
	 * go generate the memory allocation staments needed
	 * to actually define the coverage structure.
	 */
	CAsize(bkcnt,bkqueue);


#if MACH_IS_m32
	if (cc_trouble == TRUE) {
		if (fprintf(stdout,_TROUBLE,fcnt) <= PNT_FAIL) {
			fprintf(stderr,"ERROR - yylex(): %s\n", err_msg[SR109]);
			exit(FATAL);
		}
	}
	else {
#endif
			/* definition of coverage structure label */
		if (fprintf(stdout,_MAKELOC,function_name) <= PNT_FAIL) {
			fprintf(stderr,"ERROR - yylex(): %s\n", err_msg[SR109]);
			exit(FATAL);
		}
		if (fprintf(stdout,_COVDEF,function_name,fcnt) <= PNT_FAIL) {
			fprintf(stderr,"ERROR - yylex(): %s\n", err_msg[SR109]);
			exit(FATAL);
		}

			/* .size pseudoop - byte size */
		if (fprintf(stdout,_SIZE,function_name,fcnt) <= PNT_FAIL) {
			fprintf(stderr,"ERROR - yylex(): %s\n", err_msg[SR109]);
			exit(FATAL);
		}
#if MACH_IS_m32
	}
#endif


#if MACH_IS_m32
	/* memory for SCC - do only once per file if needed */
	if ((usecc == TRUE) && (savecc == FALSE)) {
		savecc = TRUE;
		if (fprintf(stdout,_DEFSCC) <= PNT_FAIL) {
			    fprintf(stderr,"ERROR - yylex(): %s\n", err_msg[SR109]);
			    exit(FATAL);
			}
	}
#endif


		/* return to prior section of ELF file */
	if (fprintf(stdout,_PREVIOUS) <= PNT_FAIL) {
		fprintf(stderr,"ERROR - yylex(): %s\n", err_msg[SR109]);
		exit(FATAL);
	}
}



/* * * * * *
 * error_alert - pass a message on to the user.  If the message
 * is a warning, we just return; if it is an error, then we exit,
 * too.
 * Arguments: LVL_{WARNING/ERROR}, who's-name-should-appear, err_msg_str.
 */

error_alert(level, who, what)
char *level, *who, *what;
{
	fprintf(stderr,"%s - %s: %s\n", level, who, what);

	if( strcmp(level,LVL_ERROR)==0 )
		exit(FATAL);
}



/* * * * * *
 * looks up a section name (a string), and
 * returns the 'Section_name' enum value corresponding to it.
 */

	/* define # of entries in table below. */
#define NUMSNAMES	(sizeof(snamecode)/sizeof(int))

enum Section_name
slookup(s)
char *s;
{
	static char *snames[] =
		{
			"text", "data", "data1",
			".line", ".debug"
		};

	static enum Section_name snamecode[] =
		{
			SCN_TEXT, SCN_DATA, SCN_DATA,
			SCN_LINE, SCN_DEBUG
		};

	register enum Section_name i;

assert((sizeof(snamecode)/sizeof(int)) == (sizeof(snames)/sizeof(char *)));

	for (i = 0; i < NUMSNAMES; i++)
		if (!strcmp(s, snames[i]))
			return(snamecode[i]);
	return(SCN_OTHER);
}



enum Section_name
get_scn()
{
	char savechar, *p;
	enum Section_name tmp_scn;

	
	/* * * * * *
	 * get terminator for section name - will be either
	 * a comma, a semicolon or the end of the string.
	 * This involves finding first char, then looking
	 * for syntactic termination.
	 */
	SKIP_WHITE_SPACE_AT(linptr);	/* scan over white space chars */

	SEEKCHARSorEOS(p,linptr,",;");
	savechar = *p;			/* mark off section name */
	*p = '\0';

	tmp_scn = slookup(linptr);

	*p = savechar;			/* restore line now */

	return(tmp_scn);		/* tellem what section it is */
}

/* * * * * *
 * this routine simulates the effects of the pseudo-opts
 * which change section -
 *   pushsection popsection section/seg previous data data1 text
 *
 * actn - directs whether argument section is to be pushed,
 * 	used directly or ignored (and we should pop or return
 * 	to the immediately prior section).
 * scn - the section to which to change - unless the actn is
 * 	pop or return, in which case this argument is ignored.
 * 
 * 
 * NB this function flags that a section change has occurred,
 * by setting the flag No_data_since_section_change to TRUE.
 * 
 */
void
sectn_change( actn, scn )
enum Section_action actn;
enum Section_name scn;
{

static enum Section_name Prior_scn;

	/* * * * * *
	 * if there's no structure there yet, get one.
	 * this lets the next block of code assume that
	 * there's always a stack with at least one valid
	 * value at the top.
	 */
	if (!Scn_capacity)
		if ( actn != ACTN_POP )
			sectn_extend();

	
	/* * * * * *
	 * as long as we're going to change the section,
	 * save the Prior current section.  If all we're
	 * going to do is go the the Prior section, then
	 * leave this alone!
	 */
	if ( actn != ACTN_PRIOR )
		Prior_scn = CUR_SCN;

	
	/* * * * * *
	 * now - handle the action they request.
	 * 
	 * PUSH - if want to push beyond capacity, expand first;
	 * 	then push the section on.
	 * POP - contract down one, if anything's there;
	 * 	handle special cases: 0 left, 1 left, n>1 left.
	 * GOTO - change current section (insure there IS one!)
	 * PRIOR - toggle back to section active JUST prior to now
	 * 	(use a default if there is none).
	 * NUKE - release the stack, if there is one.
	 */
	switch( actn )
	{
	case ACTN_PUSH:
debug(fprintf(stderr,"push %d, was %d\n",scn,*Scn_p);)
		if ( Scn_p+1 - Scn_stack >= Scn_capacity )
			sectn_extend();

		*++Scn_p = scn ;

		break;

	case ACTN_POP:
debug(fprintf(stderr,"pop: was %d\n",*Scn_p);)
	/* * * * * *
	 * if there's just 1, then make believe
	 * they wanted to 'goto text'; we won't
	 * allow an empty stack;
	 * else just back the ptr up.
	 */
		if ( Scn_p == Scn_stack )
			*Scn_p = SCN_TEXT;

		else if ( Scn_p > Scn_stack )
			Scn_p--;

		break;

	case ACTN_GOTO:
debug(fprintf(stderr,"goto %d, was %d\n",scn,*Scn_p);)
	/* * * * * *
	 * just change the top (i.e. current) one.
	 */
		*Scn_p = scn;
			
		break;

	case ACTN_PRIOR:
debug(fprintf(stderr,"previous: before, %d, after %d\n",*Scn_p,Prior_scn);)
	/* * * * * *
	 * swap the current section with
	 * the prior section.
	 */
		{
			enum Section_name tmp = *Scn_p ;

			*Scn_p = Prior_scn;
			Prior_scn = tmp;
		}
		break;

	case ACTN_NUKE:
	/* * * * * *
	 * release the stack and return.
	 */
		if (Scn_stack)
			free( Scn_stack );
		Scn_stack = 0;
		Scn_capacity = 0;
		Scn_p = 0;
		break;

	default:
		break;
	}


	No_data_since_section_change = TRUE;
}



/* * * * * *
 * this function allocates a (larger) section stack;
 * if there is none yet, it allocates a first hunk
 * and initializes the first entry to 'text'.
 * 
 * it is assumed that, if there is a stack and this routine
 * is called, that Scn_p is nonzero: that is, if it's some
 * length but not long enough, then by golly there is at
 * least ONE entry on the stack!!
 */

#define SCNBLKCT	(8)		/* a reasonable expected depth */

sectn_extend()
{
	int a_hunk_size = SCNBLKCT * sizeof(enum Section_name) ;

	if ( !Scn_stack )
	{
		Scn_stack = (enum Section_name *) bbMalloc( a_hunk_size );
		Scn_capacity = SCNBLKCT;
		Scn_p = Scn_stack ;

		*Scn_p = SCN_TEXT;	/* init top entry to 'text */
	}
	else
	{
		int offset = ( Scn_p - Scn_stack );

		Scn_stack = (enum Section_name *)
			bbRealloc( (char *) Scn_stack,
				(Scn_capacity * sizeof(enum Section_name)) +
				                   a_hunk_size );
		Scn_capacity += SCNBLKCT;
		Scn_p = Scn_stack + offset ;
	}
}




/* * * * * *
 * this routine takes apropos internal bookkeeping action,
 * plus responds with output as apropos, to pseudo-ops.
 */

pseudo ()
{
	int pop;			/* pseudo-op code */
	char savechar;			/* saved char that follows pseudo */
	char * word = linptr;		/* pointer to beginning of pseudo */

/* The idea here is to stick in a NULL after the pseudo-op, then replace
** the character we clobbered, rather than copy the pseudo-op somewhere else.
*/

	FIND_WHITE_SPACE_AT(linptr);	/* scan to white space char */
	savechar = *linptr;		/* mark end of pseudo-op */
	*linptr = '\0';

	pop = plookup(word);
debug(fprintf(stderr,"pseudo-op name, code: %s, %d\n",word,pop);)

	*linptr = savechar;


	/* if we're in a data section, we write all pseudo-ops to output,
	** except for .word's, which we must examine, and .text .
	*/

	switch (pop) {			/* dispatch on pseudo-op in text */

	    case BSS:
		sectn_change( ACTN_GOTO, SCN_BSS );
		break;

	    case TEXT:
		sectn_change( ACTN_GOTO, SCN_TEXT );
		break;

	    case DATA:
		sectn_change( ACTN_GOTO, SCN_DATA );
		break;

	    case SECTION:
			/* get name following whitespace, set scn to that */
		sectn_change( ACTN_GOTO, get_scn() );
		break;

	    case PUSHSECTN:
			/* get name following whitespace, push that scn */
		sectn_change( ACTN_PUSH, get_scn() );
		break;

	    case POPSECTN:
		sectn_change( ACTN_POP, SCN_OTHER );
		break;

	    case PREVIOUS:
		sectn_change( ACTN_PRIOR, SCN_OTHER );
		break;

	    case BYTE:
	    case HALF:
	    case WORD:
	    case UA4BYTE:
	    case UA2BYTE:
	    case STRING:
	    /* * * * * *
	     * handle the definition of data.  This is dependent
	     * on the current section (as you might see looking there).
	     * 
	     * NB this turns the flag below to false, indicating that
	     * we Have in fact processed a data pseudo-op since the
	     * last section-changing pseudo op was seen.
	     * This helps the .line and .debug section data
	     * processors, since they assume that a transition
	     * to their section Starts a new 'entry'.
	     */
		data_defn(pop);
		No_data_since_section_change = FALSE;
		break;

	    case GLOBL:
	    case FIL:
	    case ALIGN:
	    case SET:
	    case TV:
	    default:			/* ignore all unrecognized
					** text pseudo-ops
					*/
		break;
	}

	/* * * * * *
	 * if just changed to .text.. if we see that a bblk was defined
	 * for a preceding line, but no code has yet appeared for it,
	 * so that no covg instrs were inserted yet: insert them now.
	 * This is triggered by line_data(); see comment there.
	 * 
	 * be sure that all covg-generating flags are now Off.
	 */
	/*
	*	This code was removed because (1) it inserts the
	*	coverage addition in the wrong section and (2) even
	*	if it worked correctly, it needlessly breaks the line
	*	profiling in certain situations (when the next instruction
	*	does NOT need the flags and the target machine cannot
	*	save the PSW).  The alternative is to wait for the next
	*	opcode (the normal scheme) to generate the cov instr.
	*
	*	if (IN_TEXT && insertCovSinceWasOmitted) {
	*		genCovInstrs(TRUE);
	*		insertcov = insertCovSinceWasOmitted = FALSE;
	*	}
	*/
}




/* * * * * *
 * data_defn - handle a data definition pseudo-op
 * 
 * this function handles the data definition pseudo-ops
 * we care about, specifically the different flavors of
 * 'word' and 'byte'.  We especially care about these
 * when in the .debug and the .line section, since they
 * declare information we're trying to track!
 * 
 * the purpose is, to call those fcns tracking those 
 * bits of information, when in those sections, and to
 * ignore these things otherwise.
 */

static char *current_datum_p =0;	/* the pseudo-op argument item */
					/* now of interest */

data_defn( p_op_code )
int p_op_code ;
{
	/* * * * * *
	 * the idea right here, is to save a copy of the data value
	 * associated with this pseudo-op, for further processing.
	 * NB this may actually be more than one value!
	 */

	SKIP_WHITE_SPACE_AT(linptr);	/* find the start of the value(s) */
	current_datum_p = linptr;

	
	/* * * * * *
	 * now process the data found in a way apropos
	 * to the current section.
	 */
	switch( CUR_SCN ) {
	case SCN_DEBUG:
		debug_data( p_op_code );
		break;

	case SCN_LINE:
		line_data();
		break;

	default:
		/* * * * * *
		 * we just ignore data in other sections.
		 */
		break;
	}
	
}



/* * * * * *
 * attr_type - here, we store the type of the last debug attribute type,
 * for 'attribute type'-'attribute value' processing.
 * 
 * tag_type - Here we store the 'tag' type of the last debug
 * entry seen.
 */

long debug_attr_type; 
long debug_tag_type; 



/* * * * * *
 * debug_data - process data found whilst in the .debug section
 * 
 * this function processes data found in the debug section.
 * it goes through a series of states, expecting data to describe
 * debug entries, and flags global results and accumulates information
 * as apropos to the type of entry.
 * 
 * it is assumed that linptr points at the value(s)
 * associated with this data pseudo-op, and that current_datum_p
 * points at the next one to process.
 * 
 * the latter is updated as each datum is processed.
 * only the line number is read; other data are ignored.
 */
debug_data(pop)
int pop;
{
debug(fprintf(stderr,"debug line: %s\n",line);)
	/* * * * * *
	 * re-set the state to expect a new entry, if
	 * we saw a recent transition to this section.
	 */
	if(No_data_since_section_change)
		curDebState = EXPECT_DEBUG_ENTRY_LENGTH;

	/* * * * * *
	 * now, process each of the data values defined
	 * with this data pseudo-op.
	 * it is assumed that the process fcn below, advances
	 * the state itself.
	 */
	while( current_datum_p != 0  &&  FlushSeekingLabelGivenAnch==0 )
		debug_process_datum(pop);

debug(fprintf(stderr,"finished with this line.\n");)
}



/* * * * * *
 * debug_new_tag - handle seeing a new debug tag.
 * this function determines what to do with a new
 * tag.  the significance of this debug thingee
 * depends entirely on the tag that we see!
 */
enum debugState
debug_new_tag()
{
	enum debugState retval;


	switch(debug_tag_type)
	{
	case TAG_global_subroutine:
	case TAG_subroutine:
	case TAG_member_function:
	/* * * * * *
	 * these all flag the appearance of a new function
	 * on the scene.  
	 * 
	 * all initializations are handled when we see the
	 * functions' label!!!!
	 * 
	 * expect (sigh) to have to process some attr/value pairs.
	 */
debug(fprintf(stderr,"Saw function/subroutine debug entry tag.\n");)
		NOTE_FCN_DEBUG_DATA;
		retval = EXPECT_ATTR_TYPE;
		break;

	default:
	/* * * * * *
	 * anything else - note that we should ignore
	 * the entire entry.
	 */

debug(fprintf(stderr,"Saw non-function/subroutine debug entry tag.\n");)
		retval = EXPECT_TO_FLUSH_DEBUG;
		break;
	}

	return(retval);
}



/* * * * * *
 * bytesFor - indicate the (fixed) number of bytes of value
 * 		data expected to be defined for an attr type.
 * 
 * we don't particularly care about the specific attribute;
 * more, we are interested in the 'form' of the value
 * associated with this attribute type.  the form is
 * encoded in the lower 4 bits; hence the shift.
 * 
 * used by debug_attr_value.
 */
int
bytesFor(attr_type)
int attr_type;
{
	int len;

	switch(attr_type&FORM_MASK)
	{
	case FORM_STRING:	/* NUL-terminated string */
	/* * * * * *
	 * assuming that the only time we get here, is if
	 * the string is being defined by a series of
	 * .byte values...
	 */
		len = 1;
		break;

	case FORM_DATA2:	/* 2 bytes */
		len = 2;
		break;

	case FORM_ADDR:		/* relocated address */
	case FORM_REF:		/* reference to another .debug entry */
	case FORM_DATA4:	/* 4 bytes */
		len = 4;
		break;

	case FORM_DATA8:	/* 8 bytes (two 4-byte values) */
		len = 8;
		break;

	case FORM_BLOCK2:	/* block with 2-byte length */
	case FORM_BLOCK4:	/* block with 4-byte length (unused) */
	case FORM_NONE:		/* error */
	default:
		len = 0;
		break;

	}

	if (len==0)
		error_alert(LVL_ERROR, WHO_BBLK,
				"invalid FORM for attribute type");

	return(len);

}



/* * * * * *
 * bytesIn - return #of bytes defined by a datum,
 * 		given as an arg of this pseudoop.
 * 
 * used by debug_attr_value.
 */
int
bytesIn(pop)
int pop;
{
	int len;

	switch(pop)
	{
	case BYTE:
		len = 1;
		break;

	case WORD:
	case UA4BYTE:
		len = 4;
		break;

	case HALF:
	case UA2BYTE:
		len = 2;
		break;

	default:
		len = 0;
		break;
	}

	if (len==0)
		error_alert(LVL_ERROR, WHO_BBLK,
				"unexpected .debug value pseudo-op");

	return(len);
}



/* * * * * *
 * ignore_counting_bytes - ignore data, counting to determine
 * when we're done with the value for this Type/Value pair.
 * 
 * this routine tracks the number of bytes associated with
 * attribute Values which have a fixed size, or which
 * have a leading size, so that skipping them is a matter
 * of ignoring bytes as they go by.
 * 
 * Basic idea is:
 * 
 * 	IF the byte count isn't set yet, set it.
 * 	Decrement the count of bytes to ignore, in effect
 * 	  ignoring the current datum.
 * 	Set retcode to reflect whether/not count
 * 	  has gone down to zero.
 */
enum debugState
ignore_counting_bytes(pop)
int pop;
{
	static long bytesToIgnore = 0;
	enum debugState nextDebugState;


	/* * * * * *
	 * FIRST: do we have a byte count yet?
	 * -----------------------------------
	 * 
	 * if not, then this is the first datum for this
	 * attribute type/attribute value pair.  let's
	 * first set up to skip the data by
	 * setting up a skip-count.
	 */
	if (bytesToIgnore==0)

	switch(debug_attr_type&FORM_MASK)
	{
	case FORM_BLOCK2:
	case FORM_BLOCK4:
	/* * * * * *
	 * these two types have variable lengths, specified by
	 * the first datum.
	 * Special case (a dumb one): if the length says that
	 * no data will follow.  Be sure that we then expect
	 * a type next, and not a datum.
	 * 
	 * NB be sure to patch the ignore count to include the length
	 * of the count itself!  We must do this because "The
	 * value of the (2/4)-byte length is the number of
	 * information bytes that follow." (Debugging Information
	 * reqts, pp.2-3, B. K. Russell, June 2, 1988).
	 * 
	 */
		bytesToIgnore = strtol(current_datum_p, (char **) NULL, 0);

assert(bytesToIgnore>=0);

		bytesToIgnore += bytesIn(pop);
		break;

	default:
	/* * * * * *
	 * all other values are of fixed length;
	 * to compute their length is an easy lookup.
	 */
		bytesToIgnore = bytesFor(debug_attr_type);
assert(bytesToIgnore>0);
		break;
	};


	/* * * * * *
	 * SECOND: we're all set to ignore some data.
	 * ------------------------------------------
	 * 
	 * Now ignore the value data associated with
	 * the current pseudo-op.
	 * 
	 * (The count should stay non-negative.)
	 */
	bytesToIgnore -= bytesIn(pop);

assert(bytesToIgnore>=0);



	/* * * * * *
	 * LAST: .....
	 * -----------
	 * 
	 * if we skipped all the bytes we need to skip,
	 * then the next datum is a Type, and we should say so.
	 * else, have 'em look for another Value.
	 */
	if (bytesToIgnore==0)
		nextDebugState = EXPECT_ATTR_TYPE;
	else
		nextDebugState = EXPECT_ATTR_VALUE;


	return(nextDebugState);
}

/* * * * * *
 * ignore_debug_string - ignore a value that is a string,
 * terminated by a zero byte value.
 * 
 * this routine is called when a value (of an attribute
 * Type/Value pair) is to be a string.  a string may be
 * defined Either by a '.string' pseudo-op, or by a series
 * of .byte pseudo-ops.  this routine is satisfied either
 * by a series of .byte values, ending with a zero value,
 * or with a single .string value.
 * 
 * the routine indicates whether it is satisified by
 * setting the return code to expect another VALUE
 * (EXPECT_ATTR_VALUE) in the sequence, or to expect
 * a TYPE (EXPECT_ATTR_TYPE) as part of a new pair.
 */
enum debugState
ignore_debug_string(pop)
int pop;
{
	enum debugState nextDebugState;


	/* * * * * *
	 * expect, as a value, either a string or
	 * a series of byte values, ending in a 0 byte.
	 */

debug(fprintf(stderr,"debstring ``pseudo-op code'' received: %d\n",pop);)

	if (pop == STRING) {
		/* a string is complete unto itself. we're done. */
		/* want to leave bytesToIgnore == 0 */
		nextDebugState = EXPECT_ATTR_TYPE;
	}
	else if (pop == BYTE) {
		/* * * * * *
		 * IF see the 0, then we're done, and
		 * this is the last datum.  We should
		 * see an attribute Type next;
		 * 
		 * ELSE, this is NOT the terminating
		 * char.  come back and expect another byte.
		 */
		if (strtol(current_datum_p, (char **) NULL, 0) == 0)
			nextDebugState = EXPECT_ATTR_TYPE;
		else
			nextDebugState = EXPECT_ATTR_VALUE;
	}
	else
		error_alert(LVL_ERROR, WHO_BBLK,
			    ".debug: invalid string value");


	return(nextDebugState);
}

/* * * * * *
 * ignore_debug_value - ignore value of type/value pair.
 * 
 * this fcn is called to ignore the value of an type/value
 * pair for a debug entry, given that we expect this pseudo-op
 * to be defining attribute value data.  it keeps track of
 * how much data needs to be ignored, and indicates whether
 * or not it expects more data, from its return value.
 * 
 * it takes as an argument...
 * - the pseudo-op type for this datum.
 * 
 * if it expects MORE data, it returns value
 * 		EXPECT_ATTR_VALUE
 * 
 * else, if the value is now complete and an attr type
 * shall appear next, it returns
 * 		EXPECT_ATTR_TYPE
 * 
 * 
 * THE LOGIC IS AS FOLLOWS:
 * 
 * we will either be scanning for a terminator, or
 * counting off bytes to ignore.
 * (the former case is Specifically, for STRINGs.)
 * 
 * IF scanning for a terminator, then
 * 	see if we have it yet; set the return code
 * 	to reflect success/failure to find terminator.
 * 
 * Else
 * 	ignore this datum either as part of a fixed-length
 * 	datum, or part of a datum with a preceding count.
 * 	Set retcode to reflect whether/not count
 * 	  has gone down to zero.
 * 
 */
enum debugState
ignore_debug_value(pop)
int pop;
{
	enum debugState nextDebugState;


	if ( (debug_attr_type&FORM_MASK) == FORM_STRING )
		nextDebugState = ignore_debug_string(pop);

	else
		nextDebugState = ignore_counting_bytes(pop);
		

	return(nextDebugState);
}



/* * * * * *
 * debug_attr_value - handle attributes.
 * 
 * this function deals with attribute-value pairs.
 * it turns out that the only one we care about
 * is the high-address of a function (AT_high_pc).
 * all others we ignore.
 * 
 * in fact, once we see the one we want, we flag
 * that all the rest should be flushed down the tube.
 * 
 * this function is called with One Datum, even
 * if the associated pseudo-op has a list of data.
 * 
 * this function does the work of recognizing what
 * data is required for each attribute type, and
 * then progresses, ignoring or interpreting
 * that data for each type/value pair.
 * 
 * if this function finds insufficient data for
 * an attribute value, it returns a state that
 * would call it again to continue eating/processing
 * the data generated for the value of this type/value pair.
 * 
 */
enum debugState
debug_attr_value(pop)
int pop;
{
	enum debugState nextDebugState;


	switch(debug_attr_type&FORM_MASK)
	{
	case FORM_ADDR:
	/* * * * * *
	 * the only ADDR we care about is the high_pc...
	 * 
	 * expect, as a value, the end LABEL for a function.
	 * we save this label now, so that we can realize
	 * that this function has ended, when we come
	 * across this label later in the code.
	 */
		if (debug_attr_type == AT_high_pc )
		{
				/* +1 for NULL, +1 for : */
			function_end_label = bbMalloc (strlen(current_datum_p)+1 + 1);
			strcpy(function_end_label,current_datum_p);
			strcat(function_end_label,":");
debug(fprintf(stderr,"fcnendlabel: %s\n",function_end_label);)
			nextDebugState = EXPECT_TO_FLUSH_DEBUG;
		}
		else
		{
			nextDebugState = ignore_debug_value(pop);
assert(nextDebugState==EXPECT_ATTR_TYPE || nextDebugState==EXPECT_ATTR_VALUE);
		}

		break;

	default:
	/* * * * * *
	 * ignore all other values.
	 */
		nextDebugState = ignore_debug_value(pop);
assert(nextDebugState==EXPECT_ATTR_TYPE || nextDebugState==EXPECT_ATTR_VALUE);
		break;

	}

debug(fprintf(stderr,"debug attr value processed; expect next: ATTR %s\n", 
	(nextDebugState==EXPECT_ATTR_TYPE?"Type.":
		(nextDebugState==EXPECT_ATTR_VALUE?"Value.":
			"Other!")
	));
     )

	return(nextDebugState);
}



/* * * * * *
 * debug_process_datum - process one datum
 * 
 * this function handles one datum defined by a data
 * pseudo-op, which can define one or potentially a
 * comma-seperated list of data items.
 * if pressed to be accurate, truly this function
 * processes one Argument, which may be many data.
 * 
 * it updates current_datum_p, and sets it to zero
 * when there are no more items in the list.
 * 
 * this function shifts itself to subsequent states, depending
 * on what it sees.
 */
debug_process_datum(pop)
int pop;
{
	char save_char, *endp;


		/* delimit next datum - */
		/*to be cleaned up as debug_data's last act */
	SEEKCHARSorEOS(endp,current_datum_p,",;");

	save_char = *endp;
	*endp = '\0';

debug(fprintf(stderr,"debug datum: %s\n", current_datum_p);)

	
	switch(curDebState)	/* interp. datum according to expectation */
	{

	case EXPECT_DEBUG_ENTRY_LENGTH:
	/* * * * * *
	 * expect one of two things:
	 * -- constant value==4 -- meaning a null entry.
	 * -- labelE-labelS --   anything else not a label
	 * expression (e.g. if labelE has funny chars..) is Wrong0!
	 * 
	 * If we FIND a null entry, then we should just
	 * set up to expect another entry, period.
	 * 
	 * If we FIND a label expression, then
	 * save the first label (labelE), so we can
	 * seek for it when we're done with this debug entry.
	 * we like labels that consist of [a-zA-Z_.$].
	 */
		{
			char *p = strchr(current_datum_p, '-');
			int len=0, ct=0;

			if (isdigit(*current_datum_p))
			{
				/* * * * * *
				 * if a constant expr, must evaluate
				 * to '4' -else it ain't a Null debug
				 * entry, and we don`t know WHAT it is!
				 */
				if(atoi(current_datum_p)!=4)
					error_alert(LVL_ERROR,WHO_BBLK,
				             ".debug: invalid length expr");

					/* got a 4 - seek next entry. */
				curDebState = EXPECT_DEBUG_ENTRY_LENGTH;
			}
			else if (p)
			{
					/* chars + ':' */
					/* NB this is the MAX size needed.. */
					/* hopefully exactly THE size. */
				len = p - current_datum_p + 1 ;
					/* bbMalloc len for NULL too -> +1 */
				debug_end_label = bbMalloc (len+1);
				ct = sscanf(current_datum_p,
				              "%[a-zA-Z0-9_.$]",
					      debug_end_label );
				strcat(debug_end_label,":");
debug(fprintf(stderr,"len=%d,debug_end_label=%s\n",len,debug_end_label);)
				if ( ct!=1 || strlen(debug_end_label)!=len )
					error_alert(LVL_ERROR, WHO_BBLK,
				             ".debug: invalid length expr");

				curDebState = EXPECT_TAG_TYPE;
			}
			else
				error_alert(LVL_ERROR, WHO_BBLK,
					    ".debug: invalid length expr");

		}

		break;

	case EXPECT_TAG_TYPE:
	/* * * * * *
	 * expect a single numeric value, a tag type value.
	 * we return the state to which to proceed: either 
	 * EXPECT_ATTR_TYPE or EXPECT_TO_FLUSH_DEBUG.
	 */
		debug_tag_type = strtol(current_datum_p, (char **) NULL, 0);
assert(debug_tag_type>0);

		curDebState = debug_new_tag();
assert(curDebState==EXPECT_ATTR_TYPE || curDebState==EXPECT_TO_FLUSH_DEBUG);
		break;

	case EXPECT_ATTR_TYPE:
	/* * * * * *
	 * save the attribute type found, for
	 * when we want to come back to process
	 * the value.
	 */
		debug_attr_type = strtol(current_datum_p, (char **) NULL, 0);
assert(debug_tag_type>0);

		curDebState = EXPECT_ATTR_VALUE;
		break;

	case EXPECT_ATTR_VALUE:
	/* * * * * *
	 * now we process the value.  we return
	 * the state to which to proceed:  it will
	 * be either EXPECT_ATTR_TYPE, EXPECT_ATTR_VALUE
	 * (if the value is expected to continue) or
	 * EXPECT_TO_FLUSH_DEBUG.
	 */
		curDebState = debug_attr_value(pop);
assert(curDebState==EXPECT_ATTR_TYPE || curDebState==EXPECT_ATTR_VALUE || curDebState==EXPECT_TO_FLUSH_DEBUG);
		break;

	case EXPECT_TO_FLUSH_DEBUG:
	/* * * * * *
	 * flush until we see the line which ends
	 * this debug entry.
	 */
		FlushSeekingLabelGivenAnch = &debug_end_label ;

		curDebState = EXPECT_DEBUG_ENTRY_LENGTH;
		break;

	default:
	/* * * * * *
	 * default - don't advance the state.
	 */
		break;
	}

	/* for cleanliness, restore string */
	*endp = save_char;

	current_datum_p = ( save_char=='\0' || save_char==';'  ?
					0 : endp+1 );
}




/* * * * * *
 * line_data - process data found whilst in the .line section
 * 
 * this function processes the data generated for the .line
 * section. it goes through a series of states, expecting
 * pseudo-ops that define the fields for source line
 * information.  data sent to the .line section looks
 * like the following:
 * 
 * 	(header)	two 'words' of data.
 * 	(entries)	a word, a halfword, and a word.
 * 
 * it is assumed that only one header appears, and therefore
 * can be ignored at the outset.
 * 
 * it is assumed that linptr points at the value(s)
 * associated with this data pseudo-op, and that current_datum_p
 * points at the next one to process.
 * 
 * the latter is updated as each datum is processed.
 * only the line number is read; other data are ignored.
 */
line_data()
{
debug(fprintf(stderr,".line line:%s\n",line);)
	/* * * * * *
	 * this is a straightforward process of dealing
	 * with each data word/halfword sent to this
	 * section, one after another.  they can be defined
	 * in seperate pseudo-ops, or grouped together
	 * in whatever aggregations desired.
	 * 
	 * the initial state of the function below will
	 * ignore the leading header information; we only
	 * expect to see it in the beginning.
	 * 
	 * it is assumed that the process fcn below, advances
	 * the state itself.
	 */
	while( current_datum_p != 0 )
		line_process_datum();

debug(fprintf(stderr,"done with this LINE line.\n");)
}





/* * * * * *
 * line_process_datum - process one datum
 * 
 * this function handles one datum defined by a data
 * pseudo-op, which can define one or potentially a
 * comma-seperated list of data items.
 * if pressed to be accurate, truly this function
 * processes one Argument, which may be many data.
 * 
 * it updates current_datum_p, and sets it to zero
 * when there are no more items in the list.
 * 
 * this function shifts itself to subsequent states, depending
 * on what it sees.
 */
line_process_datum()
{
	char save_char, *endp;


		/* delimit next datum - */
		/*to be cleaned up as debug_data's last act */
	SEEKCHARSorEOS(endp,current_datum_p,",;");

	save_char = *endp;
	*endp = '\0';


	switch(curLineState)	/* interp. datum according to expectation */
	{

	case EXPECT_LINE_HEADER_WD1:
	/* * * * * *
	 * expect first of two words in the header.  Ignore it.
	 */
		curLineState = EXPECT_LINE_HEADER_WD2;
		break;
	case EXPECT_LINE_HEADER_WD2:
	/* * * * * *
	 * expect second of two words in the header.  Ignore it.
	 */
		curLineState = EXPECT_LINE_NUMBER;
		break;

	case EXPECT_LINE_NUMBER:
	/* * * * * *
	 * expect a single numeric value: a line number.
	 * next, we expect a stmt offset, which we'll ignore.
	 * 
	 * if we have decided that the next line we see starts
	 * the next new basic block (i.e. NextLineStartsBblk==TRUE)
	 * then add this source line to the queue of lines
	 * starting basic blocks.
	 * 
	 * NB if we get here and insertcov is true, then we missed
	 * the chance to generate coverage instructions for that
	 * basic block.  this was probably because there was no code
	 * generated for that bblk, or it was optimized away.
	 * Hence, we flag this so that the code will get generated
	 * the very next time we transition to .text, to insure that
	 * the code is associated with this (apparently null) bblk.
	 * 
	 * Indicate that coverage code must be inserted for this
	 * (i.e. the one starting with the current line) new bblk.
	 * 
	 * And.. turn off the flag that triggered this whole mess,
	 * to give us a chance to look for signs of the next one along.
	 */
		/*
		*	Note: strtol is used instead of sscanf
		*	because of a bug in sscanf.  In some versions
		*	of system V, sscanf will corrupt the given
		*	string when called as sscanf(name,"%i",&num);
		*	This has the same effect. 
		*	WFM 2/20/89
		*/
		curnum = strtol(current_datum_p, (char **) NULL, 0);

debug(fprintf(stderr,"recognize line number: %d\n",curnum);)

		if (NextLineStartsBblk) {

			startline_add();

			if (insertcov)
				insertCovSinceWasOmitted = TRUE;

			insertcov = TRUE;

			NEXTLINESTARTSBBLK_NO;
		}

		curLineState = EXPECT_STMTOFF;
		break;

	case EXPECT_STMTOFF:
	/* * * * * *
	 * expect stmt offset.  Ignore it.
	 */
		curLineState = EXPECT_LINE_ENTRY_ADDR;
		break;

	case EXPECT_LINE_ENTRY_ADDR:
	/* * * * * *
	 * expect line number entry address.  Ignore it.
	 * 
	 * next, go back to expecting a new entry!!
	 */
		curLineState = EXPECT_LINE_NUMBER;
		break;

	default:
	/* * * * * *
	 * default - don't advance the state.
	 */
		break;
	}

	/* for cleanliness, restore string */
	*endp = save_char;

	current_datum_p = ( save_char=='\0' || save_char==';'  ?
					0 : endp+1 );
}



	/* define # of entries in table below. */
#define NUMPOPS	(sizeof(popcode)/sizeof(int))

int
plookup(s)	/* look up pseudo op code */
char *s;
{
/* Note:  to improve the linear search speed, these pseudo-ops
** are ordered by frequency of occurrence in a sample of C programs.
*/
/* * * * * *
 * 
 * these have been RE-ordered for ELF, using statistics collected from
 * a substantial (333 C source line, 1343 .s source line) program.
 *
 *	 353 .uahalf	4 .word	
 *	 337 .uaword	3 .text	
 *	 146 .section	2 .ident
 *	 145 .previous	1 .version	
 *	 117 .byte	1 .file	
 *	  24 .set	1 .data	
 *	  13 .align	1 .comm	
 *	  11 .globl	
 * 
 * 10-25-88 rjp
 */

	static char *pops[] =
		{
			".2byte", ".4byte",
			".section", ".previous", ".byte",
			".set", ".align", ".globl",
			".word", ".text",

			".half", ".string",
			".data", ".bss", ".file", ".tv",
			".pushsection", ".popsection",
			".reserve", ".seg",
			/* these2 included as reqts settled down */
			".uaword",  ".uahalf"
		};

	static enum Pseudo_op_values popcode[] =
		{
			UA2BYTE, UA4BYTE,
			SECTION, PREVIOUS, BYTE,
			SET, ALIGN, GLOBL,
			WORD, TEXT,

			HALF, STRING,
			DATA, BSS, FIL, TV,
			PUSHSECTN, POPSECTN,
			RESERVE, SECTION,

			UA4BYTE, UA2BYTE
		};

	register enum Pseudo_op_values i;

assert((sizeof(popcode)/sizeof(int)) == (sizeof(pops)/sizeof(char *)));

	for (i = 0; i < NUMPOPS; i++)
		if (!strcmp(s, pops[i]))
			return(popcode[i]);
	return(POTHER);
}


/* This function will redirect a file to another existing file. */
void
CAopen(name, dir, file)
char *name;
char *dir;
FILE *file;
{
	extern void exit();		/* standard exit function */
	extern FILE *freopen();		/* standard file function */

		/* sanity check on the input */
	if (name == NULL) {
		fprintf(stderr,"ERROR - CAopen(): %s\n", err_msg[SR107]);
		exit(FATAL);
	}

		/* sanity check on the input file name */
    /* this is not checked for in VAX version
	if (*name == NULL) {
		fprintf(stderr,"ERROR - CAopen(): %s\n", err_msg[SR110]);
		exit(FATAL);
	}
    */

		/* sanity check on the input */
	if (dir == NULL) {
		fprintf(stderr,"ERROR - CAopen(): %s\n", err_msg[SR107]);
		exit(FATAL);
	}

		/* sanity check on the input */
	if (file == NULL) {
		fprintf(stderr,"ERROR - CAopen(): %s\n", err_msg[SR107]);
		exit(FATAL);
	}

		/* checks for failure of the redirection of a file */
	if (freopen(name, dir, file) == NULL) {
		fprintf(stderr,"ERROR - CAopen(): %s %s\n", err_msg[SR106], name);
		exit(FATAL);
	}
}




/* * * * * *
 * genCovInstrs - generate coverage instruction, and 
 * condition code save/restore, as directed.
 * 
 * this routine outputs an instruction which will
 * bump the corresponding counter in the execution
 * counts' (i.e. coverage) array.
 * 
 * NB some instructions depend on the value of
 * the condition code - e.g. branch on zero.
 * Though normal code generation only uses the
 * knowledge of the condition code within a single
 * statement, and hence safely within a basicblk,
 * optimization will sometimes (intelligently!)
 * capitalize on the redundent setting of the CC,
 * and make code depend on its value across bblks.
 * 
 * Hence, under some circumstances we generate
 * 'bracketing code' to save and restore the CC
 * before and after the execution of our coverage
 * instruction (because otherwise we might change
 * the semantics of the program!!).  Those circumstances
 * are determined by the caller, and the caller's
 * wishes are reflected in the argument.
 */

genCovInstrs(saveCCfirst)
BOOL saveCCfirst;
{

debug(fprintf(stderr,"genCov: saveCC=%c, insertcov=%c, WasOmitted=%c\n",
		(saveCCfirst ?'T':'F'),
		(insertcov ?'T':'F'),
		(insertCovSinceWasOmitted ?'T':'F'));    )

	if ( saveCCfirst ) {
			/* insert the savecc instructions */
#if MACH_IS_m32
		fprintf(stderr,"\n   >>> BASICBLK WARNING - not profiling function %s [trouble at line %ld]\n", function_name, curnum);
		cc_trouble = TRUE;
		/*exit(FATAL);*/
#else
		if (fprintf(stdout,_SAVECC) <= PNT_FAIL) {
			fprintf(stderr,"ERROR - yylex(): %s\n", err_msg[SR109]);
			exit(FATAL);
		}
#endif
	}

#if MACH_IS_m32
  if (cc_trouble == FALSE) {
#endif
		/* insert the coverage instruction */
	if (fprintf(stdout,_LCOVERAGE,fcnt,bkcnt*sizeof(long)-sizeof(long)) <= PNT_FAIL) {
		fprintf(stderr,"ERROR - yylex(): %s\n", err_msg[SR109]);
		exit(FATAL);
	}

	if ( saveCCfirst ) {
		    /* insert the restorecc instruction */
		if (fprintf(stdout,_RESTORECC) <= PNT_FAIL) {
			fprintf(stderr,"ERROR - yylex(): %s\n", err_msg[SR109]);
			exit(FATAL);
		}
	}
#if MACH_IS_m32
  }
#endif

}



void
opcode()
{
    char *opinstr;
    char savec;

    opinstr = linptr;	/* note beginning of mnemonic */

    FIND_WHITE_SPACE_AT(linptr);
			/* skip over instruction mnemonic */

    savec = *linptr;
    *linptr = '\0';	/* demarcate with null */

	/* checks if coverage instruction needs to be inserted */
    if (insertcov == TRUE) {
	/* checks if condition codes need to be saved */
	/* cond codes need saving if next instr is:
		(1)  an instr that checks some cond code (e.g. jge)
		(2)  an instr that does not affect cond codes already
		     set (i.e. cond codes unchanged, so maybe next
		     instr relies on cond codes set)  (e.g. jmp)
		     (jmp is bad example, since it unconditionally
		     alters flow of control)
	   Initially, these instr are the same as in table used
	   by CAinstr, except for 'call' instr
	*/

	usecc = (  (strncmp(opinstr, "jmp", 3) != 0)   &&
	           (strncmp(opinstr, "call", 4) != 0)  &&
		   (CAinstr(opinstr) == PASS)            ) ;

	/* * * * * *
	 * go generate the instruction, plus instrs to save
	 * the cc, if any of the above conditions apply.
	 */
	genCovInstrs( usecc );

	insertcov = FALSE;
    }


	/* * * * * *
	 * next, we see if this opcode defines the end
	 * of a logical block.  if so, then indicate that
	 * the code associated with the next source line
	 * entry (i.e. .line section entry) starts the
	 * next basic block.
	 */
	if ( EXPECT_START_LABEL || EXPECT_END_LABEL ) {

			/* checks the opcode instruction */
		if (CAinstr(opinstr) == PASS)
			NEXTLINESTARTSBBLK_YES;
	}

	/* put back character in parse string */
	*linptr = savec;
}




/* * * * * *
 * fcn called when leading character in line is whitespace -
 * the most common case.
 */

void
leading_white_space()
{
	SKIP_WHITE_SPACE_AT(linptr);	/* skip white space */

	switch(*linptr)			/* dispatch on next character */
	{
	case '\0':			/* line of white space */
	case '#':			/* comment */
#if MACH_IS_i386
	case '/':                   	/* comment */
#endif
	case '\n':			/* new line (shouldn't see This)*/
		break;			/* ignore */

	case '.':			/* start of pseudo-op */
		pseudo();		/* do pseudo-ops */
		break;

	default:			/* normal instruction */
		if (IN_TEXT)		/* if in text section (normal case) */
		    opcode();		/* deal with as an opcode */
		break;
	}   /* end space/tab case */

}


/* * * * * *
 * handle_label - handle a line starting with misc. char...
 * Called when leading char is either dot, or not special -
 * hence this line defines a label.
 * 
 * a label means one of several things to us:
 * 
 *  1.	it's the 'end of function' label... if it matches
 * 	the recorded name.
 * 
 *  2.	it's an 'ignorable label' - it means nothing to us.
 * 
 *  3.	it's a function name... only if it starts with a char
 * 	other than '.'.  NB this also defines a new bblk,
 * 	as in (4).
 * 
 *  4.	it notes the beginning of a basic block.
 * 
 * 
 * NB this is only called when we're in the .text section.
 */

void
handle_label()
{

	/* * * * * *
	 * case  1 *
	 * * * * * *
	 * see if this is The 'end of function' label.  If so,
	 * we can signal that that happy state is the one
	 * in which we find ourselves!!
	 * 
	 * when find the end of the function, we can...
	 *  - allocate the coverage structure, as apropos.
	 *  - turn the 'anticipate new bblk next' flag safely Off.
	 */
	
	if ( function_end_label!=0 && strcmp(linptr,function_end_label)==0 )
	{
		NOTE_END_LABEL;
debug(fprintf(stderr,"saw function end label!\n");)
		if (bkcnt>0)
			alloc_array();
		NEXTLINESTARTSBBLK_NO;
#if MACH_IS_m32
		cc_trouble = FALSE;
#endif
	}


	else 

	/* * * * * *
	 * case  2 *
	 * * * * * *
	 * check if this label starts with a distinguished
	 * char sequence.  if it does, then it does not signify
	 * the beginning of a basic block, since by convention
	 * labels with this prefix are never the target of
	 * a branch instruction (compiler, optimizer convention).
	 * 
	 * also, we must be processing the body of a function -
	 * that is, we must be looking for the end label, or
	 * perhaps the start label..
	 * 
	 * under these circumstances, a label might serve as an entry
	 * point; hence we shall define a new basic block to start
	 * here, in response.
	 */

	if ( strncmp(NONBRANCH_PREFIX, linptr, strlen(NONBRANCH_PREFIX))==0 )
	{ /* do nothing */ }


	else

	/* * * * * *
	 * case  3 *
	 * * * * * *
	 * if this is a function label, then we need
	 * to initialize for a new function.
	 * 
	 * these include: setting the expectation global state;
	 * zeroing the #of bblks found so far; incrementing the
	 * number of functions; and setting the 'anticipate that
	 * the next .line entry starts a new basic block' flag,
	 * to TRUE.
	 */

	if ( *linptr != '.' )
	{
		NOTE_START_LABEL ;		/* change state: saw fcn label */
debug(fprintf(stderr,"saw fcn Start label!\n");)

		    /* initializes flags and counters for new fcn */
		bkcnt = 0;			/* logical block counter */
		fcnt++;				/* function counter */

		if (strchr(linptr,':') == NULL) { /* error if not found */
			error_alert(LVL_ERROR, WHO_YYLEX, "Bad input format");
		}

			/* save function name */
		if (function_name !=NULL) {
debug(fprintf(stderr,"before free(function_name).\n");)
			free(function_name);
debug(fprintf(stderr,"after free(function_name).\n");)
		}
debug(fprintf(stderr,"calling strdup(linptr).\n");)
		function_name = (char *) strdup(linptr);
debug(fprintf(
	stderr,
	"function_name = %s!\n",
	(function_name ? function_name : "<null>")
);)
		function_name[strlen(function_name)-1] = '\0';  /* get rid of ':' */

		NEXTLINESTARTSBBLK_YES;
	}


	else

	/* * * * * *
	 * case  4 *
	 * * * * * *
	 * this is some compiler label that may well
	 * be an entry point.  cause the next .line
	 * entry to define a new basic block.
	 */
	NEXTLINESTARTSBBLK_YES;
}




/* * * * * *
 * yylex() - the main parsing routine.
 * 
 * this routine is responsible for iteratively reading lines
 * and processing them, scanning for labels and skipping over
 * intermediate code when that is appropriate.
 */


void
yylex()
{
    extern int CAinstr();		/* instruction lookup function */


    /* * * * * *
     * the loop below has the following basic outline:
     * 
     * 	while inputOk
     * 		{
     * 		IF  to scan for a label...
     * 			scan UNTIL you find that label!
     * 
     * 		Parse a line of assembler code.
     * 		}
     * 
     * NB that the scanning process allows the line with
     * the label, to be processed by the subsequent 'parse' code.
     * 
     * NB also that the scanning logic requires that a string be
     * malloc-ed/strdup-ed, and that it be allowed & indeed expected
     * to free it when it is done.  The flag it uses doubles as
     * a pointer to an anchor which in turn points to the string
     * which represents the label to find: "<labelname>:", sans NL.
     */

    linptr = NULL;			/* start off with no line */
    while (linptr != NULL || (linptr = lineLoad()) != NULL)
    {
debug(	linct++;)			/* bump #lines read. */

	/* * * * * *
	 * 
	 * 	IF  to scan for a label...
	 * 		scan UNTIL you find that label!
	 * 
	 */

	if ( FlushSeekingLabelGivenAnch != 0 )
	{
debug(fprintf(stderr,"seeking for label: %s\n", *FlushSeekingLabelGivenAnch);)
		while (strcmp(*FlushSeekingLabelGivenAnch,line)!=0  )
		{
			printf("%s\n",line);	/* echo non-target line */

    			linptr = lineLoad();
		debug(	linct++;)		/* bump #lines read. */

			if (linptr == NULL)	/* stop on EOF */
				break;
		}

			/* free reference string and zero the anchor */
		free(*FlushSeekingLabelGivenAnch);
		*FlushSeekingLabelGivenAnch = 0;
		 FlushSeekingLabelGivenAnch = 0;
	};


	/* * * * * *
	 * 
	 *	Parse a line of assembler code.
	 * 
	 */
	if (linptr)			/* if there's a line to process.. */
	{
debug(fprintf(stderr,"%3d>>\t%s\n",linct,linptr);)

		switch (*linptr)		/* dispatch on first char in line */
		{
		case '\0':			/* blank lines get chomped down */
		case '\n':			/* ignore new line (shouldn't see) */
		case '#':			/* check & ignore special comments */
#if MACH_IS_i386
		case '/':                       /* Comment line could start with / */
#endif
		    break;			/* ignore line */

		case ' ':			/* usual case, begins w/space|tab */
		case '\t':
			leading_white_space();
			break;

		case '.':			/* compiler label */
		default:			/* function label */
		/* * * * * *
		 * we only care about labels in the .text section,
		 * at this level.
		 */
			if (IN_TEXT)
				handle_label();	/* handle a bblk-related label */
			break;

		}	/* end first character switch */

		/* * * * * *
		 * if there is more assembler code, deal with it...
		 * 	..unless we're to push ahead, flushing
		 * 	until we see a label.
		 * NB we first check to see that there is a string,
		 *    at linptr, and that it has chars.
		 * 
		 * NB we assume no (more) comments nor labels.
		 * NB we skip the ; before processing the code.
		 */

		if (linptr && *linptr)
		while( 	     FlushSeekingLabelGivenAnch==0       &&
			( (linptr=strchr(linptr, ';')) != NULL ) )
		{
debug(fprintf(stderr,"semicolon seen -->%s<--\n",linptr);)
			if(*++linptr)
				leading_white_space();
		}

		printf("%s\n",line);		/* echo the line! */
		linptr = NULL;			/* indicate we're done with line */
	}/* end parse-assembler-line */

    }   /* end while */

    /* Print release string for version stamping at end of object file */
    if (qflag)
    	printf ("\t.ident\t\"basicblk: %s\"\n",CPPT_REL);


    /* * * * * *
     * free up dynamic structures.
     */
    sectn_change(ACTN_NUKE, SCN_OTHER);
    startline_free();
    if( line !=NULL )
	free(line);


	/* free fcn name */
    if( function_name !=NULL )
	free(function_name);
	

    return;				/* now just exit */
}



short qflag = 0;

int
main(argc, argv)
int argc;
char *argv[];
{
	extern void CAopen();		/* redirects a file */
	extern void exit();		/* standard exit function */
	int	in = 0, out = 0;	/* stdin, stdout assigned */
	int	flagc;			/* Argument count for command line parsing */
	char	**flagv;		/* Argument array */
	/* Note:  the -x option code is not used.  x is assigned to be l */

		/*
		* arguments are:   basicblk [-Q] [-x|l] [in] [out]
		*/
					/* Read past argv[0] */
	for (flagc = --argc, flagv = ++argv; flagc > 0; --flagc, ++flagv) {
		if (**flagv ==  '-') {
			switch (*++*flagv){
			    case 'l':		/* Both are -l option */
			    case 'x':		/* Default is -l option */
				break;
			    case 'Q':		/* Version stamping */
				switch( (*flagv)[2] ) {
				    case 'n':
				    case 'N':
					qflag = 0;	/* -Qn -QN */
					break;
				    default:
					qflag = 1;	/* -Q -Q* */
					break;
				}
				break;
			    default:
				fprintf (stderr,
				    "%sbasicblk: unknown option \"%c\"\n",
				    SGS, **flagv);
					
				fprintf(stderr,"%s\n", err_msg[SR111]);
				exit(FATAL);
				break;
			}
		}
		else {
			if (!in) {
			    CAopen(*flagv, "r", stdin);  /* redirect stdin to file */
			    in++;
			}
			else {
			    if (!out) {
			    	CAopen(*flagv, "w", stdout); /* redirect stdout to file */
				out++;
			    }
			    else {	/* Too many arguments on command line */
				fprintf(stderr,"basicblk: %s\n", err_msg[SR100]);
				fprintf(stderr,"%s\n", err_msg[SR111]);
				exit(FATAL);
			    }
			}
		}
	}
				
	if (!in){
		fprintf(stderr,"WARNING - %sbasicblk: %s\n", SGS, err_msg[SR101]);
	}
	else
	if (!out){
		fprintf(stderr,"WARNING - %sbasicblk: %s\n", SGS, err_msg[SR102]);
	}

	yylex();			/* call to the lexical scan */
	exit(0);
}
