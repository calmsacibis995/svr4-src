#ident	"@(#)sdb:libexecon/i386/DbregAccess.C	1.1"

#include <sys/types.h>
#include <sys/signal.h>
#include <sys/mount.h>
#include <sys/fs/s5dir.h>
#include <sys/user.h>
#include <sys/debugreg.h>
#include <sys/immu.h>
#include <sys/trap.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "prioctl.h"
#include "Expr.h"
#include "Place.h"
#include "TYPE.h"
#include "Itype.h"
#include "oslevel.h"
#include "Process.h"
#include "Interface.h"
#include "Frame.h"
#include "Rvalue.h"
#include "Source.h"
#include "Symtab.h"
#include "SrcFile.h"
#include "DbregAccess.h"

DbregAccess::DbregAccess(Process *proc)
{
  int i;
  k=proc->key;
  cur_proc=proc;
  for(i=DR_FIRSTADDR;i<=DR_LASTADDR;i++){
    debug_reg[i]=0;
    exp[i]=NULL;
  }
  debug_ctrl=0;
  debug_stat=0;
}

DbregAccess::~DbregAccess()
{
  write_control(0); // turn off watchpoints
  clear_status();
}

#if PTRACE

#define debugbase ((long)&(((struct user *) 0)->u_debugreg))
#define debugctrl ((long)&(((struct user *) 0)->u_debugreg[DR_CONTROL]))
#define debugstat ((long)&(((struct user *) 0)->u_debugreg[DR_STATUS]))

int DbregAccess::get_status()
{
  debug_stat=ptrace(3,k.pid,debugstat,0);
  if(errno!=0){
//DBG    printe("ptrace failed to read debug status, errno is %d\n",errno);    
    return(-1);
  }
  return(1);
}

int DbregAccess::clear_status()
{
// this seems to be unnecessary and returns an error
/*  ptrace(6,k.pid,debugstat,0);
  if(errno!=0){
//DBG    printe("ptrace failed to clear debug status, errno is %d\n",errno);    
    return(-1);
  }*/
  return(1);
}

int DbregAccess::read_control()
{
  debug_ctrl=ptrace(3,k.pid,debugctrl,0);
  if(errno!=0){
//DBG    printe("ptrace failed to read debug control, errno is %d\n",errno);    
    return(-1);
  }
  return(1);
}

int DbregAccess::write_control( unsigned int ctrl)
{
  ptrace(6,k.pid,debugctrl,ctrl);
  if(errno!=0){
//DBG    printe("ptrace failed to write debug control, errno is %d\n",errno);    
    return(-1);
  }
  return(1);
}

int DbregAccess::write_address( int i, unsigned int addr)
{
  ptrace(6,k.pid,debugbase+4*i,addr);
  if(errno!=0){
//DBG    printe("ptrace failed to write debug register %d, errno is %d\n",i,errno);    
    return(-1);
  }
  return(1);
}




#else 

// /proc specific goes here when it's ready

#ifndef PIOCGDBREG
#define PIOCGDBREG -1
#endif

#ifndef PIOCSDBREG
#define PIOCSDBREG -2
#endif

#define debugbase ((long)&(((struct user *) UVUBLK)->u_debugreg))
#define debugctrl ((long)&(((struct user *) UVUBLK)->u_debugreg[DR_CONTROL]))
#define debugstat ((long)&(((struct user *) UVUBLK)->u_debugreg[DR_STATUS]))

int DbregAccess::get_status()
{
#ifdef PIOCGDBREG
	do {
		::errno = 0;
		::ioctl( k.fd, PIOCGDBREG, &debug_reg );
	} while ( ::errno == EINTR );
	return ( ::errno == 0 );
#else
  if(lseek(k.fd,debugstat, 0)==-1){
//DBG    printe("lseek to debug status reg failed, errno is %d\n",errno);    
    return(-1);
  }
  if ( read(k.fd, (char*) &debug_stat, 4 )== -1){
//DBG    printe("read of debug regs failed, errno is %d\n",errno);    
    return(-1);
  }
#endif
}

int DbregAccess::clear_status()
{
  debug_stat=0;
#ifdef PIOCSDBREG
		do {
			::errno = 0;
			::ioctl( k.fd, PIOCSDBREG, &debug_reg );
		} while ( ::errno == EINTR );
		return (::errno == 0);
#else
return(-1);
#endif
}

int DbregAccess::read_control()
{
#ifdef PIOCGDBREG
	do {
		::errno = 0;
		::ioctl( k.fd, PIOCGDBREG, &debug_reg );
	} while ( ::errno == EINTR );
	return ( ::errno == 0 );
#else
  if(lseek(k.fd,debugctrl, 0)==-1){
//DBG    printe("lseek to debug control reg failed, errno is %d\n",errno);    
    return(-1);
  }
  if ( read(k.fd, (char*) &debug_ctrl, 4 )== -1){
//DBG    printe("read of debug control failed, errno is %d\n",errno);    
    return(-1);
  }
#endif
}

int DbregAccess::write_control( unsigned int ctrl)
{
debug_ctrl=ctrl;
#ifdef PIOCSDBREG
		do {
			::errno = 0;
			::ioctl( k.fd, PIOCSDBREG, &debug_reg );
		} while ( ::errno == EINTR );
		return (::errno == 0);
#else
return(-1);
#endif
}

int DbregAccess::write_address( int i, unsigned int addr)
{
  debug_reg[i]=addr;
#ifdef PIOCSDBREG
		do {
			::errno = 0;
			::ioctl( k.fd, PIOCSDBREG, &debug_reg );
		} while ( ::errno == EINTR );
		return (::errno == 0);
#else
return(-1);
#endif
}

#endif

// this function interprets and prints out the results of a data breakpoint
// if this function returns a 1, it means that a data breakpoint has been hit,
// and the process should be stopped if it isn't already

int DbregAccess::interpret_status()
{
  int i;
  extern char *default_fmt(TYPE type);

  if(get_status()==-1){
//DBG    printe("can't read status\n");
    return(-1);
  }
//DBG  printx("interpret_status:value of status register is %x\n",debug_stat);
  if(read_control()==-1){
//DBG printe("can't read control\n");
    return(-1);
  }
//DBG  printx("interpret_status:value of debug control register is %x\n",debug_ctrl);



// move these outside loop to prevent contructing more than once
  Rvalue oldrvalue;
  Rvalue rvalue;
  Place oldlvalue;
  Place lvalue;
  Frame *f;
  Iaddr pc;
// loop through the status and control registers, checking data breakpoints
// that were enabled in the control register
  for(i=DR_FIRSTADDR;i<=DR_LASTADDR;i++){
    if(debug_ctrl&(1<<(i*DR_ENABLE_SIZE))){

      if(exp[i]->rvalue(oldrvalue)==0){
	printe("error verifying original value of variable\n");
	return(-1);
      }

      if(exp[i]->lvalue(oldlvalue)==0){
	printe("error verifying original address of variable\n");
	return(-1);
      }


      f = cur_proc->curframe();
      pc=f->getreg(REG_PC);


// This is needed because the user may have set a watch on a local stack variable,
// which has gone out of scope, but the address is being reused for another 
// local variable that has been modified.  We print an error because the user has already
// seen the SIGTRAP message.

      Expr expr(exp[i]->string());

      if ( expr.eval( EV_LHS, cur_proc, pc, f) == 0 ){
	printe("error evaluating expression %s, may be out of scope\n",exp[i]->string());
	return (-1);
      }

      if(expr.lvalue(lvalue)==0){
	printe("error verifying original address of variable\n");
	return(-1);
      }

      if ( exp[i]->eval( EV_RHS, cur_proc, pc, f) == 0 ){
	printe("error evaluating expression %s, may be out of scope\n",exp[i]->string());
	return (-1);
      }
      
      if(exp[i]->rvalue(rvalue)==0){
	printe("failure to find rvalue of expression\n");
	return(-1);
      }

// this is needed to see if any chunk of the current expression has changed
      int j,changed=0;
      for(j=i;j<=DR_LASTADDR;j++){
	if(exp[j]==exp[i]){
	  if(debug_stat&(1<<j)){ // this part of the exp changed
	    changed=1;
	    break;
	  }
	}
      }
	  
      if(changed){ // we think we have a real one here
	
// this is needed because ptrace does not currently let us clear the status register
// this means that if a data breakpoint is hit, then the programmer starts another,
// but the program stops at a breakpoint, this routine thinks the data breakpoint
// has still been hit.  This test checks the original rvalue, and if they're equal,
// we silently do nothing.

        if(rvalue==oldrvalue){
	  if(f->getreg(REG_TRAPNO)==SGLSTP){ // if trap is one, then we have 
					// triggered a breakpoint, so we must print 
					// a message
	    printe("false data breakpoint detect\n");
	    printx("unchanged: %s = ", exp[i]->string());
            rvalue.print(0,default_fmt(rvalue.type()));
            printx("\n");
      	  }
	  else {	
//DBG	    printe("false data breakpoint detect\n");
	  }
	  return(-1);
        }

        if(lvalue==oldlvalue){
          ;
        }
        else {
	  printe("expression %s has changed address, may be out of scope\n",
                  exp[i]->string());
	  return(-1);
        }

      /* finally, print out new value */

       if(f->getreg(REG_TRAPNO)!=SGLSTP){ // if trap is one, then we have not
					// triggered a breakpoint, so we must print 
					// a message
	 printx("value of %s has been changed was not detected immediately\n",
                  exp[i]->string());
	}
        printx("changed: %s = ", exp[i]->string());
        rvalue.print(0,default_fmt(rvalue.type()));
        printx("\n");
// try to print out the source line that caused the modification

        Symtab *symtab;
        Symbol symbol;
        Source source;
        long line=0;
      
        pc--;
        if ( (symtab = cur_proc->find_symtab( pc )) == 0 )
	{
		printe("no source file, pc=%x\n",++pc);
	}
	else if ( symtab->find_source( pc, symbol ) == 0 )
	{
		printe("no source file, pc=%x\n",++pc);
	}
	else if ( symbol.source( source ) == 0 )
	{
		printe("no source file, pc=%x\n",++pc);
	}
	else
	{
                SrcFile *src=find_srcfile(symbol.name());
                source.pc_to_stmt( pc, line );
                if(src && line<=src->num_lines()){
		  printx("%s:%d:	%s", symbol.name(),line, src->line(line));
                }
                else {
		  printe("no source file, pc=%x\n",++pc);
	        }
	}
        break;              // right now, it's only possible to have one data breakpoint 
                            // set, so we can exit now
      }
      else {
// According to the status, the value hasn't changed.
// This will check whether the kernel has modified the value without the hardware 
// method able to detect it.
	if(oldrvalue==rvalue)
	  continue;
	else {
	  printx("value of %s has been changed was not detected immediately\n",
                  exp[i]->string());
          printx("changed: %s = ", exp[i]->string());
          rvalue.print(0,default_fmt(rvalue.type()));
          printx("\n");
          break;              // right now, it's only possible to have one data breakpoint 
	}
      }
    }
  }
// this now done in destructor
//  clear_status();
//  write_control(0); // turn off watchpoints
  if(i>DR_LASTADDR){
    return(0);        // no watchpoint triggered, don't stop process
  }
  else {
    return(1);        // stop the process if it isn't already
  }                   
}

// this routine does the lower level work of setting up a hw watchpoint

int DbregAccess::set_hw_wpt( Expr *expr, Iaddr addr, int size)
{
  
  int reg,len;
  Iaddr newaddr;

//DBG  printx("set_hw_wpt(expr=%s, addr=%x, size=%d\n",expr->string(),addr,size);
//DBG  printx("kid pid is %d\n",k.pid);

  clear_status();

  read_control();

// this loop will decompose the address and length into the fewest and largest
// size chunks to monitor.  The chunks are 4, 2 or 1 byte and there are at
// most 4 chunks
  
  for(reg=DR_FIRSTADDR;(reg<=DR_LASTADDR && size>0);addr=newaddr){
    if((size>=4) && ((addr&3)==0)){
      len=DR_LEN_4;
      size-=4;
      newaddr=addr+4;
    }
    else if((size>=2) && ((addr&1)==0)){
      len=DR_LEN_2;
      size-=2;
      newaddr=addr+2;
    }
    else {
      len=DR_LEN_1;
      size--;
      newaddr=addr+1;
    }

    // find 1st available register
      for(;reg<=DR_LASTADDR;reg++){
	if(exp[reg]==NULL){
	  exp[reg]=expr;
	  break;
	}
      }

    if(reg>DR_LASTADDR){
//DBG      printe("all hw data breakpoint registers in use\n");
      return(-1);
    }

    if(write_address(reg, addr)==-1){
//DBG      printe("failed to write data watchpoint address\n");
      return(-1);
    }
    else {
//DBG    printx("wrote %x to data breakpoint register %d\n",addr,reg);
    }
    // enable the register and set slowdown mode
      
      debug_ctrl= debug_ctrl|DR_LOCAL_SLOWDOWN|(1<<(reg*DR_ENABLE_SIZE));
    
    // set length control and write mode
      
      debug_ctrl= debug_ctrl| (((DR_RW_WRITE|len)<<(reg*DR_CONTROL_SIZE)) << 
			       DR_CONTROL_SHIFT);
  }
  if(size>0){
//DBG    printe("combination of data size and alignment too large to fix in 16 bytes\n");
    return(-1);
  }
  if(write_control(debug_ctrl)==-1){ // turn on watchpoint
//DBG    printe("failed to write debug control\n");
    return(-1);
  }
  else {
//DBG    printx("wrote %x to control register\n",debug_ctrl);
  }
}  

// This function will attempt to set up the hardware watchpoint.  If it fails, it
// will return -1 and sdb will fall back to use software watchpoints

int
DbregAccess::set_hw_watch(Expr *expr, int size)
{
	Place		lvalue;
	Frame *		f;

	f = cur_proc->curframe();
	
	if ( expr->eval( EV_LHS, cur_proc, f->getreg(REG_PC), f) == 0 ){
//DBG	  printe("invalid expression; no watchpoint altered.\n");
	  return (-1);
	}
	if(expr->lvalue(lvalue)==0){
//DBG	  printe("failure to find address of expression\n");
	  return(-1);
	}

	if(lvalue.kind==pRegister){
//DBG	  printe("cannot set hw watchpoint on register\n");
	  return(-1);
	}
	  
        Rvalue rvalue;
	
	if ( expr->eval( EV_RHS, cur_proc, f->getreg(REG_PC), f) == 0 ){
//DBG	  printe("invalid expression; no watchpoint altered.\n");
	  return (-1);
	}

	if(expr->rvalue(rvalue)==0){
//DBG	  printe("failure to find size of expression\n");
	  return(-1);
	}

	if(size==0){
	  size=rvalue.size();
	}

	if(size>16){
//DBG	  printe("cannot set hw watchpoint on data larger than 16 bytes\n");
	  return(-1);
	}


	if(set_hw_wpt(expr,lvalue.addr,size)==-1){
	  return(-1);
	}
}



