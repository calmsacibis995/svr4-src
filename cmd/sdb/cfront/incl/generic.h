/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:cfront/incl/generic.h	1.1"
/*ident	"@(#)cfront:incl/generic.h	1.6" */

#ifndef GENERICH

#define GENERICH 1

/* BSD and SystemV cpp's have different mechanisms for pasting tokens
   together:  worse yet, suns run under BSD but have SYSV mechanism
*/

#if sun	/*System V way: although BSD is true*/
#define name2(a,b)	a/**/b
#define name3(a,b,c)	a/**/b/**/c
#define name4(a,b,c,d)	a/**/b/**/c/**/d
#else
#if  BSD	/*BSD way:*/
#define name2(a,b) a\
b
#define name3(a,b,c) a\
b\
c
#define name4(a,b,c,d) a\
b\
c\
d

#else	/*System V way:*/
#define name2(a,b)	a/**/b
#define name3(a,b,c)	a/**/b/**/c
#define name4(a,b,c,d)	a/**/b/**/c/**/d
#endif
#endif

#define declare(a,t) name2(a,declare)(t)
#define implement(a,t) name2(a,implement)(t)
#define declare2(a,t1,t2) name2(a,declare2)(t1,t2)
#define implement2(a,t1,t2) name2(a,implement2)(t1,t2)


extern genericerror(int,char*);
typedef int (*GPT)(int,char*);
#define set_handler(generic,type,x) name4(set_,type,generic,_handler)(x)
#define errorhandler(generic,type) name3(type,generic,handler)
#define callerror(generic,type,a,b) (*errorhandler(generic,type))(a,b)
#endif
