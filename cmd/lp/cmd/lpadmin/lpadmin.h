/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)lp:cmd/lpadmin/lpadmin.h	1.7.2.1"

#define BEGIN_CRITICAL	{ ignore_signals(); {
#define END_CRITICAL	} trap_signals(); }

extern void		ignore_signals(),
			trap_signals();

extern int		a,
			banner,
#if	defined(DIRECT_ACCESS)
			C,
#endif
			filebreak,
			h,
			j,
			l,
			M,
			o,
			Q,
			W,
			scheduler_active;

extern char		*A,
			*c,
			*cpi,
			*d,
			*D,
			*e,
			*f,
			**f_allow,
			**f_deny,
			*F,
			**H,
			*i,
			**I,
			*length,
			*lpi,
			*m,
			modifications[128],
			*p,
			*r,
			*s,
			*stty,
			**S,
			**T,
			*u,
			**u_allow,
			**u_deny,
			*U,
			*v,
			*width,
			*x;

#if	defined(LPUSER)
extern SCALED		cpi_sdn,
			length_sdn,
			lpi_sdn,
			width_sdn;
#endif

#if	defined(PR_MAX)
extern PRINTER		*oldp;

extern PWHEEL		*oldS;
#endif

extern unsigned short	daisy;

extern char		*Local_System;

extern char		*getdflt();

extern int		ismodel(),
			output(),
			verify_form(),
			do_align();

extern void		do_fault(),
			do_mount(),
			do_printer(),
			do_pwheel(),
			done(),
			fromclass(),
			newdflt(),
			options(),
			rmdest(),
			startup(),
			usage();

#if	defined(__STDC__)
void			send_message( int , ... );
#else
extern void		send_message();
#endif
