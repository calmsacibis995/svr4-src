/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)optim:i386/regal.h	1.4"

#ifdef IMPREGAL
/* variable storage classes .. see storclass.h */
#define SCLNULL 0
#define AUTO 1
#define REGISTER 4
#define PARAM 9
/* make up a new storage class just for variables that "look
   like globals in the .s */
#define GLOBAL 200
#endif


enum valid_types {
	ONLY_BYTE,
	ALL_TYPES,
	NO_TYPES
} ;

struct regal 
{
  char	*rglname;	/* name of quantity - auto, param or global */
  int 	rglestim;		/* estimator of cycle payoff */
  int	rglscl;			/* scl of quant to put in reg */
  int	rgllen;			/* length in bytes of quantity */
  enum 	scratch_status {
      unk_scratch, /* haven't done ld analysis for this yet */
      no_scratch,  /* can't put this regal into scratch */
      ok_scratch
  } rgl_scratch_use;
  struct regal *rgl_hash_next;	/* next regal in hash chain */
  struct regal *rglnext;	/* next regal in linked list */
  enum valid_types rgl_instr_type;
};


struct assign {
	char *asrname[3];	/* register name */
	int asrfld;	/* bit location of the register */
	unsigned asrregn;	/* register number */
	int asrtype;	/* register type */
	struct assign *	h_reg;	/* pointer to overlapping "companion" reg */
	struct assign * l_reg;	/* pointer to second overlapping 
				   "companion" reg */
	int asavail;	/* AVAIL, NOTAVAIL */
	struct regal * assigned_regal; /* points to a regal assigned
			  by the register allocator, null if AVAIL or
			  assigned by compiler or a companion register
			  of an assigned register */
};

#define reg_eax 0
#define reg_ecx 1
#define reg_edx 2
#define reg_ebx 3
#define reg_esp 4
#define reg_ebp 5
#define reg_esi 6
#define reg_edi 7

#define LO (0)
#define HI (0)

#define reg_al reg_eax+LO
#define reg_bl reg_ebx+LO
#define reg_cl reg_ecx+LO
#define reg_dl reg_edx+LO

#define reg_ah reg_eax+HI
#define reg_bh reg_ebx+HI
#define reg_ch reg_ecx+HI
#define reg_dh reg_edx+HI
/* This should prolly be in some sgs header, since the compiler,
   debugger, and optimizer have to agree on it. */
