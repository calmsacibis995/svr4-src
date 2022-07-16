/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)head.sys:sys/dma.h	11.2.9.1"

/*      Copyright (c) 1988, 1989 Intel Corp.            */
/*        All Rights Reserved   */
/*
 *      INTEL CORPORATION PROPRIETARY INFORMATION
 *
 *      This software is supplied under the terms of a license
 *      agreement or nondisclosure agreement with Intel Corpo-
 *      ration and may not be copied or disclosed except in
 *      accordance with the terms of that agreement.
 */

#ifndef _SYS_DMA_H
#define	_SYS_DMA_H

#ifndef _SYS_I8237A_H
#include "sys/i8237A.h"
#endif

/* the DMA Status Structure */
struct dma_stat {
        paddr_t         targaddr;      /* physical address of buffer */
        paddr_t         targaddr_hi;   /* more for 64-bit addresses */
        paddr_t         reqraddr;      /* physical address of buffer */
        paddr_t         reqraddr_hi;   /* more for 64-bit addresses */
        unsigned short  count;         /* size of bytes in buffer */
        unsigned short  count_hi;      /* more for big blocks */
};

/* the DMA Buffer Descriptor structure */
struct dma_buf {
        unsigned short  reserved;    /* alignment pad */
        unsigned short   count;      /* size of block */
        paddr_t   address;	     /* phys addr of data block */
        paddr_t   physical;	     /* phys addr of next dma_buf */
        struct dma_buf  *next_buf;   /* next buffer descriptor */
        unsigned short  reserved_hi; /* alignment pad */
        unsigned short  count_hi;    /* for big blocks */
        unsigned long   address_hi;  /* for 64-bit addressing */
        unsigned long   physical_hi; /* for 64-bit addressing */
};

/* the DMA Command Block structure */
struct dma_cb {
        struct dma_cb  *next;       /* free list link */
        struct dma_buf *targbufs;   /* list of target data buffers */
        struct dma_buf *reqrbufs;   /* list of requestor data buffers */
        unsigned char  command;     /* Read/Write/Translate/Verify */
        unsigned char  targ_type;   /* Memory/IO */
        unsigned char  reqr_type;   /* Memory/IO */
        unsigned char  targ_step;   /* Inc/Dec/Hold */
        unsigned char  reqr_step;   /* Inc/Dec/Hold */
        unsigned char  trans_type;  /* Single/Demand/Block/Cascade */
        unsigned char  targ_path;   /* 8/16/32 */
        unsigned char  reqr_path;   /* 8/16/32 */
        unsigned char  cycles;      /* 1 or 2 */
        unsigned char  bufprocess;  /* Single/Chain/Auto-Init */
        unsigned short dummy;           /* alignment pad */
        char           *procparms;  /* parameter buffer for appl call */
        int            (*proc)();   /* address of application call routine */
};

#define DMA_CMD_READ    0x0
#define DMA_CMD_WRITE   0x1
#define DMA_CMD_TRAN    0x2
#define DMA_CMD_VRFY    0x3

#define DMA_TYPE_MEM    0x0
#define DMA_TYPE_IO     0x1

#define DMA_STEP_INC    0x0
#define DMA_STEP_DEC    0x1
#define DMA_STEP_HOLD   0x2

#define DMA_TRANS_SNGL  0x0
#define DMA_TRANS_DMND  0x1
#define DMA_TRANS_BLCK  0x2
#define DMA_TRANS_CSCD  0x3

#define DMA_PATH_8      0x0
#define DMA_PATH_16     0x1
#define DMA_PATH_32     0x2
#define DMA_PATH_64     0x3

/***
** We could use DMA_PATH_64 to mean this but why not
** just put in a separate define.
***/
#define DMA_PATH_16B    0x4     /* 16-bit path but byte count */

#define DMA_CYCLES_1    0x0
#define DMA_CYCLES_2    0x1

/***
** For the EISA bus we will use the following definitions for DMA_CYCLES
**    DMA_CYCLES_1 = Compatible timing
**    DMA_CYCLES_2 = Type "A" timing
**    DMA_CYCLES_3 = Type "B" timing
**    DMA_CYCLES_4 = Burst timing
***/
#define DMA_CYCLES_3    0x2
#define DMA_CYCLES_4    0x3

#define DMA_BUF_SNGL    0x0
#define DMA_BUF_CHAIN   0x1
#define DMA_BUF_AUTO    0x2

#define DMA_SLEEP       0x0
#define DMA_NOSLEEP     0x1

/* some common defined constants */
#ifndef PDMA
#define PDMA 5
#endif

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#ifndef NULL
#define NULL 0
#endif

/* public function routines */
#if defined(__STDC__)
extern void            dma_init(void);
extern void            dma_intr(int);
extern int             dma_prog(struct dma_cb *, int, unsigned char);
extern int             dma_swsetup(struct dma_cb *, int, unsigned char);
extern void            dma_swstart(struct dma_cb *, int, unsigned char);
extern void            dma_stop(int);
extern void            dma_enable(int);
extern void            dma_disable(int);
extern struct dma_cb  *dma_get_cb(unsigned char);
extern void            dma_free_cb(struct dma_cb *);
extern struct dma_buf *dma_get_buf(unsigned char);
extern void            dma_free_buf(struct dma_buf *);
#else
extern void            dma_init();
extern void            dma_intr();
extern int             dma_prog();
extern int             dma_swsetup();
extern unsigned char   dma_get_best_mode();
extern void            dma_swstart();
extern void            dma_stop();
extern void             dma_enable();
extern void            dma_disable();
extern struct dma_cb  *dma_get_cb();
extern void            dma_free_cb();
extern struct dma_buf *dma_get_buf();
extern void            dma_free_buf();
#endif

/* backward compatibility (XENIX)? */
struct dmareq
{   struct dmareq       *d_nxt;         /* reserved */
    unsigned short       d_chan;        /* specifies channel */
    unsigned short       d_mode;        /* direction of transfer */
    paddr_t              d_addr;        /* physical src or dst */
    long                 d_cnt;         /* byte or word (16 bit chan) count */
    int                (*d_proc)();     /* address of dma routine */
    char                *d_params;      /* driver defineable param block */
};

#endif /* _SYS_DMA_H */
