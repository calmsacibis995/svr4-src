#ident	"@(#)sdb:libsymbol/common/Object.C	1.7"
#include	"Object.h"
#include	"EventTable.h"
#include	"Process.h"
#include	"Symtable.h"
#include	<osfcn.h>
#include	<errno.h>
#include	<string.h>
#include	<fcntl.h>
#include	<sys/stat.h>
#include	"prioctl.h"

extern int	fstat(int, struct stat *);

static Circlech	objectlist;

Object::Object( int fd, dev_t dev, ino_t ino ) : link(this)
{
	fdobj = ::dup(fd);
	device = dev;
	inumber = ino;
	objectlist.add(&link);
	symtable = new Symtable(fdobj);
	etable = new EventTable;
	etable->object = this;
	protop = 0;		// accessed in Process::Process()
}

Object::~Object()
{
	delete protop;
	::close(fdobj);
}

Process *
Object::get_proto()
{
	if ( !protop ) {
		protop = new Process( fdobj, -1, 0 );
		delete protop->etable;
		protop->etable = etable;
	}

	return protop;
}

extern int	errno;

Symtable *
get_symtable( int fdobj )
{
	Circlech *	x;
	Object *	s;
	struct stat	newstat;

	if ( fdobj == -1 )
	{
		return 0;
	}
	else if ( ::fstat(fdobj,&newstat) )
	{
		::close( fdobj );
		return 0;
	}
	for (x = objectlist.next() ; x != &objectlist ; x = x->next())
	{
		s = (Object *)(x->item);
		if (newstat.st_dev == s->device && newstat.st_ino == s->inumber)
		{
			::close(fdobj);
			return s->symtable;
		}
	}
	s = new Object(fdobj,newstat.st_dev,newstat.st_ino);
	::close( fdobj );
	return s->symtable;
}

Object *
find_object(int fdobj, EventTable ** et, Symtable ** symtable )
{
	Circlech	* x;
	Object		* s;
	struct stat	newstat;

	if ( ::fstat(fdobj,&newstat) )
	{
		if ( et ) *et = 0;
		if ( symtable ) *symtable = 0;
		return 0;
	}
	for (x = objectlist.next() ; x != &objectlist ; x = x->next())
	{
		s = (Object *)(x->item);
		if (newstat.st_dev == s->device &&
			newstat.st_ino == s->inumber)
		{
			if ( et ) *et = s->etable;
			if ( symtable ) *symtable = s->symtable;
			return s;
		}
	}
	s = new Object(fdobj,newstat.st_dev,newstat.st_ino);
	if ( et ) *et = s->etable;
	if ( symtable ) *symtable = s->symtable;
	return s;
}

extern Process *	current_process;
extern Process *	last_core;

int
dispose_process( Process * process )
{
	if ( !process || process != current_process )
	{
		return 1;
	}
	else if ( process->etable == 0 || process->etable->object == 0 )
	{
		current_process = last_core;
		return 1;
	}
	else if ( process->etable->object->protop == process )
	{
		return 1;
	}
	else
	{
		current_process = process->etable->object->get_proto();
		return 1;
	}
}
