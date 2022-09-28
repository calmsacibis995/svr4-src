/*
 *	Copyrighted as an unpublished work.
 *      (c) Copyright 1990 INTERACTIVE Systems Corporation
 *      All Rights Reserved.
 *
 *	Copyright 1987, 1988, 1989 Lachman Associates, Incorporated (LAI)
 *	All Rights Reserved.
 *
 *	The copyright above and this notice must be preserved in all
 *	copies of this source code.  The copyright above does not
 *	evidence any actual or intended publication of this source
 *	code.
 *
 *	This is unpublished proprietary trade secret source code of
 *	Lachman Associates.  This source code may not be copied,
 *	disclosed, distributed, demonstrated or licensed except as
 *	expressly authorized by Lachman Associates.
 *
 *	System V STREAMS TCP was jointly developed by Lachman
 *	Associates and Convergent Technologies.
 */

/*
static char rcsid[] = "$Header: hash.c,v 1.3 88/11/07 20:43:37 ww0n Locked $";
*/


/*
 * Generalized hash table ADT
 *
 * Provides multiple, dynamically-allocated, variable-sized hash tables on
 * various data and keys.
 *
 * This package attempts to follow some of the coding conventions suggested
 * by Bob Sidebotham and the AFS Clean Code Committee of the
 * Information Technology Center at Carnegie Mellon.
 *
 *
 *
 * Copyright (c) 1988 by Carnegie Mellon.
 *
 * Permission to use, copy, modify, and distribute this program for any
 * purpose and without fee is hereby granted, provided that this copyright
 * and permission notice appear on all copies and supporting documentation,
 * the name of Carnegie Mellon not be used in advertising or publicity
 * pertaining to distribution of the program without specific prior
 * permission, and notice be given in supporting documentation that copying
 * and distribution is by permission of Carnegie Mellon.  Carnegie Mellon
 * makes no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 */


#include "hash.h"

#define TRUE		1
#define FALSE		0
#define NULL		0

/*
 * This can be changed to make internal routines visible to debuggers, etc.
 */
#define PRIVATE static



/*
 * Hash table initialization routine.
 *
 * This routine creates and intializes a hash table of size "tablesize"
 * entries.  Successful calls return a pointer to the hash table (which must
 * be passed to other hash routines to identify the hash table).  Failed
 * calls return NULL.
 */

hash_tbl *hash_Init(tablesize)
unsigned tablesize;
{
    register hash_tbl *hashtblptr;
    register unsigned totalsize;

    if (tablesize > 0) {
	totalsize = sizeof(hash_tbl)
			+ sizeof(hash_member *) * (tablesize - 1);
	hashtblptr = (hash_tbl *) malloc(totalsize);
	if (hashtblptr) {
	    memset((char *) hashtblptr, 0, totalsize);
	    hashtblptr->size = tablesize;		/* Success! */
	    hashtblptr->bucketnum = 0;
	    hashtblptr->member = (hashtblptr->table)[0];
	}
    } else {
	hashtblptr = NULL;	/* Disallow zero-length tables */
    }
    return hashtblptr;		/* NULL if failure */
}



/*
 * Recursively frees an entire linked list of bucket members (used in the open
 * hashing scheme).  Does nothing if the passed pointer is NULL.
 */

PRIVATE void hashi_FreeMember(bucketptr, free_data)
hash_member *bucketptr;
void (*free_data)();
{
    if (bucketptr) {
	/*
	 * Free next member if necessary
	 */
	hashi_FreeMember(bucketptr->next, free_data);
	(*free_data)(bucketptr->data);
	free((char *) bucketptr);
    }
}




/*
 * This routine re-initializes the hash table.  It frees all the allocated
 * memory and resets all bucket pointers to NULL.
 */

void hash_Reset(hashtable, free_data)
hash_tbl *hashtable;
void (*free_data)();
{
    hash_member **bucketptr;
    unsigned i;

    bucketptr = hashtable->table;
    for (i = 0; i < hashtable->size; i++) {
	hashi_FreeMember(*bucketptr, free_data);
	*bucketptr++ = NULL;
    }	
    hashtable->bucketnum = 0;
    hashtable->member = (hashtable->table)[0];
}



/*
 * Generic hash function to calculate a hash code from the given string.
 *
 * This function returns the sum of the squares of all the bytes.  It is
 * assumed that this result will be used as the "hashcode" parameter in
 * calls to other functions in this package.  These functions automatically
 * adjust the hashcode for the size of each hashtable.
 *
 * This algorithm probably works best when the hash table size is a prime
 * number.
 *
 * This may not be the world's best hash function.  I'm open to other
 * suggestions.  The programmer is more than welcome to supply his/her own
 * hash function as that is one of the design features of this package.
 */

unsigned hash_HashFunction(string, len)
unsigned char *string;
register unsigned len;
{
    register unsigned sum, value;

    sum = 0;
    for (; len > 0; len--) {
	value = (unsigned) (*string++ & 0xFF);
	sum += value * value;
    }
    return sum;
}



/*
 * Returns TRUE if at least one entry for the given key exists; FALSE
 * otherwise.
 */

int hash_Exists(hashtable, hashcode, compare, key)
hash_tbl *hashtable;
unsigned hashcode;
int (*compare)();
hash_datum *key;
{
    register hash_member *memberptr;

    memberptr = (hashtable->table)[hashcode % (hashtable->size)];
    while (memberptr) {
	if ((*compare)(key, memberptr->data)) {
	    return TRUE;		/* Entry does exist */
	}
	memberptr = memberptr->next;
    }
    return FALSE;			/* Entry does not exist */
}



/*
 * Insert the data item "element" into the hash table using "hashcode"
 * to determine the bucket number, and "compare" and "key" to determine
 * its uniqueness.
 *
 * If the insertion is successful 0 is returned.  If a matching entry
 * already exists in the given bucket of the hash table, or some other error
 * occurs, -1 is returned and the insertion is not done.
 */

int hash_Insert(hashtable, hashcode, compare, key, element)
hash_tbl *hashtable;
unsigned hashcode;
int (*compare)();
hash_datum *key, *element;
{
    hash_member *memberptr, *temp;
    
    hashcode %= hashtable->size;
    if (hash_Exists(hashtable, hashcode, compare, key)) {
	return -1;	/* At least one entry already exists */
    }
    memberptr = (hashtable->table)[hashcode];
    temp = (hash_member *) malloc(sizeof(hash_member));
    if (temp) {
	(hashtable->table)[hashcode] = temp;
	temp->data = element;
	temp->next = memberptr;
	return 0;	/* Success */
    } else {
	return -1;	/* malloc failed! */
    }
}



/*
 * Delete all data elements which match the given key.  If at least one
 * element is found and the deletion is successful, 0 is returned.
 * If no matching elements can be found in the hash table, -1 is returned.
 */

int hash_Delete(hashtable, hashcode, compare, key, free_data)
hash_tbl *hashtable;
unsigned hashcode;
int (*compare)();
hash_datum *key;
void (*free_data)();
{
    hash_member *memberptr, *previous, *tempptr;
    int retval;

    retval = -1;
    hashcode %= hashtable->size;

    /*
     * Delete the first member of the list if it matches.  Since this moves
     * the second member into the first position we have to keep doing this
     * over and over until it no longer matches.
     */
    memberptr = (hashtable->table)[hashcode];
    while ((*compare)(key, memberptr->data)) {
	(hashtable->table)[hashcode] = memberptr->next;
	/*
	 * Stop hashi_FreeMember() from recursively deleting the whole list!
	 */
	memberptr->next = NULL;
	hashi_FreeMember(memberptr, free_data);
	memberptr = (hashtable->table)[hashcode];
	retval = 0;
    }

    /*
     * Now traverse the rest of the list
     */
    previous = memberptr;
    memberptr = memberptr->next;
    while (memberptr) {
	if ((*compare)(key, memberptr->data)) {
	    tempptr = memberptr;
	    previous->next = memberptr = memberptr->next;
	    /*
	     * Put the brakes on hashi_FreeMember(). . . .
	     */
	    tempptr->next = NULL;
	    hashi_FreeMember(tempptr, free_data);
	    retval = 0;
	} else {
	    previous = memberptr;
	    memberptr = memberptr->next;
	}
    }
    return retval;
}



/*
 * Locate and return the data entry associated with the given key.
 *
 * If the data entry is found, a pointer to it is returned.  Otherwise,
 * NULL is returned.
 */

hash_datum *hash_Lookup(hashtable, hashcode, compare, key)
hash_tbl *hashtable;
unsigned hashcode;
int (*compare)();
hash_datum *key;
{
    hash_member *memberptr;

    memberptr = (hashtable->table)[hashcode % (hashtable->size)];
    while (memberptr) {
	if ((*compare)(key, memberptr->data)) {
	    return (memberptr->data);
	}
	memberptr = memberptr->next;
    }
    return NULL;
}



/*
 * Return the next available entry in the hashtable for a linear search
 */

hash_datum *hash_NextEntry(hashtable)
hash_tbl *hashtable;
{
    register unsigned bucket;
    register hash_member *memberptr;

    /*
     * First try to pick up where we left off.
     */
    memberptr = hashtable->member;
    if (memberptr) {
	hashtable->member = memberptr->next;	/* Set up for next call */
	return memberptr->data;			/* Return the data */
    }

    /*
     * We hit the end of a chain, so look through the array of buckets
     * until we find a new chain (non-empty bucket) or run out of buckets.
     */
    bucket = hashtable->bucketnum + 1;
    while ((bucket < hashtable->size) && 
	   !(memberptr = (hashtable->table)[bucket])) {
	bucket++;
    }

    /*
     * Check to see if we ran out of buckets.
     */
    if (bucket >= hashtable->size) {
	/*
	 * Reset to top of table for next call.
	 */
	hashtable->bucketnum = 0;
	hashtable->member = (hashtable->table)[0];
	/*
	 * But return end-of-table indication to the caller this time.
	 */
	return NULL;
    }

    /*
     * Must have found a non-empty bucket.
     */
    hashtable->bucketnum = bucket;
    hashtable->member = memberptr->next;	/* Set up for next call */
    return memberptr->data;			/* Return the data */
}



/*
 * Return the first entry in a hash table for a linear search
 */

hash_datum *hash_FirstEntry(hashtable)
hash_tbl *hashtable;
{
    hashtable->bucketnum = 0;
    hashtable->member = (hashtable->table)[0];
    return hash_NextEntry(hashtable);
}
