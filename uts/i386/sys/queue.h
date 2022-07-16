/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_QUEUE_H
#define _SYS_QUEUE_H

#ident	"@(#)head.sys:sys/queue.h	11.2.7.1"
/* Express Queue Macros */
#define R_EXP ((RQUEUE *)R_ADDR)->express
#define C_EXP ((CQUEUE *)C_ADDR)->express

/* QUEUE POINTER MACROS */

/* all pointers */
#define R_ALL(Q) ((RQUEUE *)R_ADDR)->queue[Q].p_queues.all
#define C_ALL ((CQUEUE *)C_ADDR)->queue.p_queues.all

/* pointers as a sixteen bit quantity */
#define R_SLOAD(Q) ((RQUEUE *)R_ADDR)->queue[Q].p_queues.bit16.load
#define R_SULOAD(Q) ((RQUEUE *)R_ADDR)->queue[Q].p_queues.bit16.unload

#define C_SLOAD ((CQUEUE *)C_ADDR)->queue.p_queues.bit16.load
#define C_SULOAD ((CQUEUE *)C_ADDR)->queue.p_queues.bit16.unload

/* pointers as an eight bit quantity  */
#define R_BLOAD(Q) ((RQUEUE *)R_ADDR)->queue[Q].p_queues.bit8.unload
#define R_BULOAD(Q) ((RQUEUE *)R_ADDR)->queue[Q].p_queues.bit8.unload

#define C_BLOAD ((CQUEUE *)C_ADDR)->queue.p_queues.bit8.load
#define C_BULOAD ((CQUEUE *)C_ADDR)->queue.p_queues.bit8.unload

/* job entry at load pointer for 16 bit ptrs */
#define Q_RSENTRY(Q) (RENTRY *)(R_SLOAD(Q)+(((RQUEUE *)R_ADDR)->queue[Q].entry))

/* job entry at load pointer for 8 bit ptrs */
#define Q_RBENTRY(Q) (RENTRY *)(R_BLOAD(Q)+(((RQUEUE *)R_ADDR)->queue[Q].entry))

/* job completion at unload pointer for 16 bit ptrs */
#define Q_CSENTRY (CENTRY *)(C_SULOAD+(((CQUEUE *)C_ADDR)->queue.entry))

/* job completion at unload pointer for 8 bit */
#define Q_CBENTRY (CENTRY *)(C_BULOAD+(((CQUEUE *)C_ADDR)->queue.entry))

struct com_entry{
       union{
          struct {
             unsigned bytcnt:16;     /* offset of last byte to transfer  */
                                     /* 0 transfers byte 0               */

#ifdef x86
	     /* this for the Intel side since bit fields are assigned      */
	     /* opposite from WE32000                                    */
             unsigned subdev:6;      /* Subdevice being addressed.       */
             unsigned seqbit:1;      /* flag for block available         */
             unsigned cmd_stat:1;    /* flag for command/status opcode   */
#else
#	ifdef b16
	     /* this for the Intel side since bit fields are assigned      */
	     /* opposite from WE32000                                    */
             unsigned subdev:6;      /* Subdevice being addressed.       */
             unsigned seqbit:1;      /* flag for block available         */
             unsigned cmd_stat:1;    /* flag for command/status opcode   */
#	else
	     /*                this for the m32 side                     */
             unsigned cmd_stat:1;    /* flag for command/status opcode   */
             unsigned seqbit:1;      /* flag for block available         */
             unsigned subdev:6;      /* Subdevice being addressed.       */

#	endif
#endif
             unsigned opcode:8;      /* command or status opcode         */
           }bits;
           struct {
             unsigned short bytcnt;
             char subdev;
             char opcode;
           }bytes;
        }codes;
     	long addr;     /* data or memory address of data       */
};


typedef struct {

     struct com_entry common;
     CAPP appl;		              /* application defined area      	*/
               		   	      /* CAPP for CENTRY	 	*/

}CENTRY;                     /* CENTRY in completion queue		*/


typedef struct {

        /* entry for express requests */
        CENTRY express;

        struct {
                /* Three ways of accessing load and unload ptrs */
                union {
                        /* All pointers at once */
                        long all;

                        /* 16 bit load ptr and 16 bit unload ptr */
                        struct {
                                short load;
                                short unload;
                        }bit16;

                        /* 8 bit load ptr and 8 bit unload ptr */
                        struct {
                                char pad1;
                                char load;
                                char pad2;
                                char unload;
                        }bit8;

                 }p_queues;

                 CENTRY entry[CQSIZE];

        }queue;            /* one for completion queue     */

} CQUEUE;

typedef struct {

     	struct com_entry common;
	RAPP appl;              	/* application defined area	*/
}RENTRY;			/* RENTRY in request queue		*/

typedef struct {

        /* entry for express requests */
        RENTRY express;

        struct {
                /* Three ways of accessing load and unload ptrs */
                union {
                        /* All pointers at once */
                        long all;

                        /* 16 bit load ptr and 16 bit unload ptr */
                        struct {
                                short load;
                                short unload;
                        }bit16;

                        /* 8 bit load ptr and 8 bit unload ptr */
                        struct {
                                char pad1;
                                char load;
                                char pad2;
                                char unload;
                        }bit8;

                 }p_queues;

                 RENTRY entry[RQSIZE];

        }queue[NUM_QUEUES];            /* #defined for request queue	*/

} RQUEUE;

typedef struct {		 /* sysgen data block */
	long request;   /* address of request queue */
	long complt;    /* address of cmplt queue   */
	unsigned char req_size;  /* no entries in req q  */
	unsigned char comp_size; /* no entries in cmplt q*/
	unsigned char int_vect;  /* base intrpt vector   */
	unsigned char no_rque;   /* number of req queues */
}SG_DBLK;

#endif	/* _SYS_QUEUE_H */
