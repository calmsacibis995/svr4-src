/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)attwin:head/agent.h	1.1.2.2"

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#define	A_NEWLAYER	1	/* make a new layer 			*/
#define	A_CURRENT	2	/* make layer process current 		*/
#define	A_DELETE	3	/* delete a layer 			*/
#define	A_TOP		4	/* bring a layer to top 		*/
#define	A_BOTTOM	5	/* put a layer on bottom 		*/
#define	A_MOVE		6	/* move a layer 			*/
#define	A_RESHAPE	7	/* reshape a layer 			*/
#define	A_NEW		8	/* make a new layer and send C_NEW to layers */
#define	A_EXIT		9	/* exit layers program 			*/

/* Leave some room for future mouse operations to be implemented 	*/

#define	A_ROMVERSION	20	/* tell me your rom version, e.g. 8;7;5 */
/***
#define A_STACKSIZE	21	   no longer used but reserve the number
				   to prevent confusing old terminals
***/
#define A_XTPROTO       22      /* tell me what xt protocol type to use */


typedef struct agentPoint {
	short	x;
	short	y;
} agentPoint;

typedef struct agentRectangle {
	agentPoint origin;
	agentPoint corner;
} agentRectangle;

struct agentrect{
	short	command;	/* A_NEWLAYER, A_CURRENT, A_DELETE, etc. */
	short	chan;		/* channel */
	agentRectangle r;	/* rectangle description */
};
