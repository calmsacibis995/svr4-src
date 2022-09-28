%{
/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
%}
%{
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
%}
%{
/*	  All Rights Reserved  	*/
%}

%{
/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
%}
%{
/*	UNIX System Laboratories, Inc.                     	*/
%}
%{
/*	The copyright notice above does not evidence any   	*/
%}
%{
/*	actual or intended publication of such source code.	*/
%}

%{
%}

%{
#ident	"@(#)shl:lex.l	1.4.2.1"
%}

%{

#define NOCMDS 10

static char *cmds[] = 
{
	"?",
	"create",
	"resume",
	"delete",
	"layers",
	"help",
	"quit",
	"block",
	"unblock",
	"toggle"
};

static int cmd_values[] =
{
	LX_help,
	LX_create,
	LX_resume,
	LX_delete,
	LX_layers,
	LX_help,
	LX_quit,
	LX_block,
	LX_unblock,
	LX_toggle,
};

yywrap()
{
	return(0);
}

reserved(str,length)
	char *str;
	int length;
{
	int i;

	if (length > 7)
		return(ERROR);
	else
	{
		for (i=0; i < NOCMDS; i++)
			if (strncmp(str,cmds[i], length) == 0)
				return(cmd_values[i]);

		return(ERROR);
	}
}

%}

CMD			([a-z?])+
NAME		([^ \t\n])+
DEFNAME		[(][1-7][)]|[1-7]
WHITE		([ \t])+
NL			[\n]

%START		CMD OTHER
%%
	int token;
	char *strcpy();
	char *strncpy();

<CMD>{CMD}			{
						BEGIN OTHER;
						if ((token = reserved(yytext,yyleng)) != ERROR)
							return(token);
						else
						{
							strncpy(yylval.str, yytext, NAMSIZ);
							yylval.str[NAMSIZ] = 0;
							return(LX_name);
						}
					}

<CMD,OTHER>{DEFNAME}	{
						strcpy(yylval.str,yytext);
						return(LX_defname);
					}

<CMD,OTHER>-l		{
						return(LX_long_option);
					}


<CMD,OTHER>{NAME}	{
						strncpy(yylval.str, yytext, NAMSIZ);
						yylval.str[NAMSIZ] = 0;
						return(LX_name);
					}

<CMD,OTHER>{NL}		{
							BEGIN CMD;
							return(LX_newline);
					}

<CMD,OTHER>{WHITE}	;

<CMD,OTHER>.		{
						return(ERROR);
					}

.|[\n]				{
						BEGIN CMD;
						unput(yytext[0]);
					}

%%
