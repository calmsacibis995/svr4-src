/*
 *	Copyrighted as an unpublished work.
 *      (c) Copyright 1990 INTERACTIVE Systems Corporation
 *      All Rights Reserved.
 *
 *      Copyright 1987, 1988, 1989 Lachman Associates, Incorporated (LAI)
 *      All Rights Reserved.
 *
 *      System V STREAMS TCP was jointly developed by Lachman
 *      Associates and Convergent Technologies.
 */
/*
 * Generalized hash table ADT
 *
 * Provides multiple, dynamically-allocated, variable-sized hash tables on
 * various data and keys.
 *
 * This package attempts to follow some of the coding conventions suggested
 * by Bob Sidebotham and the AFS Clean Code Committee.
 *
 *
 *
 * Copyright 1988 by Carnegie Mellon.
 *
 * Permission to use, copy, modify, and distribute this program for any
 * purpose and without fee is hereby granted, provided that this copyright
 * and permission notice appear on all copies and supporting documentation,
 * the name of Carnegie Mellon not be used in advertising or publicity
 * pertaining to distribution of the program without specific prior
 * permission, and notice be given in supporting documentation that copying
 * and distribution is by permission of Carnegie Mellon and Stanford
 * University.  Carnegie Mellon makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */


/*
 * The user must supply the following:
 *
 *	1.  A comparison function which is declared as:
 *
 *		int compare(data1, data2)
 *		hash_datum *data1, *data2;
 *
 *	    This function must compare the desired fields of data1 and
 *	    data2 and return TRUE (1) if the data should be considered
 *	    equivalent (i.e. have the same key value) or FALSE (0)
 *	    otherwise.  This function is called through a pointer passed to
 *	    the various hashtable functions (thus pointers to different
 *	    functions may be passed to effect different tests on different
 *	    hash tables).
 *
 *	    Internally, all the functions of this package always call the
 *	    compare function with the "key" parameter as the first parameter,
 *	    and a full data element as the second parameter.  Thus, the key
 *	    and element arguments to functions such as hash_Lookup() may
 *	    actually be of different types and the programmer may provide a
 *	    compare function which compares the two different object types
 *	    as desired.
 *
 *	    Example:
 *
 *		int compare(key, element)
 *		char *key;
 *		struct some_complex_structure *element;
 *		{
 *		    return !strcmp(key, element->name);
 *		}
 *
 *		key = "John C. Doe"
 *		element = &some_complex_structure
 *		hash_Lookup(table, hashcode, compare, key);
 *
 *	2.  A hash function yielding an unsigned integer value to be used
 *	    as the hashcode (index into the hashtable).  Thus, the user
 *	    may hash on whatever data is desired and may use several
 *	    different hash functions for various different hash tables.
 *	    The actual hash table index will be the passed hashcode modulo
 *	    the hash table size.
 *
 *	    A generalized hash function, hash_HashFunction(), is included
 *	    with this package to make things a little easier.  It is not
 *	    guarenteed to use the best hash algorithm in existence. . . .
 */



/*
 * Various hash table definitions
 */


#ifndef __HASHXYZ973__

#define __HASHXYZ973__

/*
 * Define "hash_datum" as a universal data type
 */
#ifdef __STDC__
typedef void hash_datum;
#else
typedef char hash_datum;
#endif

typedef struct hash_memberstruct  hash_member;
typedef struct hash_tblstruct     hash_tbl;
typedef struct hash_tblstruct_hdr hash_tblhdr;

struct hash_memberstruct {
    hash_member *next;
    hash_datum  *data;
};

struct hash_tblstruct_hdr {
    unsigned	size, bucketnum;
    hash_member *member;
};

struct hash_tblstruct {
    unsigned	size, bucketnum;
    hash_member *member;		/* Used for linear dump */
    hash_member	*table[1];		/* Dynamically extended */
};

extern hash_tbl	  *hash_Init();
extern void	   hash_Reset();
extern unsigned	   hash_HashFunction();
extern int	   hash_Exists();
extern int	   hash_Insert();
extern int	   hash_Delete();
extern hash_datum *hash_Lookup();
extern hash_datum *hash_FirstEntry();
extern hash_datum *hash_NextEntry();

#endif
