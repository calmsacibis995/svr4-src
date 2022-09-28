/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cxref:st.c	1.1"
#include <stdio.h>
#include <memory.h>
#include <string.h>
#ifdef __STDC__
#include <stdlib.h>
#else
extern char *malloc();
extern char * calloc();
#endif

static char *lst_save();
static char *st_nlookup();

/*
** Look for NUL-terminated string in linked hash tables,
** return pointer to saved version.
*/
char *
st_lookup(s)
char * s;
{
    return(st_nlookup(s, strlen(s)+1));
}


#define ST_HEAPSIZE 4096
static char *
lst_save(s, len)
char *s;
unsigned int len;
/*
** Save the pointed-at string in permanent storage.  len is the 
** total length of the string to save.
*/
{
    /* savetab points at the current storage heap.  saveleft counts
    ** the number of bytes remaining in the current storage heap.
    */
    static char *savetab;
    static unsigned int saveleft; 
    char *retval = savetab;		/* expected return value */

    if (len > saveleft) {
	/* Out of space in current storage heap.  If the name
	** is too long to fit in any heap, give it its own
	** space.  Otherwise allocate a new regular storage
	** heap and stick the string in.
	*/
	retval = malloc((unsigned) (len > ST_HEAPSIZE ? len : ST_HEAPSIZE));
	if (retval == 0) {
	    (void) fprintf(stderr,"lst_save() out of memory");
	    exit(1);
	}

	if (len < ST_HEAPSIZE) {		/* allocated new heap	*/
	    savetab = retval + len;		/* we have a new table	*/
	    saveleft = ST_HEAPSIZE - len;	/* with a new size	*/
	}
    } else {				/* In current heap; adjust */
	savetab += len;
	saveleft -= len;
    }
    return(memcpy(retval, s, len));	/* copy in the string and return */
}


/*
** The linked hash tables consist of pointers to strings.  A use
** count prevents the tables from getting excessively full.
** (We waste space to gain better search performance.)
*/
#define ST_HASHSIZE	1023

static struct st_hash
{
    char * st_hptrs[ST_HASHSIZE];	/* the pointers, proper		*/
    int st_lens[ST_HASHSIZE];		/* size of each saved string	*/
    int	st_hused;			/* slots used so far		*/
    struct st_hash * st_hnext;		/* pointer to next table	*/
} st_ihash;				/* initial hash table		*/



/*
** Look for string s with length len in the linked hash tables.
** If the string is found, return pointer to it.  Otherwise
** save the string and return a pointer to the saved version.
*/
static char *
st_nlookup( s, len )
char * s;
unsigned int len;
{
    unsigned long hashval;
    char *cp;
    char * bound;
    struct st_hash * hp;	/* pointer to current hash table */

    /*
    ** Hash on the correct number of characters.
    */
    cp = s;
    hashval = 0;

    bound = s + len;

    /* PJW hash function from Dragon book, second edition. */
    while (cp < bound) {
	unsigned long overflow;

	hashval = ( hashval << 4 ) + *cp++;
	if ((overflow = hashval & (((unsigned long) 0xF) << 28)) != 0)
	    hashval ^= overflow | (overflow >> 24);
    }
    hashval %= ST_HASHSIZE;		/* choose start bucket */

    /*
    ** Look through each table, in turn, for the name.  If we fail,
    ** save the string, enter the string's pointer, and return it.
    */
    for (hp = &st_ihash; ; hp = hp->st_hnext) {
	/* use quadratic re-hash */
	int i;				/* next probe interval */
	int probeval = hashval + 0;	/* next probe value */

	for (i = 1; i < ST_HASHSIZE; i += 2) {
	    char ** probe = &hp->st_hptrs[probeval];

	    if (*probe == 0) {
		/* empty entry */
		if (hp->st_hused > (ST_HASHSIZE + 3) / 4)
		    break;		/* This table is too full; try next */
		/* Save the string, add to table. */
		hp->st_hused++;
		*probe = lst_save(s, len);
		hp->st_lens[probeval] = len;

		return( *probe );
	    }

	    if ((hp->st_lens[probeval]==len) && (memcmp(s,*probe,len)==0))
		return(*probe);

	    /* Adjust probe point */
	    probeval += i;
	    if (probeval >= ST_HASHSIZE)
		probeval -= ST_HASHSIZE;
	}

	if (hp->st_hnext == 0) {
	    /* Need to add new hash table. */
	    hp->st_hnext = (struct st_hash *) calloc(1,sizeof(struct st_hash));
	    if (hp->st_hnext == 0) {
		(void) fprintf(stderr,"st_lookup() out of memory");
		exit(1);
	    }
	}
    }
    /*NOTREACHED*/
}
