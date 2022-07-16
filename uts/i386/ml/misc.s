/	Copyright (c) 1990 UNIX System Laboratories, Inc.
/	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
/	  All Rights Reserved

/	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
/	UNIX System Laboratories, Inc.
/	The copyright notice above does not evidence any
/	actual or intended publication of such source code.

	.ident	"@(#)kern-ml:misc.s	1.4.4.1"

/	Copyright (c) 1987, 1988 Microsoft Corporation
/	  All Rights Reserved

/	This Module contains Proprietary Information of Microsoft 
/	Corporation and should be treated as Confidential.

	.set	NBPW, 4		/ no of bytes in a word.

	.set    CR0_ET, 0x10            / extension type (1->387,0->???)
	.set    CR0_TS, 0x08            / task switched
	.set    CR0_EM, 0x04            / use math emulation
	.set    CR0_MP, 0x02            / math coprocessor present
	.set    CR0_PE, 0x01            / protection enable
	.set	PS_T, 0x100		/ trace flag

#ifdef WEITEK
	.set	WEITEK_LDCTX,	0xffc0c000	/ load context register
	.set	WEITEK_STCTX,	0xffc0c400	/ store context register
	.globl	weitek_kind
	.globl	weitek_map
	.globl	weitek_unmap
#endif

	.text

	.globl	splvalid_flag

	.align	4
	.globl	flushtlb
flushtlb:
	movl	%cr3, %eax
	movl	%eax, %cr3
	ret

	.align	4
	.globl	_cr2
_cr2:
	movl	%cr2, %eax
	ret

	.align	4
	.globl	_cr3
_cr3:
	movl	%cr3, %eax
	andl	$0x7FFFFFFF,%eax
	ret

	.align	4
	.globl  _wdr0
_wdr0:
	movl    4(%esp), %eax
	movl    %eax, %db0
	ret

	.align	4
	.globl  _wdr1
_wdr1:
	movl    4(%esp), %eax
	movl    %eax, %db1
	ret

	.align	4
	.globl  _wdr2
_wdr2:
	movl    4(%esp), %eax
	movl    %eax, %db2
	ret

	.align	4
	.globl  _wdr3
_wdr3:
	movl    4(%esp), %eax
	movl    %eax, %db3
	ret

	.align	4
	.globl  _wdr6
_wdr6:
	movl    4(%esp), %eax
	movl    %eax, %db6
	ret

	.align	4
	.globl  _wdr7
_wdr7:
	movl    4(%esp), %eax
	movl    %eax, %db7
	ret

	.align	4
	.globl  _dr0
_dr0:
	movl    %db0, %eax
	ret

	.align	4
	.globl  _dr1
_dr1:
	movl    %db1, %eax
	ret

	.align	4
	.globl  _dr2
_dr2:
	movl    %db2, %eax
	ret

	.align	4
	.globl  _dr3
_dr3:
	movl    %db3, %eax
	ret

	.align	4
	.globl  _dr6
_dr6:
	movl    %db6, %eax
	ret

	.align	4
	.globl  _dr7
_dr7:
	movl    %db7, %eax
	ret

/       load task register
	.align	4
	.globl	get_tr
get_tr:
	xorl	%eax, %eax
	str	%ax
	ret

	.align	4
	.globl  loadtr
loadtr:
	movw	4(%esp), %ax
	ltr	%ax
	ret

	.align	4
	.globl  loadldt
loadldt:
	movw	4(%esp), %ax
	lldt	%ax
	ret

/
/       Set interrupt priority level routines
/
/       spl*() sets the interrupt priority level specified by the
/       spl to ipl level mapping in ipl.h, and returns the old level.
/
/       splx(ipl) sets the interrupt priority level to ipl,
/       and returns the old level.
/
/       splint() sets the priority for an interrupt and returns old priority,
/       without enabling interupts.
/
/       splxint(ipl) restores the interrupt priority level to ipl,
/       without enabling interupts.
/
	.globl  imrport         / PIC imr port addresses
	.globl  iplmask         / table of masks dimensioned [spl] [pic]
	.globl  curmask         / array of current masks
	.globl  picmax          / highest pic index used = (npic-1)
	.globl  ipl             / current interrupt priority level
	.globl  picipl          / interrupt priority level in pics

	.align	4
	.globl	spl0
spl0:
	movl    $SPL0, %eax
	jmp	spl

	.align	4
	.globl  spl1
spl1:
	movl    $SPL1, %eax
	jmp	spl

	.align	4
	.globl  spl2
spl2:
	movl    $SPL2, %eax
	jmp	spl

	.align	4
	.globl	spl3
spl3:
	movl    $SPL3, %eax
	jmp	spl

	.align	4
	/* XENIX Support */
	.globl	splbuf
splbuf:
	/* End XENIX Support */
	.globl	spl4
spl4:
	movl    $SPL4, %eax
	jmp	spl

	.align	4
	/* XENIX Support */
	.globl	splcli
splcli:
/* End XENIX Support */
	.globl	spl5
spl5:
	movl    $SPL5, %eax
	jmp	spl

	.align	4
	.globl	splpp
splpp:
	movl    $SPLPP, %eax
	jmp	spl

	.align	4
	.globl	spl6
spl6:
	movl    $SPL6, %eax
	jmp	spl

/  4.0 added
	.align	4
	.globl	splvm
splvm:
	movl    $SPLVM, %eax
	jmp	spl
/

	.align	4
	.globl	splni
splni:
	movl    $SPLNI, %eax
	jmp	spl

/ 4.0 added
	.align	4
	.globl	splimp
splimp:
	movl    $SPLIMP, %eax
	jmp	spl
/

	.align	4
	.globl	spltty
spltty:
	movl    $SPLTTY, %eax
	jmp	spl

	.align	4
	.globl	splx
splx:
	movl	4(%esp), %eax		/ get new spl level

	/ fall through to spl

/ spl is the common routine for spl*() other than spl7() and splhi().
/
/ algorithm for spl:
/
/       turn off interrupts
/
/       oldipl = ipl;
/       ipl = newipl;
/
/	/* if new level is IPLHI, return with interrupts off */
/	if (newipl == IPLHI)
/		goto splretoff;
/
/	/* if new level is already in pic masks, just return */
/       if (newipl == picipl)
/               goto splreton;
/
/       setpicmasks();  /* load new masks into pics */
/
/splreton:
/       turn on interrupts
/splretoff:
/       return oldipl;

	.align	4
	.globl  spl     / for debugging; spl is not a public function
spl:
	/ spl is not valid unless picinit() sets the splvalid_flag
	cmpl	$0, splvalid_flag
	jnz	splstart
	movl	ipl, %eax
	ret

	/ new priority level is in %eax
splstart:
	cli                             / disable interrupts

	movl    ipl, %edx               / get current level


#ifdef DEBUG
	cmpl    $IPLHI, %eax            / if new level > IPLHI ..
	ja      splpanic                /   .. panic
	cmpl    $IPLHI, %edx            / if current level > IPLHI ..
	ja      splpanic                /   .. panic
#endif

	movl    %eax, ipl               / set new level
	pushl   %edx                    / save old level
	cmpl    $IPLHI, %eax            / if new level is IPLHI ..
	jae     splretoff               /   .. return w/ interrupts off
	cmpl    %eax, picipl            / if new level is in pics
	je      splreton                /   .. return w/ interrupts on,
					/   .. but without modifying pics

	pushl   %ebx
	pushl   %esi
	call    setpicmasks             / load masks for new level
	popl    %esi
	popl    %ebx

splreton:
	sti                             / enable interrupts
splretoff:
	popl    %eax                    / return old level
	ret

splpanic:
	call    splpanic2


/ splhi and spl7 simply do a cli to turn off interrupts,
/ and do not modify the PICs.

	.align	4
	.globl	splhi
	.globl	spl7
splhi:
spl7:
	/ spl is not valid unless picinit() sets the splvalid_flag
	cmpl	$0,splvalid_flag
	jnz	splstart2
	movl	ipl, %eax
	ret

	/ new priority level is in %eax
splstart2:
	cli                             / disable interrupts
	movl    ipl, %eax               / old priority level
#ifdef DEBUG
	cmpl    $IPLHI, %eax            / if old level > IPLHI ..
	ja      spl7panic                /   .. panic
#endif
	movl    $IPLHI, ipl             / new priority level = IPLHI
	ret

#ifdef DEBUG
spl7panic:
	call    spl7panic2
#endif


/ splint() is called from cmnint in ttrap.s to raise the interrupt priority
/ level to the level of the current interrupt, and send EOI to the pics.
/ It does not enable interrupts.
/ The interrupt number is passed to it in %edi.
/ splint() returns the old priority level in %eax,
/ or -1 if the interrupt is spurious.
/ Only %edi is preserved.
/
/       intno = %edi % 8;   picno = %edi / 8;   newipl = intpri[%edi];
/
/       /* check for spurious interrupts */
/       if (intno == 7) {         /* if interrupt on IR7, IR15, ... */
/               /* read ISR; if IS7 bit off, interrupt is spurious */
/               if (!(inb(cmdport[picno]) & 0x80)) {
/			if (%edi >= 8)
/				outb(cmdport[0], PIC_NSEOI);
/                       return -1;
/		}
/       }
/
/       oldipl = ipl;
/       ipl = newipl;
/
/       if (newipl != IPLHI)
/               setpicmasks();  /* load new masks into pics */
/
/       /* send non-specific EOI to master pic */
/       outb(cmdport[0], PIC_NSEOI);
/
/       if (picno != 0) {
/               /* send non-specific EOI to slave pic */
/               outb(cmdport[picno], PIC_NSEOI);
/       }
/
/       return oldipl;

	.align	4
	.globl  splint  / for debugging; splint is not a public function
splint:
	/ interrupts are disabled
	/ interrupt number is in %edi

	/ check for spurious interrupt reflected to IR7 of PIC

	movl	%edi, %ebx		/ %ebx <= picno  (%edi / 8)
	shrl	$3, %ebx
	movl	%edi, %eax		/ %eax <= intno  (%edi % 8)
	andl	$7, %eax
	cmpl	$7, %eax		/ if intno is IR7 of some PIC chip,
	jne	real_int		/   this must be a real interrupt

	movw	cmdport(,%ebx,2), %dx	/ get status port address

	/ PIC is in READ ISR mode
	inb	(%dx)			/ read ISR from PIC
	testb	$0x80, %al		/ check IS bit for interrupt 7
	jnz	real_int		/ if set, it's a real interrupt
	cmpl	$8, %edi		/ if intno < 8 ..
	jb	splintsret		/   .. don't clear master
	movb	$PIC_NSEOI, %al		/ send EOI to master PIC
	movw	cmdport, %dx		/ master PIC command port addr
	outb	(%dx)
splintsret:
	movl	$-1, %eax		/ return -1
	ret

	.align	4
real_int:
#ifdef DEBUG
	cmpl    %edi, nintr             / if interrupt number >= nintr
	jbe     splintpanic             /   .. panic
#endif

	movzbl  intpri(%edi), %eax      / priority level of interrupt
	movl    ipl, %edx               / current level

#ifdef DEBUG
	orl     %eax, %eax              / if new level == zero
	jz      splintpanic             /   .. panic
	cmpl    %edx, %eax              / if new level <= current level ..
	jbe     splintpanic             /   .. panic
	cmpl    $IPLHI, %eax            / if new level > IPLHI ..
	ja      splintpanic             /   .. panic
	cmpl    $IPLHI, %edx            / if old level >= IPLHI ..
	jae     splintpanic             /   .. panic
#endif

	pushl   %edx                    / save current level
	movl    %eax, ipl               / set new level
	cmpl    $IPLHI, %eax            / if new level is IPLHI ..
	jae     splinteoi               /   .. don't change the pic masks

	push	%ebx
	call    setpicmasks             / load masks for new level
	pop	%ebx

	/ send EOI to master PIC
splinteoi:
/*** 380 Support : begin ***/
#ifndef AT380
	movb    $PIC_NSEOI, %al         / non-specific EOI
	movw    cmdport, %dx            / master pic command port addr
	outb    (%dx)                   / send EOI to master pic
	orl	%ebx, %ebx		/ if master pic ..
	je      splintret               /   .. don't do slave

	/ send EOI to slave PIC
	movw    cmdport(,%ebx,2), %dx   / slave pic command port addr
	outb    (%dx)                   / send EOI to slave pic
#endif
/*** 380 Support : end ***/

splintret:
	popl    %eax                            / return the old level
	ret

#ifdef DEBUG
	.align	4
splintpanic:
	call    splintpanic2
#endif


/ splxint() is called from cmn_int in misc.s to restore the old interrupt
/ priority level after processing an interrupt.
/ It is called with interrupts disabled, and does not enable interrupts.
/ The old interrupt level is passed as an arg on the stack.
/ Only %edi is preserved.

	.align	4
	.globl  splxint         / for debugging; spl is not a public function
splxint:
	/ set up standard stack frame at least until debugged
	pushl   %ebp
	movl    %esp, %ebp

	movl    8(%ebp), %eax           / get new level arg
	movl    ipl, %edx               / get current level

#ifdef DEBUG
	cmpl    %edx, %eax              / if new level >= current level ..
	jae     splxintpanic                /   .. panic
	cmpl    $IPLHI, %eax            / if new level >= IPLHI ..
	jae     splxintpanic                /   .. panic
	cmpl    $IPLHI, %edx            / if old level > IPLHI ..
	ja      splxintpanic                /   .. panic
#endif

	movl    %eax, ipl               / set new level
	cmpl    %eax, picipl            / if level in pics is same as new
	je      splxintret              /   .. return without modifying pics

	call    setpicmasks             / load masks for new level

splxintret:
	/ restore stack frame and return
	popl    %ebp
	ret

#ifdef DEBUG
splxintpanic:
	call    splxintpanic2
#endif


/ enableint() enables an interrupt by clearing its mask bit in iplmask[0],
/ and reloading the pic masks.

	.align	4
	.globl  enableint
enableint:
	/ set up standard stack frame at least until debugged
	pushl   %ebp
	movl    %esp, %ebp
	pushl   %ebx
	pushl   %esi

	pushfl                          / save flags for IF
	cli                             / turn off interrupts

	movl    8(%ebp), %ecx           / get interrupt index arg
	btrl	%ecx, iplmask		/ clear mask bit in iplmask[0][pic]

	movl    picipl, %eax            / load masks for level in pics
	call    setpicmasks             / load masks for new level

	popfl                           / restore flags

	/ restore stack frame and return
	popl    %esi
	popl    %ebx
	popl    %ebp
	ret


/ disableint() disables an interrupt by setting its mask bit in iplmask[0],
/ and reloading the pic masks.

	.align	4
	.globl  disableint
disableint:
	/ set up standard stack frame at least until debugged
	pushl   %ebp
	movl    %esp, %ebp
	pushl   %ebx
	pushl   %esi

	pushfl                          / save flags for IF
	cli                             / turn off interrupts

	movl    8(%ebp), %ecx           / get interrupt index arg
	btsl	%ecx, iplmask		/ set mask bit in iplmask[0][pic]

	movl    picipl, %eax            / load masks for level in pics
	call    setpicmasks             / load masks for new level

	popfl                           / restore flags

	/ restore stack frame and return
	popl    %esi
	popl    %ebx
	popl    %ebp
	ret


/ setpicmasks() loads new interrupt masks into the pics.
/ It is called from other routines in this file with new level in %eax
/ and interrupts off.
/ Only %edi is preserved.
/
/ algorithm for setpicmasks():
/
/       int newipl;     /* temp for new interupt priority level */
/       int newmask;    /* temp for new mask value */
/
/       newipl = %eax;
/
/       for (i = picmax; i >= 0; i--) {
/               /* the level 0 mask has bits set for permanently disabled
/                  interrupts */
/               newmask = iplmask[newipl][i] | iplmask[0][i];
/
/               /* if the pic mask is not correct, load it */
/               if (newmask != curmask[i]) {
/                       outb(imrport[i], newmask);
/                       curmask[i] = newmask;
/               }
/       }
/
/       inb(imrport[0]);        /* to let master pic settle down */

	.align	4
	.globl  setpicmasks     / for debugging; not a public routine
setpicmasks:
	/ new interrupt priority level is in %eax

#ifdef DEBUG
	cmpl    $IPLHI, %eax            / if new level >= IPLHI
	jae     setpicmaskspanic        /   .. panic
#endif

	movl    %eax, picipl            / set pic ipl
	movl    npic, %ecx              / number of pics
	mull    %ecx                    / %eax = newipl * npic
	leal    iplmask(%eax), %ebx     / &iplmask[newipl][0]
	movl    $curmask, %esi          / &curmask[0]
	subl    %edx, %edx              / clear %edx

picloop:
	decl    %ecx                    / decrement pic index
	js      picread                 / break if index < 0

	movb    (%ebx, %ecx), %al       / new level mask
	orb     iplmask(%ecx), %al      / OR in level zero mask
	cmpb    (%esi, %ecx), %al       / compare to current mask
	je      picloop                 / don't load pic if identical

	movw    imrport(,%ecx,2), %dx   / mask register port addr for pic
	outb    (%dx)                   / output new mask that is in %al
	movb    %al, (%esi, %ecx)       / set new current mask
	jmp     picloop

	.align	4
picread:
	orl     %edx, %edx              / if we modified a pic ..
	jz      setpicmasksret
	inb     (%dx)                   /  .. read last mask register modified
					/  .. to allow the pics to settle
setpicmasksret:
	ret

#ifdef DEBUG
setpicmaskspanic:
	call    setpicmaskspanic2
#endif

/ end of spl code


/ Put 386/20 into protected mode and call mlsetup/main to do rest of
/ initialization. On return from main, we want to do an inter-level
/ return to the init process. Push enough on stack to make it look like
/		oldss
/		oldesp
/		oldefl
/		oldcs
/		oldeip
/ and initialize ds/es to point to user ds
/ Now a reti will take us to the user process.
/		
	.set	PROTPORT, 0xE0
	.set	PROTVAL, 1

	.globl	vstart
	.globl	u
	.globl	userstack
	.globl	v
	.globl	mlsetup
	.globl	cmn_err
	.globl	main

	.align	8
vstart:
#if defined (MB1) || defined (MB2)
	movw	$PROTPORT, %dx		/ dx<- PORT addr for putting
					/ 386/20 into protected mode
	movb	$PROTVAL, %al		/ value to be output for putting
					/ 386/20 into protected mode
	outb	(%dx)
#endif
	call	mlsetup

/ Until p0u has run (in mlsetup), we cannot use the kernel
/ stack in the u structure. The intial ktss is set up so we
/ use the slop space between the end of pmon's data space and
/ the start of kernel text. p0u resets it to the correct value so
/ proc 0's tss is valid. We'll switch to u_stack now.
	movl	$u+KSTKSZ, %esp

/
/ Do 80386 B1 stepping detection, if requested.
/
/ First, we check the tuneable; 2 means do automatic detection
/
	cmpl	$2,do386b1		/ if do386b1 < 2,
	jb	skip_b1_detect		/    skip B1 detection
/
/ The detection is done by looking for the presence of the Errata 5 bug,
/ which causes a single step of REP MOVS to go through 2 iterations, not 1.
/
	pushl	[idt+8+4]		/ save current debug trap
	pushl	[idt+8]
	movl	$b1_ss_trap,%eax
	movw	%ax,[idt+8]		/ set debug trap to b1_ss_trap
	shrl	$16,%eax
	movw	%ax,[idt+8+6]
	pushfl				/ save flags register
	movl	$2,%ecx			/ set up for a REP MOVS w/count of 2
	movl	$u,%esi			/   (to and from an arbitrary addr)
	movl	%esi,%edi
	pushl	$PS_T			/ set the single step flag
	popfl
	rep
	movsb
b1_ss_trap:
	addl	$12,%esp		/ skip the iret
	xorl	$1,%ecx			/ ECX has 1 if no bug, else 0 -
	movl	%ecx,do386b1		/   store 0 or 1, resp., in d0386b1
	popfl				/ restore flags
	popl	[idt+8]			/ restore debug trap
	popl	[idt+8+4]
skip_b1_detect:
/
/ Set up for floating point.  Check for any chip at all by tring to
/ do a reset.  if that succeeds, differentiate via cr0.
/
	clts                            / clear task switched bit in CR0
	fninit                          / initialize chip
	fstsw	%ax			/ get status
	orb	%al,%al			/ status zero? 0 = chip present
	jnz     mathemul                / no, use emulator
/
/ at this point we know we have a chip of some sort; 
/ use cr0 to differentiate.
/
	movl    %cr0,%edx               / check for 387 present flag
	testl	$CR0_ET,%edx            / ...
	jz      is287                   / z -> 387 not present
	movb    $FP_387,fp_kind         / we have a 387 chip
	movl	do386b1,%eax
	movl	%eax,do386b1_387	/ set flag for B1 workarounds and 387 chip
	jmp     mathchip
/
/ No 387; we must have an 80287.
/
is287:
	fsetpm				/ set the 80287 into protected mode
	movb    $FP_287,fp_kind         / we have a 287 chip
/
/ We have either a 287 or 387.
/
mathchip:
	andl    $-1![CR0_TS|CR0_EM],%edx	/ clear emulate math chip bit
	orl     $CR0_MP,%edx            / set math chip present bit
	movl    %edx,%cr0               / in machine status word
	movl	do386b1,%eax
	movl	%eax,do386b1_x87	/ set flag for B1 workarounds and math chip
	orl	%eax,%eax
	jnz	b1_enabled		/ if B1 workarounds disabled
	movl	%eax,do387cr3		/    disable do387cr3
	jmp	cont
b1_enabled:
	cmpl	$2,do387cr3		/ if = 2, auto-detect if workaround
	jb	skip_cr3_detect		/    is possible on this machine
/
/ At this point, we have determined that the 387cr3 workaround for
/ B1 stepping Errata 21 is desired.  Determine if the hardware can support
/ the workaround.
/
	movl	$0,do387cr3		/ clear do387cr3 for now
	pushl	kspt0			/ save current pte (for D0000000)
			/ set pte to 2G alias for kpd0
	movl	$0x80000001+[KPTBL_LOC+0x1000],kspt0
	movl	%cr3,%eax		/ flush tlb
	movl	%eax,%cr3
	movl	kpd0,%eax		/ read the first kpd0 entry
	cmpl	%eax,0xD0000000		/ does it match the aliased value?
	jne	cr3_disable		/ no, we can't use workaround
	incl	0xD0000000		/ change the entry via the alias
	incl	%eax
	cmpl	%eax,kpd0		/ did the kpd0 entry change also?
	jne	cr3_disable		/ no, we can't use workaround
	decl	kpd0			/ alias was successful, restore kpd0
	incl	do387cr3		/ turn on do387cr3 flag
cr3_disable:
	popl	kspt0			/ restore pte
	movl	%cr3,%eax		/ flush tlb
	movl	%eax,%cr3
skip_cr3_detect:
	cmpl	$0,do387cr3
	jz	cont
	movl	$0x80000000,fp387cr3	/ enable B1 workaround #21
	movl	%cr3, %eax
	orl	fp387cr3,%eax
	movl	%eax, %cr3
	jmp	cont
/
/ Assume we have an emulator.
/
mathemul:
	movl    %cr0,%edx
	andl    $-1!CR0_MP,%edx         / clear math chip present
	orl     $CR0_EM,%edx            / set emulate math bit
	movl    %edx,%cr0               / in machine status word
	movb    $FP_SW,fp_kind          / signify that we are emulating
cont:

#ifdef WEITEK
/
/ test for presence of weitek chip
#ifdef AT386
/ test for presence of weitek chip
/ we're going to commandeer a page of kernel virtual space to map in 
/ the correct physical addresses.  then we're going to play with what
/ we hope to be weitek addresses.  finally, we'll put things back the
/ way they belong.
/
/ extern unsigned long weitek_paddr;	/* chip physical address */
/
	cmpl	$0, weitek_paddr	/ if (weitek_paddr == 0)
	jz	weitek_skip		/	goto weitek_skip;
	pushl	%ebx
	pushl	kspt0
	movl	$0xc0000003, kspt0	/ pfn c0000, sup, writeable, present
	movl	%cr3, %eax		/ flush tlb
	movl	%eax, %cr3
	movl	$KVSBASE, %ebx		/ base address for weitek area
	movb	$WEITEK_HW, weitek_kind	/ first assume that there is a chip
	movl	$0x3b3b3b3b, 0x404(%ebx) / store a value into weitek register.
	movl	0xc04(%ebx), %eax	/ and read it back out.
	cmpl	$0x3b3b3b3b, %eax
	jnz	noweitek		/ no chip
	/ clear weitek exceptions so that floating point exceptions
	/ are reported correctly from here out
	/ initialize the 1167 timers
	movl    $0xc000c003, kspt0      / pfn c000c, sup, writeable, present
	movl	%cr3, %eax		/ flush tlb
	movl	%eax, %cr3
	movl	$0xB8000000, 0x000(%ebx)
	movl	0x400(%ebx), %eax	/ Check for 20 MHz 1163
	andl	$WEITEK_20MHz, %eax
	jnz	w_init_20MHz
	movl 	$0x16000000, 0x000(%ebx)	/ 16 MHz 1164/1165 flowthrough
						/ timer
	jmp	w_init_wt1

w_init_20MHz:
	movl	$0x56000000, 0x000(%ebx)	/ 20 MHz 1164/1165 flowthrough
	movl	$0x98000000, 0x000(%ebx)	/ timer
	
w_init_wt1:
	movl 	$0x64000000, 0x000(%ebx)	/ 1164 accumulate timer
	movl 	$0xA0000000, 0x000(%ebx)	/ 1165 accumulate timer
	movl 	$0x30000000, 0x000(%ebx)	/ Reserved mode bits (set to 0).
	movl 	weitek_cfg, %eax	/ Rounding modes and Exception
	movl 	%eax, 0x000(%ebx)	/ enables.
	movw	$0xF0, %dx		/ clear the fp error flip-flop
	movb	$0, %al
	outb	(%dx)
	/
	jmp	weitek_done
noweitek:
	movb	$WEITEK_NO, weitek_kind		/ no. no weitek
#endif /* AT386 */

#if defined(MB1) || defined(MB2)
/ test for presence of weitek chip for MB
	inb	$0xe4			/ Read the Weitek present I/O port
	testb	$1,%al			/ Is the Weitek there?
	jz	weitek_done		/ No.
	movb	$WEITEK_HW, weitek_kind	/ Yes.
#endif /* MB1 || MB2 */

weitek_done:
	popl	kspt0			/ get the old kpt0[0] back
	movl	%cr3, %eax		/ flush tlb
	movl	%eax, %cr3
	popl	%ebx
weitek_skip:

#endif /* WEITEK */

	/ 
	/ extern int	margc;
	/ extern char	*margv[];
	/
	/ main(margc, margv);
	/
	pushl	$margv
	pushl	margc

	call	main

	addl	$8,%esp			/ pop off margc and margv

	cmpl	$UVTEXT, %eax
	je	to_user
	call	*%eax			/ Kernel process. Shouldn't return
	jmp	.
to_user:
	pushl	$USER_DS		/ oldss
	pushl	$userstack		/ old esp
	pushfl				/ old efl
	pushl	$USER_CS		/ old cs
	pushl	$UVTEXT			/ old eip

	movw	$USER_DS, %ax
	movw	%ax, %ds
	movw	%ax, %es
	iret

	/ NEVER REACHED
	jmp	.


	.align	4
	.globl	idle
idle:
	call	spl0		/ enable interrupts
	hlt			/ Causes bus timeouts.
_waitloc:
	ret

	.data

	.globl	waitloc
waitloc:
	/ Address of _waitloc, used in clock.  Not clear why they
	/ need to do this instead of making _waitloc public.
	.long	_waitloc

	.text


/ The following routines read and write I/O adress space
/ outl(port address, val)
/ outw(port address, val)
/ outb(port address, val)
/ long  inw(port address)
/ ushort inw(port address)
/ unchar  inb(port address)

	.set	PORT, 8
	.set	VAL, 12

	.align	4
	/* XENIX Support */
	.globl	outd
outd:
	/* End XENIX Support */
	.globl	outl
outl:	pushl	%ebp
	movl	%esp, %ebp
	movw	PORT(%ebp), %dx
	movl	VAL(%ebp), %eax
	outl	(%dx)
	popl	%ebp
	ret

	.align	4
	.globl	outw
outw:	pushl	%ebp
	movl	%esp, %ebp
	movw	PORT(%ebp), %dx
	movw	VAL(%ebp), %ax
	data16
	outl	(%dx)
	popl	%ebp
	ret

	.align	4
	/* XENIX Support */
	.globl	iooutb
iooutb:
	/* End XENIX Support */
	.globl	outb
outb:	pushl	%ebp
	movl	%esp, %ebp
	movw	PORT(%ebp), %dx
	movb	VAL(%ebp), %al
	outb	(%dx)
	popl	%ebp
	ret

	.align	4
	/* XENIX Support */
	.globl	ind
ind:
	/* End XENIX Support */
	.globl	inl
inl:	pushl	%ebp
	movl	%esp, %ebp
	movw	PORT(%ebp), %dx
	inl	(%dx)
	popl	%ebp
	ret

	.align	4
	.globl	inw
inw:	pushl	%ebp
	movl	%esp, %ebp
	subl    %eax, %eax
	movw	PORT(%ebp), %dx
	data16
	inl	(%dx)
	popl	%ebp
	ret

	.align	4
	/* XENIX Support */
	.globl	ioinb
ioinb:
	/* End XENIX Support */
	.globl	inb
inb:	pushl	%ebp
	movl	%esp, %ebp
	subl    %eax, %eax
	movw	PORT(%ebp), %dx
	inb	(%dx)
	popl	%ebp
	ret

/
/ The following routines move strings to and from an I/O port.
/ loutw(port, addr, count);
/ linw(port, addr, count);
/* XENIX Support */
/ repinsw(port, addr, cnt) - input a stream of 16-bit words
/ repoutsw(port, addr, cnt) - output a stream of 16-bit words
/* End XENIX Support */
/
	.set	PORT, 8
	.set	ADDR, 12
	.set	COUNT, 16

	.align	4
	/* XENIX Support */
	.globl	repoutsw
repoutsw:
	/* End XENIX Support */
	.globl	loutw
loutw:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edx
	pushl	%esi
	pushl	%ecx
	movl	PORT(%ebp),%edx
	movl	ADDR(%ebp),%esi
	movl	COUNT(%ebp),%ecx
	rep
	data16
	outsl
	popl	%ecx
	popl	%esi
	popl	%edx
	popl	%ebp
	ret

	.align	4
	/* XENIX Support */
	.globl	repinsw
repinsw:
	/* End XENIX Support */
	.globl	linw
linw:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edx
	pushl	%edi
	pushl	%ecx
	movl	PORT(%ebp),%edx
	movl	ADDR(%ebp),%edi
	movl	COUNT(%ebp),%ecx
	rep
	data16
	insl
	popl	%ecx
	popl	%edi
	popl	%edx
	popl	%ebp
	ret

/* XENIX Support */
/	repinsb(port, addr, cnt) - input a stream of bytes
/	repinsd(port, addr, cnt) - input a stream of 32-bit words
/	repoutsb(port, addr, cnt) - output a stream of bytes
/	repoutsd(port, addr, cnt) - output a stream of 32-bit words

	.set	BPARGBAS, 8
	.set	io_port, BPARGBAS
	.set	io_addr, BPARGBAS+4
	.set	io_cnt,  BPARGBAS+8

/ repinsb(port, addr, count);
/ NOTE: count is a BYTE count

	.align	4
	.globl	repinsb
repinsb:
	push	%ebp
	mov	%esp,%ebp
	push	%edi

	mov	io_addr(%ebp),%edi
	mov	io_cnt(%ebp),%ecx
	mov	io_port(%ebp),%edx	/ read from right port

	rep
	insb				/ read them bytes

	pop	%edi
	pop	%ebp
	ret


/ repinsd(port, addr, count);
/ NOTE: count is a DWORD count

	.align	4
	.globl	repinsd
repinsd:
	push	%ebp
	mov	%esp,%ebp
	push	%edi

	mov	io_addr(%ebp),%edi
	mov	io_cnt(%ebp),%ecx
	mov	io_port(%ebp),%edx	/ read from right port

	rep
	insl				/ read them dwords

	pop	%edi
	pop	%ebp
	ret

/ repoutsb(port, addr, count);
/ NOTE: count is a byte count

	.align	4
	.globl	repoutsb
repoutsb:
	push	%ebp
	mov	%esp,%ebp
	push	%esi

	mov	io_addr(%ebp),%esi
	mov	io_cnt(%ebp),%ecx
	mov	io_port(%ebp),%edx

	rep
	outsb

	pop	%esi
	pop	%ebp
	ret

/ repoutsd(port, addr, count);
/ NOTE: count is a DWORD count

	.align	4
	.globl	repoutsd
repoutsd:
	push	%ebp
	mov	%esp,%ebp
	push	%esi

	mov	io_addr(%ebp),%esi
	mov	io_cnt(%ebp),%ecx
	mov	io_port(%ebp),%edx

	rep
	outsl

	pop	%esi
	pop	%ebp
	ret
/* End XENIX Support */

/
/	min() and max() routines
/
	.set	ARG1, 8
	.set	ARG2, 12

	.align	4
	.globl	min
min:
	pushl	%ebp				/ save old %ebp
	movl	%esp, %ebp
	movl	ARG1(%ebp), %eax		/ %eax <- arg1
	cmpl	ARG2(%ebp), %eax		/ compare args
	jbe	minxit				/ ? %eax <= ARG2. unsigned
	movl	ARG2(%ebp), %eax		/ %eax = arg2
	.align	4
minxit:
	popl	%ebp
	ret


	.align	4
	.globl	max
max:
	pushl	%ebp
	movl	%esp, %ebp			/ set stack frame
	movl	ARG1(%ebp), %eax		/ %eax<-arg1
	cmpl	ARG2(%ebp), %eax		/ compare args
	jae	maxit				/ ?%eax>=ARG2. unsigned
	movl	ARG2(%ebp), %eax		/ %eax = arg2
maxit:
	popl	%ebp
	ret


/
/	upc_scale(pc - u.u_prof.pr_offset, u.u_prof.pr_scale)
/		returns slot number in u.u_prof.pr_base array
/
	.set	REL_PC, 4	/ note: offsets relative to %esp
	.set	SCALE, 8

	.align	4
	.globl	upc_scale
upc_scale:
	movl	REL_PC(%esp), %eax	/ pc relative to start of region
	mull	SCALE(%esp)		/ Multiply by fractional result
	shrdl	$17,%edx,%eax		/ Scale down to a useful range (>> 17)
	ret


	.globl	oldproc
	.globl	segu_release
	.globl	curproc
#ifdef WEITEK
	.globl  weitek_save
#endif
	.align	4
#ifdef	KPERF
	.globl	KPswtch
KPswtch:
#else	/* !KPERF */
	.globl	swtch
swtch:
#endif	/* KPERF */
	call	pswtch
	/ At this point we have already done all the mapping work for
	/ the new process. The tss of the new process is mapped by
	/ JTSSSEL.  LDTSEL descr contains linear addr of the new ldt
	/ (as it will appear in the address mapping of the new process).
	movl	u+u_procp, %eax		/ Is the old process same as
	cmpl	%eax, curproc		/ the new one?
	je	noswtch			/ If so skip the context switch

#ifdef WEITEK
	/ "call	weitek_save" equivalent
	/ however we still call weitek_save to do the WEITEK_HW testing!
	movl	weitek_proc, %ecx
	jcxz	donothing
	cmpl	%ecx, u+u_procp
	jne	donothing
	pushl	$u+u_weitek_reg
	call	weitek_save
	popl	%ecx
	movl	u+u_procp, %eax		/ reload %eax with u.u_procp
donothing:
#endif

	/ clear 286 system call gate if used by this process
	movl    u+u_callgatep, %ecx     / addr of call gate in gdt
	jcxz	swnocallgate		/ if null, don't bother
	subl    %edx, %edx              / zero
	movl    %edx, 0(%ecx)
	movl    %edx, 4(%ecx)
swnocallgate:

	/ copy reference and modify bits from usertable[] to p_ubptbl[]
	pushl	%esi
	pushl	%edi
	movl	u+u_tss, %esi		/ The ljmp below will save the current
	movl	$u+KSTKSZ, t_esp0(%esi)	/ state into the outgoing tss. We must
                                        / make sure that page gets marked as
                                        / modified. This won't happen auto-
                                        / matically, since the old task segment
                                        / is addressed via the usertable pte's,
                                        / not the p_ubptbl pte's. Instead, we
					/ do it here by modifying the ESP0
					/ field in the tss before we copy the
					/ modify bits.
	movzwl	p_usize(%eax), %ecx
	movl	usertable, %esi
	movl	p_ubptbl(%eax), %edi
rmcopy_loop:
	slodl
	andl	$[PG_REF+PG_M], %eax
	orl	(%edi), %eax
	sstol
	loop	rmcopy_loop
	popl	%edi
	popl	%esi

jmptotss:
	cli				/ disable interrupts
	str	%dx			/ Remember which TSS we're in
	ljmp	$JTSSSEL, $0		/ Old process will resume at
					/ the next instruction

	/ newproc returns here for the child. Remember
	/ to preserve the register variables (ebx, esi,
	/ edi) and eax.
	/
	/ Watch out for ebx, which holds a pointer to
	/ the tss.

	.align	4
	.globl	resume
resume:
	movl	$nmi_stack+KSTKSZ, %esp	/ usable stack in case of nmi

	movl	curproc, %eax		/ eax = curproc
	movzwl	p_usize(%eax), %ecx
	movl	p_ubptbl(%eax), %esi
	movl	usertable, %edi
	rep				/ copy p_usize page table entries
	smovl				/ from p_ubptbl[] to usertable[]

	/
	/ First ublock page must be user read/write, for two reasons:
	/ 1. To get around a bug in the B1 stepping of the 80386, the
	/    kernel stack must be user readable.
	/ 2. Floating point emulators (executing in user mode) must be
	/    able to read and write the floating-point state.
	/ So, the ublock is set up so the first page contains (only) the
	/ kernel stack and the floating-point state.

	movl	usertable, %eax
	orl	$[PG_US+PG_RW], (%eax)

	call	restorepd		/ set up curproc's page directory

	/
	/ now running on new ublock
	/

	leal	gdt+[JTSSSEL!0x7], %esi	/ esi = &gdt[JTSSSEL]
	movb	7(%esi), %bh
	movb	4(%esi), %bl
	shl	$16, %ebx
	movw	2(%esi), %bx		/ ebx = &(tss used for context switch)

	movl	t_esp(%ebx), %esp	/ use kernel stack in ublock
	ltr	t_edx(%ebx)

	/* XENIX Support */
	leal	idt+[0xf0\*8], %edi	/ if necessary,
	movl	u+u_fpintgate, %eax	/ customize idt[0xf0 .. 0xff]
	movl	u+u_fpintgate+4, %esi	/ for this process
	cmpl	%eax, (%edi)
	jne	idtchg
	cmpl	%esi, 4(%edi)
	je	idtok
idtchg:
	movl	$[2\*0x10], %ecx	
idtfill:
	sstol				/ copy u_fpintgate to the idt[]
	xchgl	%eax, %esi
	loop	idtfill
idtok:
/* End XENIX Support */
	sti                             / enable interrupts

#ifdef MERGE386
	cmpl	$0, merge386enable
	je	novm86_swtch
	pusha				/ save registers
	call	vm86_swtch		/ call C routine for special processing
	popa				/ of switches to and from vm86 proc
novm86_swtch:
#endif	/* MERGE386 */

#ifdef WEITEK
	/ "call	weitek_restore"
	cmpb	$0, u+u_weitek
	je	skipit
	movl	weitek_proc, %eax
	cmpl	%eax, u+u_procp
	je	skipit
	call	init_weitek
	pushl	$u+u_weitek_reg
	call	weitek_restore
	popl	%ecx
	movl	u+u_procp, %eax
	movl	%eax, weitek_proc
skipit:
#endif

	/ We need to free the ublock for the previous
	/ process if it did an exit.
	movl	oldproc, %ecx
	jcxz	restorecallgate
	pushl	%ecx
	call	segu_release
	popl	%ecx
	movl	$0, oldproc

	/
	/ restore 286 system call gate if used by this process
	/
restorecallgate:
	movl    u+u_callgatep, %ecx     / addr of call gate in gdt
	jcxz	nocallgate
	movl    u+u_callgate, %eax
	movl    %eax, 0(%ecx)
	movl    u+u_callgate+4, %eax
	movl    %eax, 4(%ecx)
nocallgate:

#ifdef	DEBUG
	call	db_resume
#endif

	mov	t_eax(%ebx), %eax	/ restore modified register variables
	mov	t_edi(%ebx), %edi	/ (ebx, esi, edi) and eax
	mov	t_esi(%ebx), %esi	/ from the tss
	mov	t_ebx(%ebx), %ebx

noswtch:
	ret

/
/	bzero(addr, len)
/
/	This is the block zero routine; arguments are the 
/	starting address and length.
/
/	struct_zero() added to catch any references to the fast inline
/	structure zeroing routine that, for some reason, could not
/	be put inline.  An example for this is that the file could not
/	include sys/inline.h.
/
/	bzeroba(addr, bytes) - used when faulting in the last page
/	to clear non-word aligned addresses/sizes. Assumes that after
/	clearing to word size, address will be word aligned.
/	N. b.: that means it ENDS on a word boundary.

	.set	ZEROADDR, 8
	.set	ZEROCOUNT, 12

	.align	4
	.globl	bzeroba
	.globl	bzero
	.globl	struct_zero
bzero:
struct_zero:
bzeroba:
	pushl	%ebp			/ save stack base
	movl	%esp, %ebp		/ set new stack base
	pushl	%edi			/ save %edi

	movl	ZEROADDR(%ebp), %edi	/ %edi <- address of bytes to clear

	movl	$0, %eax		/ sstol val
	movl	ZEROCOUNT(%ebp), %ecx	/ get size in bytes
	cmpl	%eax, %ecx		/ short circut if len = 0
	jz	bzdone
	shrl	$2, %ecx		/ Count of double words to zero
	repz
	sstol				/ %ecx contains words to clear(%eax=0)
	movl	ZEROCOUNT(%ebp), %ecx	/ get size in bytes
	andl	$3, %ecx		/ do mod 4
	repz
	sstob				/ %ecx contains residual bytes to clear
bzdone:
	popl	%edi
	popl	%ebp
	ret


/
/	This is the block copy routine.
/
/		bcopy(from, to, bytes)
/		caddr_t from, to;
/		{
/			while( bytes-- > 0 )
/				*to++ = *from++;
/		}
/
/ THIS IS ALL NEW CODE AND HENCE SHOULD BE REVIEWED
/
/ This code assumes no faults(except page) will occur while executing this code
/ Hence this can not be used for copying data to or from user memory
/ Use copyin() and copyout() instead
/
/ Several Cases We have to take care of:
/	1) Source is word aligned.
/		Copy as many whole words as possible.
/		Go to 3 
/	2) Source is not word aligned
/		Copy bytes until source is word aligned.
/		Go to 1)
/	3) Count of bytes is less than NBPW
/		copy bytes

	.set	FROMADR, 8		/ source address
	.set	TOADR, 12		/ destination address
	.set	BCOUNT, 16		/ count of bytes to copy

	.align	4
	.globl	bcopy
bcopy:
	pushl	%ebp
	movl	%esp, %ebp		/ setup stack frame
	pushl	%ebx
	pushl	%esi
	pushl	%edi			/ save registers

/	**************************************************
/	 common entry from bcopy, fbcopy()
/	**********************************************
allcopy:
	movl	BCOUNT(%ebp), %ebx	/ get count
	orl	%ebx, %ebx
	jz	bcpdone			/ count ?= 0
	movl	FROMADR(%ebp), %esi	/ get source address
	movl	TOADR(%ebp), %edi	/ get destination address
	cmpl	$NBPW, %ebx		/ are there less bytes to copy
					/ than in a word ?
	jl	bcptail			/ treat it like the tail at end
					/ of a non-aligned bcopy
	testl	$NBPW-1, %esi		/ is source address word aligned?
	jz	bcpalign		/ do copy from word-aligned source
	/
	/ copy bytes until source address is word aligned 
	/
	movl	%esi, %ecx		/ get source address
	andl	$NBPW-1, %ecx		/ get count of bytes to copy to get %esi
					/ word aligned
	subl	%ecx, %ebx		/ decrement count of bytes to copy
	repz
	smovb				/ move bytes from %ds:si -> %es:di
	/ if no of bytes to move now is less than in a word
	/ jump to tail copy
	cmpl	$NBPW, %ebx
	jl	bcptail 

	/ %esi= word aligned source address
	/ %edi = destination address
	/ %ebx = count of bytes to copy
bcpalign:
	movl	%ebx, %ecx		/ get count of bytes to copy
	shrl	$2, %ecx		/ convert to count of words to copy
	andl	$NBPW-1, %ebx		/ %ebx gets remainder bytes to copy
	repz
	smovl				/ copy words

	/ %esi=source address
	/ %edi = dest address
	/ %ebx = byte count to copy
bcptail:
	movl	%ebx, %ecx
	repz
	smovb
bcpdone:
	subl	%eax, %eax	/ return zero
bcpfault:
	popl	%edi
	popl	%esi
	popl	%ebx
	popl	%ebp
	ret

/ **********************************************************************
/ 'fbcopy' - fast block copy from the AT&T 3B2 port.
/ fbcopy (from, to, clicks).
/ We just convert clicks to bytes, then do bcopy.
/ **********************************************************************

	.align	4
	.globl	fbcopy
fbcopy:
	pushl	%ebp
	movl	%esp, %ebp		/ setup stack frame
	pushl	%ebx
	pushl	%esi
	pushl	%edi			/ save registers

/ Convert the argument from clicks to bytes on the stack, then go to allcopy.
/
/ Shift left 12 = multiply by 4096

	shll	$12, BCOUNT(%ebp)
	jmp	allcopy


	/ Read in pathname from kernel space 
	/ spath (from, to, maxbufsize)
	/ Returns -2 if pathname is too long, otherwise returns
	/  the pathname length.

	.set	FROM, 8
	.set	TO, 12
	.set	MAXBUFSIZE, 16

	.align	4
	.globl	spath
	.globl	kpath
spath:
kpath:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%edi
	
	movl	FROM(%ebp), %esi	/ calculate the pathname length
	movl	%esi, %edi		/ %edi = %esi = start of string
	movb	$0, %al
	movl	MAXBUFSIZE(%ebp), %ecx
	incl	%ecx			/ make sure we scan long enough
	repnz
	scab				/ scan for null terminator
	decl	%edi			/ back up to null (if it's there)

	movl	%edi, %ecx
	subl	%esi, %ecx		/ Compare the pathname length
	movl	MAXBUFSIZE(%ebp), %edi	/ with the maximum size of 
	cmpl	%ecx, %edi		/ the buffer
	jbe	splenerr		/ Error, if the length is 
					/ > (maxbufsize-1), -1 for NULL

	movl	%ecx, %eax		/ return length in %eax
	incl	%ecx			/ must copy null
	movl	TO(%ebp), %edi		/ destination
	repz
	smovb

spreturn:
	popl	%edi
	popl	%esi
	popl	%ebp
	ret

splenerr:
	movl	$-2, %eax		/ return error (-2) on pathname
					/ length error 
	jmp	spreturn



/	searchdir(buf, n, target) - search a directory for target
/	Return offset into directory of match, or if no match found
/       then return offset into directory of empty slot, or if none
/	found, then return -1

	.set	BUF, 8
	.set	BUFSIZ,	12
	.set	TARGET,	16
	.set	DIRENT,	16	/ size of a directory entry
	.set	DIRSIZ,	14	/ size of a file name

	.align	4
	.globl	searchdir
searchdir:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	pushl	%esi
	pushl	%edi
	movl	BUF(%ebp), %esi			/ pointer to directory
	movl	BUFSIZ(%ebp), %ebx		/ directory length in bytes
	movl	$0, %edx			/ pointer to empty slot
			/ get length of target string
	movl	TARGET(%ebp), %edi		/ address of target name
	movl	$DIRSIZ, %ecx
	movb	$0, %al
	repnz
	scab
	movl	$DIRSIZ, %eax
	subl	%ecx, %eax			/ %eax=length of target
s_top:
	cmpl	$DIRENT, %ebx			/ length less than 16?
	jl	sdone				/ done if less
	cmpw	$0, (%esi)			/ directory entry empty?
	je	sempty				/ jump if true
	pushl	%esi				/ save start of entry
	addl	$2, %esi			/ address of file name
	movl	TARGET(%ebp), %edi		/ address of target name

	movl	%eax, %ecx			/ length of target name
	repz
	scmpb
	jcxz	smatch				/ the names match
	popl	%esi				/ restore start of entry
scont:
	addl	$DIRENT, %esi			/ increment directory pointer
	subl	$DIRENT, %ebx			/ decrement size
	jmp	s_top				/ keep looking

	.align	4
sempty:
	cmpl	$0, %edx			/ do we need an empty slot?
	jne	scont				/ jump if no
	movl	%esi, %edx			/ save current offset
	jmp	scont				/ and goto to next entry

	.align	4
smatch:
	movb	-1(%esi), %cl
	cmpb	%cl, -1(%edi)
	je 	srmatch				/ really a match
	popl	%esi				/ restore start of entry
	jmp	scont				/ not really a match.
						/ Just a substring
	.align	4
srmatch:
	popl	%esi				/ restore start of entry
	subl	BUF(%ebp), %esi		 	/ convert to offset
	movl	%esi, %eax			/ return offset
	jmp	s_exit

	.align	4
sdone:
	movl	$-1, %eax			/ save failure return
	cmpl	$0, %edx			/ empty slot found?
	je	sfail				/ jump if false
	subl	BUF(%ebp), %edx			/ convert to offset
	movl	%edx, %eax			/ return empty slot
sfail:
s_exit:
	popl	%edi
	popl	%esi
	popl	%ebx
	popl	%ebp
	ret	


#if 0 /* #ifndef DEBUG */
/
/	NOTE: if DEBUG use plock/prele C code in os/pipe.c .
/
/	plock(ip)	/* lock an inode */
/	register struct inode *ip;
/	{
/		while (ip->i_flag & ILOCK) {
/			ip->i_flag |= IWANT;
/			sleep((caddr_t)ip, PINOD);
/		}
/		ip->i_flag |= ILOCK;
/	}
/
/ NOTE:
/ We could probably remove frame save and simply reference off of the stack,
/ but 3b2 makes a reference to being able to trace this procedure; so, to
/ keep the frames in order, we have three extra instructions.

	.align	4
	.globl	plock
plock:
	pushl	%ebp
	movl	%esp, %ebp		/ establish stack frame
plretry:
	movl	8(%ebp), %edx		/ get ip off the stack
	testw	$ILOCK, i_flag(%edx)	/ is inode locked?
	jnz	plkslp			/ yes, go to sleep
	orw	$ILOCK, i_flag(%edx)	/ set locked bit
	popl	%ebp
	ret

	.align	4
plkslp:
	orw	$IWANT, i_flag(%edx)	/ set wanted bit
	pushl	$PINOD			/ push priority PINOD (2nd arg)
	pushl	%edx			/ push ip (1st arg)
	call	sleep			/ zzzzz
	addl	$8, %esp		/ cleanup stack
	jmp	plretry			/ try again

/
/	prele(ip)	/* release an inode */
/	register struct inode *ip;
/	{
/		ip->i_flag &= ~ILOCK;
/		if (ip->i_flag & IWANT) {
/			ip->i_flag &= ~IWANT;
/			wakeup((caddr_t)ip);
/		}
/	}
/
	.align	4
	.globl	prele
prele:
	pushl	%ebp
	movl	%esp, %ebp		/ establish stack frame
	movl	8(%ebp), %edx		/ get ip off the stack
	andw	$-1!ILOCK, i_flag(%edx)	/ clear locked bit
	testw	$IWANT, i_flag(%edx)	/ check wanted bit
	jz	prlret			/ not wanted, return
	andw	$-1!IWANT, i_flag(%edx)	/ clear wanted bit
	pushl	%edx			/ push ip
	call	wakeup			/ rise and shine!
	addl	$4, %esp		/ cleanup stack
prlret:
	popl	%ebp
	ret
#endif

/
/ This code is the init process; it is copied to user text space
/ and it then does exec( "/sbin/init", "/sbin/init", 0);
/
	.data
	.align	4

	.globl	icode
	.globl	szicode

	.set	_exec, 11
	.set	_open, 5
	.set	_write, 4
	.set	_close, 6

icode:
	movl	$UVTEXT, %eax		/ start of user text
	leal	argv_off(%eax), %ebx
	pushl	%ebx
	leal	sbin_off(%eax), %ebx
	pushl	%ebx
	pushl	$0		/ dummy <ret> so stack looks right in systrap
	movl	$_exec, %eax
	lcall	$USER_SCALL, $0	/ execl("/sbin/init", "/sbin/init", 0);

	/ exec failed - write an error message to /dev/sysmsg

	movl	$UVTEXT, %eax
	pushl	$2
	leal	sysmsg_off(%eax), %ebx
	pushl	%ebx
	pushl	$0		/ dummy <ret> so stack looks right in systrap
	movl	$_open, %eax
	lcall	$USER_SCALL, $0	/ fd = open("/dev/sysmsg", 2);
	movl	%eax, %esi

	movl	$UVTEXT, %eax
	pushl	$ierr_len
	leal	ierr_off(%eax), %ebx
	pushl	%ebx
	pushl	%esi
	pushl	$0		/ dummy <ret> so stack looks right in systrap
	movl	$_write, %eax
	lcall	$USER_SCALL, $0	/ write(fd, IERR_MSG, strlen(IERR_MSG));

	movl	$UVTEXT, %eax
	pushl	%esi
	pushl	$0		/ dummy <ret> so stack looks right in systrap
	movl	$_close, %eax
	lcall	$USER_SCALL, $0	/ close(fd);

	jmp	.

	.align	4
argv:
	.set	argv_off, argv - icode
	.long	UVTEXT+sbin_off
	.long	0

sbin_init:
	.set	sbin_off, sbin_init - icode
	.string	"/sbin/init"

sysmsg:
	.set	sysmsg_off, sysmsg - icode
	.string	"/dev/sysmsg"

ierr_msg:
	.set	ierr_off, ierr_msg - icode
	.string	"** Can't exec /sbin/init\n\n"
	.set	ierr_len, . - ierr_msg - 1

icode_end:

	.align	4
szicode:
	.long	icode_end - icode

	.text
	.align	4
	.globl	monitor
monitor:
	int	$1
	ret

/ The following code is used to generate a 10 microsecond delay
/ routine.  It is initialized in ml/pit.c.

	.align	4
	.globl	tenmicrosec
	.globl	microdata
tenmicrosec:
	movl	microdata, %ecx		/ Loop uses ecx.
microloop:
	loop	microloop
	ret


#ifdef AT386
/ This routine moves bytes in memory.  It is used in the display driver.
/ vcopy(from, to, count, direction)
/	direction = 0 means from and to are the high addresses (move up)
/	direction = 1 means from and to are the low addresses (move down)
	.set	FROM, 8
	.set	TO, 12
	.set	COUNT, 16
	.set	DIR, 20

	.align	4
	.globl	vcopy
vcopy:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%edi
	movl	FROM(%ebp), %esi
	movl	TO(%ebp), %edi
	movl	COUNT(%ebp), %ecx
	movl	DIR(%ebp), %eax
	orl	%eax, %eax
	jnz	doit		/ direction flag is zero by default
	std			/ move up; start at high end and decrement
doit:
	rep
	movsw
	cld			/ clear direction flag
	popl	%edi
	popl	%esi
	popl	%ebp
	ret
#endif
