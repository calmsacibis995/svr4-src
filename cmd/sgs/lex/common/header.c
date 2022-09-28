/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)lex:header.c	1.12"
/* header.c */
# include "ldefs.c"
phead1(){
	ratfor ? rhd1() : chd1();
	}

chd1(){
	if(*v_stmp == 'y') (void)fprintf(fout,"#ident\t\"lex: %s\"\n",ESG_REL);
	(void)fprintf(fout,"# include \"stdio.h\"\n");
	(void)fprintf(fout, "# define U(x) ((unsigned char)(x))\n");
	(void)fprintf(fout, "# define NLSTATE yyprevious=YYNEWLINE\n");
	(void)fprintf(fout,"# define BEGIN yybgin = yysvec + 1 +\n");
	(void)fprintf(fout,"# define INITIAL 0\n");
	(void)fprintf(fout,"# define YYLERR yysvec\n");
	(void)fprintf(fout,"# define YYSTATE (yyestate-yysvec-1)\n");
	if(optim)
		(void)fprintf(fout,"# define YYOPTIM 1\n");
# ifdef DEBUG
	(void)fprintf(fout,"# define LEXDEBUG 1\n");
# endif
	(void)fprintf(fout,"# define YYLMAX 200\n");
	(void)fprintf(fout,"# define output(c) (void)putc(c,yyout)\n");
	(void)fprintf(fout,"#if defined(__cplusplus) || defined(__STDC__)\n");
	(void)fprintf(fout,"\tint yyback(int *, int);\n");
	(void)fprintf(fout,"\tint yyinput(void);\n");
	(void)fprintf(fout,"\tint yylook(void);\n");
	(void)fprintf(fout,"\tvoid yyoutput(int);\n");
	(void)fprintf(fout,"\tint yyracc(int);\n");
	(void)fprintf(fout,"\tint yyreject(void);\n");
	(void)fprintf(fout,"\tvoid yyunput(int);\n");
	(void)fprintf(fout,"\n#ifndef __STDC__\n");
	(void)fprintf(fout,"#ifndef yyless\n");
	(void)fprintf(fout,"\tvoid yyless(int);\n");
	(void)fprintf(fout,"#endif\n");
	(void)fprintf(fout,"#ifndef yywrap\n");
	(void)fprintf(fout,"\tint yywrap(void);\n");
	(void)fprintf(fout,"#endif\n");
	(void)fprintf(fout,"#endif\n\n");
	(void)fprintf(fout,"#endif\n");
	(void)fprintf(fout, "%s%d%s\n",
  "# define input() (((yytchar=yysptr>yysbuf?U(*--yysptr):getc(yyin))==",
	ctable['\n'],
 "?(yylineno++,yytchar):yytchar)==EOF?0:yytchar)");
	(void)fprintf(fout,
"# define unput(c) {yytchar= (c);if(yytchar=='\\n')yylineno--;*yysptr++=yytchar;}\n");
	(void)fprintf(fout,"# define yymore() (yymorfg=1)\n");
	(void)fprintf(fout,"# define ECHO (void)fprintf(yyout, \"%%s\",yytext)\n");
	(void)fprintf(fout,"# define REJECT { nstr = yyreject(); goto yyfussy;}\n");
	(void)fprintf(fout,"int yyleng; extern char yytext[];\n");
	(void)fprintf(fout,"int yymorfg;\n");
	(void)fprintf(fout,"extern char *yysptr, yysbuf[];\n");
	(void)fprintf(fout,"int yytchar;\n");
	(void)fprintf(fout,"FILE *yyin = {stdin}, *yyout = {stdout};\n");
	(void)fprintf(fout,"extern int yylineno;\n");
	(void)fprintf(fout,"struct yysvf { \n");
	(void)fprintf(fout,"\tstruct yywork *yystoff;\n");
	(void)fprintf(fout,"\tstruct yysvf *yyother;\n");
	(void)fprintf(fout,"\tint *yystops;};\n");
	(void)fprintf(fout,"struct yysvf *yyestate;\n");
	(void)fprintf(fout,"extern struct yysvf yysvec[], *yybgin;\n");
	}

rhd1(){
	(void)fprintf(fout,"integer function yylex(dummy)\n");
	(void)fprintf(fout,"define YYLMAX 200\n");
	(void)fprintf(fout,"define ECHO call yyecho(yytext,yyleng)\n");
	(void)fprintf(fout,"define REJECT nstr = yyrjct(yytext,yyleng);goto 30998\n");
	(void)fprintf(fout,"integer nstr,yylook,yywrap\n");
	(void)fprintf(fout,"integer yyleng, yytext(YYLMAX)\n");
	(void)fprintf(fout,"common /yyxel/ yyleng, yytext\n");
	(void)fprintf(fout,"common /yyldat/ yyfnd, yymorf, yyprev, yybgin, yylsp, yylsta\n");
	(void)fprintf(fout,"integer yyfnd, yymorf, yyprev, yybgin, yylsp, yylsta(YYLMAX)\n");
	(void)fprintf(fout,"for(;;){\n");
	(void)fprintf(fout,"\t30999 nstr = yylook(dummy)\n");
	(void)fprintf(fout,"\tgoto 30998\n");
	(void)fprintf(fout,"\t30000 k = yywrap(dummy)\n");
	(void)fprintf(fout,"\tif(k .ne. 0){\n");
	(void)fprintf(fout,"\tyylex=0; return; }\n");
	(void)fprintf(fout,"\t\telse goto 30998\n");
	}

phead2(){
	if(!ratfor)chd2();
	}

chd2(){
	(void)fprintf(fout,"while((nstr = yylook()) >= 0)\n");
	(void)fprintf(fout,"yyfussy: switch(nstr){\n");
	(void)fprintf(fout,"case 0:\n");
	(void)fprintf(fout,"if(yywrap()) return(0); break;\n");
	}

ptail(){
	if(!pflag)
		ratfor ? rtail() : ctail();
	pflag = 1;
	}

ctail(){
	(void)fprintf(fout,"case -1:\nbreak;\n");		/* for reject */
	(void)fprintf(fout,"default:\n");
	(void)fprintf(fout,"(void)fprintf(yyout,\"bad switch yylook %%d\",nstr);\n");
	(void)fprintf(fout,"} return(0); }\n");
	(void)fprintf(fout,"/* end of yylex */\n");
	}

rtail(){
	register int i;
	(void)fprintf(fout,"\n30998 if(nstr .lt. 0 .or. nstr .gt. %d)goto 30999\n",casecount);
	(void)fprintf(fout,"nstr = nstr + 1\n");
	(void)fprintf(fout,"goto(\n");
	for(i=0; i<casecount; i++)
		(void)fprintf(fout,"%d,\n",30000+i);
	(void)fprintf(fout,"30999),nstr\n");
	(void)fprintf(fout,"30997 continue\n");
	(void)fprintf(fout,"}\nend\n");
	}
statistics(){
	(void)fprintf(errorf,"%d/%d nodes(%%e), %d/%d positions(%%p), %d/%d (%%n), %ld transitions,\n",
		tptr, treesize, nxtpos-positions, maxpos, stnum+1, nstates, rcount);
	(void)fprintf(errorf,"%d/%d packed char classes(%%k),", pcptr-pchar,pchlen);
	if(optim)(void)fprintf(errorf," %d/%d packed transitions(%%a),",nptr,ntrans);
	(void)fprintf(errorf, " %d/%d output slots(%%o)", yytop, outsize);
	(void)putc('\n',errorf);
	}
