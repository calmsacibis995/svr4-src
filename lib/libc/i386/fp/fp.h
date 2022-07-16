/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-i386:libc-i386/fp/fp.h	1.3"

/* Useful asm routines and data types for grungy 87 hacking */

#define EXCPMASK (FP_X_INV|FP_X_DNML|FP_X_DZ|FP_X_OFL|FP_X_UFL|FP_X_IMP)

typedef short _envbuf87[7]; /* buffer for f{ld,st}env instruction       */
#define _swoff   1          /* status word index in _envbuf87           */
#define _swboff  2          /* status word BYTE offset in _envbuf87     */


/* Get current fp control word */
asm
_getcw(cw)
/* struct _cw87 cw;     */
{
%reg cw;
	fstcw   (cw)
%mem cw;
	movl    cw,%eax
	fstcw   (%eax)
}

/* Load a new fp control word */
asm
_putcw(cw)
/* struct _cw87 cw;     */
{
%reg cw;
	fldcw   (cw)
%mem cw;
	movl    cw,%eax
	fldcw   (%eax)
}


/* Get current fp status word */
asm
_getsw(sw)
/* struct _sw87 sw;     */
{
%reg sw;
	fstsw   (sw)
%mem sw;
	movl    sw,%eax
	fstsw   (%eax)
}

/* Load a new fp status word.  This is tricky because there is no direct
 * way to do this.  However, we store the environment, change the stored
 * status field, and reload the environment.  A bit expensive, but...
 */
asm
_putsw(sw,buf)
/* struct _cw87 sw;     */
/* _envbuf87 buf;       */
{
%reg sw,buf;
	data16
	fnstenv (buf)
	movw    (sw),sw
	movw    sw,2(buf)
	data16
	fldenv (buf)
%reg sw; mem buf;
	movl    buf,%eax
	data16
	fnstenv (%eax)
	movw    (sw),sw
	movw    sw,2(%eax)
	data16
	fldenv (%eax)
%mem sw; reg buf;
	movl    sw,%eax
	data16
	fnstenv (buf)
	movw    (%eax),%ax
	movw    %ax,2(buf)
	data16
	fldenv (buf)
%mem sw,buf;
	movl    sw,%ecx
	movl    buf,%eax
	data16
	fnstenv (%eax)
	movw    (%ecx),%cx
	movw    %cx,2(%eax)
	data16
	fldenv (%eax)
}

