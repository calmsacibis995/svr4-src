/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)optim:i386/inline.c	1.11"
#include <sys/types.h>
#include "optim.h"
#include "optutil.h"
#include "paths.h"
#include <malloc.h>

FILE *outfp=stdout; /* pointer for output file after in-line subst, we
		       need to know where to write debugging info
		       even if we don't do inline expansions */

#ifdef IMPIL

/* in-line substitution optimization */

#define FN_OVERHD	2	/* #instructions in prologue and epilogue */
#define	MAXINSTR	20+FN_OVERHD	/* max #instrs in function candidates */
#define ILDEFAULT	20	/* default maximum percent in-line expansion
				 * per file */

static struct procnode {
	struct procnode	*pnforw;	/* forward pointer for list of procs */
	struct procnode *pnback;	/* backward pointer for list of procs */
	char		*pnname;	/* procedure name */
	int		pncalls;	/* num of times procedure is called */
	int 		pnni; 		/* num of instr in procedure */
	struct node	*pnhead;	/* proc head pointer */
	struct node	*pntail;	/* proc tail pointer */
} prochead, proctail;
static int totalni = 0;	/* total number of instructions in file */
#define PNODE		struct procnode
#define	ALLPN( prn )	prn = prochead.pnforw; prn != &proctail; prn=prn->pnforw
#define ALLPNB( prn )	prn = proctail.pnback; prn != &prochead; prn=prn->pnback
#define PNALLN(prn,pn)  pn = prn->pnhead->forw; pn != prn->pntail; pn=pn->forw
#define PNALLNB(prn,pn) pn = prn->pntail->back; pn != prn->pnhead; pn=pn->back
static char itmpname[50]; /* name of file for output before in-line subst */
static int outfd;	/* descriptor for output file after in-line subst */
static char linebuf[LINELEN];
static int intpc = -3;	/* percent limit on in-line expansion */
extern boolean zflag;	/* debug flag for in-line expansion */
extern boolean swflag;
extern int asmflag; /* flag indicating presence of asm in function */

/* function declarations */
static void iledit();
static void ilinsert();
static struct procnode *ilalloc();
extern char *tempnam();
extern int dup();
extern long strtol();
extern int unlink();

/* initialization for in-line substitution */
void
ilinit()
{
	/* initialize procedure list */
	prochead.pnforw = &proctail;
	prochead.pnback = NULL;
	proctail.pnforw = NULL;
	proctail.pnback = &prochead;

	/* default the in-line expansion limit if not set */
	if( intpc == -3 )
		switch( optmode ) {
		case OSPEED: intpc = -1; break;
		case OSIZE: intpc = 0; break;
		default: intpc = ILDEFAULT;
		}

	/* redirect stdout to tempfile to hold output before in-line subst */
	strcpy( itmpname, tempnam( TMPDIR, "ccil" ) );
	if( ( outfd = dup( 1 ) ) == -1 ) 
		fatal( "can't get file descriptor\n");
	outfp = fdopen( outfd, "w" );
	if( freopen( itmpname, "w", stdout ) == NULL )
		fatal( "can't open file %s\n", itmpname );
}
/* routine to decode the limit on percent in-line expansion 
 * from option -yu (unlimited), -ys (suppress), and -ynum (where
 * num is the max percent file growth due to in-line expansion.
 * Note that MAXINSTR instruction limit is always in effect. 
 */
	void
pcdecode( flags )
char *flags; /* pointer to first character of suboption for 'y' */
{
	char *p;
	extern void fatal();
	switch( *flags ) {
	case 'u': intpc = -1; break;	/* unlimited growth */
	case 's': intpc = -2; break;	/* suppress inline */
	case '\n':
		fatal("in-line (-y) option missing\n");
		break;
	default:			/* percent growth specified */
		intpc = ( int ) strtol( p = flags, &flags, 10 );
		flags--;
		if( flags < p ) fatal("invalid in-line (-y) option\n");
		break;
	}
	return;
}
/* routine to mark calls with bytes pushed on stack at time of call */

/* The purpose of this routine is to permit nested calls to be expanded
** in-line.  This routine does static analysis of the stack
** size and should, therefore, precede the branch optimization. */

void
ilmark()

{
	register int npush = 0;		/* number of bytes of
				   arguments pushed or moved onto the stack */
	register NODE *pn;		/* pointer to instruction node being
				   processed */
	
	/* check whether in-line expansion is being suppressed */
	if( intpc == -2 ) return;

	/* mark calls */
	for( ALLN( pn ) ) {
		switch(pn->op) {
		case PUSHW:	/* does compiler use this in calling sequences? */
			npush += 2;
			break;
		case PUSHL:
			if ( pn->op2 && strncmp(pn->op2,"TMPSRET",7) == 0) {
				/* Address of temp for struct function */
				/* being pushed - don't count it. */
				pn->op2 = NULL;
				break;
			}
			npush += 4;
			break;
		case POPW:	/* does compiler use this in calling sequences? */
			npush -= 2;
			break;
		case POPL:
			if ( usesvar("%eax",pn->op1) &&
			     pn->forw->op == XCHGL &&
			     usesvar("%eax",pn->forw->op1) &&
			     strcmp("0(%esp)",pn->forw->op2) == 0)
				/* Address of temp for struct function */
				/* being popped - don't count it. */
				break;
			npush -= 4;
			break;
		case ADDL:
			if( strncmp( pn->op2, "%esp", 4 ) != 0 ) break;
			if( pn->op1[0] != '$' ) break;
			npush -= atoi( pn->op1 + 1 );
			break;
		case SUBL:
			if( strncmp( pn->op2, "%esp", 4 ) != 0 ) break;
			if( pn->op1[0] != '$' ) break;
			npush += atoi( pn->op1 + 1 );
			break;
		case CALL:
			pn->opm = (char * ) npush;
			break;
		}
	}
}

/* gather statistics for in-line substitution:  number of instructions in a
 * function and  number of calls made within this file to a given function.
 * Eliminate as candidates functions that have locals, save registers, asms,
 * fancy label expressions as instruction operands, and switchs.
 */
void
ilstat( numnreg, numauto )
int numnreg;	/* number of registers to save and restore */
int numauto;	/* number of words of autos */
{
	register NODE *pn, *p;
	PNODE *prn, *pp;
	int nn, ni, iop, nc;
	char *ncp;

	/* check whether in-line expansion is being suppressed */
	if( intpc == -2 ) return;

	/* count CALL's and total number of instructions */
	for( ALLN( pn ) ) {
		if( isprof( pn ) ) 
			totalni -= 2;
		if( pn->op == CALL ) {
			if( ( prn = ilalloc( pn->op1 ) ) == NULL ) return;
			prn->pncalls++;
		}
		if( !islabel( pn ) && pn->op != MISC ) {
			totalni++;
			/* printf( "%d %s\n", totalni, pn->ops[0] ); */
		}
	}

	/* allocate a node for this proc if one does not exist */
	for( ALLN( pn ) ) {
		/* first label has the function name */
		if( islabel( pn ) && !is_debug_label(pn) ) {
			if( ( prn = ilalloc( pn->opcode ) ) == NULL ) return;
			break;
		}
	}
	if( pn == 0 ) return;	/* fell out of loop w/o finding function name */

	/* check conditions for being substituted in line */
	if( numnreg != 0 || numauto != 0 ) return;
		/* eliminate functions with locals or saved registers */
	if( asmflag ) return;
		/* eliminate functions with asms */

	/* check procedure size and compute total length of strings */
	nn = ni = nc = 0;		/* nn = #nodes in function */
					/* ni = #instrs in function */		
					/* nc = sum of length of all operands */		
	for( ALLN( pn ) ) {
		register char *cp;
		/* int n = (pn->op == CALL ? MAXOPS - 1: MAXOPS); */
		nn++;
		for( iop = 0; iop <= MAXOPS; iop++ ) {
			if( (cp = pn->ops[iop] ) != NULL ) {
				nc += strlen(cp)+1;
			}
			/* else break; */
		}
		if( isprof( pn ) ) 
			ni -= 2;
		if( !islabel( pn ) && pn->op != MISC ) {
			if( ++ni > MAXINSTR ) return;
		}
	}

	/* eliminate functions with fancy label expressions */
	for( ALLN( pn ) ) {	/* scan for labels */
	  if( !islabel( pn ) ) continue;
	  for( ALLN( p ) ) {	/* scan for operands containing label */
	    register int i;
	    for( i = 0; i <= MAXOPS; i++ ) {
	      register char *cp;
	      cp = p->ops[i];
	      if( cp == NULL ) continue;
	      if( *cp == *( pn->opcode )/*speed*/
		 && strcmp(cp, pn->opcode ) == 0) 
		continue;	/* operand completely matches label - ok*/
	      while( *cp != '\0' ) {
		if( *cp == *( pn->opcode )/* speed */
		   && strncmp(cp, pn->opcode, strlen(pn->opcode))==0
		   && !isalnum(cp[(unsigned)strlen(pn->opcode)])
		   )
		  return;	/* operand is expr containing label (like */
				/* "*(.L1+4)") - throw out function */
		cp++;
	      }
	    }
	  }
	}

	/* eliminate functions with switch tables */
	if( swflag ) { swflag = false; return; }

	/* save the code */
	prn->pnni = ni;
	/* allocate space for nodes and operand strings */
	if( ( p = (NODE *)malloc( ( nn + 2 ) * sizeof( NODE ) + nc ) ) == NULL )
		return;
	ncp = (char *) p + ( nn + 2 ) * sizeof( NODE );
	prn->pnhead = p;
	p->forw = p + 1;
	p->back = NULL;
		/*
                ** Copy nodes and operand strings to new list, throwing
		** away debugging information labels.
		*/
	for( ALLN( pn ) ) {
		register int i;

#define IS_DEBUG_LABEL(x) ( (x) && (x)[0]=='.' && (x)[1]=='.' && !isdigit((x)[2]))

		if(islabel(pn) && IS_DEBUG_LABEL(pn->ops[0]))
                        continue;
		p++;
		p->forw = p + 1;
		p->back = p - 1;
		for( i = 0; i <= MAXOPS; i++ ) {
			if( ( p->ops[i] = pn->ops[i] ) == NULL ) continue;
			strcpy( ncp, pn->ops[i] );
			p->ops[i] = ncp;
			ncp += strlength( ncp );
		}
		p->op = pn->op;
		p->userdata = pn->userdata;
	}
	p++;	
	p->forw = NULL;
	p->back = p - 1;
	prn->pntail = p;

	/* reposition in list by increasing size */
	/* remove from list */
	prn->pnforw->pnback = prn->pnback;
	prn->pnback->pnforw = prn->pnforw;
	/* locate proper position */
	for( ALLPNB( pp ) ) {
		if( prn->pnni >= pp->pnni ) break;
	}
	/* reinsert it */
	prn->pnforw = pp->pnforw;
	prn->pnback = pp;
	prn->pnforw->pnback = prn;
	prn->pnback->pnforw = prn;
	return;
}

/* perform the in-line substitution for whole file */
void
ilfile()
{
	register struct procnode *prn;
	int fpc, delta;
	register char *linptr;
	long start_enter_leave, stop_enter_leave, curr;
	extern void get_enter_leave();

	/* eliminate nodes for progs not in the file or not called */
	for( ALLPN( prn  ) ) {
		if( prn->pnni == 0 || prn->pncalls ==0 ) {
			prn->pnforw->pnback = prn->pnback;
			prn->pnback->pnforw = prn->pnforw;
		}
	}

	/* apply percent limit unless no limit */
	if( intpc != -1 ) {
		/* apply percent text increase constraint */
		fpc = 100 * intpc;
		for( ALLPN( prn ) ) {
			delta = 100 * ( prn->pnni - FN_OVERHD ) * 100 / totalni;
			fpc -= prn->pncalls * delta;
			if( fpc < 0 ) {
				while( fpc < 0 && prn->pncalls > 0 ) { 
					fpc += delta;
					prn->pncalls--;
				}
				break;
			}
		}
		/* eliminate candiates after cut-off point */
		if( prn != &proctail ) {
			if( ( prn->pncalls ) == 0 ) prn = prn -> pnback;
			prn->pnforw = &proctail;
			prn->pnforw->pnback = prn;
		}
	}

	/* analytic printout */
	if( zflag ) {
		int size, sumsize;
		sumsize = 0;
		if( zflag ) fprintf(stderr, "in-line expansion limit = %d\n",
			intpc );
		for( ALLPN( prn ) ) {
			size = prn->pncalls * 
				( prn->pnni -2 ) * 100 / totalni;
			sumsize += size;
			fprintf( stderr,
				"%s calls=%d inst=%d sz=%d%% t_sz=%d%% \n",
				prn->pnname,prn->pncalls,prn->pnni,size,
				sumsize );
		}
	}

	/* edit routines before inserting */
	iledit();

	/* read temp file and write output with inserted procedures */
	fclose( stdout );
	if( freopen( itmpname, "r", stdin ) == NULL )
		fatal( "can't open file %s\n", itmpname );
	get_enter_leave(&start_enter_leave,&stop_enter_leave);
	while( curr=ftell(stdin),( linptr=fgets( linebuf, LINELEN, stdin ) ) != NULL ){
		register int numauto, nargs;
		register char *cp;
		char *linp;

		if (curr > stop_enter_leave)
			get_enter_leave(&start_enter_leave,&stop_enter_leave);
		

		if( *linptr == '!' ) continue;	/* for asm handling? */
		/* default: just print out everything else except '@' lines */
		if( *linptr != '@' ) {
			fprintf( outfp, "%s", linptr );
			continue;
		}
		/* expand the call */
		/* '@' lines come before the calls, providing the name of */
		/* function and the number of bytes of argument */
		linp = linptr;
		numauto = (int) strtol( ++linp, &linp, 10 );
		/* nargs will always be 0 because the subl isn't removed */
		nargs = 0;
		linptr = linp;
		cp = ++linptr;
		while( *cp != '\n' ) cp++;
		*cp = '\0';
		if (curr < start_enter_leave) for( ALLPNB( prn ) ) {
			if( strcmp( linptr, prn->pnname ) == 0 ) {
				if( prn->pncalls <= 0 ) break;
				ilinsert( numauto, nargs, prn );
				prn->pncalls--;
				/* read and discard the call line */
				fgets( linebuf, LINELEN, stdin );
				break;
			}
		}
	}
	(void) fclose( stdin );
	unlink( itmpname );
}



/* edit routines before substitution */
static void
iledit()
{
	register NODE *pn, *qn;
	register PNODE *prn;

	for( ALLPN( prn ) ) {
		/* replace entry label with new label */
		for( PNALLN( prn, pn ) ) {
			if( islabel( pn ) ) {
				prn->pnhead->forw = pn;
				pn->back = prn->pnhead;
				pn->opcode = ":EL";
				break;
			}
		}
		/* replace leave/ret with lab or follow last jump with lab */
		for( PNALLNB( prn, pn ) ) {
			if( pn->op == LEAVE ) {
				pn->op = LABEL;
				pn->opcode = ":RL";
				prn->pntail->back = pn;
				pn->forw = prn->pntail;
				break;
			}
			if( pn->op == JMP ) {
				addi(pn, LABEL, ":RL",NULL,NULL );
				prn->pntail->back = pn;
				pn->forw = prn->pntail;
				break;
			}
		}
		/* replace other returns with jump to new label
		 * and delete profiling code */
		for( PNALLN( prn, pn ) ) {
			if( pn->op == RET ) {
				pn->op = JMP;
				pn->opcode = "jmp";
				pn->op1 = ":RL";
			}
			if( isprof( pn->forw ) ) {
				qn = pn->forw;
				DELNODE( qn );
				qn = pn->forw;
				DELNODE( qn );
			}
			if ( pn->op == LEAVE ) {
				DELNODE( pn );
			}
			if ( pn->op == PUSHL && strcmp(pn->op1,"%ebp") == 0 ) {
				DELNODE( pn );
			}
			if ( pn->op == MOVL &&
			     strcmp(pn->op1,"%esp") == 0 &&
			     strcmp(pn->op2,"%ebp") == 0 ) {
				DELNODE( pn );
			}
			if (pn->op == POPL &&
			    usesvar("%eax",pn->op1) &&
			    pn->forw->op == XCHGL &&
			    usesvar("%eax",pn->forw->op1) &&
			    strcmp("0(%esp)",pn->forw->op2) == 0) {
				qn = pn->forw;
				DELNODE( qn );
			}
		}
	}
}


/* insert a procedure */
static void
ilinsert( numauto, nargs, prn )
int numauto;	/* numbers of words of autos in calling procedure */
int nargs;	/* number of arguments procedure is called with */
PNODE *prn;	/* procedure to be inserted */
{
	register NODE *pn, *p;
	static int ilabel = 0;
	register int i;
	register char *cp, *cpn;
	char *lab;

	/* rewrite labels */
	for( PNALLN( prn, pn ) ) {
		if( !islabel( pn ) ) continue;
		cpn = pn->opcode;
		ilabel++;
		lab = getspace( 9 );
		sprintf( lab, ".I%d\0", ilabel );
		for( PNALLN( prn, p ) ) {
			for( i = 0; i <= MAXOPS; i++ ) {
				cp = p->ops[i];	
				if( cp == NULL ) continue;
				if( *cp == *cpn &&
					strcmp( cp, cpn ) == 0 )
					p->ops[i] = lab;
			}
		}
	}

	/* write out instructions, rewriting args and omitting save */
	for( PNALLN( prn, pn ) ) {
		switch( pn->op ) {
		case LABEL:
		case HLABEL:
		case DHLABEL:
			fprintf( outfp, "%s:\n", pn->opcode );
			break;
		case MISC:
			fprintf( outfp, "	%s\n", pn->opcode );
			break;
		case CALL:
			pn->opm = NULL;
			/* fall through */
		default:
			fprintf( outfp, "	%s	", pn->opcode );
			for( i = 1; i < MAXOPS + 1; i++ ) {
				if( (cp=pn->ops[i]) == NULL ) continue;
				if( i > 1 ) fprintf( outfp, "," );
				if( usesreg( cp, "%ebp" ) ) {
					char *tempptr;
					if( *cp == '*') {
						fprintf( outfp, "*" );
						cp++;
					}
					fprintf( outfp, "%ld",
						-(INTSIZE+numauto)
						+strtol(cp,&tempptr,0) );
					fprintf( outfp, "%s", tempptr);
				}
				else fprintf( outfp, "%s", cp );
			}
			fprintf( outfp, "\n" );
			break;
		}
	}

	/* reset stack pointer if necessary */
	if( nargs != 0 ) 
		fprintf( outfp, "	subl	$%d,%%esp\n", 4 * nargs );
}

/* allocate procedure nodes */
static struct procnode *
ilalloc( name )
char	*name;	/* procedure name */
{
	register PNODE *prn;

	/* check whether node already exists */
	for( ALLPN( prn ) ) {
		if( strcmp( name, prn->pnname ) == 0 ) 
			return( prn );
	}

	/* allocate node */
	if( ( prn = ( PNODE * ) 
		malloc( sizeof( PNODE ) + strlength( name ) ) ) == NULL ) 
		return( prn );
	prn->pnforw = prochead.pnforw;	
	prn->pnback = &prochead;
	prn->pnforw->pnback = prn;
	prn->pnback->pnforw = prn;
	prn->pnname = (char *) prn + sizeof( PNODE );
	strcpy( prn->pnname, name );
	prn->pncalls = 0;
	prn->pnni = 0;
	prn->pnhead = NULL;
	return( prn );
}
#endif /* IMPIL */
