/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:bkdaemon.d/proc.c	1.7.2.1"

#include	<sys/types.h>
#include	<backup.h>
#include	<bkmsgs.h>
#include	<bkdaemon.h>
#include	<bkerrors.h>
#include	<errno.h>

/* Number of proc slots to get when the process table is grown. */
#define	P_BUNCH	10

/* Does a process exist? */
#define proc_exist( pid )	(!kill( pid, 0 ) || errno != ESRCH)

extern	proc_t	*proctab;
extern	int	nprocs, proctabsz;
extern	int nactive;

extern char *strncpy();
extern void *realloc();
extern void *malloc();
extern int kill();
extern int md_terminate ();
extern void o_kill();
extern void c_terminate();
extern void bkm_cleanup();
extern void brlog();
extern char *typename();
int p_exit();

/*
	p_check() makes sure that every process we know about is still alive.
	If a process has gone away, things are cleaned up.
*/
void
p_check()
{
	register proc_t *proc;
	register i;

	proc = proctab;
	for( i = 0; i < nprocs; i++, proc++ )
		if( proc->pid && !proc_exist( proc->pid ) ) {
			switch( proc->type ) {
			case METHOD_P:
				(void) md_terminate( proc->slot, MDT_EXIT, BKERREXIT, 0, 0 );
				break;
			case OWNER_P:
				o_kill( proc->slot );
				break;
			case CONTROL_P:
				c_terminate( proc->slot );
				break;
			default:
				(void) p_exit( proc->slot );
				break;
			}
		}
}

/* Initialize a process slot */
static void
p_init( slot )
int slot;
{
	proc_t *proc;
	if( slot >= nprocs ) return;
	proc = proctab + slot;
	proc->pid = 0;
	proc->uid = 0;
	proc->type = UNUSED_P;
	proc->slot = 0;
}

/* Remove a process from the table, doing necessary cleanup */
int
p_exit( slot )
int slot;
{
	register proc_t *procp;
	if( !(procp = P_SLOT( slot ) ) ) return( FALSE );

	/* cleanup messages in the queue */
	bkm_cleanup( procp->pid, TRUE );
	p_init( slot );

	return( TRUE );
}

/* Find an empty process table slot - grow the table if necessary */
int
p_findslot( type )
int type;
{
	register i;
	register proc_t *procp;

	/* Slot 0 is not used */
	for( i = 1, procp = P_SLOT( 1 ); i < proctabsz; i++, procp++ )
		if( procp->type == UNUSED_P ) {
			p_init( i );
			procp->type = type;
			nprocs++;
			return( i );
		}

	/* No empty process slots - grow the table */
	if( !proctabsz ) {
		if( !(proctab = (proc_t *)malloc( P_BUNCH * sizeof( proc_t ) ) ) ) {
			brlog( "Unable to malloc process table." );
			return( -1 );
		}
		(void) strncpy( (char *)proctab, "", P_BUNCH * sizeof( proc_t ) );
	} else {
		if( !(proctab = (proc_t *)realloc( proctab,
			(proctabsz + P_BUNCH) * sizeof( proc_t ) ) ) ) {
			brlog( "Unable to reallocate process table." );
			return( -1 );
		}
		(void) strncpy( (char *)(proctab + proctabsz), "", P_BUNCH * sizeof( proc_t ) );
	}
	proctabsz += P_BUNCH;

	/* At this point, the table has grown, return the first slot in the new
		portion of the table */
	nprocs++;
	proctab[ nprocs ].type = type;
	return( nprocs );
}

/* Find the process in the table that has this pid */
int
p_findproc( pid )
pid_t pid;
{
	register i;
	register proc_t *procp;
	/* Slot 0 is not used */
	for( i = 1, procp = P_SLOT( 1 ); i < proctabsz; i++, procp++ ) 
		if( !(procp->type & UNUSED_P) && procp->pid == pid )
			return( i );
	return( -1 );
}

/* A child process has terminated - cleanup and do necessary stuff */
void
p_terminate( child_pid, status )
pid_t child_pid;
int status;
{
	register proc_t *proc;
	register int slot;

#ifdef TRACE
	brlog( "p_terminate(): process %ld status 0x%x", child_pid, status );
#endif

	/* if lower halfword is 0xff, then the process stopped */
	if( (status & 0xff) == 0xff ) return;

	if( (slot = p_findproc( child_pid ) ) == -1 ) {
		/*
			If the process has sent a DONE message then it is
			already cleaned up.
		*/
		return;
	}

	if( !(proc = P_SLOT( slot ) ) ) {
		brlog( "p_terminate(): p_findproc() returned out of bounds slot %d",
			slot );
		return;
	}
	if( proc->type != METHOD_P ) {
		brlog( "p_terminate(): unexpected child process type: %s", 
			typename( proc->type ) );

		/* Cleanup the process anyway */
		(void) p_exit( slot );

	} else {
		(void) md_terminate( proc->slot, MDT_EXIT, status, 0, 0 );
	}
}
