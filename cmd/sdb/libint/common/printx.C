#ident	"@(#)sdb:libint/common/printx.C	1.5"

#include	"Interface.h"
#include	"Command.h"
#include	"Vector.h"
#include	<string.h>


#define	NESTOUT	25	/* allows 24 levels of nested output (overkill?) */

static OutPut outstack[NESTOUT];

static OutPut *curoutput = outstack + 1;


int
pushoutfile(FILE *fp)
{
	if ( curoutput < outstack + NESTOUT ) {
		curoutput++;
		curoutput->type = ot_file;
		curoutput->fp = fp;
		return 0;
	} else {
		return 1;		// caller must print error message
	}
}

int
pushoutvec( Vector *vec )
{
	if ( curoutput < outstack + NESTOUT ) {
		curoutput++;
		curoutput->type = ot_vector;
		curoutput->vec = vec;
		return 0;
	} else {
		return 1;		// caller must print error message
	}
}

void
popout()
{
	if ( curoutput > outstack ) {	// all right to pop all pushed files/windows
		curoutput->type = ot_none;
		--curoutput;
	} else {		// but not to pop the zeroth entry!
		printe ( "popout: empty output stack\n" );
		abort();
	}
}

#define MAXPRINT	2048

static char lbuf[MAXPRINT];

int
vprintx( const char *fmt, va_list ap )
{
	int ret = -1;
	if ( curoutput == outstack ) {
		printe("can't do output: empty output stack\n");
		abort();
	}
	switch ( curoutput->type ) {
	case ot_file:
		ret = vfprintf ( curoutput->fp, (char *)fmt, ap );
		break;
	case ot_vector:
		ret = vsprintf( lbuf, (char *)fmt, ap );
		if ( ret >= MAXPRINT ) {
			printe(
				"vprintx: resulting string > MAXPRINT (%d) chars",
					MAXPRINT);
			abort();
		}
		curoutput->vec->add(lbuf, ret+1).drop(1);
		break;
	case ot_none:
	default:
		printe( "vprintx: invalid output type %d",
						curoutput->type );
		abort();
		break;
	}
	return ret;
}

int
vprinte( const char *fmt, va_list ap )
{
	int ret = vfprintf ( stderr, (char *)fmt, ap );
	return ret;
}

int
_printx( const char *fmt ... )
{
	va_list ap;

	va_start( ap, fmt );
	int ret = vprintx(fmt, ap);
	va_end(ap);
	return ret;
}

int
_printe( const char *fmt ... )
{
	va_list ap;

	va_start( ap, fmt );
	int ret = vprinte(fmt, ap);
	va_end(ap);
	return ret;
}
