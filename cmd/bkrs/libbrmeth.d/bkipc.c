/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:libbrmeth.d/bkipc.c	1.11.2.1"

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <limits.h>
#include <backup.h>
#include <bkmsgs.h>

/* File descriptor cache size */
#define FDCACHE_SZ 5

/* Does a process exist? */
#define proc_exist( pid )	(!kill( pid, 0 ) || errno != ESRCH)

typedef	struct bkmessage_s	{
	long destination;
	pid_t originator;
	long type;
	bkdata_t	data;
} bkmessage_t;

typedef struct bkfd_s {
	pid_t pid;
	int fd;
} fd_t;

extern int bklevels;

extern pid_t getpid();
extern void brlog();
extern int unlink();
extern unsigned int alarm();
extern void pause();

static void timeout();
static int in_pipe;
static fd_t fdcache[ FDCACHE_SZ ];
static int fd_current = 0;
static int fdcache_dot = 0;
static char fname[ PATH_MAX + 1 ];
static int myfd = -1;
static pid_t mypid, creatorpid;
static mode_t bk_omode;

extern char *sys_errlist[];
extern int sys_nerr;

char *
brmsgname( msg )
int msg;
{
	static char buffer[ 25 ];
	switch( msg ) {
	case START:	return( "START" );
	case ESTIMATE:	return( "ESTIMATE" );
	case FAILED:	return( "FAILED" );
	case DONE:	return( "DONE" );
	case GET_VOLUME:	return( "GET_VOLUME" );
	case VOLUME:	return( "VOLUME" );
	case DISCONNECTED:	return( "DISCONNECTED" );
	case SUSPEND:	return( "SUSPEND" );
	case RESUME:	return( "RESUME" );
	case CANCEL:	return( "CANCEL" );
	case TEXT:	return( "TEXT" );
	case SUSPENDED:	return( "SUSPENDED" );
	case RESUMED:	return( "RESUMED" );
	case CANCELED:	return( "CANCELED" );
	case HISTORY:	return( "HISTORY" );
	case DOT:	return( "DOT" );
	case RSRESULT: return( "RSRESULT" );
	case RSTOC:	return( "RSTOC" );
	case INVL_LBLS:	return( "INVL_LBLS" );
	default: 
		(void) sprintf( buffer, "UNKNOWN( %d )", msg );
		return( buffer );
	}
}

static char *
prerrno( L_errno )
int L_errno;
{
	static char buffer[ 30 ];
	if( L_errno < sys_nerr ) return( sys_errlist[ L_errno ] );
	(void) sprintf( buffer, "Unknown errno %d", L_errno );
	return( buffer );
}

/* Return a file descriptor from the file descriptor cache */
static int
get_fd( destination )
register pid_t destination;
{
	register i;
	register fd_t *fdp;

	/* The cache is maintained as a circular list */
	for( i = 0, fdp = fdcache; i < FDCACHE_SZ; i++, fdp++ ) {

		if( fdp->fd == 0 )
			/* Not in list */
			break;

		if( fdp->pid == destination ) {
			/* The destination is already in the cache */
			fdcache_dot = (fdp - fdcache);
			return( fdp->fd );
		}
	}

	if( i == FDCACHE_SZ ) {
		/* Cache is full - close one file descriptor */
		fdp = fdcache + ((fdcache_dot == 0)? (FDCACHE_SZ - 1): fdcache_dot - 1);

		(void) close( fdp->fd );
	}

	/* Not currently in the list */
	(void) sprintf( fname, "/tmp/%ld/%ld", creatorpid, destination );

	/* ASSERT: fdp points to an available slot */
	if( (fdp->fd = open( fname, O_RDWR )) == -1 ) {
#ifdef TRACE
		brlog( "get_fd(): open of %s returns %s", fname, prerrno( errno ) );
#endif
		fdp->fd = 0;
		fdp->pid = 0;
		return( -1 );
	}

	fdp->pid = destination;
	fdcache_dot = fdp - fdcache;

#ifdef TRACE
	brlog( "get_fd(): allocate slot %d", fdcache_dot );
#endif

	return( fdp->fd );
}

/*
	Cleanup messages in the queue if the destination process has gone away 
*/
void
bkm_cleanup( pid, in_critical )
pid_t pid;
int in_critical;
{
	bkmessage_t message;
	int fd;

#ifdef TRACE
	brlog( "bkm_cleanup(): pid: %ld in_critical %d", pid, in_critical );
#endif

	if( !pid ) return;

	/* If the process is still alive, assume that it will read its msgs. */
	if( proc_exist( pid ) ) return;

	if( !in_critical )
		BEGIN_CRITICAL_REGION;

	if( (fd = get_fd( pid )) == -1 )
		return;

	if( fcntl( fd, F_SETFL, O_NDELAY ) == -1 )
		return;

	while( read( fd, (char *) &message, sizeof( bkmessage_t ) ) > 0 )
		;

	if( fcntl( fd, F_SETFL, ~O_NDELAY ) == -1 )
		return;

	if( !in_critical )
		END_CRITICAL_REGION;
}

int
bkm_send( destination, type, data )
pid_t destination;
int type;
bkdata_t *data;
{
	bkmessage_t message;
	register rc, fd;

	if( !mypid ) mypid = getpid();

	/* Check existence of destination */
	if( !proc_exist( destination ) ) {
		bkm_cleanup( destination, FALSE );
		errno = EEXIST;
		return( -1 );
	}

	/* Avoid receiving messages here */
	BEGIN_CRITICAL_REGION;

	/* Fill in the message */
	message.destination = (pid_t) destination;
	message.originator = (pid_t) mypid;
	message.type = type;
	if( data )
		message.data = *data;

	brlog( "bkm_send(): dest %ld orig %ld type %s", destination,
		mypid, brmsgname( type ) );

	if( (fd = get_fd( destination )) == -1 ) {
		if( errno == ENOENT ) {
			/*
				The destination exists but hasn't initialized its pipe yet,
				therefore, we'll do it here.
			*/
			sprintf( fname, "/tmp/%ld/%ld", creatorpid, destination );

			while( mknod( fname, 010700, 0 ) == -1 ) {
				brlog( "bkm_send(): mknod() of %s fails: %s", fname, prerrno( errno ) );
				if( errno == EINTR )
					continue;

				if( errno == ENOTDIR )
					errno = ENOENT;

				break;
			}

			if( (fd = get_fd( destination )) == -1 ) {
				brlog( "bkm_send(): unable to get file descriptor" );
				return( -1 );
			}

		} else {
			brlog( "bkm_send(): unable to get file descriptor" );
			return( -1 );
		}
	}
		
	while( (rc = write( fd, (char *) &message,
		sizeof( bkmessage_t ) )) == -1 && errno == EINTR ) {
		/* Check existence of destination */
		if( !proc_exist( destination ) ) {
			bkm_cleanup( destination, TRUE );
			errno = EEXIST;
			return( -1 );
		}
	}

	/* Notify recipient of a new message */
	if( rc == -1 )
		brlog( "bkm_send(): write( %d ) fails: %s", fd, prerrno( errno ) );
	else {
#ifdef TRACE
		brlog( "bkm_send(): write() returns %d", rc );
#endif

		(void) kill( destination, SIGUSR1 );
	}

	END_CRITICAL_REGION;
	return( rc == -1? -1: 0 );
}

int
bkm_receive( originator, type, data )
pid_t *originator;
int *type;
bkdata_t *data;
{
	bkmessage_t message;
	register rc;

	if( !mypid ) mypid = getpid();

	if( myfd == -1 ) {
		brlog( "bkm_receive(): myfd is %ld", myfd );
		errno = EEXIST;
		return( -1 );
	}

	BEGIN_CRITICAL_REGION;

	while( (rc = read( myfd, (char *) &message, sizeof( bkmessage_t ) ) ) == -1
		&& errno == EINTR )
		;

#ifdef TRACE
	brlog( "bkm_receive(): read returns %d", rc );
#endif

	if( rc == sizeof( bkmessage_t ) ) {
		*type = message.type;
		*originator = message.originator;
		*data = message.data;
	} else if( rc == 0 ) {
		rc = -1;
		errno = ENOMSG;
	}

	END_CRITICAL_REGION;
	return( rc );
}

static char *
m_tempfile( name )
char *name;
{
	static char filename[ 40 ];

#ifdef TRACE
	char *ptr;

	if( ptr = (char *)getenv( "BKNAME" ) )
		(void) sprintf( filename, "/tmp/%.10s", ptr );
	else
#endif
		(void) sprintf( filename, "/tmp/%.10s", name );
	return( filename );
}

/*
	"Plug in" to an existing conversation.  The creator info
	is retreived from a file in /tmp.  The process id of the
	creator is returned.
*/
pid_t
bkm_init( name, waiting )
char *name;
int waiting;
{
	int send_sig = 0;
	char *fname = m_tempfile( name );
	FILE *fptr;

	mypid = getpid();

#ifdef TRACE
	brlog( "bkm_init(): name: %s waiting: %s", name,
		(waiting? "yes": "no") );
#endif

	if( !(fptr = fopen( fname, "r" )) ) {
		errno = ENOENT;
		return( -1 );
	}
	if( fscanf( fptr, "%ld", &creatorpid ) != 1 || !proc_exist( creatorpid )) {
		(void) fclose( fptr );
		errno = ENOENT;
		return( -1 );
	}
	(void) fclose( fptr );

	/* make a FIFO for this process to read from */
	sprintf( fname, "/tmp/%ld/%ld", creatorpid, mypid );

	while( mknod( fname, 010777, 0 ) == -1 ) {
		brlog( "bkm_init(): mknod() of %s fails: %s", fname, prerrno( errno ) );
		if( errno == EINTR )
			continue;

		if( errno == EEXIST ) {
			/* Pipe already exists - send us a signal to read the contents */
			send_sig = 1;
			break;
		}

		if( errno == ENOTDIR )
			errno = ENOENT;

		return( -1 );
	}

	bk_omode = waiting? O_RDWR: O_RDWR|O_NDELAY;

	if( (myfd = open( fname, bk_omode )) == -1 ) {
		brlog( "bkm_init(): open of %s returns %s", fname, prerrno( errno ) );
		return( -1 );
	}

	if( send_sig )
		(void) kill( mypid, SIGUSR1 );

	return( creatorpid );
}

/*
	Start up the IPC system - a file in /tmp is created that has the
	IPC creator and the key in it.
*/
int
bkm_start( name, waiting )
char *name;
int waiting;
{
	FILE *fptr;
	int pid;
	char *fname = m_tempfile( name );

	/* Initialize the file descriptor cache */
	(void) strncpy( (char *) fdcache, "", FDCACHE_SZ * sizeof( fd_t ) );

	creatorpid = mypid = getpid();
	if( (fptr = fopen( fname, "r" )) ) {
		if( fscanf( fptr, "%d", &pid ) == 1 ) {
			if( pid == mypid ) {
				(void) fclose( fptr );
				return( 0 );
			} else if( proc_exist( pid )) {
				(void) fclose( fptr );
				errno = EEXIST;
				return( -1 );
			}
		}
		(void) fclose( fptr );
	}

	if( (fptr = fopen( fname, "w" )) ) {
		(void) fprintf( fptr, "%d\n", mypid );
		(void) fclose( fptr );
	}

	(void) sprintf( fname, "/tmp/%ld", mypid );

	if( mkdir( fname, 0777 ) == -1 ) {
		brlog( "bkm_start(): Unable to create directory %s: %s",
			fname, prerrno( errno ) );
		return( -1 );
	}

	(void) chmod( fname, 0777 );

	if( bkm_init( name, waiting ) != mypid ) {
		brlog( "bkm_start(): conversation already exists" );
		errno = EEXIST;
		return( -1 );
	}

	return( 0 );
}

/* Null routine for receive_msg timeouts */
static void
timeout()
{
	/* This routine left intentionally blank */
#ifdef TRACE
	brlog( "timeout" );
#endif
}

/* Is this directory empty - remove empty files */
static int
empty_dir( dir )
char *dir;
{
	struct stat statbuf;
	DIR *dirp;
	struct dirent *dp;
	register some = FALSE, size = strlen( fname );

	if( !(dirp = opendir( fname ) ) )
		return( TRUE );

	while( dp = readdir( dirp ) ) {

		if( !strcmp( dp->d_name, "." ) || !strcmp( dp->d_name, ".." ) ) 
			continue;

		(void) sprintf( fname + size, "/%s", dp->d_name );

		if( stat( fname, &statbuf ) != -1 && (statbuf.st_mode & 010000)
			&& !statbuf.st_size )

			(void) unlink( fname );

		else some = TRUE;
#ifdef TRACE
		brlog( "empty_dir(): name: %s mode 0%lo size %d",
			fname, statbuf.st_mode, statbuf.st_size );
#endif
	}

#ifdef TRACE
	brlog( "empty_dir(): %s", some? "NOT empty": "empty" );
#endif
		
	return( !some );
}

/* Remove the message queue from the system */
/* Give processes 5 minutes to get their messages out of the queue */
int
bkm_exit( name )
char *name;
{
	register count = 10, empty;
	void (*save)();

#ifdef TRACE
	brlog( "bkm_exit()" );
#endif

	if( mypid != creatorpid ) {
		errno = EPERM;
		return( -1 );
	}

	/* Ignore new messages */
	(void) signal( SIGUSR1, SIG_IGN );

	/* Prevent anyone else from joining this conversation */
	(void) unlink( m_tempfile( name ) );

	/* Remove the FIFO for this process */
	(void) sprintf( fname, "/tmp/%ld/%ld", mypid, mypid );
	(void) unlink( fname );

	/* Wait for directory to clear out */
	(void) sprintf( fname, "/tmp/%ld", mypid );
	(void) chmod( fname, 0444 );

	save = signal( SIGALRM, (void (*)())timeout );

	while( !(empty = empty_dir( fname )) && count-- > 0 ) {

#ifdef TRACE
		brlog( "bkm_exit(): wait for queue to clear" );
#endif
		(void) alarm( 30 );
		pause();
		(void) alarm(0);
	}

	save = signal( SIGALRM, save );

	if( empty ) {
		(void) sprintf( fname, "/tmp/%ld", mypid );
		(void) rmdir( fname );
	}

	return( 0 );
}
