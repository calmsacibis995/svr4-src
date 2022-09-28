/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sccs:lib/comobj/putline.c	6.7"
# include	"../../hdr/defines.h"

/*
	Routine to write out either the current line in the packet
	(if newline is zero) or the line specified by newline.
	A line is actually written (and the x-file is only
	opened) if pkt->p_upd is non-zero.  When the current line from 
	the packet is written, pkt->p_wrttn is set non-zero, and
	further attempts to write it are ignored.  When a line is
	read into the packet, pkt->p_wrttn must be turned off.
*/

int	Xcreate;
FILE	*Xiop;

void
putline(pkt,newline)
register struct packet *pkt;
char *newline;
{
	static char obf[BUFSIZ];
	char *xf, *auxf();
	register char *p;
	FILE *fdfopen();
	int	stat(), xcreat(), chown(), fatal();

	if(pkt->p_upd == 0) return;

	if(!Xcreate) {
		(void) stat(pkt->p_file,&Statbuf);
		xf = auxf(pkt->p_file,'x');
		Xiop = xfcreat(xf,Statbuf.st_mode);
		setbuf(Xiop,obf);
		chown(xf, Statbuf.st_uid, Statbuf.st_gid);
	}
	if (newline)
		p = newline;
	else {
		if(!pkt->p_wrttn++)
			p = pkt->p_line;
		else
			p = 0;
	}
	if (p) {
		if(fputs(p,Xiop)==EOF)
			FAILPUT;
		if (Xcreate)
			while (*p)
				pkt->p_nhash += *p++;
	}
	Xcreate = 1;
}

void
flushline(pkt,stats)
register struct packet *pkt;
register struct stats *stats;
{
	register char *p;
	char ins[6], del[6], unc[6], hash[6];

	if (pkt->p_upd == 0)
		return;
	putline(pkt,(char *) 0);
	rewind(Xiop);

	if (stats) {
		sprintf(ins,"%.05d",stats->s_ins);
		sprintf(del,"%.05d",stats->s_del);
		sprintf(unc,"%.05d",stats->s_unc);
		for (p = ins; *p; p++)
			pkt->p_nhash += (*p - '0');
		for (p = del; *p; p++)
			pkt->p_nhash += (*p - '0');
		for (p = unc; *p; p++)
			pkt->p_nhash += (*p - '0');
	}

	sprintf(hash,"%5d",pkt->p_nhash&0xFFFF);
	for (p=hash; *p == ' '; p++)	/* replace initial blanks with '0's */
		*p = '0';
	fprintf(Xiop,"%c%c%s\n",CTLCHAR,HEAD,hash);
	if (stats)
		fprintf(Xiop,"%c%c %s/%s/%s\n",CTLCHAR,STATS,ins,del,unc);
	(void) fclose(Xiop);
}

void
xrm()
{
	if (Xiop)
		(void) fclose(Xiop);
	Xiop = 0;
	Xcreate = 0;
}
