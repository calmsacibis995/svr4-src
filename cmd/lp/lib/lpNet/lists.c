/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/



#ident	"@(#)lp:lib/lpNet/lists.c	1.3.2.1"

/*==================================================================*/
/*
*/
#include	<stdlib.h>
#include	<memory.h>
#include	"lists.h"
#include	"errno.h"
#include	"debug.h"

#define	_LIST(lp, name)			(lp)->listUnion.name

#define	STRING_LIST(lp)			_LIST(lp, stringList)
#define	STRING_MEMBERS(lp)		STRING_LIST(lp).members
#define	STRING_MEMBER(lp, i)		STRING_MEMBERS (lp)[i]
#define	STRING_MEMBER_SIZE(lp, i)	(strlen (STRING_MEMBER (lp, i))+1)

#define	POINTER_LIST(lp)		_LIST(lp, pointerList)
#define	POINTER_MEMBERS(lp)		POINTER_LIST(lp).members
#define	POINTER_MEMBER_SIZES(lp)	POINTER_LIST(lp).sizeOfMembers
#define	POINTER_MEMBER(lp, i)		POINTER_MEMBERS (lp)[i]
#define	POINTER_MEMBER_SIZE(lp, i)	POINTER_MEMBER_SIZES(lp)[i]

#define	STRUCTURE_LIST(lp)		_LIST(lp, structureList)
#define	STRUCTURE_MEMBERS(lp)		STRUCTURE_LIST(lp).members
#define	STRUCTURE_MEMBER_SIZES(lp)	STRUCTURE_LIST(lp).sizeOfMembers
#define	STRUCTURE_MEMBER(lp, i)	\
	((void *) (((char *) STRUCTURE_MEMBERS(lp)) +	\
		   (i * STRUCTURE_MEMBER_SIZES(lp))))
#define	STRUCTURE_MEMBER_SIZE(lp, i)	STRUCTURE_MEMBER_SIZES(lp)

/*------------------------------------------------------------------*/
/*
*/
	int	DefaultListSize	= 4;
	int	DefaultGrowSize = 4;
extern	int	errno;

/*------------------------------------------------------------------*/
/*
*/
#ifdef	DEBUG
#ifdef	__STDC__
void	DumpList (list *);
#else
void	DumpList ();
#endif
#else
#define	DumpList(lp)
#endif
/*==================================================================*/

/*==================================================================*/
/*
*/
list *
NewList (type, sizeOfMembers)

listType	type;
int		sizeOfMembers;
{
	/*----------------------------------------------------------*/
	/*
	*/
	int	mod;
	list	*lp;


	/*----------------------------------------------------------*/
	/*
	*/
	if (type == EmptyList)
		return	NULL;

	lp = (list *)  calloc (1, sizeof (list));
	if (lp == NULL) {
		errno = ENOMEM;
		return	NULL;
	}

	lp->type	= type;
	lp->size	= 0;
	lp->length	= 0;

	switch (type) {
	case	StructureList:
		if (sizeOfMembers <= 0) {
			free (lp);
			errno = EINVAL;
			return	NULL;
		}

		mod = sizeOfMembers % sizeof (int);

		if (mod != 0)
			sizeOfMembers += sizeof (int) - mod;

		STRUCTURE_MEMBER_SIZES(lp) = sizeOfMembers;
		break;

	default:
		break;
	}

	if (! GrowList (lp, DefaultListSize)) {
		free (lp);
		return	NULL;
	}


	return	lp;
}
/*==================================================================*/

/*==================================================================*/
/*
*/
void
FreeList (lpp)

list	**lpp;
{
	/*----------------------------------------------------------*/
	/*
	*/
	if (lpp == NULL || *lpp == NULL)
		return;

	FreeListMembers (*lpp);

	free (*lpp);

	*lpp = NULL;

	return;
}
/*==================================================================*/

/*==================================================================*/
/*
*/
void
FreeListMembers (lp)

register
list	*lp;
{
	/*----------------------------------------------------------*/
	/*
	*/
	register	int	i;


	/*----------------------------------------------------------*/
	/*
	**	We do a simple check for NULL pointers to avoid
	**	corruption of the memory arena.
	*/
	if (lp == NULL)
		return;

	switch (lp->type) {
	case	StringList:
		for (i=0; i < lp->length; i++)
			if (STRING_MEMBER (lp, i) != NULL)
			free (STRING_MEMBER (lp, i));

		if (STRING_MEMBERS (lp) != NULL) {
			free (STRING_MEMBERS (lp));
			STRING_MEMBERS (lp) = NULL;
		}
		lp->size = lp->length = 0;
		break;

	case	PointerList:
		for (i=0; i < lp->length; i++)
			if (POINTER_MEMBER (lp, i) != NULL)
				free (POINTER_MEMBER (lp, i));

		if (POINTER_MEMBERS (lp) != NULL) {
			free (POINTER_MEMBERS (lp));
			free (POINTER_MEMBER_SIZES (lp));
			POINTER_MEMBERS (lp)		= NULL;
			POINTER_MEMBER_SIZES (lp)	= NULL;
		}
		lp->size =  lp->length = 0;
		break;

	case	StructureList:
		if (STRUCTURE_MEMBERS (lp) != NULL) {
			free (STRUCTURE_MEMBERS (lp));
			STRUCTURE_MEMBERS (lp) = NULL;
		}
		lp->size = lp->length = 0;
		break;

	default:
		break;
	}


	return;
}
/*==================================================================*/

/*==================================================================*/
/*
*/
boolean
AppendToList (lp, newMember, sizeOfNewMember)

register
list	*lp;
void	*newMember;
int	sizeOfNewMember;
{
	/*----------------------------------------------------------*/
	/*
	*/
	static	char	FnName []	= "AppendToList";


	/*----------------------------------------------------------*/
	/*
	*/
	if (lp == NULL || newMember == NULL) {
		errno = EINVAL;
		return	False;
	}


	/*----------------------------------------------------------*/
	/*
	*/
	if (lp->size == lp->length)
		if (! GrowList (lp, lp->size+DefaultGrowSize))
			return	False;

	switch (lp->type) {
	case	StringList:
		STRING_MEMBER (lp, lp->length) = (char *) newMember;
		break;

	case	PointerList:
		if (sizeOfNewMember < 0) {
			errno = EINVAL;
			return	False;
		}
		TRACE (sizeOfNewMember)
		POINTER_MEMBER (lp, lp->length)		= newMember;
		POINTER_MEMBER_SIZE (lp, lp->length)	= sizeOfNewMember;
		break;

	case	StructureList:
		(void)
		memcpy (STRUCTURE_MEMBER (lp, lp->length), newMember,
			STRUCTURE_MEMBER_SIZES (lp));
		break;

	default:
		errno = EINVAL;
		return	False;
	}

	lp->length++;


	return	True;
}
/*==================================================================*/

/*==================================================================*/
/*
*/
void *
RemoveListMember (lp, index)

register
list	*lp;
int	index;
{
	/*----------------------------------------------------------*/
	/*
	*/
		void	*p;


	/*----------------------------------------------------------*/
	/*
	*/
	if (lp == NULL || index < 0 || index >= lp->length)
	{
		errno = EINVAL;
		return	NULL;
	}


	/*----------------------------------------------------------*/
	/*
	*/
	switch (lp->type) {
	case	StringList:
		p = (void *) STRING_MEMBER (lp, index);
		STRING_MEMBER (lp, index) = NULL;
		break;

	case	PointerList:
		p = POINTER_MEMBER (lp, index);
		POINTER_MEMBER (lp, index)	= NULL;
		POINTER_MEMBER_SIZE (lp, index) = 0;
		break;

	case	StructureList:
		p = STRUCTURE_MEMBER (lp, index);
		break;

	default:
		errno = EINVAL;
		return	NULL;
	}

	return	p;
}
/*==================================================================*/

/*==================================================================*/
/*
*/
boolean
CoalesceList (lp)

register
list	*lp;
{
	/*----------------------------------------------------------*/
	/*
	*/
			int	i, j;


	/*----------------------------------------------------------*/
	/*
	*/
	if (lp == NULL)
	{
		errno = EINVAL;
		return	NULL;
	}
	if (lp->length == 0)
	{
		return	True;
	}


	/*----------------------------------------------------------*/
	/*
	*/
	switch (lp->type) {
	case	StringList:
	{
		register char	*p;

		for (i=0, j=0; i < lp->length; i++)
		{
			if ((p = STRING_MEMBER (lp, i)) != NULL)
			{
				if (j < i)
				{
					STRING_MEMBER (lp, j) = p;
					STRING_MEMBER (lp, i) = NULL;
				}
				j++;
			}
		}
		lp->length = j+1;
		break;
	}

	case	PointerList:
	{
		register void	*p;

		for (i=0, j=0; i < lp->length; i++)
		{
			if ((p = POINTER_MEMBER (lp, i)) != NULL)
			{
				if (j < i)
				{
					POINTER_MEMBER (lp, j) = p;
					POINTER_MEMBER (lp, i) = NULL;
					POINTER_MEMBER_SIZE (lp, j) =
						POINTER_MEMBER_SIZE (lp, i);
					POINTER_MEMBER_SIZE (lp, i) = 0;
					j++;
				}
			}
		}
		lp->length = j+1;
		break;
	}

	case	StructureList:
		break;

	default:
		errno = EINVAL;
		return	NULL;
	}

	return	True;
}
/*==================================================================*/

/*==================================================================*/
/*
*/
list *
ApplyToList (lp, function, type, size)

register
list		*lp;
void		*(*function) ();
listType	type;
int		size;
{
	/*----------------------------------------------------------*/
	/*
	*/
	int	i;
	void	*value;
	list	*returnList;


	/*----------------------------------------------------------*/
	/*
	*/
	if (lp == NULL || function == NULL) {
		errno = EINVAL;
		return	NULL;
	}


	/*----------------------------------------------------------*/
	/*
	*/
	if (type == EmptyList) {
		errno = 0;
		returnList = NULL;
	}
	else {
		returnList = NewList (type, size);

		if (returnList == NULL)
			return	NULL;
	}


	/*----------------------------------------------------------*/
	/*
	*/
	switch (lp->type) {
	case	StringList:
		for (i=0; i < lp->length; i++) {
			value = (void *) (*function)
				(STRING_MEMBER (lp, i),
				STRING_MEMBER_SIZE (lp, i));
			if (type != EmptyList)
				(void)	AppendToList (returnList, value, size);
		}
		break;

	case	PointerList:
		for (i=0; i < lp->length; i++) {
			value = (void *) (*function)
				(POINTER_MEMBER (lp, i),
				POINTER_MEMBER_SIZE (lp, i));
			if (type != EmptyList)
				(void)	AppendToList (returnList, value, size);
		}
		break;

	case	StructureList:
		for (i=0; i < lp->length; i++) {
			value = (void *) (*function)
				(STRUCTURE_MEMBER(lp, i),
				STRUCTURE_MEMBER_SIZE (lp, i));
			if (type != EmptyList)
				(void)	AppendToList (returnList, value, size);
		}
		break;

	default:
		errno = EINVAL;
		return	NULL;
	}



	return	returnList;
}
/*==================================================================*/

/*==================================================================*/
/*
*/
int
LengthOfList (lp)

register
list	*lp;
{
	if (lp == NULL) {
		errno = EINVAL;
		return	0;
	}

	return	lp->length;
}
/*==================================================================*/

/*==================================================================*/
/*
*/
void *
ListMember (lp, index)

register
list	*lp;
int	index;
{
	if (lp == NULL || index < 0 || index > lp->length)
	{
		errno = EINVAL;
		return	NULL;
	}

	switch (lp->type) {
	case	StringList:
		return	(void *) STRING_MEMBER (lp, index);

	case	PointerList:
		return	POINTER_MEMBER (lp, index);

	case	StructureList:
		return	STRUCTURE_MEMBER (lp, index);

	default:
		return	NULL;
	}
}
/*==================================================================*/

/*==================================================================*/
/*
*/
/*
boolean
PushListMember (lp, p, size)

register
list	*lp;
void	*p;
uint	size;
{
	return	InsertListMember (lp, 0, p, size)
}
*/
/*==================================================================*/

/*==================================================================*/
/*
*/
void *
PopListMember (lp)

register
list	*lp;
{
	register void	*p;

	/*
	**  'errno' is set by 'RemoveListMember'.
	*/
	if ((p = RemoveListMember (lp, 0)) == NULL)
		return;

	(void)	CoalesceList (lp);

	return	p;
/*
	if (lp == NULL)
		errno = EINVAL;
		return	NULL;
	}
	if (lp->length == 0)
	{
		errno = 0;
		return	NULL;
	}
	switch (lp->type) {
	case	StringList:
		p = STRING_MEMBER (lp, 0);
		STRING_MEMBER (lp, 0) = NULL;
		break;

	case	PointerList:
		p = POINTER_MEMBER (lp, 0);
		POINTER_MEMBER (lp, 0) = NULL;
		break;

	case	StructureList:
		p = STRUCTURE_MEMBER (lp, 0);
		break;

	default:
		errno = EINVAL;
		return	NULL;
	}
	(void)	CoalesceList (lp);
	return	p;
*/
}
/*==================================================================*/

/*==================================================================*/
/*
*/
int	SizeofListMember (lp, index)

register
list	*lp;
int	index;
{
	if (lp == NULL || index < 0 || index > lp->length) {
		errno = EINVAL;
		return	0;
	}

	switch (lp->type) {
	case	StringList:
		return	STRING_MEMBER_SIZE (lp, index);

	case	PointerList:
		return	POINTER_MEMBER_SIZE (lp, index);

	case	StructureList:
		return	STRUCTURE_MEMBER_SIZE (lp, index);

	default:
		errno = EINVAL;
		return	NULL;
	}
}
/*==================================================================*/

/*==================================================================*/
/*
*/
boolean	GrowList (lp, size)

register
list	*lp;
int	size;
{
	/*----------------------------------------------------------*/
	/*
	*/
		int	oldSize;
		int	newSize;
		void	*newList1;
		void	*newList2;


	/*----------------------------------------------------------*/
	/*
	*/
	if (lp == NULL || size <= 0) {
		errno = EINVAL;
		return	False;
	}

	if (lp->size >= size)
		return	True;

	if (lp->size > 0)
		goto	resize;


	/*----------------------------------------------------------*/
	/*
	**  Initial size-ing.
	*/
	switch (lp->type) {
	case	StringList:
		STRING_MEMBERS (lp) = (char **)
			calloc (size, sizeof (char *));

		if (STRING_MEMBERS (lp) == NULL)
		{
			errno = ENOMEM;
			return	False;
		}
		break;

	case	PointerList:
		POINTER_MEMBERS (lp) = (void **)
			calloc (size, sizeof (void *));

		if (POINTER_MEMBERS (lp) == NULL)
		{
			errno = ENOMEM;
			return	False;
		}

		POINTER_MEMBER_SIZES (lp) = (int *)
			calloc (size, sizeof (int));

		if (POINTER_MEMBER_SIZES (lp) == NULL)
		{
			free (POINTER_MEMBERS (lp));
			POINTER_MEMBERS (lp) = NULL;
			errno = ENOMEM;
			return	False;
		}
		break;

	case	StructureList:
		STRUCTURE_MEMBERS (lp) = (void **)
			calloc (size, STRUCTURE_MEMBER_SIZES (lp));

		if (STRUCTURE_MEMBERS (lp) == NULL)
		{
			errno = ENOMEM;
			return	False;
		}
		break;

	default:
		errno = EINVAL;
		return	False;
	}

	lp->size = size;


	return	True;

	/*----------------------------------------------------------*/
	/*
	*/
resize:
	switch (lp->type) {
	case	StringList:
		newSize = size * sizeof (char *);

		newList1 = (void *)realloc (STRING_MEMBERS (lp), newSize);
		if (newList1 == NULL)
		{
			errno = ENOMEM;
			return	False;
		}
		STRING_MEMBERS (lp) = (char **) newList1;
		break;

	case	PointerList:
		oldSize = lp->size * sizeof (int);
		newSize = size * sizeof (int);
		newList2 = (void *) calloc (1, size * sizeof (int));
		if (newList2 == NULL)
		{
			errno = ENOMEM;
			return	False;
		}
		(void)
		memcpy (newList2, POINTER_MEMBER_SIZES (lp), oldSize);

		oldSize = lp->size * sizeof (void *);
		newSize = size * sizeof (void *);

		newList1 = (void *)realloc (POINTER_MEMBERS (lp), newSize);

		if (newList1 == NULL)
		{
			free (newList2);
			errno = EINVAL;
			return	False;
		}
		POINTER_MEMBERS (lp)		= (void **) newList1;
		POINTER_MEMBER_SIZES (lp)	= (int *) newList2;
		break;

	case	StructureList:
		newSize = size * STRUCTURE_MEMBER_SIZES (lp);
		newList1 = (void *)realloc (STRUCTURE_MEMBERS (lp), newSize);
		if (newList1 == NULL)
		{
			errno = ENOMEM;
			return	False;
		}
		STRUCTURE_MEMBERS (lp) = (void **) newList1;
		break;

	default:
		errno = EINVAL;
		return	False;
	}

	lp->size = size;

	return	True;
}
/*==================================================================*/

#ifdef	DEBUG
/*==================================================================*/
/*
*/
void	DumpList (lp)

register
list	*lp;
{
		int	i;
	static	char	FnName []	= "DumpList";


	if (lp == NULL)
		return;

	TRACE (lp->size)
	TRACE (lp->length)
	TRACE (lp->type)

	switch (lp->type) {
	case	StringList:
		for (i=0; i < lp->length; i++)
			TRACEs (STRING_MEMBER (lp, i))
		break;

	case	PointerList:
		for (i=0; i < lp->length; i++)
			TRACE (POINTER_MEMBER (lp, i))
		
		for (i=0; i < lp->length; i++)
			TRACE (POINTER_MEMBER_SIZE (lp, i))

		break;

	case	StructureList:
		TRACE (STRUCTURE_MEMBER_SIZES (lp))
		for (i=0; i < lp->length; i++)
			TRACE (STRUCTURE_MEMBER (lp, i))
		break;

	default:
		break;
	}


	return;
}
/*==================================================================*/
#endif
