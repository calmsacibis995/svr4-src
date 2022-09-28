/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libld:common/ldlread.c	1.7"
#include	<stdio.h>
#include	"filehdr.h"
#include	"scnhdr.h"
#include	"linenum.h"
#include	"syms.h"
#include	"synsyms.h"
#include	"ldfcn.h"

static long		lnnoptr = 0L;
static unsigned short	maxlnnos = 0;
static LDFILE		*saveldptr = NULL;


int
ldlread(ldptr, fcnindx, linenum, linent)

LDFILE		*ldptr;
long		fcnindx;
unsigned short	linenum;
LINENO		*linent;

{


    if (ldlinit(ldptr, fcnindx) == SUCCESS) {
	return(ldlitem(ldptr, linenum, linent));
    }

    return(FAILURE);

}




int
ldlinit(ldptr, fcnindx)

LDFILE		*ldptr;
long		fcnindx;

{


    SCNHDR		secthead;
    SYMENT		symbol;
    AUXENT		aux;
    LINENO		line;

    long		endlnptr;


    saveldptr = ldptr;

    if (ldtbread(ldptr, fcnindx, &symbol) == SUCCESS) {
      if (ISFCN(symbol.n_type) && (symbol.n_numaux == 1)) {
	if (FREAD((char *)&aux, AUXESZ, 1, ldptr) == 1) {

	  if (ldshread(ldptr, (unsigned short)symbol.n_scnum, &secthead) == SUCCESS) {

	    if ((lnnoptr = aux.x_sym.x_fcnary.x_fcn.x_lnnoptr) != 0L) {

	      endlnptr = secthead.s_lnnoptr + (long) (secthead.s_nlnno * LINESZ);
	      if ((secthead.s_lnnoptr<=lnnoptr)&&(lnnoptr + LINESZ<=endlnptr)) {

		/* lnnoptr should be greater or equal to s_lnnoptr
		 * and less than endlnnoptr by at least as much as LINESZ
		 */

		maxlnnos = (unsigned short) (((endlnptr - lnnoptr)/LINESZ) - 1);

		if (FSEEK(ldptr, lnnoptr, BEGINNING) == OKFSEEK) {
		  if (FREAD((char *)&line, LINESZ, 1, ldptr) == 1) {
		    if ((line.l_lnno == 0)&&(line.l_addr.l_symndx == fcnindx)) {
		      return(SUCCESS);
		    }
		  }
		}
	      }
	    } else {
	      if (FSEEK(ldptr, secthead.s_lnnoptr, BEGINNING) == OKFSEEK) {

		for (maxlnnos = secthead.s_nlnno; maxlnnos != 0; --maxlnnos) {
		  if (FREAD((char *)&line, LINESZ, 1, ldptr) != 1) {
		    lnnoptr = 0L;
		    maxlnnos = 0;
		    saveldptr = NULL;
		    return(FAILURE);
		  }

		  if ((line.l_lnno == 0) && (line.l_addr.l_symndx == fcnindx)) {
		    lnnoptr = secthead.s_lnnoptr + (long) ((secthead.s_nlnno -
			      maxlnnos) * LINESZ);
		    --maxlnnos;
		    return(SUCCESS);
		  }
		}
	      }
	    }
	  }
	}
      }
    }

    lnnoptr = 0L;
    maxlnnos = 0;
    saveldptr = NULL;
    return(FAILURE);

}




int
ldlitem(ldptr, linenum, linent)

LDFILE		*ldptr;
unsigned short	linenum;
LINENO		*linent;

{

    LINENO		line;
    int			lflag;
    unsigned short	i;


  lflag = FAILURE;

  if ((ldptr == saveldptr) && (lnnoptr != 0)) {

    if (FSEEK(ldptr, lnnoptr, BEGINNING) == OKFSEEK) {

      if (FREAD((char *)&line, LINESZ, 1, ldptr) == 1) {
	if (line.l_lnno == linenum) {
	  linent->l_lnno = line.l_lnno;
	  linent->l_addr.l_paddr = line.l_addr.l_paddr;
	  return(SUCCESS);
	}

	for (i = maxlnnos; i != 0; --i) {
	  if (FREAD((char *)&line, LINESZ, 1, ldptr) != 1) {
	    return(FAILURE);
	  }
	  if (line.l_lnno == 0) {
	    return(lflag);
	  } else if (line.l_lnno == linenum) {
	    linent->l_lnno = line.l_lnno;
	    linent->l_addr.l_paddr = line.l_addr.l_paddr;
	    return(SUCCESS);
	  } else if (line.l_lnno > linenum) {
	    if ((lflag == FAILURE) || (linent->l_lnno > line.l_lnno)) {
	      lflag = SUCCESS;
	      linent->l_lnno = line.l_lnno;
	      linent->l_addr.l_paddr = line.l_addr.l_paddr;
	    }
	  }
	}
      }
    }
  }

  return(lflag);
}

