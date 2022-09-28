/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ksh:sh/arith.c	1.3.3.1"
/*
 * shell arithmetic - uses streval library
 */

#include	"defs.h"
#include	"streval.h"

extern int	sh_lastbase;

#ifdef FLOAT
    extern double atof();
#endif /* FLOAT */

static number arith(ptr, lvalue, type, n)
char **ptr;
struct lval *lvalue;
number n;
{
	register number r= 0;
	char *str = *ptr;
	switch(type)
	{
	case ASSIGN:
	{
		register struct namnod *np = (struct namnod*)(lvalue->value);
		if(nam_istype(np, N_ARRAY))
			array_ptr(np)->cur[0] = lvalue->flag;
		nam_longput(np, n);
		break;
	}
	case LOOKUP:
	{
		register int c = *str;
		lvalue->value = (char*)0;
		if(isalpha(c))
		{
			register struct namnod *np;
			while(c= *++str, isalnum(c));
			*str = 0;
			np = nam_search(*ptr,sh.var_tree,N_ADD);
			*str = c;
			if(c=='(')
			{
				lvalue->value = (char*)e_function;
				str = *ptr;
				break;
			}
			else if(c=='[')
			{
				str =array_subscript(np,str);
			}
			else if(nam_istype(np,N_ARRAY))
				array_dotset(np,ARRAY_UNDEF);
			lvalue->value = (char*)np;
			if(nam_istype(np,N_ARRAY))
				lvalue->flag = array_ptr(np)->cur[0];
		}
		else
		{
#ifdef FLOAT
			char isfloat = 0;
#endif /* FLOAT */
			sh_lastbase = 10;
			while(c= *str++)  switch(c)
			{
			case '#':
				sh_lastbase = r;
				r = 0;
				break;
			case '.':
			{
				/* skip past digits */
				if(sh_lastbase!=10)
					goto badnumber;
				while(c= *str++,isdigit(c));
#ifdef FLOAT
				isfloat = 1;
				if(c=='e' || c == 'E')
				{
				dofloat:
					c = *str;
					if(c=='-'||c=='+')
						c= *++str;
					if(!isdigit(c))
						goto badnumber;
					while(c= *str++,isdigit(c));
				}
				else if(!isfloat)
					goto badnumber;
				set_float();
				r = atof(*ptr);
#endif /* FLOAT */
				goto breakloop;
			}
			default:
				if(isdigit(c))
					c -= '0';
				else if(isupper(c))
					c -= ('A'-10); 
				else if(islower(c))
					c -= ('a'-10); 
				else
					goto breakloop;
				if( c < sh_lastbase )
					r = sh_lastbase*r + c;
				else
				{
#ifdef FLOAT
					if(c == 0xe && sh_lastbase==10)
						goto dofloat;
#endif /* FLOAT */
					goto badnumber;
				}
			}
		breakloop:
			--str;
		}
		break;

	badnumber:
		lvalue->value = (char*)e_number;
		return(r);
	
	}
	case VALUE:
	{
		register union Namval *up;
		register struct namnod *np;
		np = (struct namnod*)(lvalue->value);
               	if (nam_istype (np, N_INTGER))
		{
#ifdef NAME_SCOPE
			if (nam_istype (np,N_CWRITE))
				np = nam_copy(np,1);
#endif
			if(nam_istype (np, N_ARRAY))
				up = &(array_find(np,A_ASSIGN)->namval);
			else
				up= &np->value.namval;
			if(nam_istype(np,N_INDIRECT))
				up = up->up;
			if(nam_istype (np, (N_BLTNOD)))
				r = (long)((*up->fp->f_vp)());
			else if(up->lp==NULL)
				r = 0;
#ifdef FLOAT
			else if(nam_istype (np, N_DOUBLE))
			{
				set_float();
       	                	r = *up->dp;
			}
#endif /* FLOAT */
			else
       	                	r = *up->lp;
		}
		else
		{
			if((str=nam_strval(np))==0 || *str==0)
				*ptr = 0;
			else
				r = streval(str, &str, arith);
		}
		return(r);
	}
	case ERRMSG:
		sh_fail(*ptr,lvalue->value);
	}

	*ptr = str;
	return(r);
}

number sh_arith(str)
char *str;
{
	return(streval(str,&str, arith));
}

