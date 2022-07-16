	.file	"mcount.s"

	.ident	"@(#)libc-i386:libc-i386/crt/mcount.s	1.5"

/ / /
/ data names
	.data

.isPending:
	.long	0

/ / /
/ text names
	.text

	.globl	_mcount


/ _mcount: call count increment routine for cc -p/-qp profiling (prof(1))

/
/ This routine is called at the entry to each subroutine compiled with
/ the -p option to cc(1).
/
/ struct counter {
/	void	(*addr)();	/* address of calling routine */
/	long	count;		/* # of times called by caller */
/ } *countbase, **%edx;
/ Assume can destroy %eax and %edx.
/


/ Call with %edx pointing to a private, static cell (initially zero);

/ Answer with that cell (now) pointing to a call count entry,
/  consisting of a ``backpointer to the function'' and an incremented
/  call count (allocate a new one for this fcn if cell was zero).

/ All knowledge of call count entry management is handled in the
/  function _mcount_newent() et. al., which all live in libc-port:gen/mon.c.


/ _mcount
/       if  nonzero cell contents       //i.e. points at an entry
/         access entry, increment counter and return.
/ 
/       else                            //fcn as of yet has no entry; get 1
/         if  a pointer request is still pending..
/           return without an entry - last request unsatisfied, and
/           a recursive request cannot be satisfied.
/ 
/         else
/           get an entry pointer (mark `request pending' until it answers)
/           if  entry pointer == 0
/             return without an entry - profiling is Off.
/ 
/           else
/             initialize this entry: set backpointer to return address,
/             set count to 1 for this call.
/             return
/ 
/           fi
/         fi
/       fi



_fgdef_(_mcount):
	_prologue_
/ Is there a counter struct addr in *%edx?
	movl	(%edx),%eax	/ %eax = ptr to counter
	testl	%eax,%eax	/ if not 0, have called before:
	jnz     .add1           /      skip initialization

/ No. Is a prior call still pending?
	movl	.isPending,%eax
	testl	%eax,%eax	/ if not 0, a call has not returned yet
	jnz     .ret		/      deny increment request

/ No prior call is pending, may call now.  Assumes new cell pointer
/  is returned in %eax.
	pushl	%edx		/	(save counter ptr)
	movl	$1,.isPending	/ NOTING mcount_newent call Pending..
	call	_fref_(_mcount_newent)
	movl	$0,.isPending	/ ..and that it is Pending no longer.
	popl	%edx		/	(restore counter ptr)

	testl	%eax,%eax	/ if NULL, counting has been disabled:
	jz	.ret		/	get out

/ Allocate new counter
	addl	$4,%eax		/ compute ptr to call count (counter.count)
	movl	%eax,(%edx)	/	and store it in caller's ptr
	movl    _esp_(0),%edx
	movl	%edx,-4(%eax)	/ set proc addr in counter structure

	movl	$1,(%eax)	/ init call count (skip increment)
	jmp	.ret
.add1:
	incl	(%eax)		/ increment counter
.ret:
	_epilogue_
	ret
