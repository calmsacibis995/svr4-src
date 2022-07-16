/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libcrypt:des_encrypt.c	1.5"

#ifdef __STDC__
	#pragma weak des_encrypt1 = _des_encrypt1
#endif
#include "synonyms.h"

void
des_encrypt1(block, L, IP, R, preS, E, KS, S, f, tempL, P, FP)
char	*block, *L, *IP, *R, *preS, *E, KS[][48], S[][64], *f, *tempL, *P, *FP;
{
	int	i;
	register int t, j, k;

	/*
	 * First, permute the bits in the input
	 */
	for(j=0; j < 64; j++)
		L[j] = block[IP[j]-1];
	/*
	 * Perform an encryption operation 16 times.
	 */
	for(i=0; i < 16; i++) {
		/*
		 * Save the R array,
		 * which will be the new L.
		 */
		for(j=0; j < 32; j++)
			tempL[j] = R[j];
		/*
		 * Expand R to 48 bits using the E selector;
		 * exclusive-or with the current key bits.
		 */
		for(j=0; j < 48; j++)
			preS[j] = R[E[j]-1] ^ KS[i][j];
		/*
		 * The pre-select bits are now considered
		 * in 8 groups of 6 bits each.
		 * The 8 selection functions map these
		 * 6-bit quantities into 4-bit quantities
		 * and the results permuted
		 * to make an f(R, K).
		 * The indexing into the selection functions
		 * is peculiar; it could be simplified by
		 * rewriting the tables.
		 */
		for(j=0; j < 8; j++) {
			t = 6*j;
			k = S[j][(preS[t+0]<<5)+
				(preS[t+1]<<3)+
				(preS[t+2]<<2)+
				(preS[t+3]<<1)+
				(preS[t+4]<<0)+
				(preS[t+5]<<4)];
			t = 4*j;
			f[t+0] = (k>>3)&01;
			f[t+1] = (k>>2)&01;
			f[t+2] = (k>>1)&01;
			f[t+3] = (k>>0)&01;
		}
		/*
		 * The new R is L ^ f(R, K).
		 * The f here has to be permuted first, though.
		 */
		for(j=0; j < 32; j++)
			R[j] = L[j] ^ f[P[j]-1];
		/*
		 * Finally, the new L (the original R)
		 * is copied back.
		 */
		for(j=0; j < 32; j++)
			L[j] = tempL[j];
	}
	/*
	 * The output L and R are reversed.
	 */
	for(j=0; j < 32; j++) {
		t = L[j];
		L[j] = R[j];
		R[j] = t;
	}
	/*
	 * The final output
	 * gets the inverse permutation of the very original.
	 */
	for(j=0; j < 64; j++)
		block[j] = L[FP[j]-1];
}
