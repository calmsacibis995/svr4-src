/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nametoaddr:straddr/straddr.c	1.8.1.2"
#include <stdio.h>
#include <tiuser.h>
#include <netdir.h>
#include <netconfig.h>
#include <sys/utsname.h>

/*
 *	The generic name to address mappings for any transport that
 *	has strings for address (e.g., ISO Starlan).
 *
 *	Address in ISO Starlan consist of arbitrary strings of
 *	characters.  Because of this, the following routines
 *	create an "address" based on two strings, one gotten
 *	from a "host" file and one gotten from a "services" file.
 *	The two strings are catenated together (with a "." between
 *	them).  The hosts file is /etc/net/starlan/hosts and
 *	contain lines of the form:
 *
 *		arbitrary_string	machname
 *
 *	To make things simple, the "arbitraty string" should be the
 *	machine name.
 *
 *	The services file is /etc/net/starlan/services and has lines
 *	of the form:
 *
 *		service_name	arbitrary_string
 *
 *	Again, to make things easer, the "arbitrary name" should be the
 *	service name.
 */

#define HOSTFILE	"/etc/net/%s/hosts"
#define SERVICEFILE	"/etc/net/%s/services"
#define MAXELEMENTS	10
#define MAXLEN		30
#define FIELD1		1
#define FIELD2		2

/*
 *	Local and external functions used.
 */

extern char *strdup(), *strtok(), *strcpy(), *calloc();
extern char *fgets(), *malloc(), *strcat();
extern FILE *fopen();
extern int   fclose(), strcmp(), strlen();

char **searchhost();
char *searchserv();
char *mergeaddr();

extern int _nderror;  /* used for error reporting */

/*
 *	_netdir_getbyname() returns all of the addresses for
 *	a specified host and service.
 */

struct nd_addrlist *
_netdir_getbyname(netconfigp, nd_hostservp)
struct netconfig    *netconfigp;
struct nd_hostserv  *nd_hostservp;
{
	char   searchfile[BUFSIZ]; /* the name of the file to  be opened   */
	char   fulladdr[BUFSIZ];   /* holds the full address string        */
	char   **addrpp;	   /* list of addresses from the host file */
	char   *port;		   /* the string from the services file    */
	char   *hostname;	   /* the host name to search for          */
	FILE   *fp;		   /* points to the hosts and service file */
	struct nd_addrlist *retp;  /* the return structure                 */
	struct netbuf *netbufp;    /* indexes through the addresses        */
	struct utsname utsname;    /* holds information about this machine */
	int    naddr;		   /* the number of address obtained       */

	_nderror = ND_OK;

	/*
	 *	If the given hostname is HOST_SELF,  simply re-set
	 *	it to the uname of this machine.  If it is HOST_ANY
	 *	or HOST_BROADCAST, return an error (since they are not
	 *	supported).
	 */

	if ((strcmp(nd_hostservp->h_host, HOST_ANY) == 0)
	 || (strcmp(nd_hostservp->h_host, HOST_BROADCAST) == 0)) {
		_nderror = ND_NOHOST;
		return(NULL);
	}

	hostname = nd_hostservp->h_host;
	if (strcmp(nd_hostservp->h_host, HOST_SELF) == 0) {
		uname(&utsname);
		hostname = utsname.nodename;
	}

	/*
	 *	Get the first part of the address.  The hosts file
	 *	is created with the local name of the starlan network
	 *	(given in netconfigp->nc_netid).
	 */

	(void)sprintf(searchfile, HOSTFILE, netconfigp->nc_netid);

	if (((fp = fopen(searchfile, "r")) == NULL)
	 || ((addrpp = searchhost(fp, hostname, &naddr, FIELD2)) == NULL)) {
		if (fp != NULL) {
			(void)fclose(fp);
		}
		_nderror = ND_NOHOST;
		return(NULL);
	}
	(void)fclose(fp);

	/*
	 *	Get the second part of the address from the services file
	 */

	(void)sprintf(searchfile, SERVICEFILE, netconfigp->nc_netid);

	if (((fp = fopen(searchfile, "r")) == NULL)
	 || ((port = searchserv(fp, nd_hostservp->h_serv, FIELD1)) == NULL)) {
		if (fp != NULL) {
			(void)fclose(fp);
		}
		_nderror = ND_NOSERV;
		return(NULL);
	}
	(void)fclose(fp);

	/*
	 *	Allocate space to hold the return structure, set the number
	 *	of address, and allocate space to hold them.
	 */

	if ((retp = (struct nd_addrlist *)malloc(sizeof(struct nd_addrlist))) == NULL) {
		_nderror = ND_NOMEM;
		return(NULL);
	}

	retp->n_cnt = naddr;
	if ((retp->n_addrs = (struct netbuf *)calloc((unsigned)naddr, sizeof(struct netbuf))) == NULL) {
		_nderror = ND_NOMEM;
		return(NULL);
	}

	/*
	 *	Now simply fill in the address by forming strings of the
	 *	form "string_from_hosts.string_from_services"
	 */

	netbufp = retp->n_addrs;
	while (naddr--) {

		(void)strcpy(fulladdr, *addrpp++);
		(void)strcat(fulladdr, ".");
		(void)strcat(fulladdr, port);

		/*
		 *	don't include the terminating NULL character in the
	 	 *	length.
		 */

		netbufp->len = netbufp->maxlen = strlen(fulladdr);
		if ((netbufp->buf = strdup(fulladdr)) == NULL) {
			_nderror = ND_NOMEM;
			return(NULL);
		}
		netbufp ++;
	}

	return(retp);
}

/*
 *	_netdir_getbyaddr() takes an address (hopefully obtained from
 *	someone doing a _netdir_getbyname()) and returns all hosts with
 *	that address.
 */

struct nd_hostservlist *
_netdir_getbyaddr(netconfigp, netbufp)
struct netconfig *netconfigp;
struct netbuf	 *netbufp;
{
	char   searchfile[BUFSIZ];        /* the name of file to be opened  */
	char   fulladdr[BUFSIZ];          /* a copy of the address string   */
	char   *addr;		          /* the "first" path of the string */
	char   *port;		          /* the "second" part of string    */
	struct nd_hostservlist *retp;     /* the return structure           */
	FILE   *fp;		          /* pointer to open files          */
	char   **hostpp;	          /* points to list of host names   */
	char   *serv;		          /* resultant service name obtained*/
	int    nhost;		          /* the number of hosts in hostpp  */
	struct nd_hostserv *nd_hostservp; /* traverses the host structures  */

	_nderror = ND_OK;

	/*
	 *	Separate the two parts of the address string.
	 */

	(void)strncpy(fulladdr, netbufp->buf, netbufp->len);
	fulladdr[netbufp->len] = '\0';
	addr = strtok(fulladdr, ".");
	port = strtok(NULL, " \n\t");

	/*
	 *	Search for all the hosts associated with the
	 *	first part of the address string.
	 */

	(void)sprintf(searchfile, HOSTFILE, netconfigp->nc_netid);

	if (((fp = fopen(searchfile, "r")) == NULL)
	 || ((hostpp = searchhost(fp, addr, &nhost, FIELD1)) == NULL)) {
		_nderror = ND_NOHOST;
		if (fp != NULL) {
			(void)fclose(fp);
		}
		return(NULL);
	}
	(void)fclose(fp);

	/*
	 *	Search for the service associated with the second
	 *	path of the address string.
	 */

	(void)sprintf(searchfile, SERVICEFILE, netconfigp->nc_netid);

	if (port == NULL) {
		_nderror = ND_NOSERV;
		return(NULL);
	}

	if (((fp = fopen(searchfile, "r")) == NULL)
	 || ((serv = searchserv(fp, port, FIELD2)) == NULL)) {
		serv = _taddr2uaddr(netconfigp, netbufp);
		strcpy(fulladdr, serv);
		free(serv);
		serv = fulladdr;
		while (*serv != '.')
			serv ++;
	}
	if (fp != NULL) {
		(void)fclose(fp);
	}

	/*
	 *	Allocate space to hold the return structure, set the number
	 *	of hosts, and allocate space to hold them.
	 */

	if ((retp = (struct nd_hostservlist *)malloc(sizeof(struct nd_hostservlist))) == NULL) {
		_nderror = ND_NOMEM;
		return(NULL);
	}

	retp->h_cnt = nhost;
	if ((retp->h_hostservs = (struct nd_hostserv *)calloc((unsigned)nhost, sizeof(struct nd_hostserv))) == NULL) {
		_nderror = ND_NOMEM;
		return(NULL);
	}

	/*
	 *	Loop through the host structues and fill them in with
	 *	each host name (and service name).
	 */

	nd_hostservp = retp->h_hostservs;

	while (nhost--) {
		if (((nd_hostservp->h_host = strdup(*hostpp++)) == NULL)
		 || ((nd_hostservp->h_serv = strdup(serv)) == NULL)) {
			_nderror = ND_NOMEM;
			return(NULL);
		}
		nd_hostservp ++;
	}

	return(retp);
}

/* 
 *	_taddr2uaddr() translates a address into a "universal" address.
 *	Since the address is a string, simply return the string as the
 *	universal address (but replace all non-printable characters with
 *	the \ddd form, where ddd is three octal digits).  The '\n' character
 *	is also replace by \ddd and the '\' character is placed as two
 *	'\' characters.
 */

char *
_taddr2uaddr(netconfigp, netbufp)
struct netconfig *netconfigp;
struct netbuf    *netbufp;
{
	char *retp;	/* pointer the return string 		     */
	char *to;	/* traverses and populates the return string */
	char *from;	/* traverses the string to be converted      */
	int i;		/* indexes through the given string          */

	if ((retp = malloc(BUFSIZ)) == NULL) {
		_nderror = ND_NOMEM;
		return(NULL);
	}
	to = retp;
	from = netbufp->buf;

	for (i = 0; i < netbufp->len; i++) {
		if (*from == '\\') {
			*to++ = '\\';
			*to++ = '\\';
		} else {
			if (*from == '\n' || !isprint(*from)) {
				(void) sprintf(to, "\\%.3o", *from);
				to += 4;
			} else {
				*to++ = *from;
			}
		}
		from++;
	}
	*to = '\0';
	return(retp);
}

/* 
 *	_uaddr2taddr() translates a universal address back into a
 *	netaddr structure.  Since the universal address is a string,
 *	put that into the TLI buffer (making sure to change all \ddd
 *	characters back and strip off the trailing \0 character).
 */

struct netbuf *
_uaddr2taddr(netconfigp, uaddr)
char	         *uaddr;
struct netconfig *netconfigp;
{
	struct netbuf *retp;  /* the return structure  			 */
	char *holdp;	      /* holds the converted address 		 */
	char *to;	      /* traverses and populates the new address */
	char *from;	      /* traverses the universal address         */

	holdp = malloc(strlen(uaddr) + 1);
	from = uaddr;
	to = holdp;

	while (*from) {
		if (*from == '\\') {
			if (*(from+1) == '\\') {
				*to = '\\';
				from += 2;
			} else {
				*to = ((*(from+1) - '0') << 6) +
				      ((*(from+2) - '0') << 3) +
				       (*(from+3) - '0');
				from += 4;
			}
		} else {
			*to = *from++;
		}
		to++;
	}
	*to = '\0';

	if ((retp = (struct netbuf *)malloc(sizeof(struct netbuf))) == NULL) {
		_nderror = ND_NOMEM;
		return(NULL);
	}
	retp->maxlen = retp->len = to - holdp;
	retp->buf = holdp;
	return(retp);
}

/*
 *	_netdir_options() is a "catch-all" routine that does
 *	transport specific things.  The only thing that these
 *	routines have to worry about is ND_MERGEADDR.
 */

int
_netdir_options(netconfigp, option, fd, par)
struct netconfig *netconfigp;
int		  option;
int		  fd;
void		 *par;
{
	struct nd_mergearg *argp;  /* the argument for mergeaddr */

	switch(option) {
	   case ND_MERGEADDR:
		argp = (struct nd_mergearg *)par;
		argp->m_uaddr = mergeaddr(netconfigp, argp->s_uaddr, argp->c_uaddr);
		return(argp->m_uaddr == NULL? -1 : 0);
	   default:
		_nderror = ND_NOCTRL;
		return(-1);
	}
}

/*
 *	mergeaddr() translates a universal address into something
 *	that makes sense to the caller.  This is a no-op in starlan's case,
 *	so just return the universal address.
 */

static char *
mergeaddr(netconfigp, uaddr, ruaddr)
struct netconfig *netconfigp;
char             *uaddr;
char             *ruaddr;
{
	return(strdup(uaddr));
}

/*
 *	searchhost() looks for the specified token in the host file.
 *	The "field" parameter signifies which field to compare the token
 *	on, and returns all values associated with the token.
 */
	
static char **
searchhost(fp, token, nelements, field)
FILE  *fp;
char  *token;
int   *nelements;
int   field;
{
	/*
 	 *	"retp" and "namebuf" make up the array of names
	 *	to return.
	 */

	static char *retp[MAXELEMENTS];
	static char namebuf[MAXELEMENTS*MAXLEN];

	char buf[BUFSIZ];	/* holds each line of the file              */
	char *fileaddr;		/* the first token in each line 	    */
	char *filehost;		/* the second token in each line	    */
	char *namep = namebuf;	/* where the next return string should go   */
	char *cmpstr;		/* the string to compare token to	    */
	char *retstr;		/* the string to return if compare succeeds */

	/*
	 *	nelements will contain the number of token found, so
	 *	initially set it to 0.
	 */

	*nelements = 0;

	/*
	 *	Loop through the file looking for the tokens and creating
	 *	the list of strings to be returned.
 	 */

	while (fgets(buf, BUFSIZ, fp) != NULL) {

		/*
		 *	Ignore comments and bad lines.
		 */

		if (((fileaddr = strtok(buf, " \t\n")) == NULL)
		 ||  (*fileaddr == '#')
		 || ((filehost = strtok(NULL, " \t\n")) == NULL)) {
			continue;
		}

		/*
		 *	determine which to compare the token to, then
		 *	compare it, and if they match, add the return
		 *	string to the list.
		 */

		cmpstr = (field == FIELD1)? fileaddr : filehost;
		retstr = (field == FIELD1)? filehost : fileaddr;

		if ((strcmp(token, cmpstr) == 0)
		 && (*nelements < MAXELEMENTS)) {
			retp[(*nelements)++] = strcpy(namep, retstr);
			namep[MAXLEN - 1] = '\0';
			namep += MAXLEN;
		}
	}

	/*
	 *	If *nelements is 0 then no matches were found.
	 */

	if (*nelements == 0) {
		return(NULL);
	}
	return(retp);
}

/*
 *	searchserve() looks for the specified token in the service file.
 *	The "field" parameter signifies which field to compare the token
 *	on, and returns the string associated with eth token.
 */

static char *
searchserv(fp, token, field)
FILE  *fp;
char  *token;
int   field;
{
	static char buf[BUFSIZ]; /* holds a line of the file		   */
	char *fileservice;	 /* the first token in each line	   */
	char *fileport;		 /* the second token in each line	   */
	char *retp;		 /* the string to be returned		   */
	char *cmpstr;		 /* the string to compare the token to	   */
	char *retstr;		 /* temporarily hold token in line of file */

	retp = NULL;

	/*
	 *	Loop through the services file looking for the token.
	 */

	while (fgets(buf, BUFSIZ, fp) != NULL) {

		/*
		 *	If comment or bad line, continue.
		 */

		if (((fileservice = strtok(buf, " \t\n")) == NULL)
		 ||  (*fileservice == '#')
		 || ((fileport = strtok(NULL, " \t\n")) == NULL)) {
			continue;
		}

		cmpstr = (field == FIELD1)? fileservice : fileport;
		retstr = (field == FIELD1)? fileport : fileservice;

		if (strcmp(token, cmpstr) == 0) {
			retp = retstr;
			break;
		}

	}
	return(retp);
}
