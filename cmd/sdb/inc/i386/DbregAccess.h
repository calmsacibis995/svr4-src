/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:inc/i386/DbregAccess.h	1.1"
#ifndef dbregaccess_h
#define dbregaccess_h
#include "Itype.h"
#include "oslevel.h"
#include "Process.h"
#include "Expr.h"

class DbregAccess {
  Process *cur_proc;
  Key k;   // pid or fd of process
// eight unsigned ints corresponding to the 8 debug registers
  unsigned int debug_reg[4]; 
  unsigned int debug_res1;
  unsigned int debug_res2;
  unsigned int debug_stat;
  unsigned int debug_ctrl;
  Expr *exp[4]; // holds pointers to expressions which are associated
                // with address in data register
  int get_status();
  int clear_status();
  int read_control();
  int write_control(unsigned int );
  int write_address( int , unsigned int );
  int set_hw_wpt( Expr *, Iaddr , int );
public:
  DbregAccess(Process *);
  ~DbregAccess();
  int interpret_status();
  int set_hw_watch( Expr * , int = 0);
};
#endif
