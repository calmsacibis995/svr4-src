/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1989, 1990 Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */

#ifndef lint
static char bps_copyright[] = "Copyright 1989, 1990 Intel Corporation 464725";
#endif /* lint */

#ident	"@(#)mbus:uts/i386/io/bps.c	1.1.1.1"


/*
 *  Multibus II Bootstrap Parameter String Driver
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/errno.h"
#include "sys/cmn_err.h"
#include "sys/bootinfo.h"

#ifdef V_3
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/immu.h"
#include "sys/sysmacros.h"
#else
#include "sys/cred.h"
#include "sys/ddi.h"
#include "sys/kmem.h"
#endif

#include "sys/bps.h"

#ifndef PRIVATE
#define PRIVATE static
#endif

#define    SPL        spl7           /* for BPS driver */

#define	A_DOLLAR		0x24
#define	A_ASTERISK		0x2a
#define	A_COMMA			0x2c
#define	A_HYPHEN		0x2d
#define	A_COLON			0x3a
#define	A_SEMICOLON		0x3b
#define	A_EQUAL			0x3d
#define	A_QUESTION		0x3f
#define bpsisdigit(c)	(((c) >= '0' && (c) <= '9') ? 1 : 0)
#define bpsisupper(c)	(((c) >= 'A' && (c) <= 'Z') ? 1 : 0)
#define bpsislower(c)	(((c) >= 'a' && (c) <= 'z') ? 1 : 0)

#ifdef V_3
#define	SETERR(x)		u.u_error = x
#define	ERRRET(x)		u.u_error = x;\
						return
#define MEMALLOC(a)		sptalloc(btoc(a), PG_RW|PG_P, 0L, ~NOSLEEP)
#define MEMFREE(x,y)	sptfree(x, btoc(y), 1)
#define PHYSTOKV(addr,size,flags)	phystokv(addr)
#else
#define	SETERR(x)		e_code = x
#define	ERRRET(x)		return(x)
#define MEMALLOC(a)		kmem_zalloc(a, KM_NOSLEEP)
#define MEMFREE(x,y) 	kmem_free(x, y)
#define PHYSTOKV(addr,size,flags)	physmap(addr,size,flags)
#endif

		struct		bps_p_itbl	*bps_p_tbl;
		struct		bps			*bps;
		char					*bps_int_store; 
		int						bps_p_tbl_size; 
		int						bps_int_size; 
		int						bps_int_count; 
PRIVATE int						init_in_progress = 0; 


/* external variables which are configured in */

extern		char		bps_tokenized_value;
extern		int			bps_use_native;
extern		int			bps_testing;
extern		int			bps_ram_addr;
extern		long		bps_max_size;

/* globals */

int bpsdevflag = 0;
PRIVATE	short bps_initted = 0;

#ifdef DEBUG
#define DEB_INIT     0x00000001    /* bpsinit */
#define DEB_START    0x00000002    /* bpsstart */
#define DEB_IOCTL    0x00000004    /* bpsioctl */
#define DEB_GETPV    0x00000008    /* bps_get_val */
#define DEB_GETWC    0x00000010    /* bps_get_wcval */
#define DEB_GETOPT   0x00000020    /* bps_get_opt */
#define DEB_GETINT   0x00000040    /* bps_get_integer */
#define DEB_GETRNG   0x00000080    /* bps_get_range */
#define DEB_GETSOK   0x00000100    /* bps_get_socket */
#define DEB_CRTTBL   0x00001000    /* bps_create_tbl */
#define DEB_RETKEY   0x00002000    /* bps_retrieve_key */
#define DEB_DETAIL   0x80000000    /* for details during debugging */

unsigned long bps_debug = 0;

#define DEBPR(x,y)    if(bps_debug & (x)) cmn_err y

#else

#define DEBPR(x,y)

#endif /* DEBUG */

/* LINTLIBRARY */
/*
 * Compare num bytes in strings
 */
PRIVATE int
bps_strncmp(string1, string2, num)
char		*string1;
char		*string2;
uint		num;
{
	num++;
	if( string1 == string2 )
		return(0);
	while( (--num > 0) && (*string1 == *string2++) )
		if( *string1++ == '\0' )
			return(0);
	if( num == 0 )
		return( 0 );
	else
		return( *string1 - *--string2 );
}

/*
 * Get length of string
 */
PRIVATE int
bps_strlen(string)
char *string;
{
	char	*string0 = string + 1;

	while( *string++ != '\0' )
		;

	return( string - string0 );
}
/*
 * Check if character is alpha-numeric
 */
PRIVATE int
bps_isalnum(c)	
unsigned char c;
{
	if (bpsisupper(c) || bpsislower(c) || bpsisdigit(c))
		return(1);
	else
		return(0);
}
/*
 * Check if character is  digit (0-9, a-f, or A-F)
 */
PRIVATE unsigned char
bps_DIGIT(x)		
unsigned char x;
{
	if (bpsisdigit(x))
		return(x - '0');
	else {
		if (bpsislower(x))
			return(x + 10 - 'a'); 
		else
			return(x + 10 - 'A');
	}
}
/*
 * Copy string string2 to string1.
 */
PRIVATE char *
bps_strcpy(string1, string2)
register char	*string1,
		*string2;
{
	register char	*str;

	str = string1;
	while( *string1++ = *string2++ )
		;
	return(str);
}
/*
 * Copy s2 to s1, truncating or null-padding to always copy n bytes
 * return s1
 */

PRIVATE char *
bps_strncpy(s1, s2, n)
register char *s1, *s2;
register size_t n;
{
	register char *os1 = s1;

	n++;				
	while ((--n > 0) &&  ((*s1++ = *s2++) != '\0'))
		;
	if (n > 0)
		while (--n > 0)
			*s1++ = '\0';
	return (os1);
}

/*
 * Return the ptr in sp at which the character c appears;
 * NULL if not found
 */

PRIVATE char *
bps_strchr(sp, c)
register char *sp, c;
{
	do {
		if(*sp == c)
			return(sp);
	} while(*sp++);
	return(NULL);
}

/*
 * our very own convert to lower case routine
 */
PRIVATE int
bps_tolower(c)
register 	int	c;
{
	if (c >= 'A' && c <= 'Z')
		c -= 'A' - 'a';
	return(c);
}

/*
 * Case-less string compare
 */
PRIVATE int
bps_ncstrncmp(src_p, target_p, len)	
char 	*src_p;
char	*target_p;
int		len;
{
	int		k;
	/* 
	 * Would have been nice to have a case-less string compare.  Until 
	 * then, force source, target to lower case and compare  one
	 * character at a time.
	 */
	for (k = 0; k < len ; ++k) {
		if ((char) bps_tolower(*src_p++) != (char) bps_tolower(*target_p++))
			return(0);
	}
	return(k);
}

PRIVATE long
bps_strtol(istr, ptr, base)
register char *istr;
char **ptr;
register int base;
{
	register long val;
	register char *str;
	unsigned char c;
	unsigned char xx, neg = 0;

	str = istr;

	DEBPR(DEB_GETINT,(CE_CONT, "bps_strtol: str %s, base %d\n", str, base)); 

	if (ptr != (char **)0)
		*ptr = str; /* in case no number is formed */
	if (base < 0 || base > 16)
		return (0); /* base is invalid -- should be a fatal error */
	c = *str;
	if (!bps_isalnum(c)) {
		while (c == ' ' || c == '\t' || c == '\n')
			c = *++str;
		switch (c) {
		case '-':
			neg++;
		case '+': /* fall-through */
			c = *++str;
		}
	}
	if (base == 0) {
		if (c != '0') {
			base = 10;
		}
		else {
			if (str[1] == 'x' || str[1] == 'X')
				base = 16;
			else
				base = 8;
		}
	}
	if ((base == 16) && (c == '0') && 
	    (str[1] == 'x' || str[1] == 'X'))
		c = *(str += 2); /* skip over leading "0x" or "0X" */
	for (val = -bps_DIGIT(c); 
		 bps_isalnum((c = *++str)) && ((xx = bps_DIGIT(c)) < (unsigned char) base); ) {
		/* accumulate neg avoids surprises near MAXLONG */
		val = base * val - xx;
	}
	if (ptr != (char **)0)
		*ptr = str;

	DEBPR(DEB_GETINT,(CE_CONT, "bps_strtol: val %d, neg %d\n", val, neg)); 

	return (neg ? val : -val);
}
/*
 * Initializes the internal tables for ease of searching. Uses either a
 * null-terminated string or the BPS left in RAM by the f/w BPS manager.
 * 
 * Will require additional work to support parameter additions, parameter
 * value modifications.
 */
PRIVATE	int
bps_create_tbl(bp_string)
char		*bp_string;
{
	struct	bps				*p; 
	struct	bps_param_hdr	*i; 
	struct	bps_p_itbl		*b_tbl;
 	int						j, l, len, count, x = 0;
	char					*temp;
	char					*c;
	char					*t_name;
	char					*save_area;
	unsigned char			ch;
	
	DEBPR(DEB_CRTTBL,(CE_CONT, "bps_create_tbl: bp_string %x\n", bp_string));
	x = SPL();
	init_in_progress++;
	splx(x);

	count = 0;
	/* 
	 * first compute the size of table needed.  We count the number 
	 * of '=' to figure that and add one for end of table.
	 */
	if (bps_use_native) {
		/*
		 * Here we examine the save area of the f/w BPS Manager 
		 */
		p = (struct bps *) bp_string;
		j = (int) p->bps_save_hdr.total;
		c = (char *) &p->parameters;
		DEBPR(DEB_CRTTBL,(CE_CONT, 
			"bps_create_tbl: total : %x, c : %x, terminator %x\n", 
				j, c, p->bps_save_hdr.terminator));
		while (j--) {
			if (*c == '=')
				count++;
			c++;
		}
	}
	else {
		c = bp_string;
		temp = bps_strchr(c, A_EQUAL);
		while (temp != NULL) {
			count++;
			c = ++temp;
			temp = bps_strchr(c, A_EQUAL);
		}
	}

	count++;
	count = count * sizeof(struct bps_p_itbl);
	if (bps_p_tbl_size)
		MEMFREE(bps_p_tbl, bps_p_tbl_size);

	bps_p_tbl_size = 0;	
	bps_p_tbl = (struct bps_p_itbl *) MEMALLOC(count);
	if (bps_p_tbl == NULL) {
		init_in_progress--;
		return(ENOMEM);
	}
	bps_p_tbl_size = count;	
	if (bps_use_native) {
		p = (struct bps *) bp_string;
		if (bps_int_size)
			MEMFREE(bps_int_store, bps_int_size);

		bps_int_size = (int) p->bps_save_hdr.total;
		bps_int_store = (char *) MEMALLOC(bps_int_size);

		if (bps_int_store == NULL) {
			MEMFREE(bps_p_tbl, bps_p_tbl_size);
			bps_int_size = 0;
			init_in_progress--;
			return(ENOMEM);
		}
		save_area = bps_int_store;

		DEBPR(DEB_CRTTBL|DEB_DETAIL,(CE_CONT, 
			"bps_create_tbl: bps_p_tbl %x, bps_int_store %x, size %d\n", 
			bps_p_tbl, bps_int_store, p->bps_save_hdr.total));
		
		b_tbl = bps_p_tbl;
		for (j=0, i = &p->bps_param_hdr; (char) i->total != NULL;j++) {

			b_tbl->param_value =  (char *)i + i->value_offset;
			b_tbl->param_name = (char *)&i->name_length;

			/* in computing the length we skip past the '=' sign */

			b_tbl->param_val_len = (int) i->total - 
				((int) i->name_length + sizeof(struct bps_param_hdr) + 2);
	
			/* copy the parameter name and null terminate */

			b_tbl->int_bps_param = save_area;
			t_name = b_tbl->param_name; 
			len = (unsigned char) *t_name;
			t_name++;
			for (l = 0; l < len; l++) {
				*save_area++ = *t_name++;
			}
			*save_area++ = 0;

			/* copy the parameter value and null terminate */
	
			b_tbl->int_bps_value = save_area;
			t_name = b_tbl->param_value;
			len = b_tbl->param_val_len;

			if (bps_tokenized_value) {
				for (l = 0; l < len;) {
					ch = *t_name++;
					l += ch;
					l++;		 /*  account for the counting character */
					while (ch--) 
						*save_area++ = *t_name++;
				}
			}
			else {
				for (l = 0; l <= len; l++) {
					*save_area++ = *t_name++;
				}
			}
			*save_area++ = 0;

			DEBPR(DEB_CRTTBL|DEB_DETAIL,(CE_CONT, 
				"bps_create_tbl: name %s, value %s, j %d\n", 
				b_tbl->int_bps_param, b_tbl->int_bps_value, j)); 

			temp = (char *)i + i->total;
			i = (struct bps_param_hdr *)temp;
			b_tbl++;
		}
		bps_int_count = j;
	}
	else {
		/* 
		 * use the parameter bp_string as the BPS, assume it is 
		 * syntactically correct
		 */
		len = bps_strlen((char *)bp_string);
		DEBPR(DEB_CRTTBL|DEB_DETAIL,(CE_CONT, 
			"bps_create_tbl: bp_string %s\n", bp_string));

		if (bps_int_size)
			MEMFREE(bps_int_store, bps_int_size);
		bps_int_size = len + 1;
		temp = (char *)bp_string;
		bps_int_store = (char *) MEMALLOC(bps_int_size); 

		if (bps_int_store == NULL) {
			MEMFREE(bps_p_tbl, bps_p_tbl_size);
			bps_int_size = 0;
			init_in_progress--;
			return(ENOMEM);
		}

		DEBPR(DEB_CRTTBL|DEB_DETAIL,(CE_CONT, 
			"bps_create_tbl: bps_p_tbl %x, bps_int_store %x, size %d\n", 
			bps_p_tbl, bps_int_store, bps_int_size));

		/* copy the BPS */

		c = bps_int_store;
		bps_strcpy(c, temp);
		c = bps_int_store;
		temp = c;
		b_tbl = bps_p_tbl;
		for (j = 0, l = 0; (l < len) && (temp != NULL); j++) {
			b_tbl->param_name = c; 
			b_tbl->int_bps_param = c;
			temp = bps_strchr(c, A_EQUAL);
			if (temp == NULL) {
				MEMFREE(bps_p_tbl, bps_p_tbl_size);
				MEMFREE(bps_int_store, bps_int_size);
				init_in_progress--;
				return(EINVAL);
			}
			c = temp;
			*c++ = 0;
			l += bps_strlen(b_tbl->param_name);
			b_tbl->param_value = c;
			b_tbl->int_bps_value = c;
			temp = bps_strchr(c, A_SEMICOLON);
			if (temp != NULL)  {
				c = temp;
				*c++ = 0;
			}
			if (*c == 0)			/* are we at the end now !! */
				temp = NULL;

			b_tbl->param_val_len = (int) bps_strlen(b_tbl->param_value);
			l += (int) b_tbl->param_val_len;

			DEBPR(DEB_CRTTBL|DEB_DETAIL,(CE_CONT, 
				"bps_create_tbl: name %s, value %s, j %d\n", 
				b_tbl->int_bps_param, b_tbl->int_bps_value, j)); 

			b_tbl++;
		}
		bps_int_count = j;
	}

	/* now null terminate the table */
	
	b_tbl->param_value = 0; 
	b_tbl->param_name = 0;
	b_tbl->param_val_len = 0;
	b_tbl->int_bps_param = 0;
	b_tbl->int_bps_value = 0;
	init_in_progress--;
	return(0);
}
/*
 * Retrieve value of parameter from BPS
 */
PRIVATE int
bps_retrieve_key(string_p, len, state_p, vbuf_len, valbuf_p)
char 			*string_p;
int				len;
int 			*state_p;
int				vbuf_len;		/* size of valbuf */
char 			*valbuf_p;
{
	struct	bps_p_itbl		*b_tbl;
	int		j, k;
	int		count = 0; 
	char 	done;
	char 	*src;
	char 	*target;
	char 	*temp;
	
	if (state_p != NULL)
		count = *state_p;
	b_tbl = bps_p_tbl;
	for (done = 0, j = 0; (j < bps_int_count) && !done; j++, b_tbl++) {
		src = b_tbl->int_bps_param;
		target = string_p;
		DEBPR(DEB_RETKEY,(CE_CONT, 
			"bps_retreive_key: source %s, target %s, len %d, count %d\n", 
				src, target, len, count));
		k = bps_ncstrncmp(src, target, len);	
		if (k == len) {

			DEBPR(DEB_RETKEY,(CE_CONT, 
				"bps_retreive_key: string matches; wc %c\n", *(target+len)));

			/* first check if we are doing normal comparison */

			if ((*(target+len) == 0) &&
				(bps_strlen(src) != bps_strlen(target))) 
					continue;
			else {
				/* 
				 * If we are doing a wild card compare with '?', ensure
				 * that target string length does not have more characters 
				 */ 
				if ((*(target+len) == A_QUESTION) &&
					(bps_strlen(src) != (len+1))) {
					continue;
				}
			}

			if (count == 0) {
				done++;
				break;
			}
			else 
				count--;
		}
	}
	if (done) {
		if (bps_strlen(b_tbl->int_bps_value) < vbuf_len) {
			temp = b_tbl->int_bps_value;
			if  (*temp == A_DOLLAR) {
				DEBPR(DEB_RETKEY,(CE_CONT, 
				"bps_retreive_key: $ substitution %s\n", temp));
				len = bps_strlen(++temp);
				return(bps_retrieve_key(temp, len, NULL, 
						vbuf_len, valbuf_p));
			}
			bps_strcpy(valbuf_p, b_tbl->int_bps_value);
			DEBPR(DEB_RETKEY,(CE_CONT, 
				"bps_retreive_key: string matches, value: %s\n", valbuf_p));
			return(0);
		}
		DEBPR(DEB_RETKEY,(CE_CONT, 
			"bps_retreive_key: insufficient room, vbuf_len: %x\n", 
				vbuf_len));
		return(ENOMEM);
	}
	DEBPR(DEB_RETKEY,(CE_CONT, 
		"bps_retreive_key: no string match for %s\n", string_p));
	return(ENODATA);	
}

/*
 * This routine frees up memory allocated previously for satisfying 
 * ioctl requests.
 */
PRIVATE void
bps_free_mem(bio_out)
struct	bps_ioctl	*bio_out;
{
	if (bio_out->string_p != NULL)
		MEMFREE(bio_out->string_p, bio_out->str_len);
	if (bio_out->valbuf_p != NULL)
		MEMFREE(bio_out->valbuf_p, bio_out->valbuf_len);
	if (bio_out->value_p != NULL)
		MEMFREE(bio_out->value_p, bio_out->value_len);
}

/*
 * Provide copyout to user space after successful completion
 * of ioctl requests.
 */
PRIVATE int
bps_copy_out(cmd, bio_out, bio_in)
int 				cmd;
struct	bps_ioctl	*bio_out;
struct	bps_ioctl	*bio_in;
{
	int		e_code = 0;
	switch (cmd) {
		case BPSGETPV:
			e_code = copyout(bio_out->valbuf_p, bio_in->valbuf_p, 
					bio_in->valbuf_len);
			break;
		case BPSGETWCPV:
			e_code = copyout(bio_out->valbuf_p, bio_in->valbuf_p, 
					bio_in->valbuf_len);
			e_code |= copyout(bio_out->state_p, bio_in->state_p, 
					sizeof(bio_in->state_p));
			break;
		case BPSGETOPTS:
			e_code = copyout(bio_out->value_p, bio_in->value_p, 
					bio_in->value_len);
			e_code |= copyout(bio_out->state_p, bio_in->state_p, 
					sizeof(bio_in->state_p));
			e_code |= copyout(bio_out->config_code, bio_in->config_code, 
					sizeof(bio_in->config_code));
			break;
		case BPSGETINTEGER:
			e_code = copyout(bio_out->lo_return_p, bio_in->lo_return_p, 
					sizeof(bio_in->lo_return_p));
			break;
		case BPSGETSOCKET:
		case BPSGETRANGE:
			e_code = copyout(bio_out->lo_return_p, bio_in->lo_return_p, 
					sizeof(bio_in->lo_return_p));
			e_code |= copyout(bio_out->hi_return_p, bio_in->hi_return_p, 
					sizeof(bio_in->hi_return_p));
			break;
		default:
			e_code = EINVAL;
	}
	return(e_code);
}

/*
 * Handle memory requests to kernel based on the user requests sent
 * through the ioctl interface.
 */
PRIVATE int
bps_get_mem(cmd, bio_in, bio_out)
int 				cmd;
struct	bps_ioctl	*bio_in;
struct	bps_ioctl	*bio_out;
{
	int		e_code = 0;
	
	bio_out->str_len = bio_in->str_len;
	bio_out->valbuf_len = bio_in->valbuf_len;
	bio_out->value_len = bio_in->value_len;
	bio_out->string_p = NULL;
	bio_out->valbuf_p = NULL;
	bio_out->value_p = NULL;

	switch (cmd) {
		case BPSGETPV:
			if (bio_out->str_len != 0) {
				bio_out->string_p = (char *) MEMALLOC(bio_out->str_len);
				if (bio_out->string_p == NULL) {
					DEBPR(DEB_IOCTL,(CE_CONT, 
						"bps_get_mem: failed to get memory %x\n",
						bio_out->str_len));
					e_code = ENOMEM; 
					break;
				}
				if (copyin(bio_in->string_p, bio_out->string_p, 
							bio_in->str_len)) {
					DEBPR(DEB_IOCTL,(CE_CONT, 
						"bps_get_mem: failed to copy in parameters\n"));
					MEMFREE(bio_out->string_p, bio_out->str_len);
					e_code = EFAULT;
					break;
				}
			}
			if (bio_out->valbuf_len != 0) {
				bio_out->valbuf_p = (char *) MEMALLOC(bio_out->valbuf_len);
				if (bio_out->valbuf_p == NULL) {
					e_code = ENOMEM; 
					break;
				}
			}	
			break;
		case BPSGETWCPV:
			if (bio_out->str_len != 0) {
				bio_out->string_p = (char *) MEMALLOC(bio_out->str_len);
				if (bio_out->string_p == NULL) {
					e_code = ENOMEM; 
					break;
				}
				if (copyin(bio_in->string_p, bio_out->string_p, 
							bio_in->str_len)) {
					MEMFREE(bio_out->string_p, bio_out->str_len);
					e_code = EFAULT;
					break;
				}
				if (copyin(bio_in->state_p, bio_out->state_p, 
						sizeof(bio_out->state_p))) 
					e_code = EFAULT;
			}
			if (bio_out->valbuf_len != 0) {
				bio_out->valbuf_p = (char *) MEMALLOC(bio_out->valbuf_len);
				if (bio_out->valbuf_p == NULL) {
					e_code = ENOMEM; 
					break;
				}
			}	
			break;
		case BPSGETOPTS:
			if (bio_out->valbuf_len != 0) {
				bio_out->valbuf_p = (char *) MEMALLOC(bio_out->valbuf_len);
				if (bio_out->valbuf_p == NULL) {
					e_code = ENOMEM; 
					break;
				}
				if (copyin(bio_in->valbuf_p, bio_out->valbuf_p, 
							bio_in->valbuf_len)) {
					MEMFREE(bio_out->valbuf_p, bio_out->valbuf_len);
					e_code = EFAULT;
					break;
				}
			}
			if (copyin(bio_in->state_p, bio_out->state_p, 
				sizeof(bio_out->state_p))) { 
				e_code = EFAULT;
				break;
			}
			if (copyin(bio_in->config_code, bio_out->config_code, 
				sizeof(bio_out->config_code))) { 
				e_code = EFAULT;
				break;
			}
			if (bio_out->str_len != 0) {
				bio_out->string_p = (char *) MEMALLOC(bio_out->str_len);
				if (bio_out->string_p == NULL) {
					e_code = ENOMEM; 
					break;
				}
				if (copyin(bio_in->string_p, bio_out->string_p, 
							bio_in->str_len)) {
					MEMFREE(bio_out->string_p, bio_out->str_len);
					e_code = EFAULT;
				}
			}
			if (bio_out->value_len != 0) {
				bio_out->value_p = (char *) MEMALLOC(bio_out->value_len);
				if (bio_out->value_p == NULL) 
					e_code = ENOMEM; 
			}
			break;
		case BPSGETINTEGER:
		case BPSGETSOCKET:
		case BPSGETRANGE:
			if (bio_out->value_len != 0) {
				bio_out->value_p = (char *) MEMALLOC(bio_out->value_len);
				if (bio_out->value_p == NULL) {
					e_code = ENOMEM; 
					break;
				}
				if (copyin(bio_in->value_p, bio_out->value_p, 
							bio_in->value_len)) {
					MEMFREE(bio_out->value_p, bio_out->value_len);
					e_code = EFAULT;
				}
			}
			break;
	}
	return(e_code);
}

/*
 *	BPS driver initialization.  Uses the BPS address stashed away in bpsinfo
 *	by the second stage bootloader and sets the initialization
 *	flag.
 */
int
bpsinit()
{	
	int		x, e_code = 0;

	DEBPR(DEB_INIT,(CE_CONT, 
		"bpsinit: bps_use_native %d, bps %x, bps_ram_addr %x\n",
			bps_use_native, bps, bps_ram_addr));

	if (bps_initted)		/* if init has been completed ok */
		return(0);

	x = SPL();
	bps_p_tbl_size = 0;
	bps_int_size = 0;	
	bps_initted = 0;
	splx(x);

	if (bps_use_native) {
#ifdef V_3
		if (bps_testing)
			bps = (struct bps *) phystokv(bps_ram_addr);
		else			
			bps = (struct bps *) phystokv(bootinfo.bpsloc);
#else
		if (bps_testing)
			bps = (struct bps *) physmap(bps_ram_addr, bps_max_size, KM_NOSLEEP);
		else			
			bps = (struct bps *) physmap(bootinfo.bpsloc, bps_max_size, KM_NOSLEEP);
#endif
		e_code = bps_create_tbl(bps);
	}
	else
		e_code =  bps_create_tbl(bpsinfo); 

	if (e_code == 0) 
		bps_initted++;

	return(e_code);
}

/* ARGSUSED */
#ifdef V_3
bpsopen( dev, flag, otyp )
dev_t	dev;
int		flag;
int		otyp;
#else
bpsopen (dev, flag, otyp, cred_p)
dev_t 	*dev;
int 	flag; 
int		otyp;
struct	cred *cred_p;
#endif
{
	return (0);
}

/* ARGSUSED */
#ifdef V_3
bpsclose(dev, flag, otyp, offset)
dev_t	dev;
int		flag;
int		otyp;
off_t	offset;
#else
bpsclose (dev, flag, otyp, cred_p)
dev_t 	*dev;
int 	flag;
int		otyp;
struct 	cred *cred_p;
#endif
{
	return (0);
}

/* ARGSUSED */
#ifdef V_3
bpsioctl(dev, cmd, addr, flag)
dev_t   dev;
int     cmd;		/* command code */
caddr_t addr;		/* user structure with parameters */
int     flag;		/* not used */
#else
bpsioctl(dev, cmd, addr, mode, cred_p, rval_p)
dev_t 	dev;
int 	cmd;
caddr_t addr;
int 	mode;
struct	cred *cred_p;
int		*rval_p;
#endif
{
	int 				x, save, e_code;
	struct	bps_ioctl	bio_in;
	struct	bps_ioctl	bio_out;
	struct	bps_ioctl	*bioctl;

	int					state, config, lo_return, hi_return = 0;

	bio_out.state_p = &state;
	bio_out.config_code = &config;
	bio_out.lo_return_p = &lo_return;
	bio_out.hi_return_p = &hi_return;
	bio_out.status = 0;

	DEBPR(DEB_IOCTL,(CE_CONT, "bpsioctl: entered cmd=%d, addr=%x\n",cmd, addr));

	if (addr != NULL) {
		if(copyin(addr, &bio_in, sizeof(struct bps_ioctl))) {
			DEBPR(DEB_IOCTL,(CE_CONT, "bpsioctl: failed on copyin\n"));
			ERRRET(EFAULT);
		}
		DEBPR(DEB_IOCTL,(CE_CONT, "bpsioctl: slen %d, vblen %d, vlen %d\n",
				bio_in.str_len, bio_in.valbuf_len, bio_in.value_len));
		
	}
	/*
	 * special case for BPSINIT, as the bps may not have been initialized !!
	 */
	if (cmd == BPSINIT) {
#ifdef V_3
		if (!suser())
#else
		if (drv_priv(cred_p) != 0)
#endif
									{
			ERRRET(EPERM);		
		}

		x = SPL();
		if ((init_in_progress != 0) || !bps_testing) {
			splx(x);
			ERRRET(EBUSY);
		}
		/* 
		 * by specifying addr to NULL we give the caller an option to 
		 * re-initialize the bps to the BPS left in RAM after completion
		 * of second-stage bootstrap 
		 */
		if (addr == NULL) {
			bps_use_native++;	
			bps_initted = 0;
			splx(x);
			e_code = bps_create_tbl(bps);
			bps_use_native--;	
		}
		else {
			if (bio_in.str_len == 0) {
				splx(x);
				ERRRET(EINVAL);
			}
			bio_out.string_p = (char *) MEMALLOC(bio_in.str_len);
			if (bio_out.string_p == NULL)  {
				splx(x);
				ERRRET(ENOMEM);
			}
			if(copyin(bio_in.string_p, bio_out.string_p, bio_in.str_len)) {
				splx(x);
				ERRRET(EFAULT);
			}
			bio_out.str_len = bio_in.str_len;
			bps_initted = 0;
			save = bps_use_native;
			bps_use_native = 0;
			splx(x);
			e_code = bps_create_tbl(bio_out.string_p);
			bps_use_native = save;
			MEMFREE(bio_out.string_p, bio_out.str_len);

			bio_out.status = e_code;
			bioctl = (struct bps_ioctl *) addr; 
			if (copyout(&bio_out.status, &bioctl->status, 
					sizeof(bio_out.status))) {
				ERRRET(EFAULT);
			}
		}
		if (e_code == 0) {
			bps_initted++;
			ERRRET(0);
		}
	}
	if (bps_initted == 0) { 
		ERRRET(ENODEV);
	}
	e_code = bps_get_mem(cmd, &bio_in, &bio_out);
	if (e_code != 0) {
		DEBPR(DEB_IOCTL,(CE_CONT, "bpsioctl: failed on bps_get_mem\n"));
		ERRRET(e_code);
	}

	switch (cmd) {
		case BPSGETPV:
			e_code = bps_get_val(bio_out.string_p, bio_out.valbuf_len, 
						bio_out.valbuf_p);
			break;
		case BPSGETWCPV:
			e_code = bps_get_wcval(bio_out.string_p, bio_out.state_p, 
						bio_out.valbuf_len, bio_out.valbuf_p);
			break;
		case BPSGETOPTS:
			e_code = bps_get_opt(bio_out.valbuf_p, bio_out.state_p, 
						bio_out.string_p, bio_out.config_code, 
						bio_out.value_len, bio_out.value_p);
			break;
		case BPSGETINTEGER:
			e_code = bps_get_integer(bio_out.value_p, bio_out.lo_return_p);
			break;
		case BPSGETSOCKET:
			e_code = bps_get_socket(bio_out.value_p, bio_out.lo_return_p, 
						bio_out.hi_return_p);
			break;
		case BPSGETRANGE:
			e_code = bps_get_range(bio_out.value_p, bio_out.lo_return_p, 
						bio_out.hi_return_p);
			break;
		default:
			ERRRET(EINVAL); 	
	}
	bio_out.status = e_code;
	bioctl = (struct bps_ioctl *) addr; 
	if (copyout(&bio_out.status, &bioctl->status, 
			sizeof(bio_out.status))) {
		DEBPR(DEB_IOCTL,(CE_CONT, 
			"bpsioctl: failed on status copy, e_code %x\n", e_code));
		ERRRET(EFAULT);
	}
	if (e_code == 0) {
		e_code = bps_copy_out(cmd, &bio_out, &bio_in);
		if (e_code) {
			DEBPR(DEB_IOCTL,(CE_CONT, 
			"bpsioctl: failed on copy out, e_code %x\n", e_code));
			ERRRET(e_code);
		}
	}

	bps_free_mem(&bio_out);
	DEBPR(DEB_IOCTL,(CE_CONT, "bpsioctl: done, e_code %x\n", e_code));
	ERRRET(0); 	
}

/*
 * Get wild card value from BPS.  Simplistic wild-carding is supported.
 * For example, ASYNC* or ASYNC?
 */
int
bps_get_wcval(string_p, state_p, vbuf_len, valbuf_p)
char 			*string_p;
int 			*state_p;
int				vbuf_len;		/* size of valbuf */
char 			*valbuf_p;
{
	int				len;
	int				e_code = 0;

	if (bps_initted == 0)
		return(-1);

	len = bps_strlen(string_p);
	if ((len > 1)  &&
		((*(string_p+len-1) == A_QUESTION) ||
		 (*(string_p+len-1) == A_ASTERISK)))  {
		e_code = bps_retrieve_key(string_p, len-1, state_p,
					vbuf_len, valbuf_p);
	
		if (e_code == 0)
			(*state_p)++;		/* set-up for next parameter */
	}
	else 
		e_code = EINVAL;

	return(e_code);
}

/*
 * Get value from BPS.  
 */
int
bps_get_val(string_p, vbuf_len, valbuf_p)
char 			*string_p ;
int				vbuf_len;		/* size of valbuf */
char 			*valbuf_p ;
{
	int		len;

	if (bps_initted == 0)
		return(-1);

	len = bps_strlen(string_p);
	return(bps_retrieve_key(string_p, len, NULL, vbuf_len, valbuf_p));
}

/*
 * Return the next option argument from the BPS value buffer.  
 */
int
bps_get_opt(valbuf_p, state_p, string_p, config_code, value_len, value_p)
char 			*valbuf_p;
int 			*state_p;
char 			*string_p;
int				*config_code;
int				value_len;
char 			*value_p ;
{
	int				len, j, save_state; 
	char  			*save_strp, *save_vbufp, *temp_p;
	char  			found;

	save_vbufp = valbuf_p;
	DEBPR(DEB_GETOPT,(CE_CONT, "bps_get_opt: state %x\n", *state_p));

	/* select the correct value part based on the state variable */

	for (save_state = *state_p; save_state > 0; save_state--) {
		temp_p = bps_strchr(save_vbufp, A_COMMA);	
		if (temp_p != NULL) {
			save_vbufp = temp_p;
			save_vbufp++;		/* skip past the comma */
		}
		else 
			return(-1);
	} 

	save_state = *state_p;
	len = bps_strlen(save_vbufp);
	found = 0; 
	while (!found) {
		save_strp = string_p;
		
		for (*config_code = 0;!found ; ++(*config_code)) {
			temp_p = bps_strchr(save_strp, A_COLON);	
			if (temp_p == NULL)						/* last option ? */
				len = bps_strlen(save_strp);
			else
				len = temp_p - save_strp;				/* length of string */
			if ((bps_ncstrncmp(save_vbufp, save_strp, len) == len) &&
				(*(save_vbufp + len) == A_COLON)) {
				++found;
				DEBPR(DEB_GETOPT,(CE_CONT, 
				"bps_get_opt: found match, config %d\n", *config_code));
			}
			else {
				if (temp_p == NULL) 					/* no more options */
					break;
				++temp_p;
				save_strp = temp_p;
			}
		}
		if (!found) {
			temp_p = bps_strchr(save_vbufp, A_COMMA);	
			if (temp_p == NULL) {
				*config_code = 0;
				*state_p = save_state;
				return(-1);
			}
			save_vbufp = temp_p;
			save_vbufp++;		/* skip past the comma */
			(*state_p)++;
		}
		DEBPR(DEB_GETOPT,(CE_CONT, 
				"bps_get_opt: found %d, state %d, value buf %s\n", 
				found, *state_p, save_vbufp));
	}
	if (found) {
		/* 
		 * note that "len" is the length of the option and account
		 * for the ":" following the name
		 */ 
		save_vbufp = save_vbufp + len + 1;

		/* now copy the value to the proper location */
		temp_p = bps_strchr(save_vbufp, A_COMMA);
		if (temp_p != NULL) 
			j = temp_p - save_vbufp;
		else
			j = bps_strlen(save_vbufp);
		/* XXX resolve $ indirection here */
		if (j < value_len) 
			(void) bps_strncpy(value_p, save_vbufp, j);
		else {
			DEBPR(DEB_GETOPT,(CE_CONT, 
				"bps_get_opt: insufficient room, value_len: %x, j %x\n", 
				value_len, j));
			return(ENOMEM);
		}
		*(value_p + j) = 0;
		(*state_p)++;
		DEBPR(DEB_GETOPT,(CE_CONT, 
				"bps_get_opt: state %d, option value %s\n", 
				*state_p, value_p));
		return(0);
	}
	return(-1);
}

/*
 * Return an Integer from the BPS value buffer.  
 */
int
bps_get_integer(value_p, int_p)
char 			*value_p;
int				*int_p;
{
	int				len;
	int				base = 0;
	char			*save_val_p;
	char			*xsave;
	unsigned char	c, free_mem = 0;
	char			*ret_val;
	
	if ((value_p == NULL) || (*value_p == 0))
		return(-1);

	/* XXX resolve $ indirection here */
	len = bps_strlen(value_p);
	c = *(value_p+len-1);
	DEBPR(DEB_GETINT,(CE_CONT, "bps_get_integer: value_p %s, len %d, c %c\n", 
			value_p, len, c));
	/* 
	 * Check if base is specified as a suffix.  If so, then make a 
	 * copy of the string because we do not want to destroy the original.
	 *
	 * First check if value is of type 0x.. or 0X.. before checking for
	 * radix specified as a suffix
	 */
	if (*(value_p+1) == 'x' || *(value_p+1) == 'X')
		save_val_p = value_p;
	else {
		if (c >= '0' && c <= '9') 
			save_val_p = value_p;
		else {
			xsave = (char *) MEMALLOC(len+1);
			if (xsave == NULL)
				return(ENOMEM);
	
			++free_mem;
			save_val_p = bps_strcpy(xsave, value_p);
			*(save_val_p+len-1) = 0;
			if (*save_val_p == 0) {
				*int_p = 0;
				return(0);
			}
	
			switch (c) {
				case 'H':	
				case 'h':	
					base = 16;
					break;
				case 'T':	
				case 't':	
					base = 10;
					break;
				case 'Q':	
				case 'q':	
				base = 8;
					break;
				case 'B':	
				case 'b':	
				case 'Y':	
				case 'y':	
					base = 2;
					break;
				default:
					base = 0; /* error - let bps_strtol figure it out ?? */
			}
		}
	}
	ret_val = NULL;
	*int_p = bps_strtol(save_val_p, &ret_val, base);

	DEBPR(DEB_GETINT,(CE_CONT, 
		"bps_get_integer: int %d, ret_val %x, save_val_p %x, base %d\n", 
			*int_p, ret_val, save_val_p, base)); 

	if (free_mem)
		MEMFREE(xsave, len+1);

	if ((ret_val == save_val_p) && (*int_p == 0))
		return(-1);
	return(0);
}

/*
 *  Get the range values from value string.
 */
int
bps_get_range(value_p, lo_range_p, hi_range_p)
char 			*value_p ;
int				*lo_range_p;
int				*hi_range_p;
{
	char 		save_value [128];
	char		*temp_p;
	int			e_code;
	
	(void) bps_strcpy(save_value, value_p);
	if (*save_value == A_HYPHEN)			/* check for - sign */
		temp_p = bps_strchr(&save_value[1], A_HYPHEN);
	else
		temp_p = bps_strchr(save_value, A_HYPHEN);

	if (temp_p == NULL) 
		return(-1);
	
	*temp_p = 0;
	temp_p++;
	e_code = bps_get_integer(save_value, lo_range_p);
	if (e_code == 0)
		e_code = bps_get_integer(temp_p, hi_range_p);

	DEBPR(DEB_GETRNG,(CE_CONT, 
		"bps_get_range: value %s, lo_range_p %d, hi_range_p %d, e_code %d\n", 
			value_p, *lo_range_p, *hi_range_p, e_code)); 
	return(e_code);
}

/*
 *  Get the socket value from value string.
 */
int
bps_get_socket(value_p, lo_16bit_p, hi_16bit_p)
char 			*value_p ;
int				*lo_16bit_p;
int				*hi_16bit_p;
{
	char 		save_value [128];
	char		*temp_p;
	int			e_code = 0;
	
	(void) bps_strcpy(save_value, value_p);
	temp_p = bps_strchr(save_value, A_COLON);
	if (temp_p == NULL) 
		return(-1);
	*temp_p = 0;
	temp_p++;
	e_code = bps_get_integer(save_value, hi_16bit_p);
	if (e_code == 0)
		e_code = bps_get_integer(temp_p, lo_16bit_p);
	return(e_code);
}

/*
 *  Kernel/Device Driver Interface to test the BPS
 */
int
bps_init(string_p)
char	*string_p;
{
	int		e_code;
	int		save, x; 	

	x = SPL();
	if ((init_in_progress != 0)  || (!bps_testing)) {
		splx(x);
		return(EBUSY);
	}
	
	/* 
	 * by specifying addr to NULL we give the caller an option to 
	 * re-initialize the bps to the BPS left in RAM after completion
	 * of second-stage bootstrap 
	 */
	if (string_p == NULL) {
		bps_use_native++;	
		bps_initted = 0;
		splx(x);
		e_code = bps_create_tbl(bps);
		bps_use_native--;	
	}
	else {
		if (bps_strlen(string_p) == 0) {
			splx(x);
			return(EINVAL);
		}
		
		bps_initted = 0;
		save = bps_use_native;
		bps_use_native = 0;
		splx(x);
		e_code = bps_create_tbl(string_p);
		bps_use_native = save;
	}
	if (e_code == 0) 
		bps_initted++;

	return(e_code);
}

/*
 *  Kernel/Device Driver Interface to ensure BPS driver has initialized
 *	prior to use 
 */
int
bps_open()
{
	int		e_code = 0;

	if (bps_initted == 0) 
		e_code = bpsinit();
	
	return(e_code);
}

int
bps_close()
{
	return(0);
}

