/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)yacc:y4.c	1.7"
# include "dextern"

# define NOMORE -1000

static void gin();
static void stin();
static void osummary();
static void aoutput();
static void arout();
static nxti();
static gtnm();

static int *ggreed;
static int *pgo;
static int *yypgo;

static int maxspr = 0;  /* maximum spread of any entry */
static int maxoff = 0;  /* maximum offset into an array */
int *optimmem;
static int *maxa;

static int nxdb = 0;
static int adb = 0;

void callopt(){

	register i, *p, j, k, *q;

	ggreed = (int *) malloc(sizeof(int) * size);
	pgo = (int *) malloc(sizeof(int) * size);
	yypgo = &nontrst[0].tvalue;

	/* read the arrays from tempfile and set parameters */

	if( (finput=fopen(TEMPNAME,"r")) == NULL ) error( "optimizer cannot open tempfile" );

	optimmem = tracemem;
	pgo[0] = 0;
	temp1[0] = 0;
	nstate = 0;
	nnonter = 0;
	for(;;){
		switch( gtnm() ){

		case '\n':
			temp1[++nstate] = (--optimmem) - tracemem;
			/* FALLTHRU */
		case ',':
			continue;

		case '$':
			break;

		default:
			error( "bad tempfile" );
			}
		break;
		}

	temp1[nstate] = yypgo[0] = (--optimmem) - tracemem;

	for(;;){
		switch( gtnm() ){

		case '\n':
			yypgo[++nnonter]= optimmem-tracemem;
			/* FALLTHRU */
		case ',':
			continue;

		case EOF:
			break;

		default:
			error( "bad tempfile" );
			}
		break;
		}

	yypgo[nnonter--] = (--optimmem) - tracemem;



	for( i=0; i<nstate; ++i ){

		k = 32000000;
		j = 0;
		q = tracemem + temp1[i+1];
		for( p = tracemem + temp1[i]; p<q ; p += 2 ){
			if( *p > j ) j = *p;
			if( *p < k ) k = *p;
			}
		if( k <= j ){ /* nontrivial situation */
			/* temporarily, kill this for compatibility
			j -= k;  /* j is now the range */
			if( k > maxoff ) maxoff = k;
			}
		tystate[i] = (temp1[i+1]-temp1[i]) + 2*j;
		if( j > maxspr ) maxspr = j;
		}

	/* initialize ggreed table */

	for( i=1; i<=nnonter; ++i ){
		ggreed[i] = 1;
		j = 0;
		/* minimum entry index is always 0 */
		q = tracemem + yypgo[i+1] -1;
		for( p = tracemem+yypgo[i]; p<q ; p += 2 ) {
			ggreed[i] += 2;
			if( *p > j ) j = *p;
			}
		ggreed[i] = ggreed[i] + 2*j;
		if( j > maxoff ) maxoff = j;
		}


	/* now, prepare to put the shift actions into the amem array */

	for( i=0; i<new_actsize; ++i ) amem[i] = 0;
	maxa = amem;

	for( i=0; i<nstate; ++i ) {
		if( tystate[i]==0 && adb>1 ) (void) fprintf( ftable, "State %d: null\n", i );
		indgo[i] = YYFLAG1;
		}

	while( (i = nxti()) != NOMORE ) {
		if( i >= 0 ) stin(i);
		else gin(-i);

		}

	if( adb>2 ){ /* print a array */
		for( p=amem; p <= maxa; p += 10){
			(void) fprintf( ftable, "%4d  ", p-amem );
			for( i=0; i<10; ++i ) (void) fprintf( ftable, "%4d  ", p[i] );
			(void) fprintf( ftable, "\n" );
			}
		}
	/* write out the output appropriate to the language */

	aoutput();

	osummary();
	ZAPFILE(TEMPNAME);
	}

static void gin(i){

	register *r, *s, *q1, *q2;
	int *p;

	/* enter gotos on nonterminal i into array amem */

	ggreed[i] = 0;

	q2 = tracemem+ yypgo[i+1] - 1;
	q1 = tracemem + yypgo[i];

	/* now, find a place for it */

	/* for( p=amem; p < &amem[new_actsize]; ++p ){ */
	p = amem;
	for( ; ; ) {
		if ( p>= &amem[new_actsize]) 
			exp_act(&p);
		if( *p ) goto nextgp;
		for( r=q1; r<q2; r+=2 ){
			s = p + *r +1;
			if( *s ) goto nextgp;
			if( s > maxa ){
				if( (maxa=s) >= &amem[new_actsize] ) 
					/* error( "amem array overflow" ); */
					exp_act(&p);
				 }
			}
		/* we have found a spot */

		*p = *q2;
		if( p > maxa ){
			if( (maxa=p) >= &amem[new_actsize] ) 
				/* error( "amem array overflow" ); */
				exp_act(&p);
			}
		for( r=q1; r<q2; r+=2 ){
			s = p + *r + 1;
			*s = r[1];
			}

		pgo[i] = p-amem;
		if(adb>1) (void) fprintf( ftable, "Nonterminal %d, entry at %d\n", i, pgo[i]);
		goto nextgi;

		nextgp:  
			++p;
	}

	/* error( "cannot place goto %d\n", i ); */

	nextgi:  ;
}

static void stin(i){
	register *r, n, nn, flag, j, *q1, *q2;
	int *s;

	tystate[i] = 0;

	/* enter state i into the amem array */

	q2 = tracemem+temp1[i+1];
	q1 = tracemem+temp1[i];
	/* find an acceptable place */

	/* for( n= -maxoff; n<new_actsize; ++n ){ */
	nn = -maxoff;
more:
	for ( n= nn; n<new_actsize; ++n) {
		flag = 0;
		for( r = q1; r < q2; r += 2 ){
			if( (s = *r + n + amem ) < amem ) goto nextn;
			if( *s == 0 ) ++flag;
			else if( *s != r[1] ) goto nextn;
			}

	/* check that the position equals another only if the states are identical */

		for( j=0; j<nstate; ++j ){
			if( indgo[j] == n ) {
				if( flag ) goto nextn;  /* we have some disagreement */
				if( temp1[j+1] + temp1[i] == temp1[j] + temp1[i+1] ){
					/* states are equal */
					indgo[i] = n;
					if( adb>1 ) (void) fprintf( ftable, 
				       "State %d: entry at %d equals state %d\n",i,n,j);
					return;
					}
				goto nextn;  /* we have some disagreement */
				}
			}

		for( r = q1; r < q2; r += 2 ){
			while ( (s = *r + n + amem ) >= &amem[new_actsize] ) { 
			/* 	error( "out of space in optimizer amem array" ); */
				exp_act( (int **)NULL );
			}
			if( s > maxa ) maxa = s;
			if( *s != 0 && *s != r[1] ) error( "clobber of amem array, pos'n %d, by %d", s-amem, r[1] );
			*s = r[1];
			}
		indgo[i] = n;
		if( adb>1 ) (void) fprintf( ftable, "State %d: entry at %d\n", i, indgo[i] );
		return;

		nextn:  ;
		}

	/* error( "Error; failure to place state %d\n", i ); */
	exp_act( (int **)NULL );
	nn = new_actsize - ACTSIZE;
	goto more;

	/* NOTREACHED */
	}

static nxti(){ /* finds the next i */
	register i, max, maxi;

	max = 0;

	for( i=1; i<= nnonter; ++i ) if( ggreed[i] >= max ){
		max = ggreed[i];
		maxi = -i;
		}

	for( i=0; i<nstate; ++i ) if( tystate[i] >= max ){
		max = tystate[i];
		maxi = i;
		}

	if( nxdb ) (void) fprintf( ftable, "nxti = %d, max = %d\n", maxi, max );
	if( max==0 ) return( NOMORE );
	else return( maxi );
	}

static void osummary(){
	/* write summary */

	register i, *p;

	if( foutput == NULL ) return;
	i=0;
	for( p=maxa; p>=amem; --p ) {
		if( *p == 0 ) ++i;
		}

	(void) fprintf( foutput, "Optimizer space used: input %d/%d, output %d/%d\n",
		optimmem-tracemem+1, new_memsize, maxa-amem+1, new_actsize );
	(void) fprintf( foutput, "%d table entries, %d zero\n", (maxa-amem)+1, i );
	(void) fprintf( foutput, "maximum spread: %d, maximum offset: %d\n", maxspr, maxoff );

	}

static void aoutput(){ /* this version is for C */


	/* write out the optimized parser */

	(void) fprintf( ftable, "# define YYLAST %d\n", maxa-amem+1 );

	arout( "yyact", amem, (maxa-amem)+1 );
	arout( "yypact", indgo, nstate );
	arout( "yypgo", pgo, nnonter+1 );

	}

static void arout( s, v, n ) char *s; int *v, n; {

	register i;

	(void) fprintf( ftable, "yytabelem %s[]={\n", s );
	for( i=0; i<n; ){
		if( i%10 == 0 ) (void) fprintf( ftable, "\n" );
		(void) fprintf( ftable, "%6d", v[i] );
		if( ++i == n ) (void) fprintf( ftable, " };\n" );
		else (void) fprintf( ftable, "," );
		}
	}


static gtnm(){

	register s, val, c;

	/* read and convert an integer from the standard input */
	/* return the terminating character */
	/* blanks, tabs, and newlines are ignored */

	s = 1;
	val = 0;

	while( (c=getc(finput)) != EOF ){
		if( isdigit(c) ){
			val = val * 10 + c - '0';
			}
		else if ( c == '-' ) s = -1;
		else break;
		}

	*optimmem++ = s*val;
	if( optimmem >= &tracemem[new_memsize] ) exp_mem(0);
	return( c );

	}

void exp_act(ptr)
int **ptr;
{
	static int *actbase;
	int i;
	new_actsize += ACTSIZE;

	actbase = amem;
	amem = (int *) realloc((char *)amem, sizeof(int) * new_actsize);
	if (amem == NULL) error("couldn't expand action table");
	
	for ( i=new_actsize-ACTSIZE; i<new_actsize; ++i) amem[i] = 0;
	if (ptr != NULL) *ptr = *ptr - actbase + amem;
	if ( memp >= amem ) memp = memp - actbase + amem;
	if ( maxa >= amem ) maxa = maxa - actbase + amem;
}
