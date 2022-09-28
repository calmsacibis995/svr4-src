/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ttymon:tmstruct.h	1.4.5.1"

/*
 * /etc/ttydefs structure
 */
struct Gdef {
	char		*g_id;		/* id for modes & speeds 	*/
	char		*g_iflags;	/* initial terminal flags 	*/
	char		*g_fflags;	/* final terminal flags 	*/
	short		g_autobaud;	/* autobaud indicator 		*/
	char		*g_nextid;	/* next id if this speed is wrong */
};

/*
 *	pmtab structure + internal data for ttymon
 */
struct pmtab {
	/* the following fields are from pmtab			*/
	char	*p_tag;		/* port/service tag		*/
	long	p_flags;	/* flags			*/
	char	*p_identity;	/* id for service to run as	*/
	char	*p_res1;	/* reserved field		*/
	char	*p_res2;	/* reserved field		*/
	char	*p_res3;	/* reserved field		*/
	char	*p_device;	/* full path name of device	*/
	long	p_ttyflags;	/* ttyflags			*/
	int	p_count;	/* wait_read count		*/
	char	*p_server;	/* full service cmd line	*/
	int	p_timeout;	/* timeout for input 		*/
	char	*p_ttylabel;	/* ttylabel in /etc/ttydefs	*/
	char	*p_modules;	/* modules to push		*/
	char	*p_prompt;	/* prompt message		*/
	char	*p_dmsg;	/* disable message		*/

	/* the following fields are for ttymon internal use	*/
	int	p_status;	/* status of entry 		*/
	int	p_fd;		/* fd for the open device	*/
	pid_t	p_pid;		/* pid of child on the device 	*/
	int 	p_inservice;	/* service invoked		*/
	int	p_respawn;	/* respawn count in this series */
	long	p_time;		/* start time of a series	*/
	uid_t	p_uid;		/* uid of p_identity		*/
	gid_t	p_gid;		/* gid of p_identity		*/
	char	*p_dir;		/* home dir of p_identity	*/
	struct	pmtab	*p_next;
};

/*
 *	valid flags for p_flags field of pmtab
 */
#define	X_FLAG	0x1	/* port/service disabled 		*/
#define U_FLAG  0x2	/* create utmp entry for the service 	*/

/*
 *	valid flags for p_ttyflags field of pmtab
 */
#define C_FLAG	0x1	/* invoke service on carrier		*/
#define H_FLAG	0x2	/* hangup the line			*/
#define B_FLAG	0x4	/* bi-directional line			*/
#define R_FLAG	0x8	/* do wait_read				*/

/*
 *	autobaud enabled flag
 */
#define A_FLAG	0x10	/* autobaud flag			*/

/*
 *	values for p_status field of pmtab
 */
#define		NOTVALID	0	/* entry is not valid		*/
#define		VALID		1	/* entry is valid		*/
#define		CHANGED		2	/* entry is valid but changed 	*/
#define		GETTY		3	/* entry is for ttymon express	*/

#define	ALLOC_PMTAB \
	((struct pmtab *)calloc((unsigned)1, \
		(unsigned)sizeof(struct pmtab)))

#define	PNULL	((struct pmtab *)NULL)
