/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamintf:interface/print_err.h	1.4.2.2"
#define S_USAGE		0
#define P_USAGE		1
#define M_USAGE		2
#define U_USAGE		3
#define OAM_LOC		4
#define ENV_PROB	5
#define NO_EXPR		6
#define NOFMLI		7
#define O_USAGE		8
#define NOTUNIQ		9
#define NOT_FOUND	10
#define NO_COMND	11
#define INV_ENT		12
#define P_HOLDER	13
#define INVNAME		14
#define NOT_OPEN	15
#define RENAMED		16
#define INVPATH		17
#define EXPR_SYNTAX	18

#define HAS_STR		O_USAGE		/* string argument above this pos */
#define HAS_TWO		RENAMED		/* two args here and below */
