/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)libplot:vt0/line.c	1.3"
extern vti;
extern xnow,ynow;
line(x0,y0,x1,y1){
	struct{char x,c; int x0,y0,x1,y1;} p;
	p.c = 3;
	p.x0 = xsc(x0);
	p.y0 = ysc(y0);
	p.x1 = xnow = xsc(x1);
	p.y1 = ynow = ysc(y1);
	write(vti,&p.c,9);
}
cont(x0,y0){
	line(xnow,ynow,xsc(x0),ysc(y0));
	return;
}
