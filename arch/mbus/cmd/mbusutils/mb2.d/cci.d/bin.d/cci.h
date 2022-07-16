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

#ident	"@(#)mbus:cmd/mbusutils/mb2.d/cci.d/bin.d/cci.h	1.3"

			/* CCI Module Constants	*/
#define 	CCI_MSG_SIZE		20
#define		INVALID_MSG_ID 		0
#define		MAX_HOSTS			20
#define		MAX_LINE_DISC		6
#define		MAX_LINES			6
#define		MAX_SUBCHANNELS		6
#define		VERSION_X			0
#define		VERSION_Y			2
	

			/* CCI Message Types */

#define		CCI_CREATE      			0x01
#define		CCI_DOWNLOAD  				0x02
#define		CCI_SET_PARAMS				0x03
#define		CCI_FREE       				0x04
#define		CCI_BIND        			0x10
#define		CCI_UNBIND        	 		0x11
#define		CCI_ATTACH      			0x20
#define		CCI_DETACH        	 		0x21
#define		CCI_SWITCH        	 		0x22
#define		CCI_GET_SERVER_INFO  		0x30
#define		CCI_GET_LINE_DISC_LIST		0x31
#define		CCI_GET_LINE_DISC_INFO		0x32
#define		CCI_GET_LINE_INFO			0x33
#define		CCI_GET_SUBCH_INFO			0x34
#define		CCI_SET_DEBUG_MODE   		0x99

			/* CCI Response Status Codes */

#define		CCI_OK          			0x0
#define		CCI_SWITCHED    			0x1
#define		CCI_LINE_DISC_LOADED 		0x1
#define		CCI_ERROR     				0x80


		/* CCI Create Request Message Structure */
		
typedef	struct	cci_create_req	{
		unsigned char	type;
		unsigned char	reserved1;
		unsigned char	line_disc_id;
	   	unsigned char	reserved2;
		unsigned long	mem_size;
		unsigned char	reserved3[12];
} cci_create_req_rec;

		/* CCI Create Response Message Structure */
		
typedef struct	cci_create_resp	{
		unsigned char		type;
		unsigned char		status;
		unsigned char		line_disc_id;
		unsigned char		reserved1;
	    unsigned long		offset;
	    unsigned long		buf_size;
	    unsigned char		reserved2[8];
		
} cci_create_resp_rec;


typedef union {
	cci_create_req_rec 	req;
	cci_create_resp_rec	rep;
} cci_create_rec;

		/* CCI Download Request Message Structure */
		
typedef struct	cci_download_req {
		unsigned char		type;
		unsigned char		reserved1;
		unsigned char		line_disc_id;
		unsigned char		reserved2[5];
	   	unsigned long		offset;
		unsigned long		psb_addr;
}	cci_download_req_rec;
		

		/* CCI Download Response Message Structure */
		
typedef	struct	cci_download_resp	{
		unsigned char		type;
		unsigned char		status;
		unsigned char		line_disc_id;
	   	unsigned char		reserved1;
	   	unsigned long		offset;
	   	unsigned long		buf_size;
	   	unsigned char		reserved2[8];
}	cci_download_resp_rec;

typedef union {
	cci_download_req_rec 	req;
	cci_download_resp_rec	rep;
} cci_download_rec;

		/* CCI Free Request Message Structure */
		
typedef	struct	cci_free_req {
		unsigned char		type;
		unsigned char		reserved1;
		unsigned char		line_disc_id;
	   	unsigned char		reserved2[17];
} cci_free_req_rec;

		/* CCI Free Response Message Structure */
		
typedef	struct	cci_free_resp	{
		unsigned char		type;
		unsigned char		status;
		unsigned char		line_disc_id;
	   	unsigned char		reserved1[17];
}	cci_free_resp_rec;

typedef union {
	cci_free_req_rec 	req;
	cci_free_resp_rec	rep;
} cci_free_rec;

		/* CCI Bind Request Message Structure */
		
typedef struct	cci_bind_req	{
		unsigned char		type;
		unsigned char		reserved1;
		unsigned char		line_disc_id;
	   	unsigned char		line;
		unsigned char		reserved3[16];
}	cci_bind_req_rec;
		

		/* CCI Bind Response Message Structure */
		
typedef	struct	cci_bind_resp	{
		unsigned char		type;
		unsigned char		status;
		unsigned char		line_disc_id;
	   	unsigned char		line;
		unsigned short		num_subchannels;
		unsigned char		reserved1[14];
}	cci_bind_resp_rec;

typedef union {
	cci_bind_req_rec 	req;
	cci_bind_resp_rec	rep;
} cci_bind_rec;


		/* CCI Unbind Request Message Structure */
		
typedef struct	cci_unbind_req	{
		unsigned char		type;
		unsigned char		reserved1;
		unsigned char		reserved2;
	   	unsigned char		line;
		unsigned char		reserved3[16];
}	cci_unbind_req_rec;
		

		/* CCI Unbind Response Message Structure */
		
typedef struct	cci_unbind_resp {
		unsigned char		type;
		unsigned char		status;
		unsigned char		reserved1;
	   	unsigned char		line;
		unsigned char		reserved2[16];
}	cci_unbind_resp_rec;

typedef union {
	cci_unbind_req_rec 	req;
	cci_unbind_resp_rec	rep;
} cci_unbind_rec;


		/* CCI Attach Request Message Structure */
		
typedef	struct	cci_attach_req	{
		unsigned char		type;
		unsigned char		reserved1[2];
		unsigned char		line;
	   	unsigned short		subchannel;
		unsigned short		def_portid;
		unsigned char		reserved2[12];
}	cci_attach_req_rec;
		

		/* CCI Attach Response Message Structure */
		
typedef struct	cci_attach_resp {
		unsigned char		type;
		unsigned char		status;
		unsigned char		reserved1;
		unsigned char		line;
		unsigned short		subchannel;
		unsigned short		portid;
	   	unsigned char 		session_stat[4];
		unsigned short		prev_host;
		unsigned char		script_info[4];
		unsigned char		reserved3[2];
}	cci_attach_resp_rec;

typedef union {
	cci_attach_req_rec 	req;
	cci_attach_resp_rec	rep;
} cci_attach_rec;

		/* CCI Switch Request Message Structure */
		
typedef struct	cci_switch_req	{
		unsigned char		type;
		unsigned char		reserved1[2];
		unsigned char		line;
	   	unsigned short		subchannel;
		unsigned char		reserved2[2];
		unsigned char		session_stat[4];
		unsigned short		new_host;
		unsigned char		reserved3[6];
}	cci_switch_req_rec;
		

		/* CCI Switch Response Message Structure */
		
typedef struct	cci_switch_resp	{
		unsigned char		type;
		unsigned char		status;
		unsigned char		reserved1;
		unsigned char		line;
		unsigned short		subchannel;
		unsigned char		reserved2[2];
	   	unsigned char		session_stat[4];
		unsigned short		prev_host;
		unsigned char		reserved3[6];

}	cci_switch_resp_rec;

typedef union {
	cci_switch_req_rec 	req;
	cci_switch_resp_rec	rep;
} cci_switch_rec;

		/* CCI Detach Request Message Structure */
		
typedef	struct	cci_detach_req {
		unsigned char		type;
		unsigned char		reserved1[2];
		unsigned char		line;
	   	unsigned short		subchannel;
		unsigned char		reserved2[2];
		unsigned char		session_stat[4];
		unsigned char		reserved3[8];
}	cci_detach_req_rec;
		

		/* CCI Detach Response Message Structure */
		
typedef struct	cci_detach_resp {
		unsigned char		type;
		unsigned char		status;
		unsigned char		reserved1;
		unsigned char		line;
		unsigned short		subchannnel;
		unsigned char		reserved3[14];

} cci_detach_resp_rec;

typedef union {
	cci_detach_req_rec 	req;
	cci_detach_resp_rec	rep;
} cci_detach_rec;


		/* CCI Get Server Info Request Message Structure */

typedef struct	cci_get_server_info_req {
		unsigned char		type;
		unsigned char		reserved1[19];
}	cci_get_server_info_req_rec;
		

		/* CCI Get Server Info Response Message Structure */
		
typedef struct	cci_get_server_info_resp {
		unsigned char		type;
		unsigned char		status;
		unsigned short		num_lines;
		unsigned char		version;
		unsigned char		reserved1[15];
}	cci_get_server_info_resp_rec;

typedef union {
	cci_get_server_info_req_rec 	req;
	cci_get_server_info_resp_rec	rep;
} cci_get_server_info_rec;

		/* CCI Get Line Discipline List Request Message Structure */

typedef struct	cci_line_disc_list_req {
		unsigned char		type;
		unsigned char		reserved1[19];
}	cci_line_disc_list_req_rec;
		

		/* CCI Get Line Discipline List Response Message Structure */
		
typedef struct	cci_line_disc_list_resp {
		unsigned char		type;
		unsigned char		status;
		unsigned char		num_line_disc;
		unsigned char		reserved1[17];
}	cci_line_disc_list_resp_rec;

typedef union {
	cci_line_disc_list_req_rec 	req;
	cci_line_disc_list_resp_rec	rep;
} cci_line_disc_list_rec;

		/* CCI Get Line Discipline Info Request Message Structure */

typedef struct	cci_line_disc_info_req {
		unsigned char		type;
		unsigned char		reserved0;
		unsigned char		line_disc_id;
		unsigned char		reserved1[17];
}	cci_line_disc_info_req_rec;
		

		/* CCI Get Line Discipline Info Response Message Structure */
		
typedef struct	cci_line_disc_info_resp {
		unsigned char		type;
		unsigned char		status;
		unsigned char		line_disc_id;
		unsigned char		state;
		unsigned char		num_hosts;
		unsigned char		reserved0;
		unsigned short		num_lines;
		unsigned char		reserved1[12];
}	cci_line_disc_info_resp_rec;

typedef union {
	cci_line_disc_info_req_rec 	req;
	cci_line_disc_info_resp_rec	rep;
} cci_line_disc_info_rec;

		/* CCI Get Line Info Request Message Structure */

typedef struct	cci_line_info_req {
		unsigned char		type;
		unsigned char		reserved0;
		unsigned char		line;
		unsigned char		reserved1[17];
}	cci_line_info_req_rec;
		

		/* CCI Get Line Info Response Message Structure */
		
typedef struct	cci_line_info_resp {
		unsigned char		type;
		unsigned char		status;
		unsigned char		line;
		unsigned char		state;
		unsigned char		num_hosts;
		unsigned char		line_disc_id;
		unsigned short		num_subchannels;
		unsigned char		reserved1[12];
}	cci_line_info_resp_rec;

typedef union {
	cci_line_info_req_rec 	req;
	cci_line_info_resp_rec	rep;
} cci_line_info_rec;

		/* CCI Get Subch Info Request Message Structure */

typedef struct	cci_subch_info_req {
		unsigned char		type;
		unsigned char		reserved0;
		unsigned char		line;
		unsigned char		reserved1;
		unsigned short		subchannel;
		unsigned char		reserved2[14];
}	cci_subch_info_req_rec;
		

		/* CCI Get Subch Info Response Message Structure */
		
typedef struct	cci_subch_info_resp {
		unsigned char		type;
		unsigned char		status;
		unsigned char		line;
		unsigned char		reserved0;
		unsigned short		subchannel;
		unsigned char		state;
		unsigned char		num_hosts;
		unsigned short		active_host;
		unsigned short		portid;
		unsigned char		reserved1[8];
}	cci_subch_info_resp_rec;

typedef union {
	cci_subch_info_req_rec 	req;
	cci_subch_info_resp_rec	rep;
} cci_subch_info_rec;

	/* CCI Set Debug Mode Request Message Structure */
		
typedef struct	cci_set_debug_mode_req {
		unsigned char		type;
		unsigned char		reserved1;
		unsigned char		command;
		unsigned char		reserved3[17];
}	cci_set_debug_mode_req_rec;
		

		/* CCI Set Debug Mode Response Message Structure */
		
typedef struct	cci_set_debug_mode_resp {
		unsigned char		type;
		unsigned char		status;
		unsigned char		command;
		unsigned char		unsol_err_count;
		unsigned char		msg_retry_count;
		unsigned char		sol_in_err_count;
		unsigned char		sol_out_err_count;
		unsigned char		init_retry_count;
		unsigned char		bad_prot_msg_count;
		unsigned char		unreported_msg_count;
		unsigned char		dropped_char_count;
		unsigned char		reserved1[8];
}	cci_set_debug_mode_resp_rec;

typedef union {
	cci_set_debug_mode_req_rec 	req;
	cci_set_debug_mode_resp_rec	rep;
} cci_set_debug_mode_rec;
