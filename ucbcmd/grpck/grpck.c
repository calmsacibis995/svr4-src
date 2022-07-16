/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucbgrpck:grpck.c	1.2.1.1"

/*******************************************************************

		PROPRIETARY NOTICE (Combined)

This source code is unpublished proprietary information
constituting, or derived under license from AT&T's UNIX(r) System V.
In addition, portions of such source code were derived from Berkeley
4.3 BSD under license from the Regents of the University of
California.



		Copyright Notice 

Notice of copyright on this source code product does not indicate 
publication.

	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
	          All rights reserved.
********************************************************************/ 

#include <stdio.h>
#include <ctype.h>
#include <pwd.h>

#define ERROR1 "Too many/few fields"
#define ERROR2a "No group name"
#define ERROR2b "Bad character(s) in group name"
#define ERROR2c "First char in group name not lower case alpha"
#define ERROR2d "Group name too long"
#define ERROR3  "Invalid GID"
#define ERROR4a "Null login name"
#define ERROR4b "Login name not found in password file"

#define MYBUFSIZE  512  /* Max line length including newline and null */

int eflag, badchar, baddigit,badlognam,colons,len,i;
char buf[MYBUFSIZE];
char tmpbuf[MYBUFSIZE];

struct passwd *getpwnam();
char *strchr();
char *nptr;
void setpwent();
char *cptr;
FILE *fptr;
int delim[MYBUFSIZE];
long gid;
int yellowpages;
int ngrpnamec;
int error();

main (argc,argv)
int argc;
char *argv[];
{
  if ( argc == 1)
    argv[1] = "/etc/group";
  else if ( argc != 2 )
       {
	 (void)fprintf (stderr,"\nusage: %s filename\n\n",*argv);
	 exit(1);
       }

  if ( ( fptr = fopen (argv[1],"r" ) ) == NULL )
  { 
	(void)fprintf (stderr,"\ncannot open file %s\n\n",argv[1]);
	exit(1);
  }

  while(fgets(buf,MYBUFSIZE,fptr) != NULL )
  {
	if ( buf[0] == '\n' )    /* blank lines are ignored */
          continue;

	for (i=0; buf[i]!=NULL; i++)
	{
	  tmpbuf[i]=buf[i];          /* tmpbuf is a work area */
	  if (tmpbuf[i] == '\n')     /* newline changed to NULL */  
	    tmpbuf[i] = NULL;
	}

	for (; i <= MYBUFSIZE; ++i)     /* blanks out rest of tmpbuf */ 
	{
	  tmpbuf[i] = NULL;
	}
	colons=0;
	eflag=0;
	badchar=0;
	baddigit=0;
	badlognam=0;
	gid=0l;
	yellowpages=0;

    /* See if this is a Yellow Pages reference */

	if(buf[0] == '+' || buf[0] == '-') {
	  yellowpages++;
	}

    /*	Check number of fields	*/

	for (i=0 ; buf[i]!=NULL ; i++)
	{
	  if (buf[i]==':')
          {
            delim[colons]=i;
            ++colons;
          }
	}
	if (colons != 3 )
	{
	  if(!yellowpages) error(ERROR1);
	  continue;
	}

	/* Check that first character is alpha and rest alphanumeric */
 
	i = 0;
	if(yellowpages) {
	  i++;
	  if(buf[1] == '\0' || buf[1] == ':') continue;
	}
	else
	{
	  if ( buf[0] == ':' )
	    error(ERROR2a);
	}
	if (!(islower(buf[i]))) {
	    error(ERROR2c);
	}
	for (ngrpnamec=1, i++; buf[i] != ':'; ngrpnamec++, i++) {
	    if (islower(buf[i]));
	    else if (isdigit(buf[i]));
	    else ++badchar;
	}

	if (badchar > 0)
	    error(ERROR2b);

	/* check for valid number of characters in groupname */
	if (ngrpnamec > 8) {
		error(ERROR2d);
	}

    /*	check that GID is numeric and <= 65535	*/

	len = ( delim[2] - delim[1] ) - 1;

	if ( len > 5 || ( !yellowpages && len == 0 ) )
	  error(ERROR3);
	else
	{
	  for ( i=(delim[1]+1); i < delim[2]; i++ )
	  {
	    if ( ! (isdigit(buf[i])))
	      baddigit++;
	    else if ( baddigit == 0 )
		gid=gid * 10 + (buf[i]) - '0';    /* converts ascii */
                                                  /* GID to decimal */
	  }
	  if ( baddigit > 0 )
	    error(ERROR3);
	  else if ( gid > 65535l || gid < 0l )
	      error(ERROR3);
	}

     /*  check that logname appears in the passwd file  */

	nptr = &tmpbuf[delim[2]];
	nptr++;
	if ( *nptr == NULL )
	  continue;	/* empty logname list is OK */
	for (;;)
	{
	  if ( ( cptr = strchr(nptr,',') ) != NULL )
	    *cptr=NULL;
	  if ( *nptr == NULL )
	    error(ERROR4a);
	  else
	  {
	    if (  getpwnam(nptr) == NULL )
	    {
	      badlognam=1;
	      error(ERROR4b);
	    }
	  }
	  if ( cptr == NULL )
	    break;
	  nptr = ++cptr;
	  (void)setpwent();
	}
  }
  exit(0);
  /* NOTREACHED */
}

    /*	Error printing routine	*/

error(msg)

char *msg;
{
	if ( eflag==0 )
	{
	  (void)fprintf(stderr,"\n\n%s",buf);
	  eflag=1;
	}

	if ( badchar != 0 )
	{
	  (void)fprintf (stderr,"\t%d %s\n",badchar,msg);
	  badchar=0;
	  return;
	}
	else if ( baddigit != 0 )
	     {
		(void)fprintf (stderr,"\t%s\n",msg);
		baddigit=0;
		return;
	     }
	     else if ( badlognam != 0 )
		  {
		     (void)fprintf (stderr,"\t%s - %s\n",nptr,msg);
		     badlognam=0;
		     return;
		  }
		  else
		  {
		    (void)fprintf (stderr,"\t%s\n",msg);
		    return;
		  }
}
