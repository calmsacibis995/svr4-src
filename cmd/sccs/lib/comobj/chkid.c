/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sccs:lib/comobj/chkid.c	6.4"
# include	"ctype.h"
# include	"../../hdr/defines.h"


char *strchr();

chkid(line,idstr)

char *line;
char *idstr;

{
	register char *lp;
	register char *p;
	extern int Did_id;
	unsigned	strlen();
	int	strncmp();

	if (!Did_id && (lp = strchr(line,'%') ) )
		if (!idstr || idstr[0]=='\0' ) 
			for( ; *lp != 0; lp++) {
				if(lp[0] == '%' && lp[1] != 0 && lp[2] == '%')
					if (isupper(lp[1]))
						switch (lp[1]) {
						case 'J':
							break;
						case 'K':
							break;
						case 'N':
							break;
						case 'O':
							break;
						case 'V':
							break;
						case 'X':
							break;
						default:
							return(Did_id++);
						}
			}
		else {
			if ( (lp = strchr(idstr,'%')) == NULL ) return(Did_id);
			for( ; *lp != 0; lp++) {
				if(lp[0] == '%' && lp[1] != 0 && lp[2] == '%')
					if (isupper(lp[1]))
						switch (lp[1]) {
						case 'J':
							break;
						case 'K':
							break;
						case 'N':
							break;
						case 'O':
							break;
						case 'V':
							break;
						case 'X':
							break;
						default:
							Did_id++;
						}
			}
			if (!Did_id) return(Did_id); /* There's no keyword in idstr */
			Did_id = 0;
			p=idstr;
			lp=line;
			while(lp=strchr(lp,*p))
				if(!(strncmp(lp,p,strlen(p))))
					return(Did_id++);
				else
					++lp;
		}

	return(Did_id);
}
