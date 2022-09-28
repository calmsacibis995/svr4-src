/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ksh:include/test.h	1.2.3.1"
/*
 *	UNIX shell
 *
 *	David Korn
 *	AT&T Bell Laboratories
 *
 */

/*
 *  These are the valid test operators
 */

#define TEST_ARITH	020	/* arithmetic operators */
#define TEST_BINOP	0200	/* binary operator */
#define TEST_PATTERN	040	/* turn off bit for pattern compares */

#define TEST_NE		(TEST_ARITH|9)
#define TEST_EQ		(TEST_ARITH|4)
#define TEST_GE		(TEST_ARITH|5)
#define TEST_GT		(TEST_ARITH|6)
#define TEST_LE		(TEST_ARITH|7)
#define TEST_LT		(TEST_ARITH|8)
#define TEST_OR		(TEST_BINOP|1)
#define TEST_AND	(TEST_BINOP|2)
#define TEST_SNE	(TEST_PATTERN|1)
#define TEST_SEQ	(TEST_PATTERN|13)
#define TEST_PNE	1
#define TEST_PEQ	13
#define TEST_EF		3
#define TEST_NT		10
#define TEST_OT		12
#define TEST_SLT	14
#define TEST_SGT	15
#define TEST_END	8

extern const char		test_unops[];
extern const struct sysnod	test_optable[];
extern const char		e_test[];
extern const char		e_bracket[];
extern const char		e_paren[];
extern const char		e_testop[];
