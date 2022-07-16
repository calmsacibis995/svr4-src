/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1988  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:cmd/mbusutils/mb2.d/cci.d/dl.d/trans.h	1.3"

#ifdef MBIIU 
#define RESET_SLEEP 3000 
#define BUF_SIZE 32000
#define DEFFILE "/etc/default/download"

typedef struct socket_struct {
	unsigned short hostid;
	unsigned short portid;
} socket_rec;

typedef struct opt_struct {
	unsigned long tosendrsvp;
	unsigned long torcvfrag;
	unsigned long fcsendside;
	unsigned long fcrcvside;
} opt_rec;

typedef struct info_struct {
	struct socket_struct socketid;
	opt_rec options;
} mb2u_info;

#endif

#ifdef RMX286
#define stdio.h "stdio.h"
#define BUF_SIZE 0xf000
#define RESET_SLEEP 100 

typedef union socket_struct {
	unsigned long socket;
	unsigned short ids[2];
} socket_rec;

#endif

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef unsigned short TOKEN;

#define BY0(dw)	(BYTE)(dw)
#define BY1(dw) *((BYTE *)(&dw)+1)
#define BY2(dw) *((BYTE *)(&dw)+2)	
#define BY3(dw) *((BYTE *)(&dw)+3)		

	/* Interconnect Space Constants */

#define MY_SLOT 0x1f
#define IC_FW_REC 0x0f
#define PSB_CONTROL_REC 0x06
#define SLOT_ID_OFFSET 0x02
#define IC_FIRST_REC_OFFSET	32
#define IC_LEN_OFFSET 1
#define IC_FW_OFFSET 2
#define IC_LOCK_OFFSET 17
#define IC_EOT 0xff

#define MAX_QUE 32
#define MAX_ARGS 2
#define MAX_SLOT 19
#define MAX_RECUR_LEVEL 17
#define MAX_CHARS 20 

#define LINE_SIZE 80
#define O_RDWR 0600
#define INVALID_ADDR -1

#define DEFAULT_DELAY 5
#define DEFAULT_PORTID 0
#define TRANSPORT_SERVICE 0x02
#define RMX_FLAG 0x04
#define DEST_PORT_ID 0x0a

#define MIN_PORT 0x800
#define NAME_START 3
#define PI_DATA_START 6

	/* Record Types */

#define LIBHEAD 0xa4
#define LHEAD 0x82
#define THEAD 0x80
#define RHEAD 0x6e
#define REGINT 0x70
#define PE_DATA 0x84
#define PI_DATA 0x86
#define EXT_DEF 0x8c
#define FIX_UP 0x9c
#define MOD_END 0x8a

	/* Modend Attribute Types */

#define N_MAIN_N_START 0x00
#define N_MAIN_START 0x40
#define INVALID 0x80
#define MAIN_START 0xc0

	/* Register Init Types */

#define CSIP_REG 0x00
#define SSSP_REG 0x40
#define DS_REG 0x80
#define ES_REG 0xc0

#define OMF86_TYPE	0x00
#define OMF286_TYPE	0x01
#define OMF386_TYPE	0x02
#define COFF386_TYPE	0x03

#define BMOD_HDR_SIZE 75
#define BOOT_LOADABLE_286 0xa2
#define BOOT_LOADABLE_386 0xb2

union overlay {	
	unsigned long long_ptr;
	WORD w[2];
	};

typedef struct port_info {
	unsigned short portid;
	unsigned char type;
	unsigned char reserved;
	unsigned short flags;
} p_info_rec;

typedef struct port_attr {
	unsigned short portid;
	unsigned char type;
	unsigned char reserved1;
	unsigned short que_size;
	unsigned short reserved2;
	unsigned short reserved3;
	unsigned short sink_port;
	unsigned long rem_socket;
	unsigned short buf_pool;
	unsigned short flags;
	unsigned char reserved4;
} p_attr_rec;
