/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libprof:cov_errs.h	1.2"
#define COV001	"dynamic structure allocation failed"
#define COV002	"covfile objectfile entry read failed"


#define	COV300	"file pointer manipulation attempt failed"
#define COV301	"new covfile write failed"
#define COV302  "covfile header write failed"
#define COV303  "covfile header read failed"
#define COV304  "covfile object file table full"
#define COV305  "unable to open object file"



#define COV350	"Failure returned by _CAhead_compare."
#define COV351	"Invalid code returned by CAobj_match."
#define COV352	"Invalid code returned by _CAobj_compare."
#define	COV353	"Bad code returned by CAor."
#define COV354	"Bad code returned by CAadd_olist."
#define COV355	"Bad code returned by CAadd_flist."
#define COV356	"Bad value returned by _CAtraverse."
#define COV357	"Bad value returned by _CAcomp_covf."
#define COV358	"Bad value returned by _CAclose_covf."
#define COV359	"Bad code returned by _CAfree_list."
#define COV360	"Bad value returned by CAobj_match/_CAobj_compare."
