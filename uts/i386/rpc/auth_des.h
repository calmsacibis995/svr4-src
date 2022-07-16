/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _RPC_AUTHDES_H
#define _RPC_AUTHDES_H

#ident	"@(#)kern-rpc:auth_des.h	1.3"

/*      @(#)auth_des.h 1.7 88/10/25 SMI      */

/*
 *  		PROPRIETARY NOTICE (Combined)
 *  
 *  This source code is unpublished proprietary information
 *  constituting, or derived under license from AT&T's Unix(r) System V.
 *  In addition, portions of such source code were derived from Berkeley
 *  4.3 BSD under license from the Regents of the University of
 *  California.
 *  
 *  
 *  
 *  		Copyright Notice 
 *  
 *  Notice of copyright on this source code product does not indicate 
 *  publication.
 *  
 *  	(c) 1986,1987,1988,1989  Sun Microsystems, Inc.
 *  	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 *  	          All rights reserved.
 */

/*
 * auth_des.h, Protocol for DES style authentication for RPC
 *  
 */

/*
 * There are two kinds of "names": fullnames and nicknames
 */
enum authdes_namekind {
	ADN_FULLNAME, 
	ADN_NICKNAME
};

/*
 * A fullname contains the network name of the client, 
 * a conversation key and the window
 */
struct authdes_fullname {
	char *name;		/* network name of client, up to MAXNETNAMELEN */
	des_block key;		/* conversation key */
	u_long window;		/* associated window */
};


/*
 * A credential 
 */
struct authdes_cred {
	enum authdes_namekind adc_namekind;
	struct authdes_fullname adc_fullname;
	u_long adc_nickname;
};



/*
 * A des authentication verifier 
 */
struct authdes_verf {
	union {
		struct timeval adv_ctime;	/* clear time */
		des_block adv_xtime;		/* crypt time */
	} adv_time_u;
	u_long adv_int_u;
};

/*
 * des authentication verifier: client variety
 *
 * adv_timestamp is the current time.
 * adv_winverf is the credential window + 1.
 * Both are encrypted using the conversation key.
 */
#define adv_timestamp	adv_time_u.adv_ctime
#define adv_xtimestamp	adv_time_u.adv_xtime
#define adv_winverf	adv_int_u

/*
 * des authentication verifier: server variety
 *
 * adv_timeverf is the client's timestamp + client's window
 * adv_nickname is the server's nickname for the client.
 * adv_timeverf is encrypted using the conversation key.
 */
#define adv_timeverf	adv_time_u.adv_ctime
#define adv_xtimeverf	adv_time_u.adv_xtime
#define adv_nickname	adv_int_u

#endif	/* _RPC_AUTHDES_H */
