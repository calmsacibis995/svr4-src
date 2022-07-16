#ident	"@(#)wd.c	1.4	92/03/01	JPB"

/*
 * Module: WD8003
 * Project: System V ViaNet
 *
 *
 *	Copyright (c) 1987, 1988, 1989 by Western Digital Corporation and
 *	Interactive Systems Corporation.
 *	All rights reserved.  Contains confidential information and
 *	trade secrets proprietary to
 *		Western Digital Corporation
 *		2445 McCabe Way
 *		Irvine, California  92714
 *
 *		Interactive Systems Corporation
 *		2401 Colorado Avenue
 *		Santa Monica, California  90404
 */

#ident "$Header: wd.c 2.3 90/06/21 $"

/*
 * Streams driver for WD 8003 Ethernet/Starlan controller
 * Implements an extended version of the AT&T Data Link Provider Interface
 * IEEE 802.2 Class 1 type 1 protocol is implemented and supports
 * receipt and response to XID and TEST but does not generate them
 * otherwise.
 * Ethernet encapsulation is also supported by binding to a SAP greater
 * than 0xFF.
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/immu.h"
#include "sys/stream.h"
#include "sys/stropts.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/dlpi.h"
#include "sys/socket.h"
#include "net/if.h"
#include "net/strioc.h"
#include "sys/inline.h"
#include "sys/kmem.h"

#ifdef MCA_BUS
#include "sys/machenv.h"
#endif

#include "sys/wd.h"
#include "sys/wdhdw.h"

#ifndef V32
#include "sys/cmn_err.h"
#include "sys/systm.h"
#include "sys/cred.h"
#include "sys/ddi.h"
#endif

#define nextsize(len) ((len+64)*2)

/* This define is to make this file compile */
/* will be clarified later */
#define dl_max_idu dl_reserved2

/* AT systems initialize in space.c; PS/2 systems determine info at init time*/

extern int wd_boardcnt;			/* number of boards */
extern int wd_multisize;		/* number of multicast addrs/board */
extern struct wddev wddevs[];		/* queue specific parameters */
extern struct wdparam wdparams[];	/* board specific parameters */
extern struct wdstat wdstats[];		/* board statistics */
#ifdef V4_IFSTAT
extern struct ifstats wdifstats[]; 	/* V.4 TCP wants this */
extern struct ifstats *ifstats;		/* this is system wide ? */
#endif
extern struct wdmaddr wdmultiaddrs[];	/* storage for multicast addrs */

extern unsigned char wdhash();
char 	*wd_getname();

/*
 *  This is for lint.
 */

#define BCOPY(from, to, len) bcopy((caddr_t)(from),(caddr_t)(to),(size_t)(len))
#define BCMP(s1, s2, len) bcmp((char*)(s1),(char*)(s2),(size_t)(len))

char wdcopyright[] = "Copyright 1987, 1988, 1989 Interactive Systems Corporation\
\n\tand Western Digital Corporation\nAll Rights Reserved.";
  /*
   * initialize the necessary data structures for the driver
   * to get called.
   * the hi/low water marks will have to be tuned
   */

static struct module_info minfo[DRVR_INFO_SZ] = {
     ENETM_ID, "wd", 0, WDMAXPKT, WDHIWAT, WDLOWAT,
     ENETM_ID, "wd", 0, WDMAXPKT, WDHIWAT, WDLOWAT,
     ENETM_ID, "wd", 0, WDMAXPKT, WDHIWAT, WDLOWAT,
  };

int wdopen(), wdclose(), wdwput(), wdwsrv(), wdrsrv();
extern wdsched();

/* read queue initial values */
static struct qinit rinit = {
   NULL, wdrsrv, wdopen, wdclose, NULL, &minfo[IQP_RQ], NULL,
};

/* write queue initial values */
static struct qinit winit = {
   wdwput, wdwsrv, NULL, NULL, NULL, &minfo[IQP_WQ], NULL,
};

struct streamtab wdinfo = { &rinit, &winit, NULL, NULL };

#ifdef MCA_BUS
extern struct machenv machenv;
#endif

#if defined(WDDEBUG)
extern int    wd_debug;
#endif

unsigned char wdbroadaddr[LLC_ADDR_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

int net_initialized;		/* keep track of current net state */

#ifndef V32
int wddevflag = D_NEW;		/* V4.0 style driver */
#endif

#define SRC_ALIGN	0
#define DEST_ALIGN	1

extern inb();                           /* i/o in byte routine */
extern void outb();			/* i/o out byte routine */

/*
 * bcmp does a binary compare of string 1 with string 2 and
 * returns 0 for equal and 1 for false.  Only "n" bytes are compared.
 */
#ifdef V32
#ifndef B_ASM
static
bcmp(s1, s2, n)
     unsigned char *s1, *s2;
{
   while (n-- >0)
     if (*s1++ != *s2++)
       return 1;
   return 0;
}

static
bcopy(s1, s2, n)
     unsigned char *s1, *s2;
{
   while (n-- >0)
     *s2++ = *s1++;
}
#else
asm int
bcmp(s1,s2,n)
{
%treg s1,s2;mem n;
   pushl %edi		/// bcmp
   pushl %esi
   pushl s1
   pushl s2
   popl  %edi
   popl  %esi
   movl n, %ecx
   xorl %eax,%eax
   repz
   cmpsb
   setne %al
   popl %esi
   popl %edi
%mem s1,s2,n;
   pushl %edi		/// bcmp
   pushl %esi
   movl  s2,%edi
   movl  s1,%esi
   movl n, %ecx
   xorl %eax,%eax
   repz
   cmpsb
   setne %al
   popl %esi
   popl %edi
}
asm int
bcopy(s1,s2,n)
{
%reg s1, s2;mem n;
   pushl %edi		/bcopy reg/reg
   pushl %esi
   pushl s2
   pushl s1
   popl %esi
   popl %edi
   movl n, %ecx
   rep
   movsb
   popl %esi
   popl %edi
%reg s1;mem s2;mem n;
   pushl %edi		/bcopy reg/mem
   pushl %esi
   pushl s1
   movl  s2,%edi
   popl %esi
   movl n, %ecx
   rep
   movsb
   popl %esi
   popl %edi
%mem s1;reg s2;mem n;
   pushl %edi		/bcopy mem/reg
   pushl %esi
   pushl s2
   movl  s1,%esi
   popl %edi
   movl n, %ecx
   rep
   movsb
   popl %esi
   popl %edi
%mem s1, s2, n;
   pushl %edi		/bcopy mem
   pushl %esi
   movl s1,%esi
   movl s2,%edi
   movl n, %ecx
   incl %ecx
   sarl $1,%ecx
   rep
   movsw
   popl %esi
   popl %edi
}
#endif
#endif

asm ushort
ntohs(val)
{
%reg val;
	movzwl	val,%eax
	rolw	$8,%ax
%mem val;
	movzwl	val,%eax
	rolw	$8,%ax
}

/*
 * This code gets called only if we're running on a PS/2 system. 
 * get_slot looks through the adapters which are present on the bus
 * and sees if a board exists with the adapter id passed to the routine
 * If no board is found, 0 is returned otherwise, the slot number of the
 * board and the POS reg information in pos_regs.
 */

static unsigned
get_slot (id, pos_regs)
unsigned id;	/* Adapter id */
unsigned char 	*pos_regs; /* Returned POS register values */
{
     int 		i,j;
     unsigned char 	slot;
     unsigned char 	id_tmp;
     unsigned 		f_id;
     unsigned		pos_addr;

     slot = 0;	/* Start looking at slot 0 (1) */

     /* Turn on the Adapter setup bit */
     slot |= 0x8;

     for (j = 0; j < 8 ;j++) {	/* Check all 8 slots */
	  outb (ADAP_ENAB,slot);		/* Select adapter by slot */
	  f_id = (unsigned char) inb (POS_1);	/* MSB of Adapter ID in POS_1 */
	  f_id = f_id << 8;
	  id_tmp = inb (POS_0);			/* LSB of ADapter ID in POS_0 */
	  f_id |= id_tmp;

	  if (f_id == id) {
	      /* Now that we found the adapter, get the POS info */
	      pos_addr = POS_0;
	      for (i = 0;i < 8;i++) {
		  pos_regs[i] = inb(pos_addr);
		  pos_addr++;
	      }
	      outb (ADAP_ENAB,0);	/* de_select adapter */
	      return (unsigned) (j);
	  }
	  slot++;
     }
     outb (ADAP_ENAB,0);	/* de_select adapter */
     return 0;			/* No adapter found */
}

/* printether - print ethernet address in compact form */
static char *hex[] = {
   "0","1","2","3","4","5","6","7","8","9","a","b","c","d","e","f"
};
static
printether(addr)
     unsigned char *addr;
{
   int i;

   for (i=0; i < 6; i++, addr++){
      if (i!=0)
	printf(":");
      printf("%s%s", hex[(*addr >> 4)&0xf], hex[*addr&0xf]);
   }
   printf("\n");
}

/*
 * wdinit is called at boot time to do any initialization needed
 * it is not called with any parameters and is called only once.
 * if multiple boards exist, they must get handled at this time.
 */

wdinit()
{
    unsigned char tmp;
    register int i, j;

    unsigned	slot_num;	/* Slot # found for adapter */
    char	pos_regs[8];	/* Array of POS reg data */
    unsigned	option;		/* temp for POS information */
    static unsigned 	adaptid[] = { ETI_ID, STA_ID, WA_ID, WD_ID};
    struct wdparam *wdp = wdparams;

    for (i = 0; i < wd_boardcnt; i++, wdp++) {

    /* check what machine were on and act appropriately */
#ifdef MCA_BUS	/* In 2.0 kernel (Merged product), machenv struct has changed */
	if( sysenvmtp->machflags & MC_BUS ) {	/* micro channel test */
       
	/****************************************************************
 	 *	Changes for PS2 board, This routine needs to look at the
	 *	POS registers to see if a board is present and if so, the
	 *	configuration of the board. These parameters where previously
	 *	set in the module/wd/space.c file.
	 *	
	 *	The procedure is ....  
	 *	Call get_slot() for every possible board type that this driver
	 *	supports. If a match is found, read the POS registers to get 
	 *	configuration for this board.
	 ***************************************************************/
       
	    /* Start of changes for PS2 */
       
	    for (j = 0; j < NUM_ID; j++) {
		if ((slot_num = get_slot (adaptid[j],pos_regs)) != 0)
		    break;
	    }
       
	    if (!slot_num) {
		printf("wd: Board (%d of %d) not present\n",i+1,wd_boardcnt);
		wdp->wd_noboard = 1;
		continue;
	    }
	    /* Now for this slot_num, get the POS register information */
       
	    option = (unsigned) pos_regs[2] & 0xFE;
	    wdp->wd_ioaddr = option << 4;		/* Base I/O address */
	    option = (unsigned) pos_regs[5] & 0x03;	/* IRQ */
	    switch (option) {

	    case 0:
		wdp->wd_int = 3;
		break;
	    case 1:
		wdp->wd_int = 4;
		break;
	    case 2:
		wdp->wd_int = 10;
		break;
	    case 3:
		wdp->wd_int = 15;
		break;
	    }
	    option = (unsigned) (pos_regs[3] & 0xFC);
	    wdp->wd_base = (caddr_t) (option << 12);  /* RAM base address */
       
	    wdp->wd_memsize = PS2_RAMSZ;

	} /* end of PS2 specific code */
#endif
	/* see if given io base is valid by calculating checksum */
	tmp = 0;
	for (j = 0; j < 8; j++)
	    tmp += inb(wdp->wd_ioaddr + UBA_STA + j);

	if (tmp != (unsigned char) 0xFF) {
#ifdef MCA_BUS
	    if( sysenvmtp->machflags & MC_BUS ) 	/* micro channel test */
#endif
		printf("wd: board not found\n");
	    wdp->wd_noboard = 1;
#ifdef MCA_BUS
	    else {
      		printf("wd: Board (%d of %d) not present or ROM checksum bad\n",
			i+1, wd_boardcnt);

		wdp->wd_noboard = 1;
	    }
#endif
	    continue;
	}

	wdp->wd_index = i;			/* board index */
	wdp->wd_init = 0;			/* board initialized */
	wdp->wd_str_open = 0;			/* number of streams open */
	wdp->wd_nextq = -1;			/* next queue to process */
	wdp->wd_ncount = 0;			/* count of bufcalls */
	wdp->wd_proms = 0;			/* number of promiscuous strs */
	wdp->wd_firstd = i * wdp->wd_minors;	/* first minor device */

	/*
	 * setup for multicast addressing
	 */
	wdp->wd_multiaddrs = &wdmultiaddrs[i*wd_multisize];	/* storage */
	wdp->wd_multicnt = 0;			/* no addresses defined */
	wdp->wd_multip = wdp->wd_multiaddrs;	/* most recently referenced
						 * multicast address */

	/* tell kernel about 8003 RAM & get a valid virtual addr for it */
	wdp->wd_rambase = (char *)physmap(wdp->wd_base,
					  wdp->wd_memsize, KM_NOSLEEP);

	/* store local Ethernet/Starlan address */
	for (j = 0; j < LLC_ADDR_LEN; j++)
	    wdp->wd_macaddr[j] = inb(wdp->wd_ioaddr + UBA_STA + j);

	wdp->wd_boardtype = GetBoardID(wdp->wd_ioaddr);
	/* V.4 specific code - actually LAI */
#ifdef V4_IFSTAT
	bzero((char *)&wdifstats[i], sizeof(struct ifstats));
	wdifstats[i].ifs_name = "wd";
	wdifstats[i].ifs_unit = i;
	wdifstats[i].ifs_active = 0; /* not up until wdboard_init */
	wdifstats[i].ifs_next = ifstats;
	wdifstats[i].ifs_mtu = minfo[IQP_WQ].mi_maxpsz;
	ifstats = &wdifstats[i];
#endif
	printf("WD8003(type %d = %s) board %d address: ", wdp->wd_boardtype,
		wd_getname(wdp->wd_boardtype), i);
	printether(wdp->wd_macaddr);
    }

    net_initialized = 0;
    bzero((char*)wddevs, sizeof(struct wddev) * wd_boardcnt);

}

/*
 * wdopen is called by the stream head to open a minor device.
 * CLONEOPEN is supported and opens the lowest numbered minor device
 * which is not currently opened.  Specific device open is allowed to
 * open an already opened stream.
 */

#ifdef V32

int
wdopen(q, dev, flag, sflag)
queue_t *q;

#else

int
wdopen(q, dev, flag, sflag, credp)
queue_t *q;
dev_t *dev;
int flag, sflag;
struct cred *credp;

#endif

{
   struct wddev *wd;
   struct wdparam *wdp = wdparams;
   mblk_t *mp;
   struct stroptions *opts;
   register int i;
   int	oldlevel, rval;
#ifdef V32
   int devmajor;
   int devminor;
#else
   major_t devmajor;
   minor_t devminor;
#endif

#if defined(WDDEBUG)
   if (wd_debug&WDSYSCALL)
#ifdef V32
     cmn_err(CE_CONT, "wdopen(%x, %x, %x, %x)\n", q, dev, flag, sflag);
#else
     cmn_err(CE_CONT, "wdopen(%x, %x, %x, %x, %x)\n", q,*dev,flag,sflag,credp);
#endif
#endif

   /*
    * find the appropriate board by its major device number
    */
#if V32
   devmajor = major(dev);
#else
   devmajor = getemajor(*dev);
#endif
   for (i = wd_boardcnt; i; wdp++, i--) 
	if (wdp->wd_major == devmajor)
	    break;

   if (i == 0 || wdp->wd_noboard) {
      /* fail open()'s if board not present */
#ifdef V32
      u.u_error = ENXIO;
      return OPENFAIL;
#else
      return ECHRNG;
#endif
   }

   oldlevel = splstr();

   /* determine type of open, CLONE or otherwise */
   if (sflag == CLONEOPEN){
      for (devminor=0; devminor < wdp->wd_minors; devminor++)
	if (wddevs[wdp->wd_firstd + devminor].wd_qptr == NULL)
	  break;
   } else 
#ifdef V32
      devminor = minor(dev);
#else
      devminor = geteminor(*dev);
#endif

   /* couldn't find an available stream */
   if (devminor >= wdp->wd_minors){
#if defined(WDDEBUG)
      if (wd_debug&WDSYSCALL)
	printf("wdopen: no devices\n");
#endif
#ifdef V32
      rval = OPENFAIL;
#else
      rval = ECHRNG;
#endif
     goto out1; 
   }

   if (q->q_ptr) {		/* already open, just return it */
#ifdef V32
     rval = (int) devminor;
#else
     *dev = makedevice(devmajor, devminor);
     rval = 0;
#endif
     goto out1;
   }

   wd = &wddevs[wdp->wd_firstd + devminor];
   WR(q)->q_ptr = (caddr_t) wd;
   q->q_ptr = (caddr_t)wd;
   wd->wd_qptr = WR(q);

   /* update stream head for proper flow control */

   if ((mp = allocb(sizeof(struct stroptions), BPRI_MED)) == NULL){
#ifdef V32
     rval = (int) devminor;
#else
     *dev = makedevice(devmajor, devminor);
     rval = 0;
#endif
     goto out1;
   }

   mp->b_datap->db_type = M_SETOPTS;
   opts = (struct stroptions *)mp->b_rptr;
   opts->so_flags = SO_HIWAT | SO_LOWAT | SO_MAXPSZ;
   opts->so_readopt = 0;
   opts->so_wroff   = 0;
   opts->so_minpsz  = 0;
   opts->so_maxpsz  = WDMAXPKT;
   opts->so_hiwat   = WDHIWAT;
   opts->so_lowat   = WDLOWAT;
   mp->b_wptr = mp->b_rptr + sizeof(struct stroptions);
   putnext(q, mp);

   /* initialize the state information needed to support DL interface */

   wd->wd_state = DL_UNBOUND;	/* it starts unbound */
   wd->wd_flags = WDS_OPEN;	/* and open */
   wd->wd_type	= DL_CSMACD;	/* reasonable default */
   wd->wd_no = (short)devminor;
   wd->wd_stats = &wdstats[wdp->wd_index];	/* board specific stats */
   wd->wd_macpar = wdp;		/* board specific parameters */
   wd->wd_flags |=
#ifdef V32
     (u.u_uid == 0 || u.u_ruid == 0) ? WDS_SU : 0; /* might need to know this later */
#else
     drv_priv(credp) ? 0 : WDS_SU;	/* drv_priv() returns 0 if privileged */
#endif

   wdp->wd_str_open++;			/* indicate how many opens */

   /*
    * return minor device of newly opened stream
    */

#if defined(WDDEBUG)
   if (wd_debug&WDSYSCALL)
     printf("\treturn %d\n", dev);
#endif

   /* initialize controller if necessary */
   if (wdp->wd_init == 0){
      wdinit_board(wd);
   }

#ifdef V32
   rval = (int) devminor;
#else
   *dev = makedevice(devmajor, devminor);
   rval = 0;
#endif
out1:
   splx(oldlevel);
   return rval;
}

/*
 * wdclose is called on last close to a stream.
 * it flushes any pending data (assumes higher level protocols handle
 * this properly) and resets state of the minor device to unbound.
 * The last close to the last open wd stream will result in the board
 * being shutdown.
 */

wdclose(q)
     queue_t *q;
{
   register struct wddev *wd;
   register struct wdparam *wdp;
   short cmd_reg;

#if defined(WDDEBUG)
   if (wd_debug&WDSYSCALL)
     printf("wdclose(%x)\n", q);
#endif

   /* flush all queues */
   flushq(q, FLUSHALL);
   flushq(OTHERQ(q), FLUSHALL);

   /* mark as closed */
   wd = (struct wddev *)q->q_ptr;
   wdp = wd->wd_macpar;
   wd->wd_qptr = NULL;
   wd->wd_state = DL_UNBOUND;	/* just in case */
   if (wd->wd_flags&WDS_PROM && --(wdp->wd_proms) <= 0){
      cmd_reg = wdp->wd_ioaddr + 0x10;
      intr_disable();		/* more efficient for this */
      outb(cmd_reg, PAGE_0);
      outb(cmd_reg + RCR, INIT_RCR); /* turn promiscuous mode off */
      wdp->wd_proms = 0; 		/* just to be sure */
      wdp->wd_devmode &= ~WDS_PROM; /* so it will work next time */

      intr_restore();
   }
   wd->wd_flags = 0;		/* nothing pending */

   /* decrement number of open streams, turn board off when none left */

   if (--(wdp->wd_str_open) == 0){
      wduninit_board(wdp);
   }
}

/*
 * wdwput is the "write put procedure"
 * It's purpose is to filter messages to ensure that only
 * M_PROTO/M_PCPROTO and M_IOCTL messages come down from above.
 * Others are discarded while the selected ones are queued to the
 * "write service procedure".
 */

wdwput(q, mp)
     queue_t *q;
     mblk_t  *mp;
{

#if defined(WDDEBUG)
   if (wd_debug&WDPUTPROC)
     printf("wdwput(%x, %x):%x\n", q, mp, mp->b_datap->db_type);
#endif

   switch(mp->b_datap->db_type){
      /* these three are processed in the */
      /* service procedure*/
    case M_IOCTL:
    case M_PROTO:
    case M_PCPROTO:
      putq(q, mp);
      break;
    case M_FLUSH:
      /* Standard M_FLUSH code */
#if defined(WDDEBUG)
      if (wd_debug&WDSRVPROC)
	   printf("wdwput: FLUSH\n");
#endif
      if (*mp->b_rptr & FLUSHW)
   	   flushq(q,FLUSHDATA);
      if (*mp->b_rptr & FLUSHR) {
   	   flushq(RD(q), FLUSHDATA);
   	   *mp->b_rptr &= ~FLUSHW;
   	   qreply (q,mp);
      } else 
   	   freemsg (mp);
      break;
    default:
printf("wd: unknown msg type = %d\n", mp->b_datap->db_type);
      freemsg(mp);		/* discard unknown messages */
   }
}

/*
 * wdwsrv is the "write service procedure"
 * Messages are processed after determining their type.
 * The LI protocol is handled here
 */

wdwsrv(q)
     queue_t *q;
{
   register mblk_t *mp;
   register struct wddev *wd;
   register struct wdparam *wdp;
   int err;

#if defined(WDDEBUG)
   if (wd_debug&WDSRVPROC)
     printf("wdwsrv(%x)\n", q);
#endif

   wd = (struct wddev *)q->q_ptr; /* to get link proto output type */
   wdp = wd->wd_macpar;

   while ((mp = getq(q)) != NULL){
      switch (mp->b_datap->db_type){
       case M_IOCTL:
	 wdioctl(q, mp);
	 break;

	 /* clean up whatever is enqueued */
       case M_FLUSH:
#if defined(WDDEBUG)
	 if (wd_debug&WDSRVPROC)
	   printf("wdwsrv: FLUSH\n");
#endif
	 freemsg(mp);		/* get rid of this one */
	 flushq(q, FLUSHDATA);		/* flush write queue */
	 flushq(OTHERQ(q), FLUSHDATA);		/* flush read queue */
	 break;

	 /* Will be an LI message of some type */
       case M_PROTO:
       case M_PCPROTO:
	 switch (wd->wd_type){
	    /* CSMA/CD is 802.2 (and startup mode) */
	  case DL_CSMACD:
	    /* ETHER is old style (DIX) type encapsulation */
	  case DL_ETHER:
	    if (err=llccmds(q, mp)){
	       int s;

	       /* error condition */
	       if (err == WDE_NOBUFFER){
		  /* quit while we're ahead */
#if defined(WDDEBUG)
		  if (wd_debug&WDDLPRIMERR)
		    printf("llccmds: non-fatal err=%d\n", err);
#endif
		  if (wdp->wd_nextq < 0)
		    wdp->wd_nextq = wd->wd_no;
		  putbq(q, mp); /* to try again, later */
		  s = spl7();
		  if (wdp->wd_ncount++ == 0)
		    bufcall(sizeof(union DL_primitives), BPRI_MED, wdsched, 
			    wd - wd->wd_no);
		  splx(s);
		  return;
	       } else if (err != WDE_OK) {
#if defined(WDDEBUG)
		  if (wd_debug&WDDLPRIMERR)
		    printf("llccmds: err=%d\n", err);
#endif
		  wdnak(q, mp, err);
		  freemsg(mp);
	       }
	    }
	    break;

	    /* No idea what this is, discard */
	  default:
	    /* error, discard */
#if defined(WDDEBUG)
	    if (wd_debug&WDRCVERR)
	      printf("wdwsrv: unknown type(%d) not support\n", wd->wd_type);
#endif
	    freemsg(mp);
	    break;
	 }
	 break;

	 /* This type of message could not have */
	 /* come from upstream, so must be special */
	 /* A later version may allow RAW output from */
	 /* the stream head */
       case M_DATA:

	 /* This is a retry of a previously */
	 /* processed UNITDATA_REQ */

	 if (wdsend(wd, mp)){
	    putbq(q, mp);
	    if (wdp->wd_nextq < 0)
		wdp->wd_nextq = wd->wd_no;
	    return;
	 }
	 freemsg(mp);		/* free on success */
	 break;
	 /* This should never happen */
       default:
printf("wd: unknown msg type(wsrv) = %d\n", mp->b_datap->db_type);
	 freemsg(mp);		/* unknown types are discarded */
	 break;
      }
   }
}

/*
 * wdrsrv - "read queue service" routine
 * called when data is put into the service queue
 * determines additional processing that might need to be done
 */

wdrsrv(q)
     queue_t *q;
{
   mblk_t *mp;
   register struct wddev *wd;

#if defined(WDDEBUG)
   if (wd_debug&WDSRVPROC)
     printf("wdrsrv(%x)\n", q);
#endif

   wd = (struct wddev *)q->q_ptr;		/* get minor device info */
   while ((mp = getq(q)) != NULL){
      /*
       * determine where message goes, then call the proper handler
       */

#if defined(WDDEBUG)
      if (wd_debug&WDDLSTATE)
	printf("wdsrv: state=%d, type=%d\n", wd->wd_state, wd->wd_type);
#endif
      switch (wd->wd_type){
       case DL_CSMACD:		/* an LLC device */
	 if (recv_llc(wd, q, mp) != WDE_OK){
	    freemsg(mp);
#if defined(WDDEBUG)
	    if (wd_debug&WDRECV)
	      printf("dropped LLC\n");
#endif
	 }
	 break;
       case DL_ETHER:		/* old fashioned ethernet */
	 if(recv_ether(wd, q, mp) != WDE_OK){
	    freemsg(mp);
#if defined(WDDEBUG)
	    if (wd_debug&WDRECV)
	      printf("dropped ETHER\n");
#endif
	 }
	 break;
       default:			/* somehow an unknown */
#if defined(WDDEBUG)
	 if (wd_debug&WDRCVERR)
	   printf("wdrsrv: unknown message type %d\n", wd->wd_type);
#endif
	 freemsg(mp);
	 break;
      }
   }
}

/*
 * wdrecv is called by the device interrupt handler to process an
 * incoming packet.  The appropriate link level parser/protocol handler
 * is called if a stream is allocated for that packet
 */

wdrecv(mp,wd)
     mblk_t *mp;
     struct wddev *wd;		/* base of device array for this board */
{
#if defined(WDDEBUG)
   register int msize;
#endif
   register struct wd_machdr *hdr = (struct wd_machdr *)mp->b_rptr;
   register struct wdparam *wdp = wd->wd_macpar;
   int lsap, dsap, msgtype, len_type_field;
   register int i;
   int found, valid;
   mblk_t *nmp;


#if defined(WDDEBUG)
   msize = msgdsize(mp);		/* size of incoming message */

   if (wd_debug&WDRECV){
      printf("wdrecv: machdr=<");
      for (i=0; i<(sizeof(struct wd_machdr)+4); i++)
	printf("%s%x", (i==0)?"":":", *(mp->b_rptr + i));
      printf(">\n");
   }
#endif

   /* length field value determines type of header parsing */

   len_type_field = LLC_LENGTH(mp->b_rptr);
   msgtype = (len_type_field > WDMAXPKT) ? DL_ETHER : DL_CSMACD;

#if defined(WDDEBUG)
   if (wd_debug&WDRECV)
     printf("wdrecv(%x): llc; %d, %d\n", mp, msize, len_type_field);
#endif

   /*
    * if promiscous mode is enabled or multicast addresses are defined,
    * test if really ours or not
    */
   if ((wdp->wd_devmode & WDS_PROM) || (wdp->wd_multicnt > 0)) 
      valid = wdlocal(mp,wd) || wdbroadcast(mp) || wdmulticast(mp,wd);
   else 
      valid = 1;

   for (i=0, found=0; i < wdp->wd_minors; i++, wd++) {

      /* skip an unopened stream or an open but not bound stream */
      if (wd->wd_qptr == NULL || wd->wd_state != DL_IDLE)
	continue;

#if defined(WDDEBUG)
      if (wd_debug&WDTRACE)
	printf("wdrcv: type=%d, sap=%x, dsap=%x\n", wd->wd_type, wd->wd_sap,
	       LLC_DSAP(mp->b_rptr));
#endif

      /* only look at open streams of the correct type */

      if (wd->wd_flags&WDS_PROM){
	 if (!canput(RD(wd->wd_qptr))){
#if defined(WDDEBUG)
	    if (wd_debug&WDBUFFER)
	      printf("wdrecv: canput failed\n");
#endif
	    wd->wd_stats->wds_blocked++;
	    wd->wd_flags |= WDS_RWAIT;
	    continue;
	 }
	 nmp = dupmsg(mp);
#if defined(WDDEBUG)
	 if (wd_debug&WDRECV)
	   printf("wdrecv: queued message to %x (%d)\n", wd->wd_qptr, wd->wd_no);
#endif
	 /* enqueue for the service routine to process */
	 putq(RD(wd->wd_qptr), mp);
	 found++;
	 mp = nmp;
	 if (nmp == NULL)
	   break;
      } else
	if (wd->wd_type != msgtype)
		/* wrong type packet */
		continue;
	else
	if (valid && (wd->wd_type == DL_CSMACD) &&
	    ((wd->wd_sap == LLC_DSAP(mp->b_rptr)) || (LLC_DSAP(mp->b_rptr) == LLC_GLOBAL_SAP))) {
	   /* if no room in the queue, skip this queue */
	   /* this is acceptable since it is the next higher */
	   /* protocol layer that has to deal with it */
	   /* - the LAN could lose it just as well as here - */

	   if (!canput(RD(wd->wd_qptr))){
#if defined(WDDEBUG)
	      if (wd_debug&WDBUFFER)
		printf("wdrecv: canput failed\n");
#endif
	      wd->wd_stats->wds_blocked++;
	      wd->wd_flags |= WDS_RWAIT;
	      continue;
	   }

	   nmp = dupmsg(mp);
#if defined(WDDEBUG)
	   if (wd_debug&WDRECV)
	     printf("wdrecv: queued message to %x (%d)\n", 
		     wd->wd_qptr, wd->wd_no);
#endif
	   /* enqueue for the service routine to process */
	   putq(RD(wd->wd_qptr), mp);
	   found++;
	   mp = nmp;
	   if (nmp == NULL)
	     break;
	} else
	  if (valid && wd->wd_type == DL_ETHER && wd->wd_sap == len_type_field){
	     /* if no room in the queue, skip this queue */
	     /* this is acceptable since it is the next higher */
	     /* protocol layer that has to deal with it */
	     /* - the LAN could lose it just as well as here - */

	     if (!canput(RD(wd->wd_qptr))){
#if defined(WDDEBUG)
		if (wd_debug&WDBUFFER)
		  printf("wdrecv: canput failed\n");
#endif
		wd->wd_flags |= WDS_RWAIT;
		wd->wd_stats->wds_blocked++;
		continue;
	     }
	     nmp = dupmsg(mp);
#if defined(WDDEBUG)
	     if (wd_debug&WDRECV)
	       printf("wdrecv: queued message to %x (%d)\n", 
			wd->wd_qptr, wd->wd_no);
#endif
	     /* enqueue for the service routine to process */
	     putq(RD(wd->wd_qptr), mp);
	     found++;
	     mp = nmp;
	     if (nmp == NULL)
	       break;
	  }
   }
   if (mp != NULL)
     freemsg(mp);
}

/*
 * process receipt of an LLC packet
 * XID and TEST are supported as well as UI
 * Type 2 should be added at some point in the future
 */

static 
recv_llc(wd, q, mp)
     struct wddev *wd;
     queue_t *q;
     mblk_t *mp;
{
   mblk_t *nmp;
   register struct wd_machdr *rcvhdr, *newhdr;
   register union DL_primitives *dlp;
   struct llcc *llccp;
   int ssap, i;

#if defined(WDDEBUG)
   if (wd_debug&(WDLLC1|WDRECV))
     printf("wdrecv: got an LLC packet: %x.%x.%x\n",
	    LLC_DSAP(mp->b_rptr), LLC_SSAP(mp->b_rptr), LLC_CONTROL(mp->b_rptr));
#endif

   /* get some information to simplify processing */
   rcvhdr = (struct wd_machdr *)mp->b_rptr;
   ssap   = LLC_SSAP(rcvhdr);

   /* for now, if this is a RESPONSE PDU, discard it */
   /* since we don't do anything with them.  Proper */
   /* support should be added at some point in the future */
   if (ssap & LLC_RESPONSE){
      freemsg(mp);
      return WDE_OK;
   }

   /* get a message block for new header information */
   /* or conversion to protocol primitive */
   if ((nmp = allocb(sizeof(union DL_primitives) + 2*LLC_LIADDR_LEN,
		     BPRI_MED)) == NULL){
      wd->wd_stats->wds_nobuffer++;
      return WDE_NOBUFFER;
   }
   /* there is now sufficient resources to do what has to be done */

   switch(LLC_CONTROL(rcvhdr)){
    case LLC_UI:
      if (!canput(q->q_next)){
	 /* avoid next queue having small hiwater mark */

	 freemsg(mp);		/* really shouldn't flow control upward */
	 wd->wd_flags |= WDS_RWAIT;
	 wd->wd_stats->wds_blocked2++;
	 freemsg(nmp);

	 return WDE_OK;
      }
      nmp->b_datap->db_type = M_PROTO; /* need to make it control info */

      dlp = (union DL_primitives *)nmp->b_rptr;
      dlp->unitdata_ind.dl_primitive = DL_UNITDATA_IND;
      dlp->unitdata_ind.dl_dest_addr_length = LLC_LIADDR_LEN;
      dlp->unitdata_ind.dl_dest_addr_offset = DL_UNITDATA_IND_SIZE;
      dlp->unitdata_ind.dl_src_addr_length = LLC_LIADDR_LEN;
      dlp->unitdata_ind.dl_src_addr_offset = DL_UNITDATA_IND_SIZE+LLC_LIADDR_LEN;

      /* insert real data for UNITDATA_ind message */
      llccp = (struct llcc *)((caddr_t)dlp+DL_UNITDATA_IND_SIZE);
      BCOPY(rcvhdr->mac_dst, llccp->lbf_addr, LLC_ADDR_LEN);
      llccp->lbf_sap = LLC_DSAP(rcvhdr);
      llccp++;			/* get next position */
      BCOPY(rcvhdr->mac_src, llccp->lbf_addr, LLC_ADDR_LEN);
      llccp->lbf_sap = LLC_SSAP(rcvhdr);
      nmp->b_wptr = nmp->b_rptr + (DL_UNITDATA_IND_SIZE + 2*LLC_LIADDR_LEN);

      mp->b_rptr += LLC_HDR_SIZE;
      if (mp->b_rptr == mp->b_wptr){
	 mblk_t *nullblk;

	 /* get rid of null block */
	 nullblk = mp;
	 mp = unlinkb(nullblk);
	 freeb(nullblk);
      }

#if defined(WDDEBUG)
      if (wd_debug&WDLLC1){
	 printf("recv_llc: data=<");
	 for (i=0; i<10; i++)
	   printf("%s%x", (i==0)?"":":", *(mp->b_rptr+i));
	 printf(">\n");
      }
#endif
      linkb(nmp, mp);
      putnext(q, nmp);
      break;

    case LLC_XID:
    case LLC_XID|LLC_P:
      rcvhdr = (struct wd_machdr *)mp->b_rptr;
      newhdr = (struct wd_machdr *)nmp->b_rptr; /* avoid damaging original */
      /* header in case it went to */
      /* multiple SAPs */
      BCOPY(rcvhdr->mac_src, newhdr->mac_dst, LLC_ADDR_LEN);
      BCOPY(wd->wd_macpar->wd_macaddr, newhdr->mac_src, LLC_ADDR_LEN);
      newhdr->mac_llc.llc.llc_dsap = rcvhdr->mac_llc.llc.llc_ssap;
      newhdr->mac_llc.llc.llc_control =
	rcvhdr->mac_llc.llc.llc_control; /* XID response is same */
      /* as XID request */
      newhdr->mac_llc.llc.llc_ssap = wd->wd_sap|LLC_RESPONSE; /* my sap for reply */
      newhdr->mac_llc.llc.llc_info[0] = LLC_XID_FMTID;
      newhdr->mac_llc.llc.llc_info[1] = LLC_SERVICES; /* services are generic */
      newhdr->mac_llc.llc.llc_info[2] = wd->wd_rws; /* window is SAP specific */
      newhdr->mac_llc.llc.llc_length = LLC_XID_INFO_SIZE+LLC_LSAP_HDR_SIZE;
      nmp->b_wptr = nmp->b_rptr + (LLC_HDR_SIZE+LLC_XID_INFO_SIZE);
      putq(WR(q), nmp);
      freemsg(mp);
      break;
    case LLC_TEST:
    case LLC_TEST|LLC_P:
      rcvhdr = (struct wd_machdr *)mp->b_rptr;
      newhdr = (struct wd_machdr *)nmp->b_rptr; /* avoid damaging original */
      /* header in case it went to */
      /* multiple SAPs */
      BCOPY(rcvhdr->mac_src, newhdr->mac_dst, LLC_ADDR_LEN);
      BCOPY(wd->wd_macpar->wd_macaddr, newhdr->mac_src, LLC_ADDR_LEN);
      newhdr->mac_llc.llc.llc_dsap = rcvhdr->mac_llc.llc.llc_ssap;
      newhdr->mac_llc.llc.llc_control = rcvhdr->mac_llc.llc.llc_control; /* TEST response is same */
      /* as TEST request */
      newhdr->mac_llc.llc.llc_ssap = wd->wd_sap|LLC_RESPONSE; /* my sap for reply */
      newhdr->mac_llc.llc.llc_length = rcvhdr->mac_llc.llc.llc_length;
      nmp->b_wptr = nmp->b_rptr + LLC_HDR_SIZE;
      mp->b_rptr += LLC_HDR_SIZE;
      linkb(nmp, mp);		/* just send back what was received */
#if defined(WDDEBUG)
      if (wd_debug&WDLLC1){
	 printf("recv_llc(test): data=<");
	 for (i=0; i<(nmp->b_wptr-nmp->b_rptr); i++)
	   printf("%s%x", (i==0)?"":":", *(nmp->b_rptr+i));
	 printf(">[%x.%x.%x]\n",LLC_DSAP(nmp->b_rptr),LLC_SSAP(nmp->b_rptr),LLC_CONTROL(nmp->b_rptr));
      }
#endif
      putq(WR(q), nmp);
      break;
    default:
      /* is this a UDERROR? */
      freemsg(mp);
      freemsg(nmp);
      break;
   }
   return WDE_OK;
}


/*
 * process receipt of non-LLC packets (type/length > 1500)
 */

static 
recv_ether(wd, q, mp)
     struct wddev *wd;
     queue_t *q;
     mblk_t *mp;
{
   mblk_t *nmp;			/* for control portion */
   register struct llca *llcp;
   struct wd_machdr *rcvhdr;
   register union DL_primitives *dlp;

#if defined(WDDEBUG)
   if (wd_debug&WDRECV)
     printf("recv_ether: got ether packet type %x\n", ETHER_TYPE(mp->b_rptr));
#endif

   if (canput(q->q_next)){

      /* get a message block for new header information */
      if ((nmp = allocb(sizeof(union DL_primitives) + 2*LLC_ENADDR_LEN,
			BPRI_MED)) == NULL){
	return WDE_NOBUFFER;
      }
      rcvhdr = (struct wd_machdr *)mp->b_rptr;

      nmp->b_datap->db_type = M_PROTO; /* need to make it control info */
      dlp = (union DL_primitives *)nmp->b_rptr;
      dlp->unitdata_ind.dl_primitive = DL_UNITDATA_IND;
      dlp->unitdata_ind.dl_dest_addr_length = LLC_ENADDR_LEN;
      dlp->unitdata_ind.dl_dest_addr_offset = DL_UNITDATA_IND_SIZE;
      dlp->unitdata_ind.dl_src_addr_length = LLC_ENADDR_LEN;
      dlp->unitdata_ind.dl_src_addr_offset = DL_UNITDATA_IND_SIZE+LLC_ENADDR_LEN;

      /* insert real data for UNITDATA_ind message */
      llcp = (struct llca *)(((caddr_t)dlp)+DL_UNITDATA_IND_SIZE);
      BCOPY(rcvhdr->mac_dst, llcp->lbf_addr, LLC_ADDR_LEN);
      llcp->lbf_sap = ETHER_TYPE(rcvhdr);
      llcp++;			/* get next position */
      BCOPY(rcvhdr->mac_src, llcp->lbf_addr, LLC_ADDR_LEN);
      llcp->lbf_sap = ETHER_TYPE(rcvhdr);
      nmp->b_wptr = nmp->b_rptr + (DL_UNITDATA_IND_SIZE + 2*LLC_ENADDR_LEN);

      mp->b_rptr += LLC_EHDR_SIZE;
      if (mp->b_rptr == mp->b_wptr){
	 mblk_t *nullblk;

	 /* get rid of null block */
	 nullblk = mp;
	 mp = unlinkb(nullblk);
	 freeb(nullblk);
      }

      linkb(nmp, mp);
      putnext(q, nmp);
#if defined(WDDEBUG)
      if (wd_debug&WDRECV)
	printf("sent upstream\n");
#endif
   } else {
      freemsg(mp);		/* shouldn't flow control upward */
      wd->wd_stats->wds_blocked2++;
   }
   return WDE_OK;
}

/*
 * process the DL commands as defined in lihdr.h
 */
static 
llccmds(q, mp)
     queue_t *q;
     mblk_t *mp;
{
   mblk_t *nmp;			/* for holding a new message when needed */
   union DL_primitives *dlp;
   int i;

   dlp = (union DL_primitives *)mp->b_rptr;
#if defined(WDDEBUG)
   if (wd_debug&WDDLPRIM)
     printf("llccmds(%x, %x):dlp=%x, dlp->prim_type=%d\n", q, mp, dlp, dlp->dl_primitive);
#endif

   switch ((int) dlp->dl_primitive){
    case DL_BIND_REQ:
#if defined(WDDEBUG)
      if (wd_debug&WDDLPRIM)
	printf("wdbindreq: lsap=%x\n", dlp->bind_req.dl_sap);
#endif
      return wdbind(q, mp);
      break;

    case DL_UNBIND_REQ:
#if defined(WDDEBUG)
      if (wd_debug&WDDLPRIM)
	printf("wdunbindreq:q=%x\n", q);
#endif
      return wdunbind(q, mp);

    case DL_UNITDATA_REQ:
#if defined(WDDEBUG)
      if (wd_debug&WDDLPRIM)
	printf("wdunitdata: %x\n", q);
#endif
      return wdunitdata(q, mp);

    case DL_INFO_REQ:
#if defined(WDDEBUG)
      if (wd_debug&WDDLPRIM)
	printf("wdinforeq\n");
#endif
      return wdinforeq(q, mp);
    case DL_ATTACH_REQ:
    case DL_DETACH_REQ:
    case DL_SUBS_BIND_REQ:
    case DL_UDQOS_REQ:
    case DL_CONNECT_REQ:
    case DL_CONNECT_RES:
    case DL_TOKEN_REQ:
    case DL_DISCONNECT_REQ:
    case DL_RESET_REQ:
    case DL_RESET_RES:
      return DL_NOTSUPPORTED;	/* these are known but unsupported */
    default:
      return DL_BADPRIM;	/* this is really bogus - tell user */
   }
}

/*
 * wdbind determines if a SAP is already allocated and
 * whether it is legal to do the bind at this time
 */

wdbind(q, mp)
     queue_t *q;
     mblk_t *mp;
{
   register int i;
   int sap;
   union DL_primitives *dlp;
   register struct wddev *wd, *twd;
   mblk_t *nmp;

   dlp = (union DL_primitives *)mp->b_rptr;
   wd = (struct wddev *)q->q_ptr;
   sap =  dlp->bind_req.dl_sap;

   if (wd->wd_qptr && wd->wd_state != DL_UNBOUND){
      return DL_BOUND;
   }

   /* not allowed to bind to a group address */
   if (sap <= WDMAXSAPVALUE && sap & LLC_GROUP_ADDR)
     return DL_BADADDR;

   wd->wd_state = DL_BIND_PENDING;

#if defined(WDDEBUG)
   if (wd_debug&WDDLSTATE)
     printf("wdbind: entered DL_BIND_PENDING\n");
#endif

   twd = wd - wd->wd_no;
   for (i=0; i<wd->wd_macpar->wd_minors; i++,twd++) {
      /*
       * exit if any other device for this board is already bound to
       * the same SAP
       */
      if (twd->wd_qptr && twd->wd_state != DL_UNBOUND &&
	  (twd->wd_state != DL_BIND_PENDING || twd->wd_qptr != q) &&
	  twd->wd_sap == sap){

	 wd->wd_state = DL_UNBOUND;
#if defined(WDDEBUG)
	 if (wd_debug&WDDLPRIMERR)
	   printf("wdbind: bind failed\n");
#endif
	 return DL_NOADDR;	/* not a valid bind */
      }
   }

   wd->wd_sap = sap;

   /* need to determine the type of address being used */
   /* assume values >= 0xFF imply some form of LLC and */
   /* values > 0xFF imply ethernet types */

   if (wd->wd_sap <= WDMAXSAPVALUE || wd->wd_sap == RETIX_ISO){
      wd->wd_type = DL_CSMACD;
      wd->wd_sdu = WDMAXPKTLLC;
   } else {
      wd->wd_type = DL_ETHER;
      wd->wd_sdu = WDMAXPKT;
   }

#if defined(WDDEBUG)
   if (wd_debug&WDDLPRIM)
     printf("wdbind: ok - type = %d\n", wd->wd_type);
#endif

   /* ACK the BIND, if possible */

   if ((nmp = allocb(sizeof(union DL_primitives)+sizeof(long), BPRI_MED)) == NULL){
      /* failed to get buffer */
      wd->wd_state = DL_UNBOUND;
      return WDE_NOBUFFER;
   }
   freemsg(mp);			/* done with old */
   /* build the ack */
   nmp->b_datap->db_type = M_PCPROTO; /* acks are PCproto's */

   dlp = (union DL_primitives *)nmp->b_rptr;
   dlp->bind_ack.dl_primitive = DL_BIND_ACK;
   dlp->bind_ack.dl_sap = wd->wd_sap;
   dlp->bind_ack.dl_addr_length = LLC_LSAP_LEN+LLC_ADDR_LEN;
   dlp->bind_ack.dl_addr_offset = DL_BIND_ACK_SIZE;
   BCOPY(wd->wd_macpar->wd_macaddr, 
	 ((caddr_t)dlp)+DL_BIND_ACK_SIZE, LLC_ADDR_LEN);
   BCOPY(&wd->wd_sap, ((caddr_t)dlp)+DL_BIND_ACK_SIZE+LLC_ADDR_LEN, 
							LLC_LSAP_LEN);
   nmp->b_wptr = nmp->b_rptr + DL_BIND_ACK_SIZE + LLC_ADDR_LEN + LLC_LSAP_LEN;

   wd->wd_state = DL_IDLE;	/* bound and ready */

#if defined(WDDEBUG)
   if (wd_debug&WDDLSTATE)
     printf("wdbind: set to idle state\n");
#endif

   qreply(q, nmp);

   return WDE_OK;
}

/*
 * wdunbind performs an unbind of an LSAP or ether type on the stream
 * The stream is still open and can be re-bound
 */

wdunbind(q, mp)
     queue_t *q;
     mblk_t *mp;
{
   struct wddev *wd;
   union DL_primitives *dlp;
   mblk_t *nmp;

   wd = (struct wddev *)q->q_ptr;

   if (wd->wd_state != DL_IDLE){
      return DL_OUTSTATE;
   }
   wd->wd_state = DL_UNBIND_PENDING;
   if ((nmp = allocb(sizeof(union DL_primitives), BPRI_MED)) == NULL){
      /* failed to get buffer */
      wd->wd_state = DL_IDLE;
      return WDE_NOBUFFER;
   }

   freemsg(mp);			/* done with old */
   /* build the ack */
   nmp->b_datap->db_type = M_PCPROTO; /* acks are PCproto's */

   dlp = (union DL_primitives *)nmp->b_rptr;
   dlp->ok_ack.dl_primitive = DL_OK_ACK;
   dlp->ok_ack.dl_correct_primitive = DL_UNBIND_REQ;
   nmp->b_wptr = nmp->b_rptr + DL_OK_ACK_SIZE;
   flushq(q, FLUSHALL);
   flushq(RD(q), FLUSHALL);
   qreply(q, nmp);
   wd->wd_state = DL_UNBOUND;
#if defined(WDDEBUG)
   if (wd_debug&WDDLPRIM)
     printf("wdunbind...\n");
#endif
   return WDE_OK;
}

/*
 * wdunitdata sends a datagram
 *	destination address/lsap is in M_PROTO message (1st message)
 *	data is in remainder of message
 */

wdunitdata(q, mp)
     queue_t *q;
     mblk_t *mp;
{
   struct wddev *wd;
   mblk_t *nmp, *tmp;
   union DL_primitives *dlp;
   register struct wd_machdr *hdr;
   struct llcc *llcp;
   ushort size = msgdsize(mp);	/* need size to check for valid data length */

   wd = (struct wddev *)q->q_ptr;
   if (wd->wd_state != DL_IDLE){
#if defined(WDDEBUG)
      if (wd_debug&WDDLPRIMERR)
	printf("wdunitdata: wrong state (%d)\n", wd->wd_state);
#endif
      return DL_OUTSTATE;
   }

   if (size > wd->wd_sdu)
     return DL_BADDATA;		/* reject packets larger than legal */

   /* make a valid header for transmit */

   if ((nmp = allocb(sizeof(union DL_primitives), BPRI_MED)) == NULL){
      /* failed to get buffer */
      return WDE_NOBUFFER;
   }
   dlp	= (union DL_primitives *)mp->b_rptr;
   hdr = (struct wd_machdr *)nmp->b_rptr;
   BCOPY((caddr_t)dlp+dlp->unitdata_req.dl_dest_addr_offset, hdr->mac_dst, LLC_ADDR_LEN);
   BCOPY(wd->wd_macpar->wd_macaddr, hdr->mac_src, LLC_ADDR_LEN);

   switch (wd->wd_type){
    case DL_CSMACD:		/* LLC1 */
      llcp = (struct llcc *)((caddr_t)dlp+dlp->unitdata_req.dl_dest_addr_offset);
      hdr->mac_llc.llc.llc_length = ntohs(msgdsize(mp) + LLC_LSAP_HDR_SIZE);
      hdr->mac_llc.llc.llc_dsap	  = llcp->lbf_sap;
      hdr->mac_llc.llc.llc_ssap	  = wd->wd_sap;
      hdr->mac_llc.llc.llc_control = LLC_UI; /* connectionless for now */
      nmp->b_wptr = nmp->b_rptr + LLC_HDR_SIZE;
#if defined(WDDEBUG)
      if (wd_debug&WDTRACE){
	 int x;
	 printf("wdunitdata: len=%d, dsap=%x, ssap=%x, ctrl=%x\n\t",
		ntohs(msgdsize(mp)), llcp->lbf_sap, wd->wd_sap, LLC_UI);
	 for(x=0;x<8;x++)printf("%s%x", (x==0)?"":":", llcp->lbf_addr[x]);
	 printf("\n");
      }
#endif
      tmp = rmvb(mp, mp);
      linkb(nmp, tmp);	/* form the new message */
      freeb(mp);			/* don't need the old head any more */
#if defined(WDDEBUG)
      if (wd_debug&WDTRACE)
	printf("llc send: just linked new header: %x %x\n",nmp, tmp);
#endif
      break;

    case DL_ETHER:		/* old ethernet */
      hdr->mac_llc.ether.ether_type = ntohs(wd->wd_sap);
      nmp->b_wptr = nmp->b_rptr + LLC_EHDR_SIZE;
      tmp = rmvb(mp, mp);
      linkb(nmp, tmp);	/* form the new message */
      freeb(mp);			/* don't need the old head any more */
      break;

    default:			/* unknown, send as is */
      break;
   }

   if (wdlocal(nmp,wd)){
      wdloop(nmp,wd);
      return WDE_OK;
   }

   if (wdbroadcast(nmp) || wdmulticast(nmp,wd))
     wdloop(dupmsg(nmp),wd);	/* must dup it if possible */

#if defined(WDDEBUG)
   if (wd_debug&WDDLPRIM)
     printf("about to attempt a send\n");
#endif

   if (wdsend(wd, nmp)){
      if (wd->wd_macpar->wd_nextq < 0)
	/* we can be at head of queue for this board */
	wd->wd_macpar->wd_nextq = wd->wd_no;
      putbq(q, nmp);
      return WDE_OK;		/* this is almost correct, the result */
      				/* is that the wdsend will happen again */
      				/* immediately, get the same result and */
      				/* then back off */
   }

#if defined(WDDEBUG)
   if (wd_debug&WDDLPRIM)
     printf("wdunitdata:...\n");
#endif
   freemsg(nmp);		/* free on success */
   return WDE_OK;
}

/*
 * wdinforeq
 * generate the response to an info request
 */

wdinforeq(q, mp)
     queue_t *q;
     mblk_t  *mp;
{
   struct wddev *wd;
   mblk_t *nmp;
   register union DL_primitives *dlp;

   wd = (struct wddev *)q->q_ptr;

   if ((nmp=allocb(sizeof(union DL_primitives)+(wd->wd_state == DL_BOUND)?16:0,
		   BPRI_MED)) == NULL){
#if defined(WDDEBUG)
      if (wd_debug&WDDLPRIM)
	printf("wdinforeq nobuf...\n");
#endif

      return WDE_NOBUFFER;
   }
   freemsg(mp);
   nmp->b_datap->db_type = M_PCPROTO; /* acks are PCproto's */

   dlp = (union DL_primitives *)nmp->b_rptr;
   dlp->info_ack.dl_primitive = DL_INFO_ACK;
   dlp->info_ack.dl_max_sdu = (wd->wd_type == DL_CSMACD) ? WDMAXPKTLLC : WDMAXPKT;
   dlp->info_ack.dl_min_sdu = 0;
				/* shouldn't this also include SAP info? */
   dlp->info_ack.dl_addr_length = LLC_ADDR_LEN;
   dlp->info_ack.dl_mac_type = wd->wd_type;
   dlp->info_ack.dl_reserved = 0;
   dlp->info_ack.dl_current_state = wd->wd_state;
   dlp->info_ack.dl_max_idu = 0; /* what is this??? */
   dlp->info_ack.dl_service_mode  = DL_CLDLS;
   dlp->info_ack.dl_qos_length = 0; /* No QOS yet */
   dlp->info_ack.dl_qos_offset = 0;
   dlp->info_ack.dl_qos_range_length = 0;
   dlp->info_ack.dl_qos_range_offset = 0;
   dlp->info_ack.dl_provider_style = DL_STYLE1;
   nmp->b_wptr = nmp->b_rptr + DL_INFO_ACK_SIZE;

   if (wd->wd_state != DL_BOUND){
				/*
				 * this condition is for eventual
				 * conversion to Style 2 use where
				 * the board isn't known until attached.
				 */
      dlp->info_ack.dl_addr_offset = 0;
   } else {
      dlp->info_ack.dl_addr_offset = DL_INFO_ACK_SIZE;
      BCOPY(wd->wd_macpar->wd_macaddr, (caddr_t)(dlp+1), LLC_ADDR_LEN);
      nmp->b_wptr += LLC_ADDR_LEN;
				/* What about the bound SAP? */
   }

   dlp->info_ack.dl_growth = 0;
#if defined(WDDEBUG)
   if (wd_debug&WDINFODMP){
      printf("info: type=%d, max=%d, min=%d\n",
	     dlp->info_ack.dl_primitive,
	     dlp->info_ack.dl_max_sdu,
	     dlp->info_ack.dl_min_sdu);
      printf("info: alen = %d, subtype=%d, class=%d, state=%d\n",
	     dlp->info_ack.dl_addr_length,
	     dlp->info_ack.dl_mac_type,
	     dlp->info_ack.dl_service_mode,
	     dlp->info_ack.dl_current_state);
   }
#endif
   qreply(q, nmp);

#if defined(WDDEBUG)
   if (wd_debug&WDDLPRIM)
     printf("wdinforeq...\n");
#endif

   return WDE_OK;
}


/*
 * wdinit_board is called to initialize the 8003 and 8390 hardware.
 * No parameters are passed to it, and it is called only during the
 * first successful call to wdbind.
 */

wdinit_board(wd)
struct wddev *wd;
{
   register int i;
   register int inval;
   register struct wdparam *wdp;
   short ctl_reg, cmd_reg;
   int init_dcr = INIT_DCR;

   wdp = wd->wd_macpar;
   ctl_reg = wdp->wd_ioaddr;
   cmd_reg = ctl_reg + 0x10;

   /* reset the 8003 & program the memory decode bits */
   outb(ctl_reg, SFTRST);
   outb(ctl_reg, 0);
   if ((wdp->wd_boardtype&(BOARD_16BIT|SLOT_16BIT))==(BOARD_16BIT|SLOT_16BIT)) {
	/* if a sixteen bit AT bus board in a sixteen	*/
	/*	bit slot, enable sixteen bit operation	*/
	/*	and include LA bits 23-19		*/
	outb(ctl_reg, (char)(((long) wdp->wd_base >> 13) & 0x3E) + MEMENA);
	outb(ctl_reg + LAAR,
	   	(char)(((long) wdp->wd_base >> 19) & 0x1F)| MEM16ENB| LAN16ENB);
	/* use word transfer mode of 8390 */
	init_dcr |= WTS;
   } else if (wdp->wd_boardtype & BOARD_16BIT) {
	/* if a sixteen bit AT bus board in an eight	*/
	/*	bit slot, can only enable sixteen bit	*/
	/*	LAN operation, but not sixteen bit	*/
	/*	memory operation (include LA19)		*/
	outb(ctl_reg, (char)(((long) wdp->wd_base >> 13) & 0x3E) + MEMENA);
	outb(ctl_reg + LAAR,
	   	(char)(((long) wdp->wd_base >> 19) & 0x01)|LAN16ENB);
	/* use word transfer mode of 8390 */
	init_dcr |= WTS;
   } else
	outb(ctl_reg, (char)(((long) wdp->wd_base >> 13) & 0x3F) + MEMENA);

   /* initialize the 8390 lan controller device */
   inval = inb(cmd_reg);
   outb(cmd_reg, inval & PG_MSK);

	
   /* board transfer mode for PS/2 is different than AT */
#ifdef MCA_BUS	/* In 2.0 kernel (Merged product), machenv struct has changed */
   if( sysenvmtp->machflags & MC_BUS ) 	/* micro channel test */
     init_dcr |= WTS;
#endif
   outb(cmd_reg + DCR, init_dcr);
   outb(cmd_reg + RBCR0, 0);
   outb(cmd_reg + RBCR1, 0);
   outb(cmd_reg + RCR, RCRMON);
   outb(cmd_reg + TCR, INIT_TCR);

   outb(cmd_reg + PSTART, TX_BUF_LEN >> 8);
   outb(cmd_reg + BNRY, TX_BUF_LEN >> 8);
   outb(cmd_reg + PSTOP, wdp->wd_memsize >> 8); /* ??? */

   outb(cmd_reg + ISR, CLR_INT);
   outb(cmd_reg + IMR, INIT_IMR);

   inval = inb(cmd_reg);
   outb(cmd_reg, (inval & PG_MSK) | PAGE_1);
   for (i = 0; i < LLC_ADDR_LEN; i++)
     outb(cmd_reg + PAR0 + i, wdp->wd_macaddr[i]);
   /*
    *  clear the multicast filter bits
    */
   for (i = 0; i < 8; i++)
     outb(cmd_reg + MAR0 + i, 0);

   outb(cmd_reg + CURR, TX_BUF_LEN >> 8);
   wdp->wd_nxtpkt = (unsigned char)((TX_BUF_LEN >> 8));

   outb(cmd_reg, PAGE_0 + ABR + STA);
   outb(cmd_reg + RCR, INIT_RCR);

   wdp->wd_txbuf_busy = 0;

   wdp->wd_init++;
#ifdef V4_IFSTAT
   wdifstats[wdp->wd_index].ifs_active = 1; /* it is active now */
#endif

   /* clear status counters */
   bzero((char *) wd->wd_stats, sizeof(struct wdstat));
   wd->wd_stats->wds_nstats = WDS_NSTATS;	/* no of statistics counters */
}

/*
 * wduninit_board is called by the last call to close. This routine
 * disables the WD8003 board and the 8390 lan controller. No parame-
 * ters are passed to this routine and no values are returned.
 */

wduninit_board(wdp)
struct wdparam *wdp;
{
   outb(wdp->wd_ioaddr, SFTRST);
   outb(wdp->wd_ioaddr, 0);

   /* reinitialize board multicast data structures */
   wdp->wd_multiaddrs = &wdmultiaddrs[wdp->wd_index*wd_multisize];
   wdp->wd_multicnt = 0;
   wdp->wd_multip = wdp->wd_multiaddrs;

   wdp->wd_init = 0;
#ifdef V4_IFSTAT
   wdifstats[wdp->wd_index].ifs_active = 0; /* it is no longer active */
#endif
}

/*
 * wdintr is the interrupt handler for the WD8003. This routine pro-
 * cesses all types of interrupts that can be generated by the 8390
 * LAN controller.
 */

wdintr(irq)
int irq;			/* interrupt level */
{
   register unsigned char int_reg, ts_reg;
   unsigned char cr_reg, orig;
   mblk_t	*bp;
   int call_wdsched = 0;	/* set if scheduler should be called */
   register int inval;
   register struct wdparam *wdp = wdparams;
   register int i;
   unsigned char maxpkt;
   caddr_t rcv_start, rcv_stop;	
   short cmd_reg;

   for (i = wd_boardcnt; i; i--, wdp++) 
       if (wdp->wd_int == irq || (wdp->wd_int == 2 && irq == 9))
	   break;

   if (i == 0) {
       printf("wdintr: irq wrong: %x\n", irq);
       return;
   }

   cmd_reg = wdp->wd_ioaddr + 0x10;
   maxpkt = (wdp->wd_memsize >> 8) - 1;	/* last valid packet */
   rcv_start = wdp->wd_rambase + TX_BUF_LEN;	/* skip past transmit buffer */
   rcv_stop = wdp->wd_rambase + wdp->wd_memsize;	/* want end of memory */

   /* disable interrupts */
   outb(cmd_reg + IMR, NO_INT);

   /* make sure CR is at page 0 */
   orig = inb(cmd_reg);
   outb(cmd_reg, orig & PG_MSK);

   if ((int_reg = inb(cmd_reg + ISR)) == NO_INT){
#if defined(WDDEBUG)
      if (wd_debug&WDINTR)
         printf("wdintr: spurious interrupt\n");
#endif
      return;
   }

   /* mask off bits that will be handled */
   outb(cmd_reg + ISR, int_reg);

#if defined(WDDEBUG)
   if (wd_debug&WDINTR)
     printf("wdintr(): int_reg=%x\n", int_reg);
#endif
   if (int_reg & CNT) {
#if defined(WDDEBUG)
      if (wd_debug&WDINTR)
	printf("CNT\n");
#endif
      wdstats[wdp->wd_index].wds_align += inb(cmd_reg + CNTR0);
      wdstats[wdp->wd_index].wds_crc   += inb(cmd_reg + CNTR1);
      wdstats[wdp->wd_index].wds_lost  += inb(cmd_reg + CNTR2);
#ifdef V4_IFSTAT
      wdifstats[wdp->wd_index].ifs_ierrors =
		    wdstats[wdp->wd_index].wds_align +
		    wdstats[wdp->wd_index].wds_crc   +
		    wdstats[wdp->wd_index].wds_lost;
#endif
   }

   if (int_reg & OVW) {
#if defined(WDDEBUG)
      if (wd_debug&WDINTR)
	printf("OVW\n");
#endif
      wdstats[wdp->wd_index].wds_fifoover++;

      /* issue STOP mode command */
      outb(cmd_reg, PAGE_0 + ABR + STP);

      /* rest of handling done after removing packets from ring	*/
      /*	in PRX handling					*/
   }

   if (int_reg & PRX) {
#if defined(WDDEBUG)
      if (wd_debug&WDINTR)
	printf("PRX: nxtpkt = %x\n", wdp->wd_nxtpkt);
#endif

      inval = inb(cmd_reg);
      outb(cmd_reg, (inval & PG_MSK) | PAGE_1);

      while (wdp->wd_nxtpkt != (unsigned char) inb(cmd_reg + CURR)) {
	 rcv_buf_t *rp, *ram_rp;
	 static rcv_buf_t rbuf;
	 unsigned short length;

	 wdstats[wdp->wd_index].wds_rpkts++;
#ifdef V4_IFSTAT
	 wdifstats[wdp->wd_index].ifs_ipackets++;
#endif

	 /* set up ptr to packet & update nxtpkt */
	 ram_rp = (rcv_buf_t *)
	   (wdp->wd_rambase + (int) (wdp->wd_nxtpkt << 8));
	 wdbcopy((char *) ram_rp, (char *) &rbuf, sizeof(rcv_buf_t),
			SRC_ALIGN);
	 rp = &rbuf;
#if defined(WDDEBUG)
	 if (wd_debug&WDINTR){
	    printf("wdrecv: status=%x, nxtpg=%x, datalen=%d\n",
		   rp->status, rp->nxtpg, rp->datalen);
	 }
#endif
	 if ((wdp->wd_nxtpkt = (unsigned char)(rp->nxtpg)) > maxpkt) {
#if defined(WDDEBUG)
	    printf("Bad nxtpkt value: nxtpkt=%x\n",wdp->wd_nxtpkt);
#endif
	    break;
	 }


	 /* get length of packet w/o CRC field */
	 length = LLC_LENGTH(&rp->pkthdr);
	 if (length > WDMAXPKT) {
		/* DL_ETHER */
		 length = rp->datalen - 4;
	 } else {
		/* DL_CSMACD */
		/* rp->datalen can be wrong (hardware bug) -- use llc length */
		/* the llc length is 18 bytes shorter than datalen... */
		length += 14;
	 }

	 if ( ((int)length > WDMAXPKT+LLC_EHDR_SIZE) || ((int)length < LLC_EHDR_SIZE) ) {
	     /* garbage packet? - toss it */
	     call_wdsched++;
	     /* set CR to page 0 & set BNRY to new value */
	     inval = inb(cmd_reg);
	     outb(cmd_reg, inval & PG_MSK);
	     if ((int)(wdp->wd_nxtpkt-1) < (TX_BUF_LEN>>8))
	       outb(cmd_reg + BNRY, (wdp->wd_memsize>>8)-1);
	     else
	       outb(cmd_reg + BNRY, wdp->wd_nxtpkt-1);
	     break;
	 }

	 /* get buffer to put packet in & move it there */
	 if ((bp = allocb(length, BPRI_MED)) != NULL ||
	     (bp = allocb(nextsize(length), BPRI_MED)) != NULL) {
	    caddr_t dp, cp;
	    unsigned cnt;

	    /* dp -> data dest; ram_rp -> llc hdr */
	    dp = (caddr_t) bp->b_wptr;
	    cp = (caddr_t) &ram_rp->pkthdr;

	    /* set new value for b_wptr */
	    bp->b_wptr = bp->b_rptr + length;

	    /*
	     * See if there is a wraparound. If there
	     * is remove the packet from its start to
	     * rcv_stop, set cp to rcv_start and remove
	     * the rest of the packet. Otherwise, re-
	     * move the entire packet from the given
	     * location.
	     */

	    if (cp + length >= rcv_stop) {
	       /* process a wraparound */
	       cnt = (int)rcv_stop - (int)cp;
	       wdbcopy(cp, dp, cnt, SRC_ALIGN);
	       length -= cnt;
	       cp = rcv_start;
	       dp += cnt;	/* have to move this, too */
	    }
	    wdbcopy(cp, dp, length, SRC_ALIGN);

	    /* Call service routine */
	    wdrecv(bp,&wddevs[wdp->wd_firstd]);

	 } else {
	    /* keep track for possible management */
	    wdstats[wdp->wd_index].wds_nobuffer ++;
#if defined(WDDEBUG)
	    if (wd_debug&WDBUFFER)
	      printf("wdintr: no buffers (%d)\n", length);
#endif
	    call_wdsched++;
	 }	 /* end if */

	 /* set CR to page 0 & set BNRY to new value */
	 inval = inb(cmd_reg);
	 outb(cmd_reg, inval & PG_MSK);
	 if (((int)wdp->wd_nxtpkt-1) < (TX_BUF_LEN>>8))
	   outb(cmd_reg + BNRY, (wdp->wd_memsize>>8)-1);
	 else
	   outb(cmd_reg + BNRY, wdp->wd_nxtpkt-1);

	 inval = inb(cmd_reg);
	 outb(cmd_reg, (inval & PG_MSK) | PAGE_1);

      } /* end while */

   } /* end if PRX int */


   /* restore CR to page 0 */
   inval = inb(cmd_reg);
   outb(cmd_reg, inval & PG_MSK);

   if (int_reg & OVW) {
      /* clear the Remote Byte Counter Registers */
      outb(cmd_reg + RBCR0, 0);
      outb(cmd_reg + RBCR1, 0);

      /* Place NIC in LOOPBACK (mode 1) */
      outb(cmd_reg + TCR, LB0_1);

      /* issue START mode command */
      outb(cmd_reg, PAGE_0 + ABR + STA);

      /* Take NIC out of LOOPBACK */
      outb(cmd_reg + TCR, INIT_TCR);
   }

   if (int_reg & RXE) {
#if defined(WDDEBUG)
      if (wd_debug&WDINTR)
	printf("RXE\n");
#endif
      /* clear network tally counters */
      wdstats[wdp->wd_index].wds_align += inb(cmd_reg + CNTR0);
      wdstats[wdp->wd_index].wds_crc   += inb(cmd_reg + CNTR1);
      wdstats[wdp->wd_index].wds_lost  += inb(cmd_reg + CNTR2);
#ifdef V4_IFSTAT
      wdifstats[wdp->wd_index].ifs_ierrors =
		    wdstats[wdp->wd_index].wds_align +
		    wdstats[wdp->wd_index].wds_crc   +
		    wdstats[wdp->wd_index].wds_lost;
#endif
   }

   if (int_reg & PTX) {
#if defined(WDDEBUG)
      if (wd_debug&WDINTR)
	printf("PTX\n");
#endif

      wdstats[wdp->wd_index].wds_xpkts++;
#ifdef V4_IFSTAT
      wdifstats[wdp->wd_index].ifs_opackets++;
#endif

      /* free the transmit buffer */
      wdp->wd_txbuf_busy = 0;
      call_wdsched++;
   }

   if (int_reg & TXE) {
#if defined(WDDEBUG)
      if (wd_debug&WDINTR)
	printf("TXE\n");
#endif
      

      wdp->wd_txbuf_busy = 0;

      ts_reg = inb(cmd_reg + TPSR);
      if (ts_reg&TSR_COL) {
	int cnt = inb(cmd_reg + TBCR0);
	wdstats[wdp->wd_index].wds_coll += cnt;
#ifdef V4_IFSTAT
	wdifstats[wdp->wd_index].ifs_collisions += cnt;
#endif
      }
      if (ts_reg&TSR_ABT) {
	wdstats[wdp->wd_index].wds_excoll ++;
#ifdef V4_IFSTAT
	wdifstats[wdp->wd_index].ifs_oerrors++;
#endif
      }
      call_wdsched++;
   }

   if (call_wdsched)
     wdsched(&wddevs[wdp->wd_firstd]);		/* reschedule blocked writers */
   /* it should be safe to do this */
   /* here */

   outb(cmd_reg + IMR, INIT_IMR);
   outb(cmd_reg, orig);		/* put things back the way they were found */
}


/*
 * wdsched
 *	this function is called when the interrupt handler
 *	determines that the board is now capable of handling
 *	a transmit buffer.  it scans all queues for a specific board 
 * 	to determine the order in which they should be rescheduled, if at all.
 *	round-robin scheduling is done by default with modifications
 *	to determine which queue was really the first one to attempt
 *	to transmit.  Priority is not considered at present.
 */

wdsched(fwd)
struct wddev *fwd;		/* first device for this board */
{
   register int i, j;
   static int rrval = 0;	/* rrval is used to do pure round-robin */
   register struct wddev *wd;
   register struct wdparam *fwdp;  /* board specific parameters */
   register int sent = 0;

#if defined(WDDEBUG)
   if (wd_debug&WDSCHED)
     printf("wdsched()\n");
#endif

   fwdp = fwd->wd_macpar;
   i = (fwdp->wd_nextq < 0) ? rrval : fwdp->wd_nextq;
   wd = fwd + i;
   for (j=0; j < fwdp->wd_minors; j++){
      if (wd->wd_flags&WDS_XWAIT){
	 mblk_t *mp;
	 qenable(wd->wd_qptr);
	 wd->wd_flags &= ~WDS_XWAIT;
	 fwdp->wd_ncount--;	/* decrement to allow rescheduling */

	 /*
	  * if there is a preformatted output message (M_DATA), then send it
	  * now rather than waiting for the Streams scheduler.  Only do one on
	  * any given pass.
	  */
	 if (!sent && (mp=getq(wd->wd_qptr))){
	    if (mp->b_datap->db_type != M_DATA || wdsend(wd, mp))
	      putbq(wd->wd_qptr, mp);
	    else {
	       freemsg(mp);
	       sent++;
	    }
	 }

      }
      if (wd->wd_flags&WDS_RWAIT){
	 qenable(RD(wd->wd_qptr));
	 wd->wd_flags &= ~WDS_RWAIT;
      }
      i = (i + 1) % fwdp->wd_minors;	/* wrap around */
      if (i == 0)
	  wd = fwd;
      else
	  wd++;
   }
   rrval = (rrval+1) % fwdp->wd_minors;
   fwdp->wd_nextq = -1;
}

/*
 * wdsend is called when a packet is ready to be transmitted. A pointer
 * to a M_PROTO or M_PCPROTO message that contains the packet is passed
 * to this routine as a parameter. The complete LLC header is contained
 * in the message block's control information block, and the remainder
 * of the packet is contained within the M_DATA message blocks linked to
 * the main message block.
 */

wdsend(wd, mb)
     struct wddev *wd;
     mblk_t	*mb;		/* ptr to message block containing packet */
{
   register unsigned int length; /* total length of packet */
   unsigned char 	*txbuf;	/* ptr to transmission buffer area on 8003 */
   register mblk_t	*mp;	/* ptr to M_DATA block containing the packet */
   register int i;
   register struct wdparam *wdp;
   short cmd_reg;
#ifndef V32
   int oldlevel;
#endif

   /* see if the transmission buffer is free */

   if (!can_write(wd)) {
      extern ipl, picipl;
#if defined(WDDEBUG)
      if (wd_debug&WDINTR)
	printf("can't write!\n");
#endif
      /* can't get to board so exit w/ error value */
      return(1);
   }

   wdp = wd->wd_macpar;
   cmd_reg = wdp->wd_ioaddr + 0x10;
   txbuf = (unsigned char *) wdp->wd_rambase;
   length = 0;

   /* load the packet header onto the board */

#if defined(WDDEBUG)
   if (wd_debug&WDBOARD)
     printf("wdsend: wdbcopy(%x, %x, %x) base=%x\n",
	    mb->b_rptr, txbuf, (int)(mb->b_wptr - mb->b_rptr), wdp->wd_rambase);
#endif

   wdbcopy( mb->b_rptr, txbuf, (int)(mb->b_wptr - mb->b_rptr), DEST_ALIGN);
   length += (unsigned int)(mb->b_wptr - mb->b_rptr);

   mp = mb->b_cont;

   /*
    * load the rest of the packet onto the board by chaining through
    * the M_DATA blocks attached to the M_PROTO header. The list
    * of data messages ends when the pointer to the current message
    * block is NULL
    */
   do {

#if defined(WDDEBUG)
      if (wd_debug&WDBOARD)
	printf("wdsend: (2) wdbcopy(%x, %x, %x) base=%x\n",
	       mp->b_rptr, txbuf+length, (int)(mp->b_wptr - mp->b_rptr), 
	       wdp->wd_rambase);
#endif

      wdbcopy( mp->b_rptr, txbuf + length,
	    (int)(mp->b_wptr - mp->b_rptr), DEST_ALIGN);
      length += (unsigned int)(mp->b_wptr - mp->b_rptr);
      mp = mp->b_cont;
   } while (mp != NULL);

#if defined(WDDEBUG)
   if (wd_debug&WDSEND){
      printf("wdsend: machdr=<");
      for (i=0; i<(sizeof(struct wd_machdr)+4); i++)
	printf("%s%x", (i==0)?"":":", *(txbuf + i));
      printf(">\n");
   }
#endif

   /* check length field for proper value; pad if needed */
   if (length < WDMINSEND)
     length = WDMINSEND;

#if defined(WDDEBUG)
   if (wd_debug&WDBOARD)
     printf("wdsend: length=%d\n", length);
#endif

   /* packet loaded; now tell 8390 to start the transmission */

#ifdef V32
   intr_disable();
#else
   oldlevel = splhi();
#endif
   i = inb(cmd_reg);
   outb( cmd_reg, i & PG_MSK);
   outb( cmd_reg + TPSR, 0);
   outb( cmd_reg + TBCR0, (unsigned char) length);
   outb( cmd_reg + TBCR1, (unsigned char)(length >> 8));
   i = inb(cmd_reg);
   outb( cmd_reg, i | TXP);

#ifdef V32
   intr_restore();
#else
   splx(oldlevel);
#endif

#if defined(WDDEBUG)
   if (wd_debug&WDBOARD)
     printf("wdsend:...\n");
#endif

   /* transmission started; report success */
   return(0);

}

/*
 * can_write provides a semaphore mechanism for the operation of the
 * tx_buf_busy flag.
 */

static 
can_write(wd)
     struct wddev *wd;
{
#ifndef V32
   int oldlevel;
#endif

#ifdef V32
   intr_disable();
#else
   oldlevel = splhi();
#endif
   if (wd->wd_macpar->wd_txbuf_busy) {
      /* buffer is in use; restore interrupts and return failure value */
      wd->wd_flags |= WDS_XWAIT; /* wait for transmitter */
#ifdef V32
      intr_restore();
#else
      splx(oldlevel);
#endif
      return(0);
   } else {
      /* buffer is free; claim it, restore interrupts and return true */
      wd->wd_macpar->wd_txbuf_busy++;
#ifdef V32
      intr_restore();
#else
      splx(oldlevel);
#endif
      return(1);
   }
}

/*
 * wdlocal checks to see if the message is addressed to this
 * system by comparing with the board's address
 */

wdlocal(mp,wd)
     mblk_t *mp;
     struct wddev *wd;
{
   return BCMP((caddr_t)(mp->b_rptr), wd->wd_macpar->wd_macaddr, LLC_ADDR_LEN) == 0;
}

wdbroadcast(mp)
     mblk_t *mp;
{
   return BCMP((caddr_t)(mp->b_rptr), wdbroadaddr, LLC_ADDR_LEN) == 0;
}

/*
 * wdmulticast checks to see if a multicast address that is
 * being listened to is being addressed.
 */

wdmulticast(mp,wd)
     mblk_t *mp;
     struct wddev *wd;
{
   register int i;
   register struct wdparam *wdp = wd->wd_macpar;
   register struct wdmaddr *firstp;

   if (wdp->wd_multicnt == 0)
      return(0);		/* no multicast addresses defined */

   /*
    * search the list of multicast addrs for this board, starting with
    * the most recently referenced;  this may require wraparound on the list
    */
   i = wdp->wd_multip - wdp->wd_multiaddrs;
   firstp = wdp->wd_multip;
   do {
      if (BCMP((caddr_t)(mp->b_rptr), wdp->wd_multip->entry, LLC_ADDR_LEN) == 0)
	return(1);
      if (++i >= wd_multisize) {	/* wraparound */
	 i = 0;
	 wdp->wd_multip = wdp->wd_multiaddrs;
      }
      else 
	 wdp->wd_multip++;
   } while (firstp != wdp->wd_multip);
   return(0);
}

/*
 * wdloop puts a formerly outbound message onto the input
 * queue.  Needed since the board can't receive messages it sends
 * to itself
 */

wdloop(mp,wd)
     mblk_t *mp;
     struct wddev *wd;
{
#if defined(WDDEBUG)
   if (wd_debug&WDRECV)
     printf("wdloop: loop back packet\n");
#endif
   if (mp != NULL)
     wdrecv(mp,wd - wd->wd_no);
}
/*
 * wdnack builds an error acknowledment for the primitive
 * All operations have potential error acknowledgments
 * unitdata does not have a positive ack
 */

wdnak(q, mp, err)
     queue_t *q;
     mblk_t  *mp;
{
   mblk_t *nmp;
   union DL_primitives *dlp, *orig;

   if ((nmp=allocb(sizeof(union DL_primitives), BPRI_MED)) == NULL){
      putbq(q, mp);
      bufcall(sizeof(union DL_primitives), BPRI_MED, qenable, q);
      return WDE_NOBUFFER;
   }

   /* sufficient resources to make an ACK */

   /* make original message easily used to get primitive in question */
   orig = (union DL_primitives *)mp->b_rptr;

   dlp = (union DL_primitives *)nmp->b_rptr;

   dlp->error_ack.dl_primitive = DL_ERROR_ACK;
   dlp->error_ack.dl_error_primitive= orig->dl_primitive; /* this is the failing opcode */
   dlp->error_ack.dl_errno = (err & WDE_SYSERR) ? DL_SYSERR : err;
   dlp->error_ack.dl_unix_errno = (err & WDE_SYSERR) ? (err&WDE_ERRMASK) : 0;

   nmp->b_wptr = nmp->b_rptr + DL_ERROR_ACK_SIZE;
   nmp->b_datap->db_type = M_PCPROTO; /* acks are PCproto's */

   qreply(q, nmp);
   return WDE_OK;
}

/*
 * wdioctl handles all ioctl requests passed downstream.
 * This routine is passed a pointer to the message block with
 * the ioctl request in it, and a pointer to the queue so it can
 * respond to the ioctl request with an ack.
 */

wdioctl(q, mb)
     queue_t	*q;
     mblk_t	*mb;
{
   struct iocblk *iocp;
   struct wddev  *wd;
   mblk_t *stats, *bt;
   short cmd_reg;
   struct wdparam *wdp;

   iocp = (struct iocblk *) mb->b_rptr;
   wd = (struct wddev *)q->q_ptr;
   wdp = wd->wd_macpar;
   mb->b_datap->db_type = M_IOCACK; /* assume ACK */
   cmd_reg = wdp->wd_ioaddr + 0x10;

   switch (iocp->ioc_cmd) {

    case INITQPARMS:
      if (iocp->ioc_error = initqparms(mb, minfo, MUXDRVR_INFO_SZ))
	mb->b_datap->db_type = M_IOCNAK;
      else
	mb->b_datap->db_type = M_IOCACK;
      iocp->ioc_count = 0;
      qreply(q, mb);
      return;
    case NET_INIT:

      if ((iocp->ioc_count != 0) || net_initialized) {
	 /* iocp->ioc_error =  --some error value--; */
	 goto iocnak;
      }

      net_initialized = 1;

      /* clear errstats structure? */
      /* anything else? */

      qreply(q, mb);

      break;

    case NET_UNINIT:

      if ((iocp->ioc_count != 0) || !net_initialized) {
	 /* iocp->ioc_error = --some error value--; */
	 goto iocnak;
      }

      net_initialized = 0;

      qreply(q, mb);

      break;

    case NET_GETBROAD:
    case NET_ADDR: {
       unsigned char *dp;
       register int i;

       /* if we don't have a six byte data buffer send NAK */
       if (iocp->ioc_count != LLC_ADDR_LEN) {
	  iocp->ioc_error = EINVAL;
	  goto iocnak;
       }

       /* get address of location to put data */
       dp = mb->b_cont->b_rptr;

       /* see which address is requested & move it in buffer */
       if (iocp->ioc_cmd == NET_GETBROAD)

	 /* copy Ethernet/Starlan broadcast address */
	 for (i = 0; i < LLC_ADDR_LEN; i++) {
	    *dp = wdbroadaddr[i];
	    dp++;
	 }
       else
	 /* copy host's physical address */
	 for (i = 0; i < LLC_ADDR_LEN; i++) {
	    *dp = wdp->wd_macaddr[i];
	    dp++;
	 }

       /* send the ACK */
       qreply(q, mb);
       break;
    }

    case DLSADDR:		/* set MAC address to new value */
      if (mb->b_cont && (mb->b_cont->b_wptr - mb->b_cont->b_rptr)==LLC_ADDR_LEN){
	 BCOPY(mb->b_cont->b_rptr, wdp->wd_macaddr, LLC_ADDR_LEN);
      } else {
	 iocp->ioc_error = EINVAL;
	 mb->b_datap->db_type = M_IOCNAK;
      }
      qreply(q, mb);
      break;

    case DLSMULT: {		/* turn on a multicast address */
       register struct wdmaddr *mp;
       register int i;
       int row,col;
       unsigned char val;
       int err = 0;
#ifndef V32
       int oldlevel;
#endif

       if (wdp->wd_multicnt >= wd_multisize) {
          iocp->ioc_error = ENOSPC;
	  err++;
       }
       else if(mb->b_cont && 
	  ((mb->b_cont->b_wptr - mb->b_cont->b_rptr)==LLC_ADDR_LEN) &&
	  /* check for illegal or previously defined addresses */
	  ((unsigned char)mb->b_cont->b_rptr[0] & 0x01)) {

	  if(wdmulticast(mb->b_cont,wd) != 0) {
	     qreply(q, mb);
	     break;
	  }
	  mp = wdp->wd_multiaddrs;
	  for (i = 0; i < wd_multisize; i++,mp++)
	     /* find the first empty slot */
	     if (mp->entry[0] == 0)
	        break;
	  wdp->wd_multicnt++;
	  BCOPY(mb->b_cont->b_rptr,mp->entry,LLC_ADDR_LEN);
       } else {
	  iocp->ioc_error = EINVAL;
	  err++;
       }
       if (err) {
	  mb->b_datap->db_type = M_IOCNAK;
	  qreply(q,mb);
	  return;
       }
       qreply(q,mb);
       /*
	* map the address to the range [0,63]
	* and set the corresponding multicast register filter bit 
	*/
       mp->filterbit = wdhash(mp->entry);
       row = mp->filterbit/8;
       col = mp->filterbit%8;
#ifdef V32
       intr_disable();
#else
       oldlevel = splhi();
#endif
       outb(cmd_reg, PAGE_1);
       val = inb(cmd_reg+MAR0+row);
       val |= 0x01 << col;
       outb(cmd_reg+MAR0+row, val);
       outb(cmd_reg, PAGE_0);
#ifdef V32
       intr_restore();
#else
       splx(oldlevel);
#endif
       break;
    }

    case DLDMULT: {		/* delete a multicast address */
       register unsigned char index;
       register int i;
       struct wdmaddr *mp;
       unsigned char val;
       int row,col;

       if(mb->b_cont && 
	  ((mb->b_cont->b_wptr - mb->b_cont->b_rptr)==LLC_ADDR_LEN) &&
	  (wdp->wd_multicnt > 0) &&
	  (wdmulticast(mb->b_cont,wd))) {
	     register unsigned char *c = wdp->wd_multip->entry;
	     for (index = 0; index < LLC_ADDR_LEN; index++,c++)
		*c = 0;
	     wdp->wd_multip->filterbit = 0xFF;
	     wdp->wd_multicnt--;
       } else {
	  iocp->ioc_error = EINVAL;
	  mb->b_datap->db_type = M_IOCNAK;
          qreply(q,mb);
	  return;
       }
       qreply(q,mb);
       /*
	* map the address to the range [0,63]
	* and turn the corresponding multicast register filter bit off 
	*/
       index = wdhash((unsigned char *)mb->b_cont->b_rptr);
       /*
	* make sure the filter bit isn't referenced by another 
	* defined multicast address
	*/
       mp = wdp->wd_multiaddrs;
       for (i = 0; i < wd_multisize; i++,mp++) {
	  if (mp->entry[0] && index == mp->filterbit)
	     return;
       }

       outb(cmd_reg, PAGE_1);
       row = index/8;
       col = index%8;
       val = inb(cmd_reg+MAR0+row);
       val &= ~(0x01 << col);
       outb(cmd_reg+MAR0+row, val);
       outb(cmd_reg, PAGE_0);
       break;
    }

    case DLGMULT: {		/* get multicast address list */
       register struct wdmaddr *mp;
       register int i;
       int found = 0;
       unsigned char *dp;

       if (iocp->ioc_count <= 0) {
	  /* no space provided; return number of multicast addresses defined */
	  iocp->ioc_rval = wdp->wd_multicnt;
       } else {
	  /* copy as many addresses as space allows */
	  dp = mb->b_cont->b_rptr;
	  mp = wdp->wd_multiaddrs;
	  for (i = 0;(i < wd_multisize) && (dp < mb->b_cont->b_wptr);i++,mp++) 
	     if (mp->entry[0]) {
		 BCOPY(mp->entry,dp,LLC_ADDR_LEN);
		 dp += LLC_ADDR_LEN;
		 found++;
	     }
	  iocp->ioc_rval = found;
	  mb->b_cont->b_wptr = dp;
       }
       qreply(q,mb);
       break;
    }
	  
    case NET_WDBRDTYPE:

      if ((bt = allocb(sizeof(wdp->wd_boardtype), BPRI_MED)) == NULL){
	      iocp->ioc_error == ENOSR;
	      goto iocnak;
      }
      BCOPY((caddr_t) &wdp->wd_boardtype, bt->b_wptr, 
	    sizeof(wdp->wd_boardtype));
      bt->b_wptr += sizeof(wdp->wd_boardtype);
      linkb(mb, bt);
      iocp->ioc_count = sizeof(wdp->wd_boardtype);

      qreply(q, mb);
      break;

    case NET_WDSTATUS:

      if ((stats = allocb(sizeof(struct wdstat), BPRI_MED)) == NULL){
	      iocp->ioc_error == ENOSR;
	      goto iocnak;
      }
      BCOPY((caddr_t) wd->wd_stats, stats->b_wptr, sizeof(struct wdstat));
      stats->b_wptr += sizeof(struct wdstat);
      linkb(mb, stats);
      iocp->ioc_count = sizeof(struct wdstat);

      qreply(q, mb);
      break;

    case NET_GETSTATUS:

      /* copy the errstats counters into data block. */
      if (mb->b_cont == NULL ||  iocp->ioc_count <= 0){
	      iocp->ioc_error = EINVAL;
	      goto iocnak;
      }
	  
      BCOPY(wd->wd_stats, mb->b_cont->b_rptr, 
	    min(iocp->ioc_count, sizeof(struct wdstat)));

      qreply(q, mb);
      break;

    case NET_SETPROM:			/* toggle promiscuous mode */
      if (wd->wd_flags & WDS_SU || iocp->ioc_uid == 0){
	 if (wd->wd_flags & WDS_PROM){	/* disable promiscuous mode */
	    wd->wd_flags &= ~WDS_PROM;
	    wdp->wd_proms--;
	    if (wdp->wd_proms <= 0) {
		 outb(cmd_reg, PAGE_0);
		 wdp->wd_proms = 0; 		/* just to be sure */
		 wdp->wd_devmode &= ~WDS_PROM;
		 outb(cmd_reg + RCR, INIT_RCR);
	    }
	 } else {			/* enable promiscuous mode */
	    wd->wd_flags |= WDS_PROM;
	    wdp->wd_proms++;
	     if (!(wdp->wd_devmode & WDS_PROM)) {
		 outb(cmd_reg, PAGE_0);
		 wdp->wd_devmode |= WDS_PROM;
		 outb(cmd_reg + RCR, INIT_RCR|PRO);
	     }
	 }
	 qreply(q, mb);
	 break;
      }
      goto iocnak;

    default:
      /* iocp_error = --some default error value--; */

    iocnak:
      /* NAK the ioctl request */
      mb->b_datap->db_type = M_IOCNAK;
      qreply(q, mb);

   } /* end switch */

} /* end wdioctl */

unsigned char
wdhash(addr)
unsigned char addr[];
{
	register int i, j;
	union crc_reg crc;
	unsigned char fb, ch;

	crc.value = 0xFFFFFFFF;
	for(i = 0; i < LLC_ADDR_LEN; i++) {
	    ch = addr[i];
	    for (j = 0; j < 8; j++) {
		fb = crc.bits.a31 ^ ((ch >> j) & 0x01);
		crc.bits.a25 ^= fb;
		crc.bits.a22 ^= fb;
		crc.bits.a21 ^= fb;
		crc.bits.a15 ^= fb;
		crc.bits.a11 ^= fb;
		crc.bits.a10 ^= fb;
		crc.bits.a9 ^= fb;
		crc.bits.a7 ^= fb;
		crc.bits.a6 ^= fb;
		crc.bits.a4 ^= fb;
		crc.bits.a3 ^= fb;
		crc.bits.a1 ^= fb;
		crc.bits.a0 ^= fb;
		crc.value = (crc.value << 1) | fb;
	    }
	}
	return((unsigned char)(crc.value >> 26));
}


struct wd_type {
	char *name;
	unsigned int tag;
} wd_type[] = {
	"WD8003E",      ETHERNET_MEDIA,
	"WD8003S",      STARLAN_MEDIA,
	"WD8003WT",     TWISTED_PAIR_MEDIA,
	"WD8003W",      TWISTED_PAIR_MEDIA | INTERFACE_CHIP,
	"WD8003EB",     ETHERNET_MEDIA | INTERFACE_CHIP,
	"WD8003ET/A",   ETHERNET_MEDIA | MICROCHANNEL,
	"WD8003ST/A",   STARLAN_MEDIA | MICROCHANNEL,
	"WD8003E/A",    ETHERNET_MEDIA | MICROCHANNEL | INTERFACE_CHIP,
	"WD8003SH/A",   STARLAN_MEDIA | MICROCHANNEL | INTERFACE_CHIP,
	"WD8003W/A",    TWISTED_PAIR_MEDIA | MICROCHANNEL | INTERFACE_CHIP,
	"WD8013EBT",    ETHERNET_MEDIA | BOARD_16BIT,
	"WD8013EB",     ETHERNET_MEDIA | BOARD_16BIT | INTERFACE_CHIP,
	"WD8013ET",     TWISTED_PAIR_MEDIA | BOARD_16BIT | INTERFACE_CHIP,
	"WD8023E",      ETHERNET_MEDIA | INTELLIGENT | INTERFACE_CHIP,
	NULL, 0
};

char *
wd_getname(type)
	int type;
{
	struct wd_type *tt;

	type &= STATIC_ID_MASK;
	for (tt = wd_type; tt->name; tt++) {
		if (tt->tag == type)
			return(tt->name);
	}
	return("UNKNOWN");
}

/******************************************************************************
*******************************************************************************
	A George Kalwitz Production, 1989
*******************************************************************************
******************************************************************************/

/******************************************************************************

This module provides an algorithm for identifying the type of 
	Western Digital LAN Adapter being used
	e.g. WD8003E, WD8003ST/A, WD8023E, etc.

Assumes interrupts are enabled on entry and returns with interrupts enabled

***************************************************************************/

#define	BID_FALSE	0
#define	BID_TRUE	1

/* Register offset definitions...since different boards have different names
	for register offsets 0x00-0x07, generic names will be assigned */
#define	BID_REG_0		0x00
#define	BID_REG_1		0x01
#define	BID_REG_2		0x02
#define	BID_REG_3		0x03
#define	BID_REG_4		0x04
#define	BID_REG_5		0x05
#define	BID_REG_6		0x06
#define	BID_REG_7		0x07

/* Register offset definitions...the names for this registers are consistant
	across all boards, so specific names will be assigned */
#define	BID_LAN_ADDR_0		0x08	/* these 6 registers hold the */
#define	BID_LAN_ADDR_1		0x09	/*     LAN address for this node */
#define	BID_LAN_ADDR_2		0x0A
#define	BID_LAN_ADDR_3		0x0B
#define	BID_LAN_ADDR_4		0x0C
#define	BID_LAN_ADDR_5		0x0D
#define	BID_BOARD_ID_BYTE	0x0E	/* identification byte for WD boards */
#define	BID_CHCKSM_BYTE		0x0F	/* the address ROM checksum byte */

/**** Masks bits for the board revision number in the BID_BOARD_ID_BYTE ****/
#define	BID_BOARD_REV_MASK	0x1E

/*** Misc. definitions ***/
#define	BID_MSZ_583_BIT		0x08	/* memory size bit in 583 */
#define	BID_ALTERNATE_MODE	0x10	/* alternate mode for intelligent */
#define BID_SIXTEEN_BIT_BIT	0x01	/* bit has 16 bit capability info */

/*** Defs for board rev numbers greater than 1 ***/
#define	BID_MEDIA_TYPE_BIT	0x01
#define	BID_SOFT_CONFIG_BIT	0x20
#define	BID_RAM_SIZE_BIT	0x40
#define	BID_BUS_TYPE_BIT	0x80

/**** defs for identifying the 690 ****/
#define BID_CR		0x10		/* Command Register	*/
#define BID_TXP		0x04		/* transmit packet */
#define	BID_TCR		0x1D		/* Transmit Configuration Register */
#define	BID_TCR_VAL	0x18		/* test value for 690 or 8390 */
#define	BID_PS0		0x00		/* register page select - 0 */
#define BID_PS1		0x40		/* register page select - 1 */
#define	BID_PS2		0x80		/* register page select - 2 */
#define	BID_PS_MASK	0x3F		/* to mask off page select bits */

#define	WD8003E		ETHERNET_MEDIA
#define	WD8003EBT	WD8003E		/* functionally identical to WD8003E */
#define	WD8003S		STARLAN_MEDIA
#define	WD8003SH	WD8003S		/* functionally identical to WD8003S */
#define	WD8003WT	TWISTED_PAIR_MEDIA
#define	WD8003W		(TWISTED_PAIR_MEDIA | INTERFACE_CHIP)
#define	WD8003EB	(ETHERNET_MEDIA | INTERFACE_CHIP)
#define	WD8003ETA	(ETHERNET_MEDIA | MICROCHANNEL)
#define	WD8003STA	(STARLAN_MEDIA | MICROCHANNEL)
#define	WD8003EA	(ETHERNET_MEDIA | MICROCHANNEL | INTERFACE_CHIP)
#define	WD8003SHA	(STARLAN_MEDIA | MICROCHANNEL | INTERFACE_CHIP)
#define	WD8003WA	(TWISTED_PAIR_MEDIA | MICROCHANNEL | INTERFACE_CHIP)
#define	WD8013EBT	(ETHERNET_MEDIA | BOARD_16BIT)
#define	WD8013EB	(ETHERNET_MEDIA | BOARD_16BIT | INTERFACE_CHIP)
#define	WD8023E		(ETHERNET_MEDIA | INTELLIGENT | INTERFACE_CHIP)


/***************************************************************************

this is the main level routine for finding the board type

	ENTER:	parameter = base I/O address of LAN adapter board

	RETURN:	long value defining the boards ID

****************************************************************************/

GetBoardID (base_io)
int      base_io;	/* base I/O address of LAN adapter board */
{
	long	board_id;			/* holds return value */
	int	board_rev_number;
	extern	long	bid_get_base_info ();
	extern	long	bid_get_media_type ();
	extern	long	bid_get_hrdwr_info ();
	extern	long	bid_get_ram_size ();

	board_id = 0;
	board_rev_number = bid_get_board_rev_number (base_io);
	if (!(board_rev_number))
		return (0);			/* it must be an 8000 board */
	if (bid_check_micro_channel (base_io, board_rev_number))
		board_id |= MICROCHANNEL;
	board_id |= bid_get_base_info (base_io, board_id);
	board_id |= bid_get_media_type (base_io, board_rev_number);
	if (board_rev_number > 1)
           board_id |= bid_get_hrdwr_info (base_io, board_rev_number, board_id);
	board_id |= bid_get_ram_size (base_io, board_rev_number, board_id);
	if (!(board_id & INTELLIGENT))
	{
                if (bid_check_for_690 (base_io))
                    board_id |= NIC_690_BIT;
	}
	return (board_id);
}

/******************************************************************************


******************************************************************************/
static	long
bid_get_base_info (address, current_id)
int	address;
long	current_id;
{
	long	base_bits;

	base_bits = 0;
	if (current_id & MICROCHANNEL)
	{
		if (bid_has_interface_chip (address))
			base_bits |= INTERFACE_CHIP;
		return (base_bits);
	}
	else
	{
		if (bid_register_aliasing (address))
                    return (base_bits);
		if (bid_has_interface_chip (address))
			base_bits |= INTERFACE_CHIP;
		if (bid_board_sixteen_bit (address))
		{
			base_bits |= BOARD_16BIT;
			if (bid_slot_sixteen_bit (address))
				base_bits |= SLOT_16BIT;
			return (base_bits);
		}
		if (bid_intelligent_board (address))
                {
			base_bits |= INTELLIGENT;
			return (base_bits);
                }
	}
	return (base_bits);	/* return the proper board ID */
}

/*****************************************************************************

this checks to see if the machine/board is micro channel
	if the rev number in the board is less than 2, this routine
		will rely on the machines system I/O to figure it out
	else it will look at the bit in the boards ID byte to 
		figure it out

******************************************************************************/
static	int
bid_check_micro_channel (address, rev_number)
int	address;
int	rev_number;
{
	if (rev_number < 2)
		return (bid_micro_channel ());
	else
		return (inb (address+BID_BOARD_ID_BYTE) & BID_BUS_TYPE_BIT);
}

/*****************************************************************************

this finds out if the board is being used in a micro channel machine
	it uses System I/O Port 0x96
		AT System -> 0x96 maps to DMA Page Register Space
		MC System -> 0x96 maps to Channel Position Select Register
	in a MC System, bits 4, 5, and 6 of register 0x96 may be written
	with 0's, but will always read 1's

******************************************************************************/
int
bid_micro_channel ()
{
	unsigned        delay;
	int     save_val;
	int     ret_val;
#if M_XENIX
	int     splvar;
#endif

	intr_disable();
	ret_val = BID_FALSE;            /* initialize to AT machine */
	save_val = inb (0x96);          /* save previous value */
	if (save_val & 0x80)            /* MC machine cant have hob set */
	{
		intr_restore ();
		return (ret_val);       /* show AT machine */
	}
	outb (0x96, 0);                 /* write a safe value */
	for (delay = 0; delay < 0x100; delay++)
	{
		inb (0x96);
	}
	if (inb (0x96) == 0x70)         /* did mc bits stay 1's? */
	{
		outb (0x96, 0x03);
		for (delay = 0; delay < 0x100; delay++)
		{
			inb (0x96);
		}
		if (inb (0x96) == 0x73)
			ret_val = BID_TRUE;     /* show MC machine */
	}
	outb (0x96, save_val);          /* restore previous value */
	intr_restore ();
	return (ret_val);
}

/*****************************************************************************

this returns the Media Type

******************************************************************************/
static  long
bid_get_media_type (address, rev_number)
int     address;
int     rev_number;
{
	if (inb (address+BID_BOARD_ID_BYTE) & BID_MEDIA_TYPE_BIT)
		return (ETHERNET_MEDIA);
	else
	{
		if (rev_number == 1)
			return (STARLAN_MEDIA);
		else
			return (TWISTED_PAIR_MEDIA);
	}
}

/*****************************************************************************

this checks to see if an interface chip is present (i.e. WD83C583 or WD83C593)
	RETURNS:        1 if present
			0 if not

******************************************************************************/
static  int
bid_has_interface_chip (address)
int  address;
{
    int ret_val;
    int temp_val;

    temp_val = inb (address+BID_REG_7);         /* save old value */
    outb (address+BID_REG_7, 0x35);             /* write general purpose reg */
    if ((inb (address+BID_REG_7) & 0xFF) != 0x35)       /* if it didn't stick */
	ret_val = BID_FALSE;                    /* then show no chip */
    else
    {
	outb (address+BID_REG_7, 0x3A);         /* try another value */
	if ((inb (address+BID_REG_7) & 0xFF) != 0x3A)   /* if it didn't stick */
	    ret_val = BID_FALSE;                        /* then show no chip */
	else
	    ret_val = BID_TRUE;                 /* both stuck, show true */
    }
    if (ret_val)                                /* if a chip is there */
	outb (address+BID_REG_7, temp_val);     /* then restore the value */
			/* otherwise, we dont know what a restore will do */
			/* could read 0xFF and alias to REG_0 on write */
    return (ret_val);
}

/*****************************************************************************

this checks to see if the board has register aliasing
	that is, reading from register offsets 0-7 will return the contents
	of register offsets 8-F

******************************************************************************/
static  int
bid_register_aliasing (address)
int  address;
{
 if ((inb (address+BID_REG_1) & 0xFF) != (inb (address+BID_LAN_ADDR_1) & 0xFF))
     return (BID_FALSE);
 if ((inb (address+BID_REG_2) & 0xFF) != (inb (address+BID_LAN_ADDR_2) & 0xFF))
     return (BID_FALSE);
 if ((inb (address+BID_REG_3) & 0xFF) != (inb (address+BID_LAN_ADDR_3) & 0xFF))
     return (BID_FALSE);
 if ((inb (address+BID_REG_4) & 0xFF) != (inb (address+BID_LAN_ADDR_4) & 0xFF))
     return (BID_FALSE);
 if ((inb (address+BID_REG_7) & 0xFF) != (inb (address+BID_CHCKSM_BYTE) & 0xFF))
     return (BID_FALSE);
 return (BID_TRUE);             /* all were equal...show aliasing true */
}

/*****************************************************************************

this finds out if the adapter board being used is an intelligent one
	does this by testing the fourth bit of the fourth register
	for alternate mode select
	RETURNS:        1 if intelligent
			0 if not

******************************************************************************/
static  int
bid_intelligent_board (address)
int  address;
{
	if (inb (address+BID_REG_4) & BID_ALTERNATE_MODE)
		return (BID_TRUE);              /* show intelligent board */
	else
		return (BID_FALSE);             /* show stupid board */
}

/*****************************************************************************

this checks to see if the board is capable of 16 bit memory moves
	it does this by checking bit zero of register one
	if this bit is unchangable by software, then the card has 16 bit
	capability
	RETURNS:        1 if capable of 16 bit moves
			0 if not

******************************************************************************/
static  int
bid_board_sixteen_bit (address)
int  address;
{
	int  ret_val;
	int  register_hold;

	/* read previous contents */
	register_hold = (inb (address+BID_REG_1) & 0xFF);

	/* write back what was read with bit zero flipped */
	outb (address+BID_REG_1, register_hold^BID_SIXTEEN_BIT_BIT);

	/* if we changed the value, write it back and show no 16 bit
	   capability, else show 16 bit capability */
	if ((register_hold & BID_SIXTEEN_BIT_BIT) ==
		       (inb (address+BID_REG_1) & BID_SIXTEEN_BIT_BIT))
	{
		ret_val = 1;                    /* we did not change it */
		/* if sixteen bit board, always reset bit one */
		register_hold &= 0xFE;
	}
	else
		ret_val = 0;
	outb (address+BID_REG_1, register_hold);
	return (ret_val);
}

/*****************************************************************************

this finds out if the 16 bit capable board is plugged in a 16 bit slot
	it does this by reading bit 0 of the register one,
	if the value is 1 => 16 bit board in 16 bit slot
	else not
	RETURNS:        1 if 16 bit board in 16 bit slot
			0 if not

******************************************************************************/
static  int
bid_slot_sixteen_bit (address)
int  address;
{
	if (inb (address+BID_REG_1) & BID_SIXTEEN_BIT_BIT)
		return (BID_TRUE);
	else
		return (BID_FALSE);
}

/*****************************************************************************

this finds out if the board is using a 690 and not an 8390 NIC
	it tests the multicast address registers
	the 690 will not write these registers, but the 8390 will

	RETURNS:        1 if using 690
			0 if using 8390

******************************************************************************/
static  int
bid_check_for_690 (address)
int     address;
{
	int     temp_CR,
		work_CR,
		temp_TCR;
	int     ret_val;
#if M_XENIX
	int     splvar;
#endif

	ret_val = BID_FALSE;
	intr_disable ();
	/* dont maintain TX bit */
	temp_CR = ((inb (address+BID_CR) & ~BID_TXP) & 0xFF); /* save orig CR */
	work_CR = temp_CR & BID_PS_MASK;        /* mask off pg sel bits */
	outb (address+BID_CR, work_CR | BID_PS2);       /* select page 2 */
	temp_TCR = inb (address+BID_TCR);       /* save orig TCR */

	outb (address+BID_CR, work_CR);         /* select page 0 */
	outb (address+BID_TCR, BID_TCR_VAL);    /* write test value */
	outb (address+BID_CR, work_CR | BID_PS2);       /* select page 2 */
	if ((inb (address+BID_TCR) & BID_TCR_VAL) == BID_TCR_VAL)
		ret_val = BID_FALSE;    /* show 8390 */
	else
		ret_val = BID_TRUE;     /* show 690 */

	outb (address+BID_CR, work_CR);         /* select page 0 */
	outb (address+BID_TCR, temp_TCR);       /* put back original value */
	outb (address+BID_CR, temp_CR);         /* put back original value */
	intr_restore ();
	return (ret_val);
}

/*****************************************************************************

this returns the board revision number found in the board id byte
of the lan address ROM
	first it will read the value of that register,
	then it will mask off the relavent bits,
	and finally shift them into a 0 justified position

******************************************************************************/
static  int
bid_get_board_rev_number (address)
int     address;
{
	return ((inb (address+BID_BOARD_ID_BYTE) & BID_BOARD_REV_MASK) >> 1);
}

/*****************************************************************************

this gets the extra information from a board which has a revision number
larger than 1
the extra info is based on the revision number and also the current board id

******************************************************************************/
static  long
bid_get_hrdwr_info (address, rev_number, current_id)
int    address;
int    rev_number;
long   current_id;
{
	int     temp;
	long    temp_id;
	long    extra_bits;

	extra_bits = 0;
	temp = inb (address+BID_BOARD_ID_BYTE);
	if (temp & BID_SOFT_CONFIG_BIT)
	{
		temp_id = current_id & STATIC_ID_MASK;
		if ((temp_id == WD8003EB) || (temp_id == WD8003W))
			extra_bits |= ALTERNATE_IRQ_BIT;
	}
	return (extra_bits);
}

/*****************************************************************************

this gets the size of the RAM on board the LAN adapter
	could return an 'I dont know' answer

******************************************************************************/
static  long
bid_get_ram_size (address, rev_number, current_id)
int     address;
int     rev_number;
long    current_id;
{
    int   large_ram_flag;

    if (rev_number < 2)                 /* if pre-board rev */
    {
	if (current_id & BOARD_16BIT)
	    return (RAM_SIZE_16K);
	if (current_id & MICROCHANNEL)
	    return (RAM_SIZE_16K);
	if (current_id & INTERFACE_CHIP)
	{
	    if (inb (address+BID_REG_1) & BID_MSZ_583_BIT)
		return (RAM_SIZE_32K);
	    else
		return (RAM_SIZE_8K);
	}
	return (RAM_SIZE_UNKNOWN);      /* default for old boards */
    }
    else                                /* must be board with new rev info */
    {
	large_ram_flag = inb (address+BID_BOARD_ID_BYTE) & BID_RAM_SIZE_BIT;
	switch ((int) (current_id & STATIC_ID_MASK))
	{
	    case WD8003E:
	    case WD8003S:
	    case WD8003WT:
	    case WD8003W:
	    case WD8003EB:
		if (large_ram_flag)
		    return (RAM_SIZE_32K);
		else
		    return (RAM_SIZE_8K);
	    case WD8003ETA:
	    case WD8003STA:
	    case WD8003EA:
	    case WD8003SHA:
	    case WD8003WA:
		return (RAM_SIZE_16K);
	    case WD8013EBT:
	    case WD8013EB:
		if (large_ram_flag)
		    return (RAM_SIZE_64K);
		else
		    return (RAM_SIZE_16K);
	    case WD8023E:
		return (RAM_SIZE_32K);
	    default:
		return (RAM_SIZE_UNKNOWN);
	}
    }
}

