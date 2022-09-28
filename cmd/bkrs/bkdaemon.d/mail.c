/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:bkdaemon.d/mail.c	1.7.3.1"

#include	<sys/types.h>
#include	<pwd.h>
#include	<stdio.h>
#include	<backup.h>
#include	<bkmsgs.h>
#include	<bkdaemon.h>
#include	<bkerrors.h>
#include	<errno.h>

/* Format strings for what goes in the mail message */
#define ML_FORMAT1	(unsigned char *)\
"  job_id          start time       end time      tried   succeeded failed\n"
#define ML_FORMAT2	(unsigned char *)\
"------------------------------------------------------------------------------\n"
#define ML_FORMAT3	(unsigned char *) "back-%-10d %s  %s  %4d      %4d    %4d\n"
#define ML_FORMAT4	(unsigned char *)\
	"The following backup requires operator intervention:\n"
#define	ML_FORMAT5	(unsigned char *)\
	"    job_id            tag                     time   volume\n"
#define	ML_FORMAT6	(unsigned char *) "back-%-10d %14s %s %8s\n"

extern owner_t	*ownertab;
extern int	ownertabsz;
extern proc_t	*proctab;
extern int	proctabsz;
extern method_t	*methodtab;
extern int	methodtabsz;

extern char *asctime();
extern struct tm *localtime();
extern struct passwd *getpwuid();
extern char *strcpy();
extern time_t time();
extern void brlog();

void
send_mail_done( o_slot )
int o_slot;
{
	register proc_t *proc;
	register owner_t *owner;
	char tmpfname[ 20 ], command[ 80 ], startbuf[ 26 ], stopbuf[ 26 ];
	FILE *fptr;
	time_t now;

	if( !(owner = O_SLOT( o_slot )) ) {
		brlog(
			"send_mail_done(): given bad slot number(%d)", o_slot );
		return;
	}
	if( !(proc = P_SLOT( owner->p_slot ) ) ) {
		brlog( "send_mail_done(): o_slot %d has bad p_slot %d", o_slot,
			owner->p_slot );
		return;
	}
	(void) sprintf( tmpfname, "/tmp/bkm-%ld", proc->pid );
	if( !(fptr = fopen( tmpfname, "w+" )) ) {
		brlog( "send_mail_done(): cannot open temp file, errno %d",
			errno );
		return;
	}

	(void) fprintf( fptr, (char *)ML_FORMAT1 );
	(void) fprintf( fptr, (char *)ML_FORMAT2 );

	(void) cftime(startbuf, "%D %R", &(owner->starttime));

	now = time( 0 );
	(void) cftime(stopbuf, "%D %R", &now);

	/* Remove the newline characters from the print strings */
	startbuf[ 24 ] = stopbuf[ 24 ] = '\0';

	(void) fprintf( fptr, (char *)ML_FORMAT3, proc->pid, startbuf, stopbuf, 
		owner->m_succeeded + owner->m_failed, owner->m_succeeded, 
		owner->m_failed );

	(void) fclose( fptr );
	(void) sprintf( command, "mail %s <%s; rm -f %s >/dev/null 2>&1", owner->owner,
		tmpfname, tmpfname );
	(void) system( command );
}

int
send_mail_waiting( m_slot, label )
int m_slot;
char *label;
{
	register proc_t *proc;
	register owner_t *owner;
	register method_t *method;
	char tmpfname[ 20 ], command[ 80 ], timebuf[ 26 ];
	struct passwd *pwptr;
	time_t now;
	FILE *fptr;

	if( !(method = MD_SLOT( m_slot )) ) {
		brlog( "send_mail_waiting(): given bad m_slot %d", m_slot );
		return( BKINTERNAL );
	}
	if( !(owner = O_SLOT( method->o_slot )) ) {
		brlog( "send_mail_waiting(): m_slot %d has bad o_slot %d",
			m_slot, method->o_slot );
		return( BKINTERNAL );
	}
	if( !(proc = P_SLOT( owner->p_slot ) ) ) {
		brlog( "send_mail_waiting(): o_slot %d has bad p_slot %d", 
			method->o_slot, owner->p_slot );
		return( BKINTERNAL );
	}
	if( !(pwptr = getpwuid( proc->uid )) ) {
		brlog( "send_mail_waiting(): getpwuid() fails for uid %ld",
			proc->uid );
		return( BKINTERNAL );
	}

	(void) sprintf( tmpfname, "/tmp/bkm-%ld", proc->pid );
	if( !(fptr = fopen( tmpfname, "w+" )) ) {
		brlog( "send_mail_waiting(): cannot open temp file %s, errno %d",
			tmpfname, errno );
		return( BKFILE );
	}

	(void) fprintf( fptr, "%s%s", ML_FORMAT4, ML_FORMAT5 );

	now = time( 0 );
	(void) strcpy( timebuf, asctime( localtime( &now ) ) );
	timebuf[ 24 ] = '\0';	/* remove NL char */

	(void) fprintf( fptr, (char *)ML_FORMAT6, proc->pid, method->entry.tag,
		timebuf, label );

	(void) fclose( fptr );

	(void) sprintf( command, "mail %s <%s; rm -f %s >/dev/null 2>&1", pwptr->pw_name,
		tmpfname, tmpfname );

	(void) system( command );
	endpwent();

	return( BKSUCCESS );
}
