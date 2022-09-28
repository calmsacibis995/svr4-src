/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ksh:shlib/assign.c	1.3.3.1"
/*
 *   NAM_PUTVAL (NP, STRING)
 *
 *        Assign STRING to NP.
 *
 *   NAM_FPUTVAL (NP, STRING)
 *
 *        Assign STRING to NP even if readonly.
 *
 *
 *   See Also:  nam_longput(III), nam_free(III), nam_strval(III)
 */

#include	"sh_config.h"
#include	"name.h"
#ifdef MULTIBYTE
#   include	"national.h"
#endif /* MULTIBYTE */

extern char	*strcpy();
extern char	*strchr();
extern void	utol(),ltou();
extern void	free();
extern void	nam_rjust();
extern void	sh_fail();


#ifdef MULTIBYTE
static unsigned char *savep;
static unsigned char savechars[ESS_MAXCHAR+1];
static int ja_size();
#else
#define size	 np->value.namsz
#endif /* MULTIBYTE */

/*
 *   NAM_PUTVAL (NP, STRING)
 *
 *        struct namnod *NP;
 *     
 *        char *STRING;
 *
 *   Assign the string given by STRING to the namnod given by
 *   NP.  STRING is converted according to the namflg field
 *   of NP before assignment.  
 *
 *   If NP is an array, then the element given by the
 *   current index is assigned to.
 *   
 *   Any freeable space associated with the old value of NP
 *   is released.
 *
 *   If the copy on write,N_CWRITE flag is set then the assignment
 *   is made on a copy of the np created on the last shell tree.
 * 
 */

static char forced = 0;

void nam_fputval(np,string)
struct namnod *np;
char *string;
{
	forced++;
	nam_putval(np,string);
	forced = 0;
#ifdef apollo
	if(nam_istype(np,N_EXPORT))
	{
		short namlen, vallen;
		char *vp = nam_strval(np);
		namlen =strlen(np->namid);
		vallen = strlen(vp);
		ev_$set_var(np->namid,&namlen,vp,&vallen);
	}
#endif /* apollo */
}

void nam_putval(np,string)
register struct namnod *np;
char *string;
{
	register char *sp=string;
	register union Namval *up;
	register char *cp;
#ifdef MULTIBYTE
	register int size = 0;
#endif /* MULTIBYTE */
	register int dot;
#ifdef apollo
	/* reserve space for UNIX to host file name translation */
	char pathname[256];
	short pathlen;
#endif	/* apollo */
#ifdef NAME_SCOPE
	if (nam_istype (np,N_CWRITE))
		np = nam_copy(np,1);
#endif	/* NAME_SCOPE */
	if (forced==0 && nam_istype (np, N_RDONLY|N_RESTRICT))
	{
		if(nam_istype (np, N_RDONLY))
			sh_fail(np->namid,e_readonly);
		else
			sh_fail(np->namid,e_restricted);
		/* NOTREACHED */
	}
	if(nam_istype (np, N_ARRAY))
		up = &(array_find(np,A_ASSIGN)->namval);
	else
		up= &np->value.namval;
	if (nam_istype (np, N_INDIRECT))
		up = up->up;
	nam_offtype(np,~N_IMPORT);
	if (nam_istype (np, N_INTGER))
	{
		long l;
#ifdef FLOAT
		extern double sh_arith();
		if (nam_istype(np, N_DOUBLE))
		{
			if(up->dp==0)
				up->dp = new_of(double,0);
			*(up->dp) = sh_arith(sp);
			return;
		}
#else
		extern long sh_arith();
#endif /* FLOAT */
		if (nam_istype (np, N_CPOINTER))
		{
			up->cp = sp;
			return;
		}
		l = (sp? (long)sh_arith(sp) : (sh_lastbase=10,0));
		if(np->value.namsz == 0)
			np->value.namsz = sh_lastbase;
		if (nam_istype (np, N_BLTNOD))
		{
			(*up->fp->f_ap)(l);
			return;
		}
		if(up->lp==0)
			up->lp = new_of(long,0);
		*(up->lp) = l;
		if(l && *sp++ == '0')
			np->value.namflg |= N_UNSIGN;
		return;
	}
#ifdef apollo
	if (nam_istype (np, N_HOST) && sp)
	{
		/* this routine returns the host file name given the UNIX name */
		/* other non-unix hosts that use file name mapping should change this */
		unix_fio_$get_name(sp,pathname,&pathlen);
		pathname[pathlen] = 0;
		sp = pathname;
	}
#endif	/* apollo */
	if ((nam_istype (np, N_RJUST|N_ZFILL|N_LJUST)) && sp)
	{
		for(;*sp == ' '|| *sp=='\t';sp++);
        	if ((nam_istype (np, N_ZFILL)) && (nam_istype (np, N_LJUST)))
			for(;*sp=='0';sp++);
#ifdef MULTIBYTE
		if(size = np->value.namsz)
			size = ja_size((unsigned char*)sp,size,nam_istype(np,N_RJUST|N_ZFILL));
#endif /* MULTIBYTE */
	}
	if ((!nam_istype (np, N_FREE|N_ALLOC)) && (up->cp != NULL))
		free(up->cp);
	if (nam_istype (np, N_ALLOC))
		cp = up->cp;
	else
	{
        	np->value.namflg &= ~N_FREE;
        	if (sp)
		{
			dot = strlen(sp);
			if(size==0 && nam_istype(np,N_LJUST|N_RJUST|N_ZFILL))
				np->value.namsz = size = dot;
			else if(size > dot)
				dot = size;
			cp = malloc(((unsigned)dot+1));
		}
		else
			cp = NULL;
		up->cp = cp;
	}
	if (!sp)
		return;
	if (nam_istype (np, N_LTOU))
		ltou(sp,cp);
	else if (nam_istype (np, N_UTOL))
		utol(sp,cp);
	else
        	strcpy (cp, sp);
	if (nam_istype (np, N_RJUST) && nam_istype (np, N_ZFILL))
		nam_rjust(cp,size,'0');
	else if (nam_istype (np, N_RJUST))
		nam_rjust(cp,size,' ');
	else if (nam_istype (np, N_LJUST))
        {
         	sp = strlen (cp) + cp;
		*(cp = (cp + size)) = 0;
		for (; sp < cp; *sp++ = ' ');
         }
#ifdef MULTIBYTE
	/* restore original string */
	if(savep)
		ja_restore();
#endif /* MULTIBYTE */
	return;
}

#ifdef MULTIBYTE
/*
 * handle left and right justified fields for multi-byte chars
 * given physical size, return a logical size which reflects the
 * screen width of multi-byte characters
 * Multi-width characters replaced by spaces if they cross the boundary
 * <type> is non-zero for right justified  fields
 */

static int ja_size(str,size,type)
unsigned char *str;
int size;
{
	register unsigned char *cp = str;
	register int c;
	register int n = size;
	int oldn;
	while(c = *cp++)
	{
		oldn = n;
		/* find character set number */
		c = echarset(c);
		/* allow room for excess input bytes */
		if(c)
		{
			n += (in_csize(c)-out_csize(c)+(c>=2));
			cp += (in_csize(c)-(c==1));
		}
		size -= out_csize(c);
		if(size<=0 && type==0)
			break;
	}
	/* check for right justified fields that need truncating */
	if(size <0)
	{
		if(type==0)
		{
			/* left justified and character crosses field boundary */
			n = oldn;
			/* save boundary char and replace with spaces */
			size = in_csize(c)+(c>2);
			savechars[size] = 0;
			while(size--)
			{
				savechars[size] = *--cp;
				*cp = ' ';
			}
			savep = cp;
		}
		size = -size;
		if(type)
			n -= (ja_size(str,size,0)-size);
	}
	return(n);
}

int ja_restore()
{
	register unsigned char *cp = savechars;
	while(*cp)
		*savep++ = *cp++;
	savep = 0;
}
#endif /* MULTIBYTE */
