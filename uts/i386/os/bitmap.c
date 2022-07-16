/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-os:bitmap.c	1.3"

/*
 * Operations on bitmaps of arbitrary size
 * A bitmap is a vector of 1 or more ulongs.
 * The user of the package is responsible for range checks and keeping
 * track of sizes.
 */

#include "sys/types.h"
#include "sys/bitmap.h"

/*
 * Return index of first available bit in denoted bitmap, or -1 for
 * failure.  Size is the cardinality of the bitmap; that is, the
 * number of bits.
 * No side-effects.  In particular, does not update bitmap.
 * Caller is responsible for range checks.
 */
index_t
bt_availbit(bitmap, nbits)
	register ulong		*bitmap;
	size_t			nbits;
{
	register index_t	maxword;	/* index of last in map */
	register index_t	wx;		/* word index in map */
	
	/*
	 * Look for a word with a bit off.
	 * Subtract one from nbits because we're converting it to a
	 * a range of indices.
	 */
	nbits -= 1;
	maxword = nbits >> BT_ULSHIFT;
	for (wx = 0; wx <= maxword; wx++) {
		if (bitmap[wx] != ~0) {
			break;
		}
	}
	if (wx <= maxword) {
		/*
		 * Found a word with a bit off.  Now find the bit in the word.
		 */
		register index_t	bx;	/* bit index in word */
		register index_t	maxbit;	/* last bit to look at in word */
		register ulong		word;
		register ulong		bit;

		maxbit = wx == maxword ? nbits & BT_ULMASK : BT_NBIPUL - 1;
		word = bitmap[wx];
		bit = 1;
		for (bx = 0; bx <= maxbit; bx++, bit <<= 1) {
			if (!(word & bit)) {
				return wx << BT_ULSHIFT | bx;
			}
		}
	}
	return -1;
}


/*
 * Find highest order bit that is on, and is within or below
 * the word specified by wx.  Uses fast binary search
 * and assumes 32 bit word size.
 */
void
bt_gethighbit(mapp, wx, bitposp)
	register ulong	*mapp;
	register int	wx;
	int		*bitposp;
{
	register ulong	word;

	while ((word = mapp[wx]) == 0) {
		wx--;
		if (wx < 0) {
			*bitposp = -1;
			return;
		}
	}
	
	if (word & 0xffff0000) {
		if (word & 0xff000000) {
			if (word & 0xf0000000) {
				if (word & 0xc0000000) {
					if (word & 0x80000000) {
						*bitposp = wx << BT_ULSHIFT|31;
					} else {
						*bitposp = wx << BT_ULSHIFT|30;
					}
				} else if (word & 0x20000000) {
					*bitposp = wx << BT_ULSHIFT|29;
				} else {
					*bitposp = wx << BT_ULSHIFT|28;
				}
			} else if (word & 0x0c000000) {
				if (word & 0x08000000) {
					*bitposp = wx << BT_ULSHIFT|27;
				} else {
					*bitposp = wx << BT_ULSHIFT|26;
				}
			} else if (word & 0x02000000) {
				*bitposp = wx << BT_ULSHIFT|25;
			} else {
				*bitposp = wx << BT_ULSHIFT|24;
			}
		} else if (word & 0x00f00000) {
			if (word & 0x00c00000) {
				if (word & 0x00800000) {
					*bitposp = wx << BT_ULSHIFT|23;
				} else {
					*bitposp = wx << BT_ULSHIFT|22;
				}
			} else if (word & 0x00200000) {
				*bitposp = wx << BT_ULSHIFT|21;
			} else {
				*bitposp = wx << BT_ULSHIFT|20;
			}
		} else if (word & 0x000c0000) {
			if (word & 0x00080000) {
				*bitposp = wx << BT_ULSHIFT|19;
			} else {
				*bitposp = wx << BT_ULSHIFT|18;
			}
		} else if (word & 0x00020000) {
			*bitposp = wx << BT_ULSHIFT|17;
		} else {
			*bitposp = wx << BT_ULSHIFT|16;
		}
	} else if (word & 0x0000ff00) {
		if (word & 0x0000f000) {
			if (word & 0x0000c000) {
				if (word & 0x00008000) {
					*bitposp = wx << BT_ULSHIFT|15;
				} else {
					*bitposp = wx << BT_ULSHIFT|14;
				}
			} else if (word & 0x00002000) {
				*bitposp = wx << BT_ULSHIFT|13;
			} else {
				*bitposp = wx << BT_ULSHIFT|12;
			}
		} else if (word & 0x00000c00) {
			if (word & 0x00000800) {
				*bitposp = wx << BT_ULSHIFT|11;
			} else {
				*bitposp = wx << BT_ULSHIFT|10;
			}
		} else if (word & 0x00000200) {
			*bitposp = wx << BT_ULSHIFT|9;
		} else {
			*bitposp = wx << BT_ULSHIFT|8;
		}
	} else if (word & 0x000000f0) {
		if (word & 0x000000c0) {
			if (word & 0x00000080) {
				*bitposp = wx << BT_ULSHIFT|7;
			} else {
				*bitposp = wx << BT_ULSHIFT|6;
			}
		} else if (word & 0x00000020) {
			*bitposp = wx << BT_ULSHIFT|5;
		} else {
			*bitposp = wx << BT_ULSHIFT|4;
		}
	} else if (word & 0x0000000c) {
		if (word & 0x00000008) {
			*bitposp = wx << BT_ULSHIFT|3;
		 } else {
			*bitposp = wx << BT_ULSHIFT|2;
		}
	} else if (word & 0x00000002) {
		*bitposp = wx << BT_ULSHIFT|1;
	} else {
		*bitposp = wx << BT_ULSHIFT|0;
	}
	return;
}


/*
 * Search the bitmap for a consecutive pattern of 1's.
 * Search starts at position pos1.
 * Returns 1 on success and 0 on failure.
 * Side effects.
 * Returns indices to the first bit (pos1)
 * and the last bit (pos2) in the pattern.
 */
int 
bt_range(bitmap, pos1, pos2, nbits)
	register ulong *bitmap;
	register size_t *pos1;
	register size_t *pos2;
	size_t	nbits;
{
	register size_t inx;

	for (inx = *pos1; inx < nbits; inx++)
		if (BT_TEST(bitmap, inx))
			break;

	if (inx == nbits)
		return(0);

	*pos1 = inx;

	for (; inx < nbits; inx++)
		if (!BT_TEST(bitmap, inx))
			break;	

	*pos2 = inx - 1;

	return(1);
}
