/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_V86_H
#define _SYS_V86_H

#ident	"@(#)head.sys:sys/v86.h	1.1.3.1"

/*
 * LIM 4.0 changes:
 * Dual-Mode Floating Point support:
 * Selectable OPcode emulation hook:
 * Copyright (c) 1989 Phoenix Technologies Ltd.
 * All Rights Reserved.
*/

#include	<sys/seg.h>

typedef	unsigned char	uchar;

#define NV86TASKS       20              /* Maximum number of V86 tasks  */

#define V86VIRTSIZE     0x0100  /* Size of virtual 8086 in pages ( 1M bytes)*/
#define V86WRAPSIZE     0x0010  /* Size of copy/wrap in pages    (64K bytes)*/
#define V86SIZE         0x0400  /* Max size of v86 segment       ( 4M bytes)*/

/*
 * Definitions for Lotus/Intel/Microsoft Expanded Memory Emulation
*/
#define V86EMM_PGSIZE       0x4000  /* Size of Expanded memory page         */
#define V86EMM_LBASE    0x00200000  /* Expected lowest EMM logical page addr*/
#define V86EMM_LENGTH   0x00200000  /* Expected size EMM logical pages      */

/*
 * LIM 3 definitions -- for backward compatibility.
*/
#define V86EMM_PBASE    0x000D0000  /* EMM physical page address            */
#define V86EMM_PBASE0   0x000D0000  /* EMM physical page address            */
#define V86EMM_PBASE1   0x000D4000  /* EMM physical page address            */
#define V86EMM_PBASE2   0x000D8000  /* EMM physical page address            */
#define V86EMM_PBASE3   0x000DC000  /* EMM physical page address            */
#define V86EMM_NUMPGS	4	    /* # LIM 3 EMM physical pages           */

/*
 *  General definitions for a dual mode process
*/

#define XTSSADDR        ((caddr_t)0x110000L)    /* XTSS addr in user mem*/
#define ARPL            0x63            /* ARPL inst, V86 invalid opcode*/
#define V86_RCS         0xF000          /* Reset CS for V86 mode        */
#define V86_RIP         0xFFF0          /* Reset IP for V86 mode        */
#define V86_DELTA_NICE  10              /* Preferential nice for v86    */
#define V86_TIMER_BOUND 10              /* Pending ticks limit for ECT  */

#define V86_SLICE       25              /* Default timeslice, >1 v86proc*/
#define V86_SLICE_SHIFT 3               /* Max shift for time slice     */

#define	V86_PRI_NORM	0		/* Normal priority state	*/
#define	V86_PRI_LO	1		/* Low priority state, busywait	*/
#define	V86_PRI_HI	2		/* Hi priority state, pseudorupt*/
#define V86_PRI_XHI     3               /* Extra high, urgent pseudorupt*/
#define	V86_SHIFT_NORM	0		/* Timeslice shift for norm pri	*/
#define	V86_SHIFT_LO	V86_SLICE_SHIFT	/* Timeslice shift for lo   pri */
#define	V86_SHIFT_HI	V86_SLICE_SHIFT	/* Timeslice shift for hi   pri */

/*  The maximum number of arguments that can be used for v86() system
**  call is dependent on the maximum number of arguments allowed for
**  the sysi86 call in sysent.c. Since the v86() system call is a sub-
**  function (SI86V86) of the sysi86 call and v86() itself has sub-
**  functions, the maximum arguments for each sub-function of the v86()
**  system call is two less than the maximum allowed for the sysi86
**  system call. The value defined here is assumed in the library.
*/

#define V86SC_MAXARGS   3               /* Includes sub-func # of v86() */

/*  SI86V86 is a sub system call of the system call "sysi86". The following
**  definitions are sub system calls for SI86V86 and are processed in v86.c
*/

#define V86SC_INIT      1               /* v86init() system call        */
#define V86SC_SLEEP     2               /* v86sleep() system call       */
#define V86SC_MEMFUNC   3               /* v86memfunc() system call     */
#define V86SC_IOPL      4               /* v86iopriv () system call     */
/*
**  The V86 timer has a frequency of 18.2 times a second. The Unix
**  timer has a frequency of 100 times a second. So the following
**  value ensures that the ECT gets an interrupt at least as often
**  as it needs one.
*/

#define V86TIMER        5               /* Every 50 ms (20 times a second) */

/*  Software Interrupt mask bit array definitions
*/

#define V86_IMASKSIZE   256             /* Max number of software INTs  */
#define V86_IMASKBITS   ((V86_IMASKSIZE + 31) / 32)
					/* # of bits for software INTs  */

/*  The offsets of members "xt_viflag", "xt_signo" and "xt_hdlr" are
**  hard coded in the file "v86enter.s". So any changes to the struc-
**  that changes these offsets have to be reflected in this file.
**  This file is part of the ECT.
**
**  NOTE: The value of "xt_intr_pin" should be 0xFF or 0.
*/

typedef struct v86xtss
{   struct tss386 xt_tss;               /* Normal TSS structure         */
    unsigned int *xt_vflptr;		/* Ptr to 8086 virtual flags	*/
    unsigned char xt_magictrap;         /* Saved byte of virt intr      */
    unsigned char xt_magicstat;         /* Status of magic byte         */
    unsigned char xt_tslice_shft;       /* Time slice shift requested   */
    unsigned char xt_intr_pin;          /* Interrupt to virtual machine */
    time_t xt_lbolt;                    /* Lightning bolt value         */
    unsigned int xt_viflag;             /* Virtual interrupt flag       */
    unsigned int xt_vimask;             /* Mask for virtual interrupts  */
    unsigned int xt_signo;              /* Sig number on V86VI_SIGHDL   */
    int (* xt_hdlr)();                  /* Sig handler for V86VI_SIGHDL */
    unsigned int xt_timer_count;        /* Number of pending timer ticks*/
    unsigned int xt_timer_bound;        /* Ticks before forcing ECT in  */
    uint xt_imaskbits[V86_IMASKBITS];   /* Bit map for software INTs    */
    unsigned int xt_oldbitmap[32];      /* For I/O bitmap on old ECTs   */
    unsigned char xt_magic[4];          /* XTSS version indicator       */
    unsigned int xt_viurgent;           /* Mask for urgent interrupts   */
    unsigned int xt_vitoect;            /* Mask for interrupts to ECT   */
    ushort xt_vp_relcode;		/* VP/ix release code number    */
    ushort xt_oemcode;			/* OEM code number              */
    ushort xt_op_emul;			/* OPcode emulation enable mask */
    ushort xt_rsvd0;			/* Reserved for future expansion*/
    unsigned int xt_reserved[8];        /* Reserved for future expansion*/
}   xtss_t;

/*
 * VP/ix release code (refers to kernel,
 * but non-driver based, supported functionality).
 *
 * 1 = D-M FP + LIM 4.0
 * 2 = Ver.1 + OPcode emulation xtss hook.
 *
 * set into xt_vp_relcode in vx/v86.c:v86init()
*/
#define	VP_RELCODE	2

/* OPcode emulation bit defines (one bit/OPcode). */
#define EN_VTR		0x0001		/* CGA status port read */

/* DEFINE for CGA status emulation table (allocated in vx/space.c) */
#define CS_MAX		32

/*  Definitions for the field "xt_magicstat". The location "xt_magictrap"
**  is valid only when "xt_magicstat" field is set to XT_MSTAT_OPSAVED.
*/

#define XT_MSTAT_NOPROCESS      0       /* Do not process virtual intr  */
#define XT_MSTAT_PROCESS        1       /* Process virtual intr         */
#define XT_MSTAT_OPSAVED        2       /* v86 program opcode saved     */
#define XT_MSTAT_POSTING        3       /* Phoenix defined              */

struct v86parm
{   xtss_t   *xtssp;                    /* Ptr to XTSS in user data     */
    unsigned long szxtss;               /* Length in bytes of XTSS      */
    unsigned char magic[4];             /* XTSS version indicator       */
    unsigned long szbitmap;             /* Length in bytes of I/O bitmap*/
};

typedef struct v86memory
{   int      vmem_cmd;                  /* Sub command for screen func  */
    paddr_t  vmem_physaddr;             /* Physical memory base for map */
    caddr_t  vmem_membase;              /* Screen memory base           */
    int      vmem_memlen;               /* Length of screen memory      */
} v86memory_t;

/*  Definitions for the field "vmem_cmd".
*/

#define V86MEM_MAP      1               /* Map virt addr to physical    */
#define V86MEM_TRACK    2               /* Track memory modifications   */
#define V86MEM_UNTRACK  3               /* Untrack memory modifications */
#define V86MEM_UNMAP    4               /* Unmap virt addr              */
#define V86MEM_EMM      5               /* LIM expanded memory emulation*/
#define V86MEM_GROW     6               /* Grow Lim expanded memory     */

typedef struct v86dat
{
    proc_t  *vp_procp;                  /* Ptr to process's proc struc  */
    sel_t    vp_oldtr;                  /* Old task register            */
    unsigned short vp_szxtss;           /* Size of XTSS in user space   */
    xtss_t  *vp_xtss;                   /* Address of nailed XTSS       */
    v86memory_t vp_mem;                 /* Memory map/track definitions */
    char     vp_wakeup;                 /* Wakeup process on virt intr  */
    char     vp_slice_shft;             /* Last slice shift from xtss   */
    char     vp_nice;                   /* Requested nice from sys call */
    char     vp_hpnice;                 /* Nice value for high priority */
    char     vp_new_nice;		/* New nice value from v86setint*/
    char     vp_pri_state;		/* Priority state = hi,norm,lo	*/
					/* Virtual86 task fp save area  */
    uchar    vp_fpvalid;		/* Flag if saved state is valid */
    uchar    vp_fpproc;			/* Which task used the floating */
					/* point last.                  */
    union {
    	struct	vfpstate {		/* floating point extension state */
    		int	state[27];	/* 287/387 saved state */
    		int	status;		/* status word saved at exception */
    	} vp_fpstate;
    	struct	vfpemul {		/* FP emulator state */
    		char	fp_emul[246];	/* (extras for emulator) */
    		char	fp_epad[2];
    	} vp_fpemul;
    } vp_fpu;
    struct seg_desc	vp_xtss_desc;	/* Segment descriptor for xtss  */
}   v86_t;

/*
 * vp_fpvalid and vp_fpproc flag defines.
*/
#define	V86FPV_NOTVALID	0x00		/* V86 fp save area is not valid */
#define	V86FPV_VALID	0x01		/* V86 fp save area is valid */
#define	V86FPP_V86LAST	0x01		/* V86 task last to use FPU */
#define	V86FPP_ECTLAST	0x02		/* ECT task last to use FPU */

/*  Virtual interrupt bit definitions for the field "vp_viflag".
**  The low order 16 bits reflect the setting of the AT hardware
**  interrupts. The high order 16 bits are used for other inter-
**  rupts.
**
**  NOTE: The value of V86VI_SIGHDL is hard coded in the file
**  "v86enter.s". Any change to this value has to reflected in
**  this file. This file is part of the ECT.
*/

#define V86VI_NONE      0x00000000      /* No interrupts                */
#define V86VI_TIMER     0x00000001      /* Virtual timer interrupt      */
#define V86VI_KBD       0x00000002      /* Scancode rcvd when buf empty */
#define V86VI_SLAVE     0x00000004      /* Can be reused when needed    */
#define V86VI_SERIAL1   0x00000008      /* Serial port 1 state change   */
#define V86VI_SERIAL0   0x00000010      /* Serial port 0 state change   */
#define V86VI_PRL1      0x00000020      /* Parallel port 1 state change */
#define V86VI_MOUSE     0x00000020      /* Microsoft mouse              */
#define V86VI_DISK      0x00000040      /* Fixed and floppy disk        */
#define V86VI_PRL0      0x00000080      /* Parallel port 0 state change */

#define V86VI_RCLOCK    0x00000100      /* Realtime Clock interrupt     */
#define V86VI_NET       0x00000200      /* Network interrupts           */
#define V86VI_RSVD_1    0x00000400      /* Reserved 1                   */
#define V86VI_RSVD_2    0x00000800      /* Reserved 2                   */
#define V86VI_RSVD_3    0x00001000      /* Reserved 3                   */
#define V86VI_COPROC    0x00002000      /* Coprocessor interrupt        */
#define V86VI_FDISK     0x00004000      /* Fixed disk controller        */
#define V86VI_RSVD_4    0x00008000      /* Reserved 4                   */

#define V86VI_DIV0      0x00010000      /* Divide by 0 (vector 0)       */
#define V86VI_SGLSTP    0x00020000      /* Single step intr (vector 1)  */
#define V86VI_BRKPT     0x00040000      /* Break point intr (vector 3)  */
#define V86VI_OVERFLOW  0x00080000      /* Overflow fault (vector 4)    */
#define V86VI_BOUND     0x00100000      /* Bound exception (vector 5)   */
#define V86VI_INVOP     0x00200000      /* Invalid opcode (vector 6)    */
#define V86VI_SIGHDL    0x00400000      /* Virtual signal hdlr interrupt*/
#define V86VI_MEMORY    0x00800000      /* Tracking memory has changed  */
#define V86VI_LBOLT     0x01000000      /* Lbolt value has changed      */

/* Virtual interrupt subcodes for V86VI_SERIAL[01] interrupts.  These values
** appear in  the fields "xt_s[01]flag".
*/
#define V86SI_DATA      0x00000001      /* New data available from kbd  */
#define V86SI_MODEM     0x00000002      /* Modem status line[s] changed */

#define V86_MAGIC0      'X'             /* Byte for XTSS magic[0]       */
#define V86_MAGIC1      'T'             /* Byte for XTSS magic[1]       */

#endif	/*  _SYS_V86_H */

