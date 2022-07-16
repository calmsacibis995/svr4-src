/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)master:events/stubs.c	1.3"

ev_checkq(){ return(0); }
ev_config(){ return(0); }
ev_dr_post(){nopkg();}
ev_eqdtovp(){ nopkg(); }
ev_evsys(){ nopkg(); }
ev_evtrapret(){ nopkg(); }
ev_exec(){ return(0); }
ev_exit(){ return(0); }
ev_fork(){ return(0); }
ev_gotsig(){ return(0); }
ev_intr_restart(){ return(0); }
ev_istrap(){ return(0); }
ev_kev_post(){ nopkg(); }
ev_kev_free(){ return(0); }
ev_mem_alloc(){ nopkg(); }
ev_mem_free(){ return(0); }
ev_newpri(){ return(0); }
ev_signal(){ return(0); }
ev_stream_post(){ nopkg(); }
ev_traptousr(){ return(0); }
