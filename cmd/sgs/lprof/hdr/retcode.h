/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)lxprof:hdr/retcode.h	1.1"
#define		XOPT		0
#define		LOPT		1

#define 	OK		0
#define		ALL_FAIL	1
#define		AM_FAIL		2
#define		COMP_FAIL	3
#define		COND_FAIL	4
#define		COV_FAIL	5
#define		COVOBJ_FAIL	6
#define		DATA_FAIL	7
#define 	ENV_FAIL	8
#define 	EOD_FAIL	9
#define		EOF_FAIL	10
#define		FULL_FAIL	11
#define		FUNC_FAIL	12
#define		IM_FAIL		13
#define		MRG_FAIL	14
#define		OBJ_FAIL	15
#define		SIZE_FAIL	16
#define		SRC_FAIL	17
#define		SRCF_FAIL	18
#define		SYS_FAIL	19
#define		TIME_FAIL	20
#define		WRITE_FAIL	21

#define 	BUG_FAIL	199

#define		FALSE		0
#define		TRUE		1

#define 	MACH_MASK	0x01
#define		VER_MASK	0x02
#define		EDIT_MASK	0x04
#define		COMP_MASK	0x08



/**********************************/
/*    CAcov_join RETURN CODES     */
/**********************************/


#define		MRG_OK		0
#define		MRG_FA1		1
#define		MRG_FA2		2
#define		MRG_FA3		3
