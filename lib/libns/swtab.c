/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1984, 1986, 1987, 1988, 1989 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)libns:swtab.c	1.3.5.1"

#include "nserve.h"
#include "sys/types.h"
#include "sys/rf_cirmgr.h"
#include "pn.h"

/* these are the indicies into sw_tab.
   note that the orders must match the opcodes */

pntab_t sw_tab[NUMSWENT] = {	"RFV", RF_RF,
				"NSV", RF_NS,
				"RSP", RF_AK 
			   };

/* these are the indicies into du_tab.
   note that the orders must match the opcodes */

pntab_t du_tab[NUMDUENT] = {	"MNT", MNT
			   };
