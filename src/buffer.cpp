#include "buffer.h"
#include <stdlib.h>
#include <string.h>

#define MIN(x,y)	((x) <= (y) ? (x) : (y))

buffer::buffer(unsigned int size /*= 0*/)
	:_ptr(NULL),
	 _use(0),
	 _size(0),
	 _zero(false)
{
	expand(size);
}

buffer::buffer(const void* data, unsigned int size)
	:_ptr(NULL),
	_use(0),
	_size(0),
	_zero(false)
{
	save(data, size);
}

buffer::buffer(const buffer& right)
	:_ptr(NULL),
	_use(0),
	_size(0)
{	
	if (right.iszero())
		save(NULL, right._use);
	else
		save(right._ptr, right._use);
}

buffer::~buffer()
{
	clear();
}

bool  buffer::expand(unsigned int size, bool copy /*= false*/)
{
	if (_size >= size)
		return true;

	void* p = malloc(size);
	if (NULL == p)
		return false;

	if (copy && !iszero() && _use > 0)
		memcpy(p, _ptr, _use);

	if (NULL != _ptr)
		free(_ptr);
	_ptr = p;
	_size = size;
	return true;
}

void buffer::clear()
{
	if (_ptr)
		free(_ptr);
	
	_ptr = NULL;
	_use = 0;
	_size = 0;
	setzero(false);
}

unsigned int buffer::capacity() const
{
	return _size;
}

unsigned int buffer::length() const
{
	return _use;
}

bool buffer::isnull() const
{
	return (NULL == _ptr ? true : false);
}

bool buffer::iszero() const 
{
	return _zero;
}

bool buffer::isdatazero() const
{
	if (!iszero())
		return false;

	if (isnull())
		return true;
	
	for (unsigned int i = 0; i < length(); ++i){
		if (((char*)_ptr)[i])
			return false;
	}
	return true;
}

void buffer::setzero(bool b)
{
	_zero = b;
}

void* buffer::get(unsigned int size /*= 0*/)
{
	if (0 == size)
		return _ptr;

	if(expand(size, true))
		return _ptr;

	return NULL;
}

int buffer::storage(void* data, unsigned int size/*= -1*/, unsigned int length/*= -1*/)
{
	if (-1 == length){
		if (NULL == data)
			return -1;
		length = strlen((const char*)data);
	}

	if (length > size)
		return -1;

	clear();

	if (-1 == size)
		size = length;

	if (0 == size)
		return -1;
	
	_ptr = data;
	_size = size;
	_use = length;

	if (NULL == _ptr && length > 0)
		setzero(true);
	
	return length;
}

int buffer::save(const void* data, unsigned int size/*= -1*/)
{
	if (-1 == size){
		if (NULL == data)
			return 0;
		size = strlen((const char*)data);
	}

	if (0 == size){
		_use = 0;
		setzero(false);
		return 0;
	}

	if(!expand(size))
		return 0;

	if(NULL == data)
		setzero(true);
	else
		memcpy(_ptr, data, size);
	_use = size;
	return size;
}

int buffer::save(unsigned int size)
{
	if (size > _size)
		return 0;
	_use = size;
	if (iszero())
		setzero(false);
	return size;
}

int buffer::append(const void* data, unsigned int size/*= -1*/)
{
	if (-1 == size){
		if (NULL == data)
			return 0;
		size = strlen((const char*)data);
	}
	
	if (0 == size)
		return 0;

	if (0 == _use)
		return save(data, size);

	if (size > _size - _use && _use + size < _size)
		return 0;
	if (!expand(_use+size, true))
		return 0;
	
	if (NULL == data){
		if (!iszero())
			memset((char*)_ptr+_use, 0, size);
	}
	else{
		if (iszero())
			memset((char*)_ptr, 0, _use);
		memcpy((char*)_ptr+_use, data, size);
		setzero(false);
	}
	_use += size;
	return size;
}

int buffer::copyto(void* data, unsigned int start, unsigned int size)
{
	unsigned int copy;
	
	if (NULL == data || 0 == size || start >= _use)
		return 0;

	copy = MIN(size, _use - start);

	if (iszero())
		memset(data, 0, copy);
	else
		memcpy(data, (char*)_ptr+start, copy);
	return copy;
}

int buffer::pick(unsigned int start, unsigned int count /*= -1*/)
{
	unsigned int move;
	
	if (start >= _use || NULL == _ptr || iszero())
		return 0;

	move = MIN(count, _use - start);
	memmove(_ptr, (char*)_ptr+start, move);
	_use = move;

	return move;
}

buffer& buffer::operator=(const buffer& right)
{
	if (this == &right)
		return *this;

	if (right.iszero())
		save(NULL, right._use);
	else
		save(right._ptr, right._use);
	
	return *this;
}

buffer& buffer::operator+(const buffer& right)
{
	if (right.iszero())
		append(NULL, right._use);
	else
		append(right._ptr, right._use);

	return *this;
}

buffer& buffer::operator+=(const buffer& right)
{
	if (right.iszero())
		append(NULL, right._use);
	else
		append(right._ptr, right._use);

	return *this;
}

bool buffer::operator==(const buffer& right)
{
	if (length() != right.length())
		return false;

	if (iszero() && right.iszero())
		return true;

	if (iszero() && right.isdatazero())
		return true;

	if (right.iszero() && isdatazero())
		return true;

	for (unsigned int i = 0; i < length(); ++i){
		if (((char*)_ptr)[i] != ((char*)right._ptr)[i])
			return false;
	}

	return true;
}


