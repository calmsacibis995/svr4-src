/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:filter/postscript/common/request.h	1.1.2.1"

/*
 *
 * Things used to handle special PostScript requests (like manual feed) globally
 * or on a per page basis. All the translators I've supplied accept the -R option
 * that can be used to insert special PostScript code before the global setup is
 * done, or at the start of named pages. The argument to the -R option is a string
 * that can be "request", "request:page", or "request:page:file". If page isn't
 * given (as in the first form) or if it's 0 in the last two, the request applies
 * to the global environment, otherwise request holds only for the named page.
 * If a file name is given a page number must be supplied, and in that case the
 * request will be looked up in that file.
 *
 */

#define MAXREQUEST	30

typedef struct {

	char	*want;
	int	page;
	char	*file;

} Request;


