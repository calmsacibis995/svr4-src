/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

%{
#ident	"@(#)shl:yacc.y	1.3.2.1"
%}
%{
#include	"defs.h"

#define		ERROR	'e'

int flag;

int (*func)();
int destroy();
int block();
int unblock();

char *prs();
%}

%union {			/* Yacc value stack */
	  int ival;
	  char str[256];
       }

%token	<str>	LX_name		LX_defname

%token	<ival>	LX_create	LX_resume	LX_delete	LX_layers	LX_quit
%token	<ival>	LX_newline	LX_long_option			LX_help		LX_block
%token	<ival>	LX_unblock	LX_toggle

%type	<str>	name

%start	input

%%

input			:	input_line

				|	input input_line

				;

input_line		:	cmd 						{
													prompt();
												}

				|	error						{
													fprintf(stderr, "syntax error\n");
												}

					 	eol						{
													yyerrok;
													prompt();
												}

				|	eol							{
													prompt();
												}
				;

cmd				:	LX_create  eol				{
													create("");
												}

				|	LX_create LX_name eol		{
													create($2);
												}

				|	LX_resume eol				{
													resume_current();
												}

				|	LX_resume name eol			{
													resume($2);
												}

				|	LX_delete 					{
													func = destroy;
												}
						name_list eol

				|	LX_layers eol				{
													all_layers(0);
												}

				|	LX_layers 
					    LX_long_option eol		{
													all_layers(1);
												}

				|	LX_layers 					{	
													flag = 0;
												}
							layers_list	eol

				|	LX_layers 
					    LX_long_option			{
													flag = 1;
												}
							layers_list eol

				|	LX_quit eol					{
													kill_all();
													YYACCEPT;
												}

				|	LX_help	eol					{
													help();
												}

				|   LX_toggle eol				{
													toggle();
												}

				|	LX_block					{
													func = block;
												}
						name_list eol

				|	LX_unblock					{
													func = unblock;
												}
						name_list eol

				|	name eol					{	
													resume($1);
												}
				;

layers_list		:	name						{
													one_layer($1, flag);
												}

				|	layers_list name			{
													one_layer($2, flag);
												}
				;

name_list		:	name						{
													(*func)($1);
												}

				|	name_list name				{
													(*func)($2);
												}
				;

name			:	LX_name

				|	LX_defname

				;

eol				:	LX_newline
				;

%%

yyerror(s)			/* Yacc required error routine */
	char *s;
{
}


#include	"lex.c"
