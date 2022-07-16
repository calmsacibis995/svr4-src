/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/delkey.c	1.3"
#include	"curses_inc.h"

/* Delete keys matching pat or key from the key map. */

delkey(sends, keyval)
char	*sends;
int	keyval;
{
    register	_KEY_MAP	*kp, **kpp = cur_term->_keys, **fpp, **dpp;
    register	int     	mask = 0, cmp, numkeys = cur_term->_ksz;
    int				counter = 0, i, num_deleted_keys = 0;
    short			*lkorder = &(cur_term->_lastkey_ordered),
				*first_macro = &(cur_term->_first_macro),
				*lmorder = &(cur_term->_lastmacro_ordered);

    /* for ease of determination of key to delete */
    if (sends)
	mask |= 01;
    if (keyval >= 0)
	mask |= 02;

    /* check each key */
    while (++counter < numkeys)
    {
	kp = *kpp;
	cmp = 0;
	if (sends && (strcmp(sends, kp->_sends) == 0))
	    cmp |= 01;
	if (kp->_keyval == keyval)
	    cmp |= 02;

	/* found one to delete */
	if (cmp == mask)
	{
	    num_deleted_keys++;
	    /*
	     * If it was an externally created key, then the address
	     * of the sequence will be right after the structure.
	     * See the malloc in newkey.
	     */
	    if (kp->_sends == ((char *) kp + sizeof(_KEY_MAP)))
		free(kp);

	    /* shift left other keys */
	    i = (numkeys - counter) - 1;
	    for (fpp = kpp, dpp = kpp + 1; i > 0; i--, fpp++, dpp++)
		*fpp = *dpp;
	    if (counter <= *lmorder)
	    {
		if (counter < *first_macro)
		{
		    if (counter <= *lkorder)
			(*lkorder)--;
		    (*first_macro)--;
		}
		(*lmorder)--;
	    }
	}
	else
	    kpp++;
    }

/* Check if we've crossed boundary and/or hit 0 */

    if ((cur_term->_ksz -= num_deleted_keys) == 0)
	delkeymap(cur_term);
    else
	cur_term->_keys = (_KEY_MAP **) realloc ((char *) cur_term->_keys, (unsigned) cur_term->_ksz);

    return (num_deleted_keys);
}
