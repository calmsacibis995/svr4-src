/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */

#ident	"@(#)kern-os:nmi.c	1.3.3.1"

/*
 *	NMI Support
 *
 *	This module gets called when an NMI occurs.
 *	It tries to determine whether the NMI was caused by a memory parity error.
 *	If so, it tries to find the bad location and react appropriately.
 *
 *	This implementation is specific to the AT-style architecture.
 */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/immu.h"
#include "sys/bootinfo.h"
#include "sys/cmn_err.h"
#include "sys/reg.h"
#include "sys/pit.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "vm/faultcatch.h"
#ifdef MB2
#include "sys/ics.h"
extern ics_slot_t ics_slotmap[];
#ifndef NODEBUGGER
extern void (*cdebugger)();
#endif
#define ICS_PARITY	0x43
#endif

extern int	(*nmi_hook)();

#define PORT_B		0x61	/* System Port B */
#define IOCHK_DISABLE	0x08	/* Disable I/O CH CK */
#define PCHK_DISABLE	0x04	/* Disable motherboard parity check */
#define IOCHK		0x40	/* I/O CH CK */
#define PCHK		0x80	/* Motherboard parity check */

#define CMOS_PORT	0x70	/* CMOS Port */
#define NMI_ENABLE	0x0F	/* Enable NMI interrupt */
#define NMI_DISABLE	0x8F	/* Disable NMI interrupt */
#define AUTOBOOT	1	/* Arg for oemreboot to trigger system reboot*/
#define	NMI_MASK	0x0F	/* Mask NMI bits in status/control port */
#define	E8R1_PRODNUM	0x49e8	/* product id number for E8R1 EISA system */
#define	FAR_HIGH	0x844	/* high bits of Fault Address Register */
#define	FAR_LOW		0x840	/* low bits of Fault Address Register */
int system_reboot = 0;		/* flag to cause reboot by second sanity loop*/
extern int eisa_bus, sanity_clk;
extern unsigned int eisa_brd_id;


paddr_t	nmi_addr;

asm void scan_block(addr, length)
{
%mem addr,length;
	pushl	%esi
	movl	addr, %esi
	movl	length, %ecx
	rep
	lodsb
	popl	%esi
}

int _nmi_catch(r0ptr)
	int	*r0ptr;
{
	nmi_addr = kvtophys(r0ptr[ESI]);
	return 1;
}


int _nmi_hook(r0ptr)
	int	*r0ptr;
#if defined (MB1) || defined (MB2)
#ifdef MB1
{
	cmn_err(CE_PANIC, "Parity error address unknown.");

}
#endif /* MB1 */
#ifdef MB2
{
	unsigned short	i, ics_gen_cntl;

	i = ics_find_rec(ics_myslotid(), ICS_PARITY_CONTROL_TYPE);
	if (i != 0xffff) {
		if((ics_read(ics_myslotid(), i+ICS_PARITY_OFFSET) & ICS_PARITY_MASK) == ICS_PARITY_MASK )
			cmn_err(CE_PANIC, "Parity error.");
	} 

	/* if a CSM/001 isn't in slot 0 */

	if (strcmp("CSM/001", ics_slotmap[0].s_pcode) != 0){ 
		ics_gen_cntl = ics_read(ics_myslotid(),ICS_GeneralControl);
		if ((ics_gen_cntl & ICS_SWNMI_MASK) == ICS_SW_NMI ){
#ifndef NODEBUGGER
			(*cdebugger)(r0ptr);
			/* clear debug reg for next time */
			ics_write(ics_myslotid(),ICS_GeneralControl,(ics_gen_cntl & ~ICS_SWNMI_MASK));
#endif
		} else {
			dri_printf("Unexpected NMI in system mode\n");
#ifndef NODEBUGGER
			(*cdebugger)(r0ptr);
#endif
		}
	}
	return (0);

}
#endif /* MB2 */
#else
{
	register int	stat, motherboard;
	paddr_t		board_addr = -1, addr;
	int		i;
	int sanity_ctl	 = SANITY_CTL;		/* EISA santity ctl word */
        int sanity_ctr0	 = SANITY_CTR0;		/* EISA santity timer */
	int sanity_mode = PIT_C0|PIT_ENDSIGMODE|PIT_READMODE;
	int sanity_latch = PIT_C0|PIT_ENDSIGMODE;
	unsigned int sanitynum = SANITY_NUM;	/*int interval for sanitytimer*/	int sanity_port = SANITY_CHECK;
	int sanity_enable = ENABLE_SANITY;
	int sanity_reset = RESET_SANITY;
	unsigned char byte;
	unsigned int leftover;
	unsigned long far_addr = 0;
	unsigned int valid_flag = 0;
	unsigned int fcflags;

	/* Save the current fault-catch flags, and disable fault-catching
	   during this routine.  Note that we may get here before it's safe
	   to access "u.", but then it would be to early for printf's to work,
	   so it doesn't matter how we panic, and--except for the case of an
	   unknown type of NMI, which we don't really expect to happen--we're
	   going to panic anyway.
	*/
	fcflags = u.u_fault_catch.fc_flags;
	u.u_fault_catch.fc_flags = 0;

	/* check to see if accessing past equipped memory caused the nmi */
	/* first check to see if we are executing on a E8R1 system */
	if (eisa_brd_id == E8R1_PRODNUM) {
		/* obtain the address causing the nmi */
		/* from the Fault Address Register (FAR) */
		far_addr = inw(FAR_HIGH);
		far_addr <<= 16;
		far_addr += inw(FAR_LOW);
		/* search through memory to find if a valid address */
		for (i = 0; i < bootinfo.memavailcnt; i++) {
			if ((far_addr >= bootinfo.memavail[i].base) &&
			    (far_addr < bootinfo.memavail[i].base
					+ bootinfo.memavail[i].extent)) {
				valid_flag = 1;
			}
		}
		if (valid_flag == 0) {
			/* read the NMI status and control port */
			/* mask status bits */
			byte = (inb(PORT_B) & NMI_MASK);
			/* clear parity error */
			outb(PORT_B, byte | PCHK_DISABLE);
			/* re-enable parity */
			outb(PORT_B, byte & ~PCHK_DISABLE);
			if ((fcflags & (CATCH_BUS_TIMEOUT|CATCH_ALL_FAULTS))
			 && !servicing_interrupt()) {
				u.u_fault_catch.fc_flags = 0;
				u.u_fault_catch.fc_errno = EFAULT;
				r0ptr[EIP] = (int)u.u_fault_catch.fc_func;
				return;
			}
			else {
				cmn_err(CE_PANIC, "Accessing past equipped memory") ;
			}
		}
	}

	/* check if sanity timer caused nmi by latching and then reading ctr0 */
	if (eisa_bus && sanity_clk) {
		byte = inb(sanity_port);
		cmn_err(CE_NOTE,"!sanity NMI byte = %x",byte);
		if (byte & FAILSAFE_NMI)  {
                        outb(sanity_ctl,sanity_latch);
                        outb(sanity_ctl,sanity_mode);
                        byte = inb(sanity_ctr0);
                        leftover = inb(sanity_ctr0);
			leftover = leftover <<8 + byte;
			cmn_err(CE_NOTE,"!sanity count = %x",leftover);
			if (system_reboot) {
                                outb(sanity_port, sanity_reset);
                                oemreboot(AUTOBOOT);
			}
			else {
                                system_reboot++;
                                outb(sanity_ctl, sanity_mode);
                                byte = sanitynum;
                                outb(sanity_ctr0, byte);
                                byte = sanitynum>>8;
                                outb(sanity_ctr0, byte);
                                outb(sanity_port, sanity_reset);
                                outb(sanity_port, sanity_enable);
                                cmn_err(CE_PANIC, "Sanity Timer Went off ");
			}
		}
	}

	if (!((motherboard = ((stat = inb(PORT_B)) & PCHK)) || (stat & IOCHK))) {
		u.u_fault_catch.fc_flags = fcflags;
		return 0;	/* Not a parity error */
	}

	/* Disable NMI until we're ready for it */
	outb(CMOS_PORT, NMI_DISABLE);

	/* If it's not the motherboard try to identify the particular board */
	if (!motherboard) {
		nmi_int();	/* calls rmc driver to test if sanity timeout */
		for (i = 0; i < bootinfo.memavailcnt && board_addr == -1; i++) {
			if (bootinfo.memavail[i].extent == 0)
				break;
			for (addr = bootinfo.memavail[i].base;
					addr < bootinfo.memavail[i].base
						+ bootinfo.memavail[i].extent;
							addr += 0x10000) {
#if 0
		for (i = 0; i < MAX_MEM_SEG && board_addr == -1; i++) {
			if (bootinfo.memsegs[i].seg_size == 0)
				break;
			for (addr = bootinfo.memsegs[i].base_paddr;
					addr < bootinfo.memsegs[i].base_paddr
						+ bootinfo.memsegs[i].seg_size;
							addr += 0x10000) {
#endif
				*(caddr_t)xphystokv(addr) = *(caddr_t)xphystokv(addr);
				if (!(inb(PORT_B) & IOCHK)) {
					board_addr = addr;
					break;
				}
			}
		}
		/* Clear the I/O CH CK flip/flop */
		outb(PORT_B, inb(PORT_B) | IOCHK_DISABLE);
		outb(PORT_B, inb(PORT_B) & ~IOCHK_DISABLE);
		tenmicrosec();
		stat = (inb(PORT_B) & IOCHK);
	} else {
		/* Clear the motherboard parity check flip/flop */
		outb(PORT_B, inb(PORT_B) | PCHK_DISABLE);
		outb(PORT_B, inb(PORT_B) & ~PCHK_DISABLE);
		tenmicrosec();
		stat = (inb(PORT_B) & PCHK);
	}

	/* Try to reproduce the problem and identify the exact address */
	nmi_addr = -1;
	if (stat == 0) {
		nmi_hook = _nmi_catch;
		outb(CMOS_PORT, NMI_ENABLE);
		for (i = 0; i < bootinfo.memavailcnt && nmi_addr == -1; i++) {
			if (bootinfo.memavail[i].extent == 0)
				break;
			scan_block(xphystokv(bootinfo.memavail[i].base),
				   bootinfo.memavail[i].extent);
		}
#if 0
		for (i = 0; i < MAX_MEM_SEG && nmi_addr == -1; i++) {
			if (bootinfo.memsegs[i].seg_size == 0)
				break;
			scan_block(xphystokv(bootinfo.memsegs[i].base_paddr),
				   bootinfo.memsegs[i].seg_size);
		}
#endif
		outb(CMOS_PORT, NMI_DISABLE);
		nmi_hook = _nmi_hook;
	}

	/* Tell the user what we found out */
	if (board_addr == -1) {
		cmn_err(CE_CONT, "\n\nFATAL: Parity error on %s.",
			motherboard? "the motherboard" : "an add-on card");
	} else {
		cmn_err(CE_CONT,
"\n\nFATAL: Parity error on an add-on card which starts at address 0x%X.",
			board_addr);
	}
	if (nmi_addr == -1)
		cmn_err(CE_PANIC, "Parity error address unknown.");
	else
		cmn_err(CE_PANIC, "Parity error at address 0x%X.", nmi_addr);
}
#endif /* MB1 || MB2 */

void nmi_init()
{
	nmi_hook = _nmi_hook;
}
