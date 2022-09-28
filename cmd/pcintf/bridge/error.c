/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/error.c	1.1"
#include	"system.h"
#include	"sccs.h"
SCCSID(@(#)error.c	3.14	LCC);	/* Modified: 20:19:24 6/21/89 */

/*****************************************************************************

	Copyright (c) 1984, 1988 Locus Computing Corporation.
	All rights reserved.
	This is an unpublished work containing CONFIDENTIAL INFORMATION
	that is the property of Locus Computing Corporation.
	Any unauthorized use, duplication or disclosure is prohibited.

*****************************************************************************/

#include <errno.h>
#include "pci_types.h"
#include <string.h>
#include <version.h>

extern int errno;

int err_code;				/* save value of "response" for */
					/*   future calls of pci_get_ext_err */

struct req_specs {			/* Contains info on a DOS request */
    int request,			/* The number (name) of the request */
	err_range [ RESPONSES + 1 ],	/* Limited resps. expected by DOS req */
	default_response;		/* Resp. when errno is out of range */

} req_specs[] = {			/* table of requests and their info */
{ PCI_CREATE, 	 {3,4,5,32,80,0},			ACCESS_DENIED 	},
{ PCI_DELETE, 	 {2,3,5,0}, 				ACCESS_DENIED 	},
{ OLD_OPEN, 	 {1,2,3,4,5,8,12,32,0}, 		ACCESS_DENIED 	},
{ NEW_CLOSE, 	 {6,0}, 				FILDES_INVALID 	},
{ READ_SEQ,	 {5,6,33,0},				ACCESS_DENIED	},
{ READ_RAN, 	 {1,2,3,4,5,6,8,13,15,16,18,33,0},	FILDES_INVALID	},
{ WRITE_SEQ,	 {5,6,33,0},				ACCESS_DENIED	},
{ WRITE_RAN, 	 {1,2,3,4,5,6,8,13,15,16,18,33,0},	FILDES_INVALID	},
{ CHDIR, 	 {3,0}, 				PATH_NOT_FOUND 	},
{ MKDIR, 	 {3,5,0}, 				ACCESS_DENIED 	},
{ RMDIR, 	 {3,5,16,0}, 				ACCESS_DENIED 	},
{ CHMOD,	 {2,3,5,0},				PATH_NOT_FOUND	},
{ NEW_OPEN, 	 {1,2,3,4,5,8,12,32,0}, 		ACCESS_DENIED 	},
{ FS_STATUS, 	 {1,2,3,4,5,6,8,13,15,16,18,0}, 	FAILURE 	},
{ SET_STATUS, 	 {1,3,5,0}, 				ACCESS_DENIED  	},
{ FILE_SIZE,  	 {1,2,3,4,5,6,8,13,15,16,18,0}, 	FAILURE 	},
{ L_SEEK,	 {1,6,33,0},				INVALID_FUNCTION},
{ OLD_CLOSE, 	 {6,0}, 		        	FILDES_INVALID 	},
{ DEVICE_INFO_C, {1,5,6,13,15,0}, 			FILDES_INVALID 	},
{ LOCK,		 {1,5,6,8,13,33,0},			LOCK_VIOLATION  },
{ CONSOLE_READ,  {1,2,3,4,5,6,8,13,15,16,18,0}, 	FAILURE 	},
{ 0, 		 { 0 }, 				FAILURE 	}
};


struct exterrtab {
	unsigned char	error,
			class,
			action,
			locus;
} exterrtab[] = {
	{ 0x1, 0x7, 0x4, 0xFF },
	{ 0x2, 0x8, 0x3, 0x2 },
	{ 0x3, 0x8, 0x3, 0x2 },
	{ 0x4, 0x1, 0x4, 0x1 },
	{ 0x5, 0x3, 0x3, 0xFF },
	{ 0x6, 0x7, 0x4, 0x1 },
	{ 0x7, 0x7, 0x5, 0x5 },
	{ 0x8, 0x1, 0x4, 0x5 },
	{ 0x9, 0x7, 0x4, 0x5 },
	{ 0xA, 0x7, 0x4, 0x5 },
	{ 0xB, 0x9, 0x3, 0x1 },
	{ 0xC, 0x7, 0x4, 0x1 },
	{ 0xD, 0x9, 0x4, 0x1 },
	{ 0xF, 0x8, 0x3, 0x2 },
	{ 0x10, 0x3, 0x3, 0x2 },
	{ 0x11, 0xD, 0x3, 0x2 },
	{ 0x12, 0x8, 0x3, 0x2 },
	{ 0x13, 0xB, 0x7, 0x2 },
	{ 0x14, 0x4, 0x5, 0x1 },
	{ 0x15, 0x5, 0x7, 0xFF },
	{ 0x20, 0xA, 0x2, 0x2 },
	{ 0x21, 0xA, 0x2, 0x2 },
	{ 0x24, 0x1, 0x4, 0x5 },
	{ 0x32, 0x9, 0x3, 0x3 },
	{ 0x50, 0xC, 0x3, 0x2 },
	{ 0x52, 0x1, 0x4, 0x2 },
	{ 0x53, 0xD, 0x4, 0x1 },
	{ 0x54, 0x1, 0x4, 0xFF },
	{ 0x55, 0xC, 0x3, 0x3 },
	{ 0x56, 0x3, 0x3, 0x1 },
	{ 0x57, 0x9, 0x3, 0x1 },
	{ 0x0, 0x0, 0x0, 0x0 }
};


/*
 * err_handler			Is the bridge error handler which converts UNIX
 *				system call error codes to the corresponding 
 *				MS-DOS	error codes.  If the appropriate DOS
 *				code assigned is not one that the corresponding
 *				DOS system call might return, it is converted
 *				to an appropriate response (default).
 */

void
err_handler(response, req_in, path)
unsigned char 	*response;		/* the DOS response to be returned */
int 		req_in;			/* the DOS request being simulated */
char		*path;			/* pathname on which sys call failed */
{
    register int i;
    register int j;
    char *c;

    for (i=0; (req_specs[i].request) && (req_in != req_specs[i].request); i++)
	;	/* find the appropriate table entry for request */

    switch(errno) {
	case ENOENT:
	    if (req_in == CHDIR || req_in == RMDIR)
		*response = PATH_NOT_FOUND;
	    else {
		if (path && (c = strrchr(path, '/')))
		    c[(c == path) ? 1 : 0] = '\0';	/* leave '/' if root */
		else
		    path = ".";
		*response = access(path, 00) ? PATH_NOT_FOUND : FILE_NOT_FOUND;
	    }
	    break;

	case ENOTDIR:
	    /*  if access(path, 00) succeeds, then the last path component
	    **  is the one that resulted in ENOTDIR.
	    */
	    *response = access(path, 00) ? PATH_NOT_FOUND : ACCESS_DENIED;
	    break;

	case EMFILE:
	    *response = TOO_MANY_FILES;
	    break;

	case EEXIST:
	    *response = (req_in == RMDIR) ? ACCESS_DENIED : FILE_EXISTS;
	    break;

	case EACCES:
	case EPERM:
	case EROFS:
	case ETXTBSY:
	    *response = ACCESS_DENIED;
	    break;

	case EBADF:
	    *response = FILDES_INVALID;
	    break;

	case EFBIG:
	case ENOMEM:
	    *response = INSUFFICIENT_MEMORY;
	    break;

	case EFAULT:
	    *response = MEM_BLOCK_INVALID;
	    break;

	case EINVAL:
	case ESRCH:
	    *response = DATA_INVALID;
	    break;
	
	case ENODEV:
	    *response = DRIVE_INVALID;
	    break;

	case EISDIR:
	    *response = ATTEMPT_TO_REMOVE_DIR;
	    break;

	case ENFILE:
	case ENOSPC:
	    *response = NO_MORE_FILES;
	    break;

#ifdef RLOCK 
	case ENOSHARE:	
	    *response = SHARE_VIOLATION;
	    break;
#endif  /* RLOCK */

	case EBUSY:
	case EDOM:
	case EAGAIN:
	    *response = LOCK_VIOLATION;
	    break;

	default:
	    *response = req_specs[i].default_response;
	    break;
    }

    if (!(bridge_ver_flags & V_ERR_FILTER)) {
	for (j=0; (req_specs[i].err_range[j]) &&
				(*response != req_specs[i].err_range[j]); j++)
	    ;		/* check if assigned response is in request's range */
			/* if not, assign the default response */
	if (!req_specs[i].err_range[j])
	    *response = req_specs[i].default_response;
    }
}



/*
 * pci_get_ext_err		Formerly defined in p_get_err.c
 *				returns additional error information for
 *				the error response of previous system call
 *				(imports extern err_code holding previous
 *				 value of *response)
 */

void
pci_get_ext_err(addr)
struct output *addr;
{
    register int i;

    if (err_code == FILE_EXISTS) {
	/*
	** FILE_EXISTS is far greater than the other error codes, so we
	** save table space by checking for it directly
	*/
	addr->text[0] = 2;
	addr->text[1] = 2;		/* set all values to two */
	addr->text[2] = 2;
    }
    else {

	for(i=0; exterrtab[i].error; i++)
	    if (exterrtab[i].error == err_code) 
		break;

	if(exterrtab[i].error) {
	    addr->text[0] = exterrtab[i].class;
	    addr->text[1] = exterrtab[i].action;
	    addr->text[2] = exterrtab[i].locus;
	}

    }

    addr->hdr.res = err_code;

/*
 *	This has been "fixed" on the DOS side. You do not to need this 
 *	statement.  It should be fixed better later.
 *	*((int*)&addr->text[3]) = err_code;
*/

    addr->hdr.t_cnt = 3;
}
