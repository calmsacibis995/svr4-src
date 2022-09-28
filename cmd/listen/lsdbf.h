/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)listen:lsdbf.h	1.3.10.1"

/*
 * lsdbf.h:	listener data base file defines and structs
 */

#define DBFCOMMENT	'#'		/* end of line comment char	*/
#define DBFWHITESP	" \t"		/* space, tab: white space 	*/
#define DBFTOKENS	" \t:"		/* space, tab, cmnt token seps	*/
#define DBFTOKSEP	':'		/* seps in _pmtab file 		*/

/*
 * defines for flag characters -- used in the dfb_flags field
 *   these are the flags defined by SAF
 */

#define	DBF_UTMP	0x01		/* create a utmp entry for service */
#define DBF_OFF		0x02		/* service is turned off	*/
#define	DBF_UNKNOWN	0x80		/* indicates unkown flag character */

/*
 * arguments to read_dbf
 */

#define DB_INIT		0
#define	DB_REREAD	1

/*
 * service code parameters
 */

#define	DBF_INT_CODE	"1"		/* intermediary proc svc code	*/
#define DBF_SMB_CODE	"2"		/* MS-NET server proc svc code	*/

#define	PRV_ADR_SZ	64		/* size of a private address entry */

/*
 * current database version
 */

#define VERSION	4


/*
 * database structure
 */

typedef struct {
	int	dbf_flags;		/* flags			*/
	int	dbf_sflags;		/* listener-specific flags	*/
	char	*dbf_svc_code;		/* null terminated service code	*/
	char	*dbf_id;		/* user id for server to run as */
	char	*dbf_res1;		/* reserved field		*/
	char	*dbf_res2;		/* reserved field		*/
	char	*dbf_res3;		/* reserved field		*/
	char	*dbf_prv_adr;		/* null terminated private address*/
	char	*dbf_modules;		/* optional modules to push	*/
	char	*dbf_cmd_line;		/* null terminated cmd line	*/
	int	dbf_fd;			/* calls for service come in on */
					/*   this fd -- filled in when	*/
					/*   this entry is bound to net	*/
	int	dbf_maxcon;		/* maximum number of outstanding*/
					/*   connections on this fd	*/
	int	dbf_prognum;		/* program number (RPC only)	*/
	int	dbf_version;		/* version number (RPC only)	*/
} dbf_t;


/*
 * listener-specific flags (dbf_sflags)
 */

#define CFLAG	0x1	/* dbf_cmd_line is a command	*/
#define	PFLAG	0x2	/* dbf_cmd_line is a pipe	*/
#define DFLAG	0x4	/* allocate a dynamic address	*/
