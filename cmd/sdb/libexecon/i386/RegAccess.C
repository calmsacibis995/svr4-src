#ident	"@(#)sdb:libexecon/i386/RegAccess.C	1.12"
#define lint	1

#include	"RegAccess.h"
#include	"Reg1.h"
#include	"prioctl.h"
#include	"i_87fp.h"
#include	"Interface.h"
#include	<memory.h>
#include	<osfcn.h>
#include	<sys/reg.h>
#include	<sys/signal.h> 
#include	<sys/fs/s5dir.h>
#include	<sys/user.h>
#include	<errno.h>
#include	"Core.h"

#define ar0offset	((long)&(((struct user *)0)->u_ar0))

static void real2double(fp_tempreal* ,int*);

RegAccess::RegAccess()
{
	key.fd = -1;
	key.pid = -1;
	core = 0;
	corefd = -1;
	fpcurrent = 0;
}

int
RegAccess::setup_core( int cfd, Core *coreptr )
{
	corefd = cfd;
	core = coreptr;
	key.fd = cfd;
	key.pid = -1;
	return 1;
}

extern int errno ;

static char *tagname[] = {"VALID","ZERO ","INVAL","EMPTY" };



#if PTRACE

int
RegAccess::setup_live( Key k )
{
	key.pid = k.pid;
	key.fd = -1;
	core = 0;
	::errno = 0;
	gpbase = ::ptrace( 3, key.pid, ar0offset, 0 );
	return ( ::errno == 0 );
}

int
RegAccess::update( prstatus & prstat )
{
	if ( key.pid != -1 )
	{
		::errno = 0;
		gpbase = ::ptrace( 3, key.pid, ar0offset, 0 );
		return ( ::errno == 0 );
	}
	return 0;
}

int
RegAccess::readlive( RegRef regref, long * word )
{
	int	i,j,k;

	if ( key.pid == -1 )
	{
		return 0;
	}
	if (regref == FP_STACK) {
		
		int	fp_valid, *state_off;
		long	fpvalid_off,*fp; 
		int	fpregvals[12];
		struct	sdbfpstate_t sdbfpstate;
		int 	tag;

		//
		// Offset of flag byte in the u-block 
		//
		fpvalid_off = (long) &((struct user *)0)->u_fpvalid;
		//
		// force the child's floating point state into the u_block 
		//
		asm("fnop");
		//
		// 8 bits per byte, byte-reversed, get u_fpvalid, one byte 
		//
		::errno = 0;
		fp_valid = ptrace(3,key.pid,fpvalid_off,0) >> ((fpvalid_off&0x3)*8) & 0xff ;
		if (::errno) {
			printe("cannot read u_fpvalid\n");
			return 0 ;
		}
		if (fp_valid) {
			//
			// floating point was used. get fp stack.
			//
			// get state offset
			//
			state_off = (int *) ((struct user *)0)->u_fps.u_fpstate.state; 
			//	
			// fill up sdbfpstate
			//
			fp = (long *) &sdbfpstate;
			for (i = 0; i<sizeof(sdbfpstate)/sizeof(long); i++) {
				::errno = 0;
				*fp++ = ptrace(3,key.pid,(long)state_off++,0);
				if (::errno) {
					printe("cannot read u_fpstate\n");
					return 0 ;
				}
			}
			//
			// convert stack of reals to doubles
			//
			for (i = 0; i < 8; i++ )
				real2double(&sdbfpstate.fp_stack[i], &fpregvals[i*2]);
			//
			// fp stack pointer
			//
			unsigned int fpsp = sdbfpstate.fp_status >> 11 & 0x7;
			printf("FPSW 0x%08.8x    ",sdbfpstate.fp_status);
			printf("FPCW 0x%08.8x    ",sdbfpstate.fp_control);
			printf("FPIP 0x%.8x    ",sdbfpstate.fp_ip);
			printf("FPDP 0x%.8x\n", sdbfpstate.fp_data_addr);
			for (i = 0; i < 8 ; i++ )
			{
				k = (fpsp + i) % 8;
				tag = sdbfpstate.fp_tag >> (k * 2) & 0x3 ;
				printf("%.5s [ %s ] 0x",regs[FP_INDEX+k].name,
							  tagname[tag]);
				for (j=4 ; j>=0 ; j--) printf("%.4x ",
					sdbfpstate.fp_stack[i][j]);
				printf("== %.14g ",fpregvals[i*2],fpregvals[i*2+1]);
				printf("\n");
			}
			
		}
	}
	else {
		errno = 0;
		i = ptrace( 3, key.pid, ar0offset, 0 );
		gpbase = ::ptrace( 3, key.pid, ar0offset, 0 );
		offset = gpbase + regs[regref].offset * sizeof(int);
		sz = regs[regref].size;
		i = 0;
		do {
			::errno = 0;
			word[i] = ::ptrace(3,key.pid,offset,0);
			sz -= sizeof(long);
			offset += sizeof(long);
			++i;
		} while ( sz > 0 );
		return (::errno == 0 );
	}
}

int
RegAccess::writelive( RegRef regref, long * word )
{
	int	i;
	long	offset;
	long	sz;

	if ( key.pid == -1 )
	{
		return 0;
	}

	offset = gpbase + regs[regref].offset * sizeof(int);
	sz = regs[regref].size;
	i = 0;
	do {
		::errno = 0;
		::ptrace(6,key.pid,offset,word[i]);
		sz -= sizeof(long);
		offset += sizeof(long);
		++i;
	} while ( sz > 0 );
	return (::errno == 0 );
}

//
//  /proc
//
#else

int
RegAccess::setup_live( Key k )
{
	key.pid = k.pid;
	key.fd = k.fd;
	core = 0;
	return 1;
}

int
RegAccess::update( prstatus & prstat )
{
	if ( key.pid != -1 )
	{
		::memcpy( (char*)gpreg, (char*)prstat.pr_reg, sizeof(gpreg) );
		return 1;
	}
	return 0;
}

int
RegAccess::readlive( RegRef regref, long * word )
{
	int	i,j,k;

	if ( key.fd == -1 )
	{
		return 0;
	}
	if (regref == FP_STACK) {
		
		int	fp_valid;
		int	fpregvals[16];
		struct	sdbfpstate_t sdbfpstate;
		int 	tag;
		union {
			struct	user u;
			char u_block[ MAXUSIZE*NBPC ];
		}ub;

		//
		// force the child's floating point state into the u_block 
		//
		asm("fnop");
		//
		// read the u-block
		//
		if (::ioctl(key.fd, PIOCGETU, &ub.u) != 0 ) {
			printe("cannot read u-block\n");
			return 0 ;
		}
		fp_valid = ub.u.u_fpvalid;
		if (fp_valid) {
			//
			// floating point was used. get fp stack.
			//
			// fill up sdbfpstate
			//
			::memcpy( (char*) &sdbfpstate,
				(char *)&ub.u.u_fps.u_fpstate.state,
				sizeof(sdbfpstate));
			//
			// convert stack of reals to doubles
			//
			for (i = 0; i < 8; i++ )
				real2double(&sdbfpstate.fp_stack[i], &fpregvals[i*2]);
			//
			// fp stack pointer
			//
			unsigned int fpsp = sdbfpstate.fp_status >> 11 & 0x7;
			printf("FPSW 0x%08.8x    ",sdbfpstate.fp_status);
			printf("FPCW 0x%08.8x    ",sdbfpstate.fp_control);
			printf("FPIP 0x%.8x    ",sdbfpstate.fp_ip);
			printf("FPDP 0x%.8x\n", sdbfpstate.fp_data_addr);
			for (i = 0; i < 8 ; i++ )
			{
				k = (fpsp + i) % 8;
				tag = sdbfpstate.fp_tag >> (k * 2) & 0x3 ;
				printf("%.5s [ %s ] 0x",regs[FP_INDEX+k].name,
							  tagname[tag]);
				for (j=4 ; j>=0 ; j--) printf("%.4x ",
					sdbfpstate.fp_stack[i][j]);
				printf("== %.14g ",fpregvals[i*2],fpregvals[i*2+1]);
				printf("\n");
			}
			
		}
	}
	else 
		word[0] = gpreg[regs[regref].offset];
	return 1;
}

int
RegAccess::writelive( RegRef regref, long * word )
{
	if ( regref != FP_STACK )
	{
		gpreg[regs[regref].offset] = word[0];
		do {
			::errno = 0;
			::ioctl( key.fd, PIOCSREG, &gpreg );
		} while ( ::errno == EINTR );
		return (::errno == 0);
	}
	else
		printe("cannot write to floating point stack\n");
}

#endif

Iaddr
RegAccess::top_a_r()
{
	return getreg( REG_AP );
}

Iaddr
RegAccess::getreg( RegRef regref )
{
	Iaddr	addr;
	long	word[3];

	if ( readcore( regref, word ) || readlive( regref, word ) )
	{
		addr = word[0];
	}
	else
	{
		addr = 0;
	}
	return addr;
}

int
RegAccess::readreg( RegRef regref, Stype stype, Itype & itype )
{
	long	word[3];

	if ( readcore( regref, word ) || readlive( regref, word ) )
	{
		if (regref != FP_STACK) {
			switch (stype)
			{
			case SINVALID:	return 0;
			case Schar:	itype.ichar = word[0];		break;
			case Suchar:	itype.iuchar = word[0];		break;
			case Sint1:	itype.iint1 = word[0];		break;
			case Suint1:	itype.iuint1 = word[0];		break;
			case Sint2:	itype.iint2 = word[0];		break;
			case Suint2:	itype.iuint2 = word[0];		break;
			case Sint4:	itype.iint4 = word[0];		break;
			case Suint4:	itype.iuint4 = word[0];		break;
			case Saddr:	itype.iaddr = word[0];		break;
			case Sbase:	itype.ibase = word[0];		break;
			case Soffset:	itype.ioffset = word[0];	break;
			case Sxfloat:	itype.rawwords[2] = word[2];
			case Sdfloat:	itype.rawwords[1] = word[1];
			case Ssfloat:	itype.rawwords[0] = word[0];	break;
			default:	return 0;
			}
		}
		return 1;
	}
	return 0;
}

int
RegAccess::readcore( RegRef regref, long * word )
{
	
	register greg_t     *greg;
	register fpregset_t *fpreg;
	int i,j,k;

	if ( core == 0 )
		return 0;

	greg = core->getstatus()->pr_reg;
	fpreg = core->getfpregs();

	if  (regref == FP_STACK) {
		
		int	fpregvals[256];
		struct	sdbfpstate_t sdbfpstate;
		int 	tag;

		if (fpreg == 0)
			return 0;
		//
		// floating point was used. get fp stack.
		//
		// get state offset
		//
		//	
		// fill up sdbfpstate
		//
		::memcpy( (char*) &sdbfpstate, (char*) fpreg, sizeof(sdbfpstate) );
		//
		// convert stack of reals to doubles
		//
		for (i = 0; i < 8; i++ )
			real2double(&sdbfpstate.fp_stack[i], &fpregvals[i*2]);
		//
		// fp stack pointer
		//
		unsigned int fpsp = sdbfpstate.fp_status >> 11 & 0x7;
		printf("FPSW 0x%08.8x    ",sdbfpstate.fp_status);
		printf("FPCW 0x%08.8x    ",sdbfpstate.fp_control);
		printf("FPIP 0x%.8x    ",sdbfpstate.fp_ip);
		printf("FPDP 0x%.8x\n", sdbfpstate.fp_data_addr);
		for (i = 0; i < 8 ; i++ )
		{
			k = (fpsp + i) % 8;
			tag = sdbfpstate.fp_tag >> (k * 2) & 0x3 ;
			printf("%.5s [ %s ] 0x",regs[FP_INDEX+k].name,
						  tagname[tag]);
			for (j=4 ; j>=0 ; j--) printf("%.4x ",
				sdbfpstate.fp_stack[i][j]);
			printf("== %.14g ",fpregvals[i*2],fpregvals[i*2+1]);
			printf("\n");
		}
		
	}

	else 
		*word   = greg[regs[regref].offset];
       
	return 1;
}

int
RegAccess::writereg( RegRef regref, Stype stype, Itype & itype )
{
	long	word[3];

	switch (stype)
	{
		case SINVALID:	return 0;
		case Schar:	word[0] = itype.ichar;		break;
		case Suchar:	word[0] = itype.iuchar;		break;
		case Sint1:	word[0] = itype.iint1;		break;
		case Suint1:	word[0] = itype.iuint1;		break;
		case Sint2:	word[0] = itype.iint2;		break;
		case Suint2:	word[0] = itype.iuint2;		break;
		case Sint4:	word[0] = itype.iint4;		break;
		case Suint4:	word[0] = itype.iuint4;		break;
		case Saddr:	word[0] = itype.iaddr;		break;
		case Sbase:	word[0] = itype.ibase;		break;
		case Soffset:	word[0] = itype.ioffset;	break;
		case Sxfloat:	word[2] = itype.rawwords[2];
		case Sdfloat:	word[1] = itype.rawwords[1];
		case Ssfloat:	word[0] = itype.rawwords[0];	break;
		default:	return 0;
	}
	return ( writecore( regref, word ) || writelive( regref, word ) );
}


static void
real2double(fp_tempreal *tempreal,int* doubled)
{
	asm("fwait");
	asm("movl	8(%ebp),%eax");
	asm("fwait");
	asm("fldt	(%eax)");
	asm("movl	12(%ebp),%eax");
	asm("fwait");
	asm("fstpl	(%eax)");
}
int
RegAccess::writecore( RegRef regref, long * word )
{
	long	offset;
	long	sz;
	long	fp;
	long	greg;

	if ( core == 0 )
	{
		return 0;
	}
	fp = core->fpregbase();
	greg = (long) &((prstatus_t *)core->statusbase())->pr_reg;

	offset = greg + regref * sizeof(int);	
	sz = regs[regref].size;
	return (::put_bytes(key,offset,word,sz) == sz);
}

int
RegAccess::display_regs( int num_per_line )
{
	RegAttrs *p;
	Itype	  x;
	int	  i;

	i = 1;
	for( p = regs;  p->ref != FP_STACK;  p++ ) {
		readreg( p->ref, Suint4, x );
		if ( i >= num_per_line )
		{
			printx( "%s	%#10x\n", p->name, x.iuint4 );
			i = 1;
		}
		else
		{
			i++;
			printx( "%s	%#10x\t", p->name, x.iuint4 );
		}
	}
	if ( i != 1 )
		printx("\n");
	readreg( FP_STACK, Suint4, x );	// prints stack as side effect
	return 1;
}
