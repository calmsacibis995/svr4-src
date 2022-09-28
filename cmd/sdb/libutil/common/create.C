#ident	"@(#)sdb:libutil/common/create.C	1.7"
#include	"oslevel.h"
#include	"Process.h"
#include	"Monitor.h"
#include	"Interface.h"
#include	"utility.h"
#include	<osfcn.h>
#include	<fcntl.h>
#include	<string.h>

extern int putenv(char *);

extern char	executable_name[100];

Process *
create_process( char * cmdline, sigset_t sigset )
{
	int		pid;
	Key		key;
	void *		child_io;
	Process *	process;
	char *		execline = new char[ 6 + ::strlen(cmdline) ];
	char *		p;
	int		fd;

	::strcpy( execline, "exec " );
	::strcat( execline, cmdline );
	child_io = 0;
	::strncpy( executable_name, cmdline, 99 );
	if ( (p = ::strchr( executable_name, ' ' )) != 0 ) *p ='\0';
	if ( (fd = ::open( executable_name, O_RDONLY )) == -1 )
	{
		printe("no executable named '%s' exists.\n",executable_name);
		process = 0;
	}
	else if ( (pid = ::fork()) == 0 )
	{
		stop_self_on_exec();
		//redirect_childio( child_io );
		putenv("LD_BIND_NOW=1");
		::execlp( "/bin/sh", "sh", "-c", execline, 0 );
		//cleanup_childio( child_io );
		::exit(1);
	}
	else if ( pid == -1 )
	{
		printe("internal error: fork() failed in creating process\n");
		process = 0;
	}
	else if ( ::get_key(pid,key) == 0 )
	{
		printe("internal error: get key failure for forked process\n");
		process = 0;
	}
	else
	{
		process = new Process( key, 1 );
		current_process = process;
		process->child_io = child_io;
		process->set_sig_catch( sigset );
		e_monitor.add( process );
		e_monitor.track( process );
	}
	::close( fd );
	delete execline;
	return process;
}
