/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)head.sys:sys/recover.h	11.1.6.1"

extern struct rd_user * cr_rduser();
extern void del_rduser();
extern void clean_GEN_rd();
extern void cleanup();


/* opcodes */
#define REC_DISCONN	1	/* circuit gone */
#define REC_KILL	2	/* exit */
#define REC_FUMOUNT	3	/* forced unmount */
#define REC_MSG		4 

/* rfdaemon msgflag flags */
#define  MORE_SERVE	0x001
#define  DISCONN	0x002
#define  RFSKILL	0x004
#define  FUMOUNT	0x008
#define  DEADLOCK	0x010

/* active general and specific RDs */
#define ACTIVE_GRD(R) 	((R->rd_stat & RDUSED) && \
	    		(R->rd_qtype & GENERAL) && (R->rd_user_list))
#define ACTIVE_SRD(R)   ((R->rd_stat & RDUSED) && \
		         (R->rd_user_list != NULL) && \
		         (R->rd_qtype & SPECIFIC))

/* This is the structure that gets passed to the user daemon */
#define ULINESIZ 120	/* want a 128-char buffer */
struct u_d_msg {
	int opcode;
	int count;
	char res_name[ULINESIZ];
};

