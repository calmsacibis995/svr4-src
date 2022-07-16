/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nametoaddr:npack/npack.c	1.6.1.2"
#include <stdio.h>
#include <tiuser.h>
#include <netdir.h>
#include <netconfig.h>
#include <ctype.h>
#include <sys/utsname.h>

/*
 *	The generic name to address mappings for NPACK.
 *
 *	Address in NPACK consist of a port (32 bits) followed by
 *	and address (64 bits).  The following routines create the
 *	address from a "host" file and an "services" file.
 *	The hosts file is /etc/net/npack/hosts and
 *	contain lines of the form:
 *
 *		16_hexadecimal_digit_address	machname
 *
 *	The services file is /etc/net/npack/services and has lines
 *	of the form:
 *
 *		service_name	8_hexadecimal_digit_port
 *
 */

#define HOSTFILE	"/etc/net/%s/hosts"
#define SERVICEFILE	"/etc/net/%s/services"
#define MAXELEMENTS	10
#define MAXLEN		30
#define FIELD1		1
#define FIELD2		2

/*
 *	The length of the port and address in bytes.
 */

#define	PORTLENGTH	4
#define	ADDRLENGTH	8

#define	toxdigit(c)	((isdigit(c))?((int)(c)-'0'):(toupper(c)-(int)'A'+10))
#define	tochar(c)	((c < 10)?((char)(c) + '0'):((char)(c)+'A'-(char)10))

/*
 *	Local and external functions used.
 */

extern char *strdup(), *strtok(), *strcpy(), *calloc();
extern char *fgets(), *malloc(), *strcat(), *memcpy();
extern FILE *fopen();
extern int   fclose(), strcmp(), strlen();
extern long  strtol();

char **searchhost();
char *searchserv();
char *mergeaddr();
int   dohex();

extern int _nderror;  /* used for error reporting */

/*
 *	_netdir_getbyname() returns all of the addresses for
 *	a specified host and service.
 */

struct nd_addrlist *
_netdir_getbyname(netconfigp, nd_hostservp)
struct netconfig   *netconfigp;
struct nd_hostserv *nd_hostservp;
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
	 *	Get the address of the host.  The hosts file
	 *	is created with the local name of the npack network
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
	 *	Get the port number of the address from the services file
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
	 *	Now simply fill in the address by calling "dohex"
	 *	to transform the hexadecimal string into a hexidecimal
	 *	number and places it into the buffer provided.
	 */

	netbufp = retp->n_addrs;
	if (dohex(port, fulladdr) != PORTLENGTH) {
		_nderror = ND_NOSYM;
		return(NULL);
	}
	while (naddr--) {

		/*
		 *	The port number takes up PORTLENGTH bytes;  the
		 *	address takes up ADDRLENGTH
		 */

		if (dohex(*addrpp++, &fulladdr[PORTLENGTH]) != ADDRLENGTH) {
			_nderror = ND_NOSYM;
			return(NULL);
		}
		fulladdr[PORTLENGTH + ADDRLENGTH] = '\0';

		netbufp->len = PORTLENGTH + ADDRLENGTH;
		netbufp->maxlen = PORTLENGTH + ADDRLENGTH;
		if ((netbufp->buf = malloc(PORTLENGTH + ADDRLENGTH)) == NULL) {
			_nderror = ND_NOMEM;
			return(NULL);
		}
		(void)memcpy(netbufp->buf, fulladdr, PORTLENGTH + ADDRLENGTH);
		netbufp ++;
	}

	return(retp);
}

/*
 *	_netdir_getbyaddr() takes an address and returns all hosts with
 *	that address.
 */

struct nd_hostservlist *
_netdir_getbyaddr(netconfigp, netbufp)
struct netconfig *netconfigp;
struct netbuf	 *netbufp;
{
	char   searchfile[BUFSIZ];        /* the file to  be opened         */
	char   fulladdr[BUFSIZ];          /* holds uaddr if no service      */
	char   addr[ADDRLENGTH*2+1];      /* string representation of addr  */
	char   port[PORTLENGTH*2+1];      /* string representation of port  */
	struct nd_hostservlist *retp;     /* the return structure           */
	FILE   *fp;		          /* pointer to open files          */
	char   **hostpp;	          /* points to list of host names   */
	char   *serv;		          /* resultant service name obtained*/
	int    nhost;		          /* the number of hosts in hostpp  */
	struct nd_hostserv *nd_hostservp; /* traverses the host structures  */
	int    i;		          /* indexes through full address   */
	char   tempval;		          /* hold value to turn into a char */

	_nderror = ND_OK;

	/*
	 *	Separate the address into port and host address
	 */

	for (i = 0; i < PORTLENGTH; i++) {
		tempval = (netbufp->buf[i] & 0xF0) >> 4;
		port[i*2] = tochar(tempval);
		tempval = netbufp->buf[i] & 0x0F;
		port[i*2+1] = tochar(tempval);
	}
	port[PORTLENGTH * 2] = '\0';
	
	for (i = 0; i < ADDRLENGTH + 1; i++) {
		tempval = (netbufp->buf[i + PORTLENGTH] & 0xF0) >> 4;
		addr[i*2] = tochar(tempval);
		tempval = netbufp->buf[i + PORTLENGTH] & 0x0F;
		addr[i*2+1] = tochar(tempval);
	}
	addr[ADDRLENGTH * 2] = '\0';

	/*
	 *	Search for all the hosts associated with the
	 *	given host address.
	 */

	(void)sprintf(searchfile, HOSTFILE, netconfigp->nc_netid);

	if (((fp = fopen(searchfile, "r")) == NULL)
	 || ((hostpp = searchhost(fp, addr, &nhost, FIELD1)) == NULL)) {
		if (fp != NULL) {
			(void)fclose(fp);
		}
		_nderror = ND_NOHOST;
		return(NULL);
	}
	(void)fclose(fp);

	/*
	 *	Search for the service associated with the given
	 *	port number.
	 */

	(void)sprintf(searchfile, SERVICEFILE, netconfigp->nc_netid);

	if (((fp = fopen(searchfile, "r")) == NULL)
	 || ((serv = searchserv(fp, port, FIELD2)) == NULL)) {
		serv = _taddr2uaddr(netconfigp, netbufp);
		strcpy(fulladdr, serv);
		fulladdr[11] = '\0';
		free(serv);
		serv = fulladdr;
	}
	(void)fclose(fp);

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
 *	The universal address for npack is a string of the form
 *	"p1.p2.h1.h2.h3.h4" where p1 and  p2 are the decimal representation
 *	of the port number (each being 8 bits), and h1 - h4 are the
 *	decimal representation of the host address (8 bits each).
 */

char *
_taddr2uaddr(netconfigp, netbufp)
struct netconfig *netconfigp;
struct netbuf    *netbufp;
{
	char buf[BUFSIZ];   /* buffer to hold universal address */
	int  i;	    	    /* indexes through buffer           */
	int  place = 0;	    /* current place to put address     */

	for (i = 0; i < PORTLENGTH + ADDRLENGTH; i++) {
		place += sprintf(&buf[place], "%.2x.", netbufp->buf[i]);
	}

	/*
	 *	Get rid of the last "."
	 */

	buf[place - 1] = '\0';
	return(strdup(buf));
}

/* 
 *	_uaddr2taddr() translates a universal address back into a
 *	netaddr structure.
 */

struct netbuf *
_uaddr2taddr(netconfigp, uaddr)
struct netconfig *netconfigp;
char	         *uaddr;
{
	struct netbuf *retp;       /* the return structure        */
	char   tempbuf[BUFSIZ];    /* temporarily holds the uaddr */
	char   *placep;		   /* traverses the uaddr         */
	int    i;		   /* indexes through the netbuf  */

	/*
	 *	Allocate space to hold the return structure and fill in
	 *	the lengths.
	 */

	if ((retp = (struct netbuf *)malloc(sizeof(struct netbuf))) == NULL) {
		_nderror = ND_NOMEM;
		return(NULL);
	}

	retp->maxlen = retp->len = ADDRLENGTH+PORTLENGTH;
	if ((retp->buf = malloc(ADDRLENGTH + PORTLENGTH)) == NULL) {
		_nderror = ND_NOMEM;
		return(NULL);
	}

	/*
	 *	Convert the character digits into longs for the netbuf.
	 */

	(void)strcpy(tempbuf, uaddr);
	placep = strtok(tempbuf, ".");
	for (i = 0; i < ADDRLENGTH + PORTLENGTH; i++) {
		retp->buf[i] = (char)strtol(placep, (char **)NULL, 16);
		placep = strtok(NULL, ".");
	}
	return(retp);
}

/*
 *	_netdir_options() is a "catch-all" routine that does
 *	transport specific things.  The only thing that npack
 *	has to worry about is ND_MERGEADDR.
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
 *	that makes sense to the caller.  This is a no-op in npack's case,
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
	char *p;		/* traverses the addr turning to upper case */

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
		 *	Change all lower case hex digits to upper case
		 *	to help in comparisons later.
		 */

		for (p = fileaddr; *p; p++)
			*p = toupper(*p);

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
	char *p;		 /* changes the port num into upper case   */

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

		/*
		 *	Change all lower case hex digits to upper case
		 *	to help in comparisons later.
		 */

		for (p = fileport; *p; p++)
			*p = toupper(*p);

		cmpstr = (field == FIELD1)? fileservice : fileport;
		retstr = (field == FIELD1)? fileport : fileservice;

		if (strcmp(token, cmpstr) == 0) {
			retp = retstr;
			break;
		}

	}
	return(retp);
}

/*
 *	dohex() takes a string that consists of a string representation of
 *	a hexidecimal number, and converts it into hexidecimal digits
 *	(placing the result into outbuf).
 */

static int
dohex(srcstr, outbuf)
char	*srcstr;
char	*outbuf;
{
	int	curr;       /* used to index through the output buffer */
	int	i;	    /* also indexes through the output buffer  */
	int	shift = 0;  /* boolean that specifies to shift digits  */
	char	*endp;	    /* the end of the input string             */

	/*
	 *	The length of the src string must be 8 (if it is a port
	 *	number) or 16 (if it is an address).
	 */

	curr = strlen(srcstr);
	if (curr != 8 && curr != 16) {
		outbuf[0] = '\0';
		return(0);
	}

	/*
	 *	Find the end of the string and make sure it is \0.
	 */

	for (endp = srcstr; *endp && isxdigit(*endp); ++endp)
		;

	if (*endp != '\0') {
		outbuf[0] = '\0';
		return(0);
	}
	--endp;

	/*
	 *	set "curr" to the last element of the buffer that the
	 *	converted string will go into (if the string was 8, then
	 *	the hex representation will take up 4 bytes, hence the
	 *	last element is 3).
	 */

	curr = (curr / 2) - 1;
	for (i = 0; i <= curr; i++)
		outbuf[i] = '\0';

	/*
	 *	create the hex representation of the string.
	 */

	while (curr >= 0) {
		outbuf[curr] |= (toxdigit(*endp) << shift);
		if ((shift = (shift) ? 0 : 4) == 0)
			--curr;
		--endp;
	}

	return(strlen(srcstr) / 2);
}
