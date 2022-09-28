#ident	"@(#)sdb:libdbgen/common/Vector.C	1.2"
#include	"Vector.h"
#include	<stdio.h>
#include	<malloc.h>
#include	<memory.h>

// BSIZ is the minimum growth of the vector in bytes.
#define BSIZ	100

// does not handle malloc or realloc returing NULL. it should Fail(...) in these
// cases.
void
Vector::getmemory(int howmuch)
{
	int	sz;

	if (total_bytes == 0)
	{
		sz = howmuch < BSIZ ? BSIZ : howmuch;
		vector = (char *)malloc(sz);
		total_bytes = sz;
	}
	else
	{
		sz = howmuch < BSIZ ? BSIZ : howmuch;
		total_bytes = total_bytes + sz;
		vector = (char *)realloc(vector,total_bytes);
	}
}

Vector::Vector()
{
	bytes_used = 0;
	total_bytes = 0;
	vector = 0;
}

Vector::Vector(Vector & v)
{
	vector = (char *)malloc(v.total_bytes);
	::memcpy(vector,v.vector,v.total_bytes);
	total_bytes = v.total_bytes;
	bytes_used = v.bytes_used;
}

Vector::~Vector()
{
	if (vector) free(vector);
}

Vector &
Vector::add(void * p, int sz)
{
	if (sz > (total_bytes - bytes_used))
	{
		getmemory(sz);
	}
	::memcpy(vector + bytes_used, (char *)p, sz);
	bytes_used += sz;
	return *this;
}

Vector &
Vector::operator=(Vector & v)
{
	if (this != &v)
	{
		if (vector) free (vector);
		vector = (char *)malloc(v.total_bytes);
		::memcpy(vector,v.vector,v.total_bytes);
		total_bytes = v.total_bytes;
		bytes_used = v.bytes_used;
	}
	return *this;
}

Vector &
Vector::take(Vector & v)
{
	if (this != &v)
	{
		if (vector) free (vector);
		vector = v.vector;
		total_bytes = v.total_bytes;
		bytes_used = v.bytes_used;
		v.vector = 0;
		v.total_bytes = 0;
		v.bytes_used = 0;
	}
	return *this;
}

Vector &
Vector::report(char * msg)
{
	if (msg)
		printf("%s\n",msg);
	printf("\ttotal bytes : %d (%#x)\n",total_bytes,total_bytes);
	printf("\tbytes used : %d (%#x)\n",bytes_used,bytes_used);
	printf("\tvector : (%#x) >%s<\n",vector,vector);
	if (bytes_used != 0)
		printf("\tvector[%d] : %c (%#x)\n",bytes_used-1,
			vector[bytes_used-1],vector[bytes_used-1]);
	printf("\tvector[%d] : %c (%#x)\n",bytes_used,
		vector[bytes_used],vector[bytes_used]);
	return *this;
}
