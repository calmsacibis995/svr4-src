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
#ident	"@(#)cmd-inet:usr.sbin/in.named/tools/nslookup/commands.l	1.2.4.1"

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 		PROPRIETARY NOTICE (Combined)
 * 
 * This source code is unpublished proprietary information
 * constituting, or derived under license from AT&T's UNIX(r) System V.
 * In addition, portions of such source code were derived from Berkeley
 * 4.3 BSD under license from the Regents of the University of
 * California.
 * 
 * 
 * 
 * 		Copyright Notice 
 * 
 * Notice of copyright on this source code product does not indicate 
 * publication.
 * 
 * 	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
 * 	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 * 	          All rights reserved.
 *  
 */

/*
 *******************************************************************************
 *
 *  commands.l
 *
 *	Lex input file for the nslookup program command interpreter.
 *	When a sequence is recognized, the associated action
 *	routine is called. The action routine may need to
 *	parse the string for additional information.
 *
 *  Recognized commands: (identifiers are shown in uppercase)
 *
 *	server NAME	- set default server to NAME, using default server
 *	lserver NAME	- set default server to NAME, using initial server
 *	finger [NAME]	- finger the optional NAME
 *	root		- set default server to the root
 *	ls NAME		- list the domain NAME
 *	view FILE	- sorts and view the file with more
 *	set OPTION	- set an option
 *	help 		- print help information
 *	? 		- print help information
 *	exit		- exit
 *	opt[ions]	- print options, current server, host
 *	NAME		- print info about the host/domain NAME 
 *			  using default server.
 *	NAME1 NAME2	- as above, but use NAME2 as server
 *
 *
 *   yylex Results:
 *	0		upon end-of-file.
 *	1		after each command.
 *  
 *******************************************************************************
 */

#include "res.h"
#define yywrap() (1)
extern char rootServerName[];

%}
WS	[ \t]
LET	[A-Za-z0-9.*]
NAME	[A-Za-z0-9.*=_/-]
%%
^{WS}*server{WS}+{LET}{NAME}*{WS}*$ 	{ 
					    /* 
					     * 0 == use current server to find
					     *	    the new one.
					     * 1 == use original server to find
					     *	    the new one.
					     */
					    SetDefaultServer(yytext, 0); 
					    return(1);
					}
^{WS}*lserver{WS}+{LET}{NAME}*{WS}*$ 	{ 
					    SetDefaultServer(yytext, 1); 
					    return(1);
					}
^{WS}*root{WS}*$ 			{ 
					    SetDefaultServer(rootServerName, 1);
					    return(1);
					}
^{WS}*finger({WS}+{LET}{NAME}*)?{WS}+>>?{WS}*{NAME}+{WS}*$ 	{
					    /* 
					     * 2nd arg. 
					     *  0 == output to stdout
					     *  1 == output to file
					     */
					    Finger(yytext, 1); 
					    return(1);
					}
^{WS}*finger({WS}+{LET}{NAME}*)?{WS}*$ 	{ 
					    Finger(yytext, 0); 
					    return(1);
					}
^{WS}*view{WS}+{NAME}+{WS}*$ 	{ 
					    ViewList(yytext); 
					    return(1);
					}
^{WS}*ls{WS}+(("-a"|"-h"|"-m"|"-s"|"-d"){WS}+)?{LET}{NAME}*{WS}+>>?{WS}+{NAME}+{WS}*$	{ 
  					    /* 
  					     * 2nd arg. 
  					     *  0 == output to stdout
					     *  1 == output to file
					     */
					    ListHosts(yytext, 1);
					    return(1);
					}
^{WS}*ls{WS}+(("-a"|"-h"|"-m"|"-s"|"-d"){WS}+)?{LET}{NAME}*{WS}*$ 	{ 
					    ListHosts(yytext, 0);
					    return(1);
					}
^{WS}*set{WS}+{NAME}+{WS}*$	 	{ 
					    SetOption(yytext); 
					    return(1);
					}
^{WS}*help{WS}*$ 			{ 
					    extern void PrintHelp();

					    PrintHelp();
					    return(1);
					}
^{WS}*"?"{WS}*$ 			{ 
					    extern void PrintHelp();

					    PrintHelp();
					    return(1);
					}
^{WS}*exit{WS}*$ 			{ 
					    return(0);
					}
^{WS}*opt(ions)?{WS}*$ 			{ 
					    ShowOptions(TRUE); 
					    return(1);
					}
^{WS}*{LET}{NAME}*{WS}+>>?{WS}*{NAME}+{WS}*$ 	{
					    /* 
					     * 0 == output to stdout
					     * 1 == output to file
					     */
					    LookupHost(yytext, 1); 
					    return(1);
					}
^{WS}*{LET}{NAME}*{WS}*$		{
					    LookupHost(yytext, 0); 
					    return(1);
					}
^{WS}*{LET}{NAME}*{WS}+{LET}{NAME}*{WS}+>>?{WS}*{NAME}+{WS}*$ 	{
					    /* 
					     * 0 == output to stdout
					     * 1 == output to file
					     */
					    LookupHostWithServer(yytext, 1); 
					    return(1);
					}
^{WS}*{LET}{NAME}*{WS}+{LET}{NAME}*{WS}*$	{
					    LookupHostWithServer(yytext, 0); 
					    return(1);
					}
^{WS}*\n				{ 
					    return(1);
					}
^.*\n					{ 
					    printf("Unrecognized command: %s", 
					    		yytext); 
					    return(1);
					}
\n					{ ; }
%%
