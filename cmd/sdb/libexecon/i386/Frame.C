#ident	"@(#)sdb:libexecon/i386/Frame.C	1.8"

// Frame.c -- stack frames and register access, i386 version

#include "Reg.h"
#include "Frame.h"
#include "RegAccess.h"
#include "Process.h"
#include "Interface.h"
#include "Symtab.h"
#include "Attribute.h"
#include "oslevel.h"
#include "Instr.h"

extern int debugflag;
#define DPR	if ( debugflag ) printe

//
// offsets to saved regisers in signal handler stack frame
//
#define SIG_EIPOFF      0x74
#define SIG_EBPOFF      0x54
#define SIG_ESPOFF      0x80

struct frameid {
	Iaddr ap;
	Iaddr fp;
};

struct framedata {
	Process	*proc;
	int	 level;		// 0 is top frame;
	unsigned long epoch;	
	Iaddr	 accv[NGREG];	// "access vector"
	RegRef	 saved_regs[NGREG];	// saved registers
	int	 nargwds;
	int	noprolog;
	framedata();
};

FrameId::FrameId(Frame *frame)
{
	id = 0;
	if ( frame ) {
		id = new frameid;
		id->ap = frame->getreg( REG_EBP );
		id->fp = frame->getreg( REG_EBP );
	}
}

FrameId::~FrameId()
{
	delete id;
}

FrameId &
FrameId::operator=( FrameId & other )
{
	if ( other.id == 0 && id == 0 )
		return *this;
	else if ( other.id == 0 )
	{
		delete id;
		id = 0;
		return *this;
	}
	else if ( id == 0 ) id = new frameid;
	*id = *other.id;
	return *this;
}

void
FrameId::print( char * s )
{
	if (s ) printf(s);
	if ( id == 0 )
		printf(" is null.\n");
	else
		printf(" ap is %#x, fp is %#x\n",id->ap,id->fp);
}

int
FrameId::operator==( FrameId& other )
{
	if ( (id == 0) && ( other.id == 0 ) )
		return 1;
	else if ( id == 0 )
		return 0;
	else if ( other.id == 0 )
		return 0;
	else if ( id->ap != other.id->ap )
		return 0;
	else if ( id->fp != other.id->fp )
		return 0;
	else
		return 1;

}

int
FrameId::operator!=( FrameId& other )
{
	return ! (*this == other);
}

framedata::framedata()
{
	proc = 0;
	level = 0;
	epoch = 0;
	for ( register int i = 0; i < NGREG ; i++ ) {
		accv[i] = 0;
		saved_regs[i] = 0;
	}
	nargwds = -1;
	noprolog = -1;
}

Frame::Frame( Process *proc ) : (1)
{
	data = new framedata;
	data->proc = proc;
	data->epoch = proc->epoch;	// data->epoch never changes
	DPR("new topframe() == %#x noprolog = %d\n", this, data->noprolog);
}

Frame::Frame( Frame *prev ) : (0)
{
	data = new framedata;
	prev->append( this );
	*data = *(prev->data);	// copy framedata struct
	data->level++;		// bump level
	data->nargwds = -1;	// invalidate arg words
	data->noprolog = -1;	// invalidate prolog flag
	DPR("new next frame(%#x) == %#x\n", prev, this);
}

Frame::~Frame()
{
	DPR("%#x.~Frame()\n", this);
	unlink();
	delete data;
}

int
Frame::valid()
{
	return this && data && data->epoch == data->proc->epoch;
}

int
Frame::isleaf()
{
	return 0;		// for now
}

FrameId
Frame::id()
{
	FrameId *fmid = new FrameId(this);
	return *fmid;
}


Frame *
Frame::caller()
{
	DPR("%#x.caller()\n", this);
	Frame *p = (Frame *) next();
	if ( p ) 
		return p;
				 // try to construct  a new frame
	DPR("no next, building it\n");
	Iaddr pc, ap, fp, sp;
	Iaddr prevfp, prevpc;
	Itype itype;
	pc = getreg(REG_PC);
	DPR("%#x.caller() pc == %#x\n", this, pc);
	if ( data->proc->in_text(pc) ) {
		int i;
		ap = getreg(REG_EBP);
		DPR("%#x.caller() ap == %#x\n", this, ap);
		fp = getreg(REG_EBP);
		DPR("%#x.caller() fp == %#x\n", this, fp);
		sp = getreg(REG_ESP);
		DPR("%#x.caller() sp == %#x\n", this, sp);
		 i = data->proc->read(fp , Saddr, itype);
		prevfp = itype.iaddr;
		DPR("%#x.caller() prevfp == %#x ( i = %d)\n", this, prevfp, i);
		if ( data->proc->in_stack(fp) && data->proc->in_stack(prevfp) ) {
			Iaddr fn;
			Symbol entry;

			DPR("caller() look for entry data = 0x%x\n", pc); /*dbg*/
			entry = data->proc->find_entry( pc );
			if ( !entry.isnull() ) {
				//
				// get address of function start
				//
				fn = entry.pc(an_lopc); // function start
				if ( fn != 0 ) {
				    DPR("entry lopc = %#x\n", fn); /*dbg*/
				    //
				    // look for function prolog
				    //
				    if ( data->noprolog < 0 ) {
					 if ( data->proc->instr.fcn_prolog_sr(fn, 0,
							data->saved_regs) == 0 )
					    data->noprolog = 1;
					 else	
					    data->noprolog = 0;
				    }
			    	} // if (fn != 0)
			} // entry.isnull
			//
			// if no entry or no attribute, assume no prolog
			//
			if (data->noprolog < 0 )
				data->noprolog = 1;
			if ( data->noprolog == 1 ) {
				//
				// no prolog
				// look for a pc value on the stack
				//
				Iaddr stackword;

				DPR("caller() no prolog\n"); /*dbg*/
				data->noprolog = 1;
				while ( sp < fp ) {
					data->proc->read(sp, Saddr, itype);
                			stackword = itype.iaddr;
					if ( data->proc->in_text(stackword) &&
					    data->proc->instr.iscall(stackword) ) {
						prevpc = stackword;
						break;
					}
				 	sp += sizeof(int);
			  	} 
	
			}
			else {
				//
				// normal stack frame
				//
				data->proc->read(fp+4, Saddr, itype);
				prevpc = itype.iaddr;
			} 
		}
		else
			return 0;
		DPR("%#x.caller() prevpc = %#x\n", this, prevpc);
		if ( !data->proc->in_text(prevpc) ) {
			DPR("%#x.caller() no prev frame, returns 0\n", this);
			return 0;	// no previous frame
		}
		p = new Frame(this);
		//
		// if prevpc  == u_sigreturn, the current frame is a 
		// signal handler, and the caller's context was pushed on the
		// stack by the kernel.
		//
		if (prevpc == get_sigreturn(data->proc->key) ) {
			p->data->accv[REG_PC] = fp + SIG_EIPOFF;
			p->data->accv[REG_EBP] = fp + SIG_EBPOFF;
			p->data->accv[REG_ESP] = fp + SIG_ESPOFF;
		}
		else if (data->noprolog) {
			p->data->accv[REG_PC] = sp;
			p->data->accv[REG_ESP] = sp + 4; 
		}
		else {
			int i, saved_regs_off;

			p->data->accv[REG_PC] = fp + 4;
			p->data->accv[REG_EBP] = fp;
			p->data->accv[REG_ESP] = fp + 8;
			//
			// set saved registers
			//
			saved_regs_off = data->saved_regs[0];
			for (i = 1; data->saved_regs[i] != 0; i++) {
				p->data->accv[data->saved_regs[i]] = 
					fp - saved_regs_off - i*sizeof(int);
			}

		}
		p->data->noprolog     = -1;
	}
	DPR("%#x.caller() accv[PC] = %#x, [AP] = %#x, [FP] = %#x [ESP]=%#x\n", this,
		p->data->accv[REG_PC], p->data->accv[REG_EBP],
		p->data->accv[REG_EBP], p->data->accv[REG_ESP]);
	return p;
}

Frame *
Frame::callee()
{
	return is_head() ? (Frame*) 0 : (Frame*) Prev();
}

int
Frame::readreg( RegRef which, Stype what, Itype& dest )
{
	if ( data->level == 0 ) {
		return !data->proc->readreg( which, what, dest );
	} else {			// on stack, possibly
		DPR("%#x.readreg(%d, %d, %#x) level = %d\n", this, which, what,
			&dest, data->level);
		if ( which <= REG_PC ) {	// IU regs
			if ( data->accv[which] ) {
				if ( which == REG_ESP) { // special case
					dest.iaddr =  (Iaddr) data->accv[which];
					return 0;
				}
				DPR("readreg() does read(%#x)\n", data->accv[which]);
				return !data->proc->read(data->accv[which],
								what, dest);
			} else {
				DPR("readreg() gets real register\n");
				return !data->proc->readreg( which, what, dest );
			}
		} else {			// FP regs (how?)
			printe("can't readreg() FP regs from stack\n");
			return -1;
		}
	}
#if 0
	long x, y, z;
	int yvalid = 0, zvalid = 0;

	if ( !valid() ) {
		printe("frame is not valid\n");
		return -1;			// frame is no longer valid
	}

	if ( data->level == 0 ) {
//		DPR("%#x.readreg(%d, %d, %#x) level = 0\n", this, which, what, &dest);
		if ( which <= REG_PC ) {	// IU reg
			// get bytes from register;
			x = data->proc->haltstatus.pr_reg[which];
		} else {			// FP reg
			register fpregset_t *fp = &data->proc->fpregs;

			switch (which) {

			case REG_ASR:
				x = fp->f_asr;
				break;

			case REG_DR:
				x = fp->f_dr[0];
				y = fp->f_dr[1];
				z = fp->f_dr[2];
				yvalid = zvalid = 1;
				break;

			case REG_X0:
				z = fp->f_fpregs[0][2];	zvalid = 1;
			case REG_D0:
				y = fp->f_fpregs[0][1];	yvalid = 1;
			case REG_F0:
				x = fp->f_fpregs[0][0];
				break;

			case REG_X1:
				z = fp->f_fpregs[1][2];	zvalid = 1;
			case REG_D1:
				y = fp->f_fpregs[1][1];	yvalid = 1;
			case REG_F1:
				x = fp->f_fpregs[1][0];
				break;

			case REG_X2:
				z = fp->f_fpregs[2][2];	zvalid = 1;
			case REG_D2:
				y = fp->f_fpregs[2][1];	yvalid = 1;
			case REG_F2:
				x = fp->f_fpregs[2][0];
				break;

			case REG_X3:
				z = fp->f_fpregs[3][2];	zvalid = 1;
			case REG_D3:
				y = fp->f_fpregs[3][1];	yvalid = 1;
			case REG_F3:
				x = fp->f_fpregs[3][0];
				break;

			default:
				printe("Frame::readreg(): bad RegRef (%d)\n",
					which);
			}
		}
	} else {			// on stack, possibly
//		DPR("%#x.readreg(%d, %d, %#x) level = %d\n", this, which, what,
//			&dest, data->level);
		if ( which <= REG_PC ) {	// IU regs
			if ( data->accv[which] ) {
//				DPR("readreg() does read(%#x)\n", data->accv[which]);
				data->proc->read(data->accv[which], Suint4, dest);
				x = dest.iuint4;
			} else {
//				DPR("readreg() gets real register\n");
				x = data->proc->haltstatus.pr_reg[which];
			}
		} else {			// FP regs (how?)
			printe("can't readreg() FP regs from stack\n");
			return -1;
		}
	}

	// convert to proper type

	switch (what) {
	case SINVALID:
		printe("Frame:readreg(): what == SINVALID\n");
		abort();
		break;
	case Schar:
	case Suchar:
		dest.ichar = x;
		break;
	case Sint1:
	case Suint1:
		dest.iint1 = x;
		break;
	case Sint2:
	case Suint2:
		dest.iint2 = x;
		break;
	case Sint4:
	case Suint4:
	case Ssfloat:
	case Saddr:
	case Sbase:
	case Soffset:
		dest.iint4 = x;
		break;
	case Sdfloat:
		if ( yvalid ) {
			dest.iint4 = x;
			dest.rawwords[1] = y;
		} else {
			printe("Frame::readreg(): bad size for double\n");
			return -1;
		}
		break;
	case Sxfloat:
		if ( yvalid && zvalid ) {
			dest.iint4 = x;
			dest.rawwords[1] = y;
			dest.rawwords[2] = z;
		} else {
			printe("Frame::readreg(): bad size for extended\n");
			return -1;
		}
		break;
	}
	return 0;
#endif
}

int
Frame::writereg( RegRef which, Stype what, Itype& dest )
{
	return !data->proc->writereg(which, what, dest);
}

Iaddr
Frame::getreg( RegRef which )
{
	Itype itype;
	if ( readreg( which, Saddr, itype ) ) {
		if(!data->proc->is_proto())
			printe("can't getreg(%d)\n", which);
		return 0;
	}
	return itype.iaddr;
}

Iint4
Frame::argword(int n)
{
	Itype itype;
	Iaddr ap = getreg( REG_EBP );
	data->proc->read( ap + 8 + (4*n), Sint4, itype );
	return itype.iint4;
}

int
Frame::nargwds()
{
	Iaddr prevpc, frame;
	Itype itype;

	if ( data->nargwds < 0 ) {
		Iaddr pc, fn;
		Symbol entry;

		DPR("nargbyte() look for entry data = 0x%x\n",data); /*dbg*/
		pc = getreg(REG_PC);
		entry = data->proc->find_entry( pc );
		if ( entry.isnull() ) {
			return 0;
		}
		//
		// get address of function start
		//
		fn = entry.pc(an_lopc); // function start
		if ( fn != 0 ) {
			if ( data->noprolog < 0 ) {
				if ( data->proc->instr.fcn_prolog_sr(fn, 0, data->saved_regs) == 0 ) 
					data->noprolog = 1;
				else
					data->noprolog = 0;
			}
			if (data->noprolog > 0) {
				return 0;			
			}
		}
		else
			return 0;
		frame = getreg(REG_EBP);
		data->proc->read(frame + 4, Saddr, itype);
		prevpc = itype.iaddr;
		if ( !data->proc->in_text(prevpc) ) {
			return 0;
		}
		data->nargwds = data->proc->instr.nargbytes(prevpc) / 4;
	}
	if (debugflag)
		printf("nargwds: return %d\n",data->nargwds);
	return data->nargwds;
}

Iaddr
Frame::pc_value()
{
	Iaddr	esp, ebp;
	Itype	itype;

	if ( data->accv[REG_EIP] != 0 )
	{
		return getreg( REG_EIP ) - 1;
	}
	ebp = getreg( REG_EBP );
	esp = getreg( REG_ESP );
	if ( ebp == esp )
	{
		return getreg( REG_EIP );
	}
	data->proc->read( esp, Saddr, itype );
	if ( itype.iaddr == ebp )
	{
		data->proc->read( esp + 4, Saddr, itype );
		return itype.iaddr - 1;
	}
	else if ( data->proc->in_text(itype.iaddr) &&
		data->proc->instr.iscall(itype.iaddr) )
	{
		return itype.iaddr - 1;
	}
	else
	{
		return getreg( REG_EIP );
	}
}
