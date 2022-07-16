/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)master:weitek/stubs.c	1.3"

#include <sys/weitek.h>

/* Weitek stubs */

char weitek_kind = WEITEK_NO;
unsigned long weitek_cfg = 0L;
unsigned long weitek_paddr = 0L;
weitek_map(){}
weitek_unmap(){}
init_weitek_pt(){}
map_weitek_pt(){}
unmap_weitek_pt(){}
weitek_save(){ weitek_kind = WEITEK_NO; }
struct proc *weitek_proc = (struct proc *) 0;
weitek_restore(regs) long *regs; {}
init_weitek(){}
wemulate(vaddr) long vaddr; {}
weitek_reset_intr(){}
clear_weitek_ae(){}
weitintr(i) int i; {}
int	weitek_pt = -1;
int get87() {return(0);}
