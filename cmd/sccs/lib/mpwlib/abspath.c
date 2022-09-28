/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sccs:lib/mpwlib/abspath.c	6.3"
static char	pop();
static void	push();

char *abspath(p)
char *p;
{
int state;
int slashes;
char *stktop;
char *slash="/";
char *inptr;
char c;

	state = 0;
	stktop = inptr = p;
	while (c = *inptr)
		{
		 switch (state)
			{
			 case 0: if (c=='/') state = 1;
				 push(&inptr,&stktop);
				 break;
			 case 1: if (c=='.') state = 2;
					else state = 0;
				 push(&inptr,&stktop);
				 break;
			 case 2:      if (c=='.') state = 3;
				 else if (c=='/') state = 5;
				 else             state = 0;
				 push(&inptr,&stktop);
				 break;
			 case 3: if (c=='/') state = 4;
					else state = 0;
				 push(&inptr,&stktop);
				 break;
			 case 4: for (slashes = 0; slashes < 3; )
					{
					 if(pop(&stktop)=='/') ++slashes;
					 if (stktop < p) return((char *) -1);
					}
				 push(&slash,&stktop);
				 slash--;
				 state = 1;
				 break;
			 case 5: pop(&stktop);
				 if (stktop < p) return((char *) -1);
				 pop(&stktop);
				 if (stktop < p) return((char *) -1);
				 state = 1;
				 break;
			}
		}
	*stktop='\0';
	return(p);
}

static void
push(chrptr,stktop)

char **chrptr;
char **stktop;

{
	**stktop = **chrptr;
	(*stktop)++;
	(*chrptr)++;
}

static char
pop(stktop)

char **stktop;

{
char chr;
	(*stktop)--;
	chr = **stktop;
	return(chr);
}	
