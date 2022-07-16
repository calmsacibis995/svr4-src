/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:cmd/mbusutils/mb2.d/ics.d/reset.d/reset.h	1.3"

	/* Interconnect Space Constants */

#define BST_LOCAL_RESET 	0x80
#define BST_CMPLT		0x20
#define BST_RMT_PRCL	0x80
#define BST_IDX_SUPPORT	0x20
#define	BST_INPT_PNDNG		0x08
#define	BST_INDATA_VLD		0x01
#define	BST_INDATA_ACPT		0x02
#define	BST_OUTPUT_PNDNG	0x08
#define	BST_OUTDATA_VLD		0x01
#define	BST_OUTDATA_ACPT	0x02

#define MAX_ARGS	1

#define BUF_SIZE	8192

#define RESET_SLEEP 255

/* Execute Test Request Parcel constants */
#define	TstLvl	4
#define	ErrReprtLvl	4
#define	ErrAction	0xff
#define	TstInit	0

/* Parcel Types */
#define ExecTstReqParcel	0x12
#define ExecTstRespParcel	0x13
#define PrmptRdReqParcel	0x1f
#define PadWrtReqParcel		0x1e

#define OK_STATUS	0
#define ERR_STATUS	(-1)

#define PRMPTOFFSET	3

/* Execute Test Request Parcel */
struct x_tst_rq_parcel {
	unsigned char parcel_type;
	unsigned char test_id;
	unsigned char testing_lvl;
	unsigned char error_rep_lvl;
	unsigned char error_action;
	unsigned char test_init;
};

/* Execute Test Response Parcel */
struct x_tst_resp_parcel {
	unsigned char parcel_type;
	unsigned char test_id;
	unsigned short status;
};

#ifndef lint
#pragma pack(1)
#endif

/* Prompt and Read Request Parcel */
struct prmpt_rd_req_parcel {
	unsigned char parcel_type;
	unsigned short length;
};

/* Prompt and Read Response Parcel */
struct prmpt_rd_resp_parcel {
	unsigned char parcel_type;
	unsigned short length;
	unsigned short status;
	char	param[2];
};

/* Pad Write Request Parcel */
struct pad_wrt_req_parcel {
	unsigned char parcel_type;
	unsigned short length;
	unsigned char err_rep_lvl;
	unsigned char this_line;
	unsigned char column;
};

#ifndef lint
#pragma pack()
#endif
