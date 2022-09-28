/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sco:mapio.c	1.1"
#include <sys/types.h>
#include <sys/at_ansi.h>
#include <sys/kd.h>
#include <sys/ascii.h>
#include <stdio.h>
#include <ctype.h>

#ifdef DEBUG
keymap_t keymap;
main()
{
	fparsemap(stdin,&keymap);
	fprintmap(stdout,&keymap);
}
#endif

fparsemap(pF,pK)
keymap_t *pK;
FILE *pF;
{
	static char aCtok[8];
	static char aClock[2];
	int i,Nscan,flag;
	
		
	fscanblanks(pF);
	while (!feof( pF )){
		fscantok(pF,aCtok);
		Nscan = atoi( aCtok );
#ifdef DEBUG
		printf("token '%s' ",aCtok);
		printf("key number %d\n", Nscan);
#endif
		fscanblanks(pF);
		if(Nscan > pK->n_keys - 1 ){
			pK->n_keys = Nscan + 1;
		}
	
		for( i = 0; i < 8 ; i++ ){
			fscantok(pF,aCtok);
#ifdef DEBUG
			printf("token '%s' ",aCtok);
#endif
			pK->key[Nscan].map[i] = translate(aCtok,&flag);
			pK->key[Nscan].spcl |= flag << (7 - i);
			fscanblanks(pF);
		}
	
		fscanblanks(pF);
		fscantok(pF,aClock);
#ifdef DEBUG
		printf("token '%s'\n",aClock);
#endif
		switch( aClock[0] ){
		case 'O': pK->key[Nscan].flgs = L_O; break;
		case 'C': pK->key[Nscan].flgs = L_C; break;
		case 'N': pK->key[Nscan].flgs = L_N; break;
		}
	
		fscanblanks(pF);
	}
}

fscantok(pF,pC)
FILE *pF;
char *pC;
{
	
	*pC = getc( pF );
	if( '\'' == *pC ){
		*++pC = getc(pF);
		if( '\\' == *pC ){
			*++pC = getc(pF);
		}
		*++pC = getc(pF);
		*pC = 0;
	} else {
		while ( *pC != ' ' && *pC != '\n' && *pC != '\t' )
			*++pC = getc(pF);
		ungetc( *pC , pF);
		*pC = 0;
	}
}


int line = 0;

fscanblanks(pF)
FILE *pF;
{
	int c;
	
	c = getc(pF);
	while ( ' ' == c || '\t' == c || '\n' == c || '#' == c ) {
		if( '#' == c ){
			while( c != '\n' ) 
				c = getc(pF);
			line++;
		} else if ('\n' == c ){
			c = getc(pF);
			line++;
		} else
			c = getc(pF);
	}
	ungetc( c, pF);
}

int
translate( pC,pflag )
char *pC;
int *pflag;
{
	*pflag = 0;
	switch(pC[0]){
	default:   fprintf(stderr,"line %d:token '%s' not recognized\n", line, pC);
		   return 0;
	case '\'': return pC[1] == '\\' ? pC[2] : pC[1]; 
	case '0':  switch( pC[1] ) {
		   case 'x': return basen( 16, pC+2 );
		   default:  return basen( 8, pC+1 );
		   }
	case '1':case '2':case '3':case '4':case '5':
	case '6':case '7':case '8':case '9':
		  return atoi(pC);
	case 'a': switch (pC[1]) {
		  case 'g':
			*pflag = 1;
			return K_AGR;
		  default:
			return A_ACK;
		  }
	case 'b': switch( pC[1] ){
		  case 'e': return A_BEL;
		  case 'r': *pflag = 1;return K_BRK;
		  case 's': return A_BS;
		  case 't': return A_GS;
		  default: return K_NOP;
		  }
	case 'c': switch( pC[1] ){
		  case 'a': return A_CAN;
		  case 'l': *pflag = 1; return K_CLK;
		  case 'r': return A_CR;
		  }
	case 'd': switch( pC[2] ){
		  case '1': return A_DC1;
		  case '2': return A_DC2;
		  case '3': return A_DC3;
		  case '4': return A_DC4;
		  case 'l': return A_DEL;
		  case 'e': return A_DLE;
		  case 'b': *pflag = 1; return K_DBG;
		  }
	case 'e': switch( pC[1] ){
		  case 'm': return A_EM;
		  case 'n': return A_ENQ;
		  case 'o': return A_EOT;
		  case 's': return A_ESC;
		  case 't': return pC[2]=='b'? A_ETB:A_ETX;
		  }
	case 'f': switch( pC[1] ){
		  case '[': *pflag = 1; return K_ESL;
		  case 'N': *pflag = 1; return K_ESN;
		  case 'O': *pflag = 1; return K_ESO;
		  case 'n': *pflag = 1; return K_FRCNEXT;
		  case 'p': *pflag = 1; return K_FRCPREV;
		  case 's': return A_FS;
		  default:  *pflag = 1;
			    return KF+atoi(pC+4);
		  }
	case 'g': return A_GS;
	case 'h': return A_HT;
	case 'l': *pflag = 1;
		  switch(pC[1]){
		  case 'a': return K_LAL;
		  case 'c': return K_LCT;
		  case 's': return K_LSH;
		  }
	case 'm': *pflag = 1; return KF+atoi(pC+3);
	case 'n': switch( pC[1] ){
		  case 'a': return A_NAK;
		  case 'l': return pC[2] ? (*pflag=1,K_NLK): A_NL;
		  case 'o': *pflag = 1; return K_NOP;
		  case 'p': return A_NP;
		  case 's': return pC[2] ? (*pflag=1,K_NEXT): A_US;
		  case 'u': return A_NUL;
		  }
	case 'p': *pflag = 1; return K_PREV;
	case 'r': switch( pC[1] ){
		  case 'a': *pflag=1; return K_RAL;
		  case 'c': *pflag=1; return K_RCT;
		  case 'e': *pflag=1; return K_RBT;
		  case 's': return pC[2] ? (*pflag=1,K_RSH): A_RS;
		  }
		  break;
	case 's': switch( pC[1] ) {
		  case 'c': *pflag=1; return K_VTF + atoi(pC+3);
		  case 'i': return A_SI;
		  case 'l': *pflag = 1; return K_SLK;
		  case 'o': return pC[2] ? A_SOH : A_SO;
		  case 't': return A_STX;
		  case 'u': return A_SUB;
		  case 'y': return pC[2]=='n'?A_SYN:(*pflag=1,K_SRQ);
		  }
		  break;
	case 'v': return A_VT;
	}
	fprintf(stderr, "line %d: unknown keyword %s\n",line,pC);
	return A_NUL;
}

int
basen( base, pC )
int base;
char *pC;
{
	int val = 0;

	while ( *pC ){
		val *=  base;
		val += *pC >= 'a' ? *pC - 'a' : *pC - '0';
		pC++;
	}
	return val;
}

fprintmap( pF, base, pK )
FILE *pF;
keymap_t *pK;
int base;
{
	int i,j;
	
	fprintf(pF,"#                                                        alt\n");
	fprintf(pF,"# scan                      cntrl          alt    alt    cntrl   lock\n");
	fprintf(pF,"# code  base  shift  cntrl  shift   alt    shift  cntrl  shift   state\n");
	for( i = 0 ; i < pK->n_keys; i++ ){
		fprintbase(pF, base, i);
		for( j = 0 ; j < 8 ; j++ ){
			fprintval( pF, pK->key[i].map[j], ((pK->key[i].spcl)>>(7-j))&1 );
		}
		fprintf(pF,"  %c\n", "OCN"[pK->key[i].flgs]);
	}
}

fprintbase( pF, base, val)
FILE *pF;
int base, val;
{
	int i;
	char buf[6], *bp; 

	if(base == 10) {
		fprintf(pF, "%6d  ", val );
		return;
	} 
	bp = buf;
	if ( base == 16 ) { 
		sprintf(bp,"%x",val );
		for ( i=0; i <4; i++ ) {
			buf[5-i] = buf[3-i];	
		}
		buf[1]=120; /* 'x' in decimal */
	} else {
		sprintf(bp,"%o",val );
		for ( i=0; i <5; i++ ) {
			buf[5-i] = buf[4-i];	
		}
	}
	buf[0]=48;  /* '0' in decimal */
	fprintf(pF,"%6s  ", buf);
}

char *ascnames[] = {
"nul","soh","stx","etx","eot","enq","ack","bel",
"bs","ht","nl","vt","np","cr","so","si",
"dle","dc1","dc2","dc3","dc4","nak","syn","etb",
"can","em","sub","esc","fs","gs","rs","ns",
"' '","'!'","'\"'","'#'","'$'","'%'","'&'","'\\''",
"'('","')'","'*'","'+'","','","'-'","'.'","'/'",
"'0'","'1'","'2'","'3'","'4'","'5'","'6'","'7'",
"'8'","'9'","':'","';'","'<'","'='","'>'","'?'",
"'@'","'A'","'B'","'C'","'D'","'E'","'F'","'G'",
"'H'","'I'","'J'","'K'","'L'","'M'","'N'","'O'",
"'P'","'Q'","'R'","'S'","'T'","'U'","'V'","'W'",
"'X'","'Y'","'Z'","'['","'\\\\'","']'","'^'","'_'",
"'`'","'a'","'b'","'c'","'d'","'e'","'f'","'g'",
"'h'","'i'","'j'","'k'","'l'","'m'","'n'","'o'",
"'p'","'q'","'r'","'s'","'t'","'u'","'v'","'w'",
"'x'","'y'","'z'","'{'","'|'","'}'","'~'","del"
};

char *fnames[]={
"nop","soh","lshift","rshift","clock","nlock","slock",
"alt","btab","ctl","lalt","ralt","lctrl","rctrl","agr"
};
char *fnames2[]={
"sysreq","break","fN","fO","f[","reboot","debug","nscr","pscr",
"fnscrn","fpscrn"
};

fprintval( pF, val, flag)
FILE *pF;
int val;
int flag;
{
	if( flag == 1 ){
		if( val <= K_AGR )
			fprintf(pF, "%-7s", fnames[val]);
		else if( val < K_FUNF )
			fprintf(pF, "%-7s", ascnames[val]);
		else if( val <= K_FUNL )
			fprintf(pF, "fkey%02d ", val-KF );
		else if( val < K_VTF )
			fprintf(pF, "%-7s", fnames2[val - K_SRQ]);
		else 
			fprintf(pF, "scr%02d  ", val - K_VTF );
	} else {
		if( val < 128 )
			fprintf(pF, "%-7s", ascnames[val]);
		else 
			fprintf(pF, "0x%02x   ", val );
	}
}
