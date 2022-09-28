/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libyp:ypdefs.h	1.2.3.1"

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*	PROPRIETARY NOTICE (Combined)
*
* This source code is unpublished proprietary information
* constituting, or derived under license from AT&T's UNIX(r) System V.
* In addition, portions of such source code were derived from Berkeley
* 4.3 BSD under license from the Regents of the University of
* California.
*
*
*
*	Copyright Notice 
*
* Notice of copyright on this source code product does not indicate 
*  publication.
*
*	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
*	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
*          All rights reserved.
*/ 
/*      @(#)ypdefs.h 1.3 88/02/08 SMI      */

/*
 * ypdefs.h
 * Special, internal keys to YP maps.  These keys are used
 * by various maintain functions of the YP and invisible
 * to yp clients.  By definition, any key beginning with yp_prefix is
 * an internal key.
 */

#define USE_YP_PREFIX \
	static char yp_prefix[] = "YP_"; \
	static int  yp_prefix_sz = sizeof (yp_prefix) - 1;

#define USE_YP_MASTER_NAME \
	static char yp_master_name[] = "YP_MASTER_NAME"; \
	static int  yp_master_name_sz = sizeof (yp_master_name) - 1;
#define MAX_MASTER_NAME 256

#define USE_YP_LAST_MODIFIED \
	static char yp_last_modified[] = "YP_LAST_MODIFIED"; \
	static int  yp_last_modified_sz = sizeof (yp_last_modified) - 1;
#define MAX_ASCII_ORDER_NUMBER_LENGTH 10

#define USE_YP_INPUT_FILE \
	static char yp_input_file[] = "YP_INPUT_FILE"; \
	static int  yp_input_file_sz = sizeof (yp_input_file) - 1;

#define USE_YP_OUTPUT_NAME \
	static char yp_output_file[] = "YP_OUTPUT_NAME"; \
	static int  yp_output_file_sz = sizeof (yp_output_file) - 1;

#define USE_YP_DOMAIN_NAME \
	static char yp_domain_name[] = "YP_DOMAIN_NAME"; \
	static int  yp_domain_name_sz = sizeof (yp_domain_name) - 1;

#define USE_YP_SECURE \
	static char yp_secure[] = "YP_SECURE"; \
	static int  yp_secure_sz = sizeof (yp_secure) - 1;

/*
 * Definitions of where the YP servers keep their databases.
 * These are really only implementation details.
 */

#define USE_YPDBPATH \
	static char ypdbpath[] = "/var/yp"; \
	static int  ypdbpath_sz = sizeof (ypdbpath) - 1;

#define USE_DBM \
	static char dbm_dir[] = ".dir"; \
	static char dbm_pag[] = ".pag";

