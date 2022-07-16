/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pc586:sys/pc586.h	1.3"

/*	Copyright (c) 1987, 1988, 1989 Intel Corp.		*/
/*	  All Rights Reserved  	*/
/*
 *	INTEL CORPORATION PROPRIETARY INFORMATION
 *
 *	This software is supplied under the terms of a license
 *	agreement or nondisclosure agreement with Intel Corpo-
 *	ration and may not be copied or disclosed except in
 *	accordance with the terms of that agreement.
 */

/* INCLUDES */

#include <sys/types.h>
#include <sys/stream.h>
#include <sys/param.h>
#include <sys/errno.h>
#include <sys/sysmacros.h>
#include <sys/stropts.h>
#include <sys/strstat.h>
#include <sys/lihdr.h>
#include "sys/82586.h"
#include "sys/debug.h"
#include "sys/immu.h"
#include "sys/sysmacros.h"
#include "sys/rtc.h"

#define	MAC_ADD_SIZE	6	/* size of a MAC address */
#define MAX_PACK_SIZE	1500	/* maximum size of an ethernet packet */

/* typedefs that they forgot to put in lihdr.h */

typedef struct  DL_info_req            DL_info_req_t ;
typedef struct  DL_bind_req            DL_bind_req_t ;
typedef struct  DL_unbind_req          DL_unbind_req_t ;
typedef struct  DL_unitdata_req        DL_unitdata_req_t ;
typedef struct  DL_info_ack            DL_info_ack_t ;
typedef struct  DL_bind_ack            DL_bind_ack_t ;
typedef struct  DL_error_ack           DL_error_ack_t ;
typedef struct  DL_ok_ack              DL_ok_ack_t ;
typedef struct  DL_unitdata_ind        DL_unitdata_ind_t;


/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
/*                      BOARD SPECIFIC #DEFINES                               */
/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/


#define OFFSET_NORMMODE 0x3000  /* 0=esi loopback, 1 normal data xfer */
#define OFFSET_CHAN_ATT 0x3002  /* 0=clear 586 channel attention, 1 = set */
#define OFFSET_RESET    0x3004  /* 0=clear 586 h/w reset, 1=set */
#define OFFSET_INT_ENAB 0x3006  /* 0=disable board interrupts 1=enable */
#define OFFSET_16B_XFER 0x3008  /* 0=8bit xfer, 1=16bit xfer (for at) */
#define OFFSET_SYS_TYPE 0x300a  /* 0=pc or pc/xt, 1=at */
#define OFFSET_INT_STAT 0x300c  /* 0=board's interrupt active, 1=inactive */ 
#define OFFSET_ADDR_PROM 0x2000 /* first byte of on-board ethernet id */

#define EXTENDED_ADDRESS 0x20000 /* used when board addr is above 1 meg */
#define OFFSET_SCP      0x7ff6  /* 586's scp points to the iscp */
#define OFFSET_ISCP     0x7fee  /* this points to the system control block */
#define OFFSET_SCB      0x7fde  /* points to rcv frame and command unit areas */
#define OFFSET_RU       0x4000  /* the RAM for frame descriptors, receive
                                   buffer descriptors & rcv buffers is fixed at
                                   0x4000 to 0x7800 on the half-card */
#define OFFSET_RBD	0x4228	/* rbd+rbuf must start on 32 bit boundry */
#define OFFSET_CU       0x7814  /* RAM offset for CBLs,  TBDs, etc  
                                   xmt area is from 0x7814 to 0x7f00 
                                   from 0x7f01 to 0x7fff is scp, iscp, scb */ 
#define OFFSET_TBD      0x7914   /* allows 256 bytes for command block */
#define OFFSET_TBUF     0x7924   /* start of user data to send*/
#define N_FD              25     /* 856 frame descriptor */
#define N_RBD             25     /* 586 rcv buffer descriptor */
#define RCVBUFSIZE       532     /* 532 * 2 = max size used by tcp */ 
#define DL_DEAD         0xffff  /* suppliments 82586.h datalink states */

#define CMD_0           0       /* used on 586 half-card command registers */
#define CMD_1           0xffff  /* the mate of the above */
#define TRUE            1     
#define FALSE           0


/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
/*     CONSTANTS AND STRUCTS SPECIFIC TO THIS DRIVER (BUT NOT THE BOARD)      */
/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/


#define LSAP_FREE 0            /* pc586_open has not allocated this ptype */
#define LSAP_ALLOCATED 1       /* pc586_open has successfully opened this ptype*/
#define LSAP_ATTACHED 2        /* an upstream module rcvs at the given ptype */


#define N_LSAP   32            /* number of ptypes per installable board */


typedef struct { int     maj_dev;   /* mknod and space.c -> pc586init() */
                 int     seated;    /* is the board plugged into AT bus ? */ 
                 int     cmd_timer; /* -1 no cmd (i.e., use pc586wput to restart
                                       586 CU), 0 reset, >0 running */
                 int     board_currently_open; /* ++ ea open and -- ea close */
                 int     board_interrupt; /* PC AT 8259 level */
                 int     round_robin;  /* used by interrupt() get xmt packet */
                 fd_t    *begin_fd, *end_fd; /* keeps tabs of fd for host cpu */
                 rbd_t   *begin_rbd, *end_rbd; /* ditto, see rcv_ & xmt_packet*/
                
		 char    *p_cmd_prom; /* phys addr of cmd regs and enet id */
		 char	 *p_virt_cmd_prom; /* virtual address of above */
		 char	 *p_static_ram; /* phys addr of 16KB static ram */
		 char	 *p_virt_static_ram; /* virtual address of above */

                 unsigned char   mac_add[MAC_ADD_SIZE]; /* enet host id */
               } board_stuff_t;

/* to receive connectionless data, ptype_state must be set to DL_IDLE
   ptype must contain proper ptype and the read q ptr must point to the
   module that will be consuming data on this ptype
*/
typedef struct { int ptype;
                 int ptype_state; 
                 queue_t *read_q;
                 queue_t *write_q;
                 board_stuff_t *p_board_stuff;      } ptype_t ; 

/* this struct contains all information needed to talk to the 586 */
typedef struct { ptype_t ptype[N_LSAP];
                 board_stuff_t board_stuff; } struct586_t ;

/* all accesses to RAM on the pc586 board M*U*S*T be 16 bit accesses.
   therefore the following struct is used to pack and unpack unsigned short 
*/
typedef struct { union     { char       a[2];
                             ushort        b; }  c;        }  pack_ushort_t;


typedef struct { union     { char       a[4];
                             unsigned long b; }  c;        }  pack_ulong_t;


/* since the PC AT has 16 interrupt levels, a table will be built
   with the 586 half-card interrupts in it for quick reference during
   interrupt service routine time. note: 12 entries will be 0xffff the
   other four will be either 1, 2, 3 or 4. 0xffff means no board at this
   level.
*/
/* extern int interrupt_586[16]; -- declared in space.c */
