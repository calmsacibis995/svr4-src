/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)yacc:y3.c	1.8"
# include "dextern"

static void go2gen();
static void precftn();
static void wract();
static void wrstate();
static void wdef();

	/* important local variables */
static int lastred;  			/* number of the last reduction of a state */
int *defact;
extern int *toklev;
extern int cwp;

/* print the output for the states */
void output()
{
	int i, k, c;
	register WSET *u, *v;

	(void) fprintf( ftable, "yytabelem yyexca[] ={\n" );

	SLOOP(i) { /* output the stuff for state i */
		nolook = !(tystate[i]==MUSTLOOKAHEAD);
		closure(i);
		/* output actions */
		nolook = 1;
		aryfil(temp1, ntoksz+nnontersz+1, 0);
		WSLOOP(wsets,u) {
			c = *(u->pitem);
			if (c>1 && c<NTBASE && temp1[c]==0) {
				WSLOOP(u,v) {
					if (c == *(v->pitem)) 
						putitem(v->pitem+1, (LOOKSETS *)0);
				}
				temp1[c] = state(c);
			}
			else if (c > NTBASE && temp1[(c -= NTBASE)+ntokens]==0){
				temp1[ c+ntokens ] = amem[indgo[i]+c];
			}
		}

		if (i == 1) 
			temp1[1] = ACCEPTCODE;

		/* now, we have the shifts; look at the reductions */

		lastred = 0;
		WSLOOP(wsets,u) {
			c = *(u->pitem);
			if (c<=0) { /* reduction */
				lastred = -c;
				TLOOP(k) {
					if (BIT(u->ws.lset,k)) {
						if (temp1[k] == 0) 
							temp1[k] = c;
						else if (temp1[k]<0) { /* reduce/reduce conflict */
							if (foutput!=NULL)
								(void) fprintf(foutput,
						        "\n%d: reduce/reduce conflict (red'ns %d and %d ) on %s",
						        i, -temp1[k], lastred, symnam(k) );
						    if( -temp1[k] > lastred ) temp1[k] = -lastred;
						    ++zzrrconf;
						}
						else  /* potential shift/reduce conflict */
							precftn( lastred, k, i );
					}
				}
			}
		}
		wract(i);
	}

	(void) fprintf(ftable, "\t};\n");

	wdef("YYNPROD", nprod);

}

static int pkdebug = 0;
apack(p, n ) int *p;{ /* pack state i from temp1 into amem */
	int off;
	register *pp, *qq;
	int *q, *r, *rr;

	/* we don't need to worry about checking because we
	   we will only look up entries known to be there... */

	/* eliminate leading and trailing 0's */

	q = p+n;
	for( pp=p,off=0 ; *pp==0 && pp<=q; ++pp,--off ) /* EMPTY */ ;
	if( pp > q ) return(0);  /* no actions */
	p = pp;

	/* now, find a place for the elements from p to q, inclusive */

	/* for( rr=amem; rr<=r; ++rr,++off ){ */  /* try rr */
	rr = amem;
	for ( ; ; ++rr, ++off) {
		if (rr >= &amem[new_actsize-1]) 
			exp_act(&rr);
		qq=rr;
		for( pp=p ; pp<=q ; ++pp,++qq){
			if( *pp ){
				if ( qq >= &amem[new_actsize-1]) {
					r=rr;
					exp_act(&rr);
					qq = qq - r + rr;
				}
				if( *pp != *qq && *qq != 0 ) goto nextk;
			}
		}

		/* we have found an acceptable k */

		if( pkdebug && foutput!=NULL ) 
			(void) fprintf( foutput, "off = %d, k = %d\n", off, rr-amem );

		qq=rr;
		for( pp=p; pp<=q; ++pp,++qq ){
			if( *pp ){
				if( qq >= &amem[new_actsize-1] ) {
					r=rr;
					exp_act(&rr);
					qq = qq - r + rr;
				}
				if( qq>memp ) memp = qq;
				*qq = *pp;
			}
		}
		if( pkdebug && foutput!=NULL ){
			for( pp=amem; pp<= memp; pp+=10 ){
				(void) fprintf( foutput, "\t");
				for( qq=pp; qq<=pp+9; ++qq ) 
					(void) fprintf( foutput, "%d ", *qq );
				(void) fprintf( foutput, "\n");
			}
		}
		return( off );

		nextk: ;
	}
	/* error("no space in action table" ); */
	/* NOTREACHED */
}

void go2out(){ /* output the gotos for the nontermninals */
	int i, j, k, best, count, cbest, times;

	(void) fprintf( ftemp, "$\n" );  /* mark begining of gotos */

	for( i=1; i<=nnonter; ++i ) {
		go2gen(i);

		/* find the best one to make default */

		best = -1;
		times = 0;

		for( j=0; j<=nstate; ++j ){ /* is j the most frequent */
			if( tystate[j] == 0 ) continue;
			if( tystate[j] == best ) continue;

			/* is tystate[j] the most frequent */

			count = 0;
			cbest = tystate[j];

			for( k=j; k<=nstate; ++k ) if( tystate[k]==cbest ) ++count;

			if( count > times ){
				best = cbest;
				times = count;
				}
			}

		/* best is now the default entry */

		zzgobest += (times-1);
		for( j=0; j<=nstate; ++j ){
			if( tystate[j] != 0 && tystate[j]!=best ){
				(void) fprintf( ftemp, "%d,%d,", j, tystate[j] );
				zzgoent += 1;
				}
			}

		/* now, the default */

		zzgoent += 1;
		(void) fprintf( ftemp, "%d\n", best );

		}



	}

static int g2debug = 0;
static void go2gen(c){ /* output the gotos for nonterminal c */

	int i, work, cc;
	ITEM *p, *q;


	/* first, find nonterminals with gotos on c */

	aryfil( temp1, nnonter+1, 0 );
	temp1[c] = 1;

	work = 1;
	while( work ){
		work = 0;
		PLOOP(0,i){
			if( (cc=prdptr[i][1]-NTBASE) >= 0 ){ /* cc is a nonterminal */
				if( temp1[cc] != 0 ){ /* cc has a goto on c */
					cc = *prdptr[i]-NTBASE; /* thus, the left side of production i does too */
					if( temp1[cc] == 0 ){
						  work = 1;
						  temp1[cc] = 1;
						  }
					}
				}
			}
		}

	/* now, we have temp1[c] = 1 if a goto on c in closure of cc */

	if( g2debug && foutput!=NULL ){
		(void) fprintf( foutput, "%s: gotos on ", nontrst[c].name );
		NTLOOP(i) if( temp1[i] ) (void) fprintf( foutput, "%s ", nontrst[i].name);
		(void) fprintf( foutput, "\n");
		}

	/* now, go through and put gotos into tystate */

	aryfil( tystate, nstate, 0 );
	SLOOP(i){
		ITMLOOP(i,p,q){
			if( (cc= *p->pitem) >= NTBASE ){
				if( temp1[cc -= NTBASE] ){ /* goto on c is possible */
					tystate[i] = amem[indgo[i]+c];
					break;
					}
				}
			}
		}
	}

static void precftn(r,t,s){ /* decide a shift/reduce conflict by precedence.
	/* r is a rule number, t a token number */
	/* the conflict is in state s */
	/* temp1[t] is changed to reflect the action */

	int lp,lt, action;

	lp = levprd[r];
	lt = toklev[t];
	if( PLEVEL(lt) == 0 || PLEVEL(lp) == 0 ) {
		/* conflict */
		if( foutput != NULL ) (void) fprintf( foutput, "\n%d: shift/reduce conflict (shift %d, red'n %d) on %s",
						s, temp1[t], r, symnam(t) );
		++zzsrconf;
		return;
		}
	if( PLEVEL(lt) == PLEVEL(lp) ) action = ASSOC(lt) & ~04;
	else if( PLEVEL(lt) > PLEVEL(lp) ) action = RASC;  /* shift */
	else action = LASC;  /* reduce */

	switch( action ){

	case BASC:  /* error action */
		temp1[t] = ERRCODE;
		return;

	case LASC:  /* reduce */
		temp1[t] = -r;
		return;

		}
	}

static void wract(i){ /* output state i */
	/* temp1 has the actions, lastred the default */
	int p, p0, p1;
	int ntimes, tred, count, j;
	int flag;

	/* find the best choice for lastred */

	lastred = 0;
	ntimes = 0;
	TLOOP(j){
		if( temp1[j] >= 0 ) continue;
		if( temp1[j]+lastred == 0 ) continue;
		/* count the number of appearances of temp1[j] */
		count = 0;
		tred = -temp1[j];
		levprd[tred] |= REDFLAG;
		TLOOP(p){
			if( temp1[p]+tred == 0 ) ++count;
			}
		if( count >ntimes ){
			lastred = tred;
			ntimes = count;
			}
		}

	/* for error recovery, arrange that, if there is a shift on the
	/* error recovery token, `error', that the default be the error action */
	if( temp1[2] > 0 ) lastred = 0;

	/* clear out entries in temp1 which equal lastred */
	TLOOP(p) if( temp1[p]+lastred == 0 )temp1[p]=0;

	wrstate(i);
	defact[i] = lastred;

	flag = 0;
	TLOOP(p0){
		if( (p1=temp1[p0])!=0 ) {
			if( p1 < 0 ){
				p1 = -p1;
				goto exc;
				}
			else if( p1 == ACCEPTCODE ) {
				p1 = -1;
				goto exc;
				}
			else if( p1 == ERRCODE ) {
				p1 = 0;
				goto exc;
			exc:
				if( flag++ == 0 ) (void) fprintf( ftable, "-1, %d,\n", i );
				(void) fprintf( ftable, "\t%d, %d,\n", tokset[p0].value, p1 );
				++zzexcp;
				}
			else {
				(void) fprintf( ftemp, "%d,%d,", tokset[p0].value, p1 );
				++zzacent;
				}
			}
		}
	if( flag ) {
		defact[i] = -2;
		(void) fprintf( ftable, "\t-2, %d,\n", lastred );
		}
	(void) fprintf( ftemp, "\n" );
	return;
	}

static void wrstate(i){ /* writes state i */
	register j0,j1;
	register ITEM *pp, *qq;
	register WSET *u;

	if( foutput == NULL ) return;
	(void) fprintf( foutput, "\nstate %d\n",i);
	ITMLOOP(i,pp,qq) (void) fprintf( foutput, "\t%s\n", writem(pp->pitem));
	if( tystate[i] == MUSTLOOKAHEAD ){
		/* print out empty productions in closure */
		WSLOOP( wsets+(pstate[i+1]-pstate[i]), u ){
			if( *(u->pitem) < 0 ) (void) fprintf( foutput, "\t%s\n", writem(u->pitem) );
			}
		}

	/* check for state equal to another */

	TLOOP(j0) if( (j1=temp1[j0]) != 0 ){
		(void) fprintf( foutput, "\n\t%s  ", symnam(j0) );
		if( j1>0 ){ /* shift, error, or accept */
			if( j1 == ACCEPTCODE ) (void) fprintf( foutput,  "accept" );
			else if( j1 == ERRCODE ) (void) fprintf( foutput, "error" );
			else (void) fprintf( foutput,  "shift %d", j1 );
			}
		else (void) fprintf( foutput, "reduce %d",-j1 );
		}

	/* output the final production */

	if( lastred ) (void) fprintf( foutput, "\n\t.  reduce %d\n\n", lastred );
	else (void) fprintf( foutput, "\n\t.  error\n\n" );

	/* now, output nonterminal actions */

	j1 = ntokens;
	for( j0 = 1; j0 <= nnonter; ++j0 ){
		if( temp1[++j1] ) (void) fprintf( foutput, "\t%s  goto %d\n", symnam( j0+NTBASE), temp1[j1] );
		}
	}

static void wdef( s, n ) char *s; { /* output a definition of s to the value n */
	(void) fprintf( ftable, "# define %s %d\n", s, n );
	}

void warray( s, v, n ) char *s; int *v, n; {

	register i;

	(void) fprintf( ftable, "yytabelem %s[]={\n", s );
	for( i=0; i<n; ){
		if( i%10 == 0 ) (void) fprintf( ftable, "\n" );
		(void) fprintf( ftable, "%6d", v[i] );
		if( ++i == n ) (void) fprintf( ftable, " };\n" );
		else (void) fprintf( ftable, "," );
		}
	}

void hideprod(){
	/* in order to free up the mem and amem arrays for the optimizer,
	/* and still be able to output yyr1, etc., after the sizes of
	/* the action array is known, we hide the nonterminals
	/* derived by productions in levprd.
	*/

	register i, j;

	j = 0;
	levprd[0] = 0;
	PLOOP(1,i){
		if( !(levprd[i] & REDFLAG) ){
			++j;
			if( foutput != NULL ){
				(void) fprintf( foutput, "Rule not reduced:   %s\n", writem( prdptr[i] ) );
				}
			}
		levprd[i] = *prdptr[i] - NTBASE;
		}
	if( j ) (void) fprintf( stderr, "%d rules never reduced\n", j );
	}
