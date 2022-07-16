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

#ident	"@(#)mbus:cmd/mbusutils/mb2.d/cci.d/bin.d/main.h	1.3"

typedef struct socket_struct {
	unsigned short hostid;
	unsigned short portid;
} socket_rec;

typedef struct info_struct {
	struct socket_struct socketid;
	unsigned char tid;
	unsigned long msgtyp;
	unsigned long reference;
} mb2_minfo;

	/* Download program type definitions */


	typedef union req_struct {
		unsigned char  			buf[20];
		cci_create_req_rec		create_req;
		cci_free_req_rec		free_req;
		cci_download_req_rec	dnload_req;
		cci_bind_req_rec		bind_req;
		cci_unbind_req_rec		unbind_req;
		cci_attach_req_rec		att_req;
		cci_switch_req_rec		sw_req;
		cci_detach_req_rec		det_req;
		cci_get_server_info_req_rec	srvinfo_req;
		cci_line_disc_list_req_rec	ldl_req;
		cci_line_disc_info_req_rec	ldi_req;
		cci_line_info_req_rec	line_req;
		cci_subch_info_req_rec	subch_req;
		
	} req_rec;

	typedef union reply_struct {
		unsigned char  			buf[20];
		cci_create_resp_rec		create_rep;
		cci_download_resp_rec	dnload_rep;
		cci_bind_resp_rec		bind_rep;
	} reply_rec;
