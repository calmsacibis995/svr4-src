/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nadmin.rfs:other/rfs/system/bin/getaddr.c	1.1.3.1"
#include <netconfig.h>
#include <netdir.h>
#include <tiuser.h>
#include <stdio.h>

#define NULL 0
/* arguments:argv[1] =netspec
             argv[2] =hostname
             argv[3] =domain
             argv[4] =type(primary or secondary)
*/

main(argc,argv)
int argc;
char *argv[];

{
struct nd_hostserv 	nd_hostserv;
struct netconfig	*netconfigp;
struct nd_addrlist	*nd_addrlistp;
struct netbuf 		*addr;
char			*netspec;
char			*domain;
char			*type;
int			j;
char			buf[300];
FILE			*fp,*efp;

int i;

nd_hostserv.h_host=argv[2];
nd_hostserv.h_serv="listen";
netspec=argv[1];
domain=argv[3];
type=argv[4];


/* transport not installed */
if ((netconfigp=getnetconfigent(netspec)) == NULL) {
	exit(1);
}

/* name to address mapping not setup correctly */
if (netdir_getbyname(netconfigp, &nd_hostserv, &nd_addrlistp) != 0) {
	exit(2);
}

addr=nd_addrlistp->n_addrs;

sprintf(buf,"/etc/rfs/%s",argv[1]);

if (access(buf,0) == -1 ) {
	mkdir(buf,0755);
}

strcat(buf,"/rfmaster");
if ((fp=fopen(buf, "a")) == NULL) {
	exit(2);
}
fprintf(fp,"%s\t%s\t%s.%s\n%s.%s\tA\t",domain,type,domain,argv[2],domain,argv[2]);
fprintf(fp,"\\x");

for (i=0; i<addr->len; i++) {
	j = (addr->buf[i] >> 4) & 0xf;
	if (j > 9)
		j += 'a' - 10;
	else
		j += '0';
	fprintf(fp,"%c",j);
	j = addr->buf[i] & 0xf;
	if (j > 9)
		j += 'a' - 10;
	else
		j += '0';
	fprintf(fp,"%c",j);
}

fprintf(fp,"\n");
fclose(fp);
exit(0);

}
