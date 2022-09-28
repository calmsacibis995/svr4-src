/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:cmd/lpsched/dispatch.h	1.4.4.1"

# include	<time.h>

# include	"lpsched.h"

#if	defined(__STDC__)

void		s_accept_dest ( char * , MESG * );
void		s_alloc_files ( char * , MESG * );
void		s_cancel ( char * , MESG * );
void		s_cancel_request ( char * , MESG * );
void		s_complete_job ( char * , MESG * );
void		s_disable_dest ( char * , MESG * );
void		s_enable_dest ( char * , MESG * );
void		s_end_change_request ( char * , MESG * );
void		s_inquire_class ( char * , MESG * );
void		s_inquire_printer_status ( char * , MESG * );
void		s_inquire_remote_printer ( char * , MESG * );
void		s_inquire_request ( char * , MESG * );
void		s_inquire_request_rank ( char * , MESG * );
void		s_load_class ( char * , MESG * );
void		s_load_filter_table ( char * , MESG * );
void		s_load_form ( char * , MESG * );
void		s_load_printer ( char * , MESG * );
void		s_load_printwheel ( char * , MESG * );
void		s_load_system ( char * , MESG * );
void		s_load_user_file ( char * , MESG * );
void		s_mount ( char * , MESG * );
void		s_move_dest  ( char * , MESG * );
void		s_move_request ( char * , MESG * );
void		s_print_request ( char * , MESG * );
void		s_quiet_alert ( char * , MESG * );
void		s_reject_dest ( char * , MESG * );
void		s_send_fault ( char * , MESG * );
void		s_shutdown ( char * , MESG * );
void		s_start_change_request ( char * , MESG * );
void		s_unload_class ( char * , MESG * );
void		s_unload_filter_table ( char * , MESG * );
void		s_unload_form ( char * , MESG * );
void		s_unload_printer ( char * , MESG * );
void		s_unload_printwheel ( char * , MESG * );
void		s_unload_system ( char * , MESG * );
void		s_unload_user_file ( char * , MESG * );
void		s_unmount ( char * , MESG * );
void		r_new_child ( char * , MESG * );
void		r_send_job ( char * , MESG * );
void		s_job_completed ( char * , MESG * );
void		s_child_done ( char * , MESG * );

#else

void		s_accept_dest(),
		s_alloc_files(),
		s_cancel(),
		s_cancel_request(),
		s_complete_job(),
		s_disable_dest(),
		s_enable_dest(),
		s_end_change_request(),
		s_inquire_class(),
		s_inquire_printer_status(),
		s_inquire_remote_printer(),
		s_inquire_request(),
		s_inquire_request_rank(),
		s_load_class(),
		s_load_filter_table(),
		s_load_form(),
		s_load_printer(),
		s_load_printwheel(),
		s_load_system(),
		s_load_user_file(),
		s_mount(),
		s_move_dest (),
		s_move_request(),
		s_print_request(),
		s_quiet_alert(),
		s_reject_dest(),
		s_send_fault(),
		s_shutdown(),
		s_start_change_request(),
		s_unload_class(),
		s_unload_filter_table(),
		s_unload_form(),
		s_unload_printer(),
		s_unload_printwheel(),
		s_unload_system(),
		s_unload_user_file(),
		s_unmount(),
		r_new_child(),
		r_send_job(),
		s_job_completed(),
		s_child_done();

#endif
	
/**
 ** dispatch_table[]
 **/

/*
 * The dispatch table is used to decide if we should handle
 * a message and which function should be used to handle it.
 *
 * D_ADMIN is set for messages that should be handled
 * only if it came from an administrator. These entries should
 * have a corresponding entry for the R_... message case, that
 * provides a routine for sending back a MNOPERM message to those
 * that aren't administrators. This is needed because the response
 * message varies in size with the message type.
 */

typedef struct DISPATCH {
	void			(*fncp)();
	ushort			flags;
}			DISPATCH;

#define	D_ADMIN		0x01	/* Only "lp" or "root" can use msg. */
#define D_BADMSG	0x02	/* We should never get this message */
#define	D_SYSTEM	0x04	/* Only siblings may use this message */
