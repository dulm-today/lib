#ifndef __BUFFER_H__
#define __BUFFER_H__

struct buffer
{	
	void*			_ptr;
	unsigned int	_use;
	unsigned int	_size;
	bool			_zero;

	buffer(unsigned int size = 0);
	buffer(const void* data, unsigned int size);
	buffer(const buffer& right);
	~buffer();

	void clear();
	unsigned int capacity() const;
	unsigned int length() const;
	bool isnull() const;
	bool iszero() const;
	bool isdatazero() const;
	void setzero(bool b);
	
	void* get(unsigned int size = 0);

	int storage(void* data, unsigned int size = -1, unsigned int length = -1);
	int save(const void* data, unsigned int size = -1);
	int save(unsigned int size);
	int append(const void* data, unsigned int size = -1);
	
	int copyto(void* data, unsigned int start, unsigned int size);
	int pick(unsigned int start, unsigned int count = -1);

	buffer& operator=(const buffer& right);
	buffer& operator+(const buffer& right);
	buffer& operator+=(const buffer& right);
	bool operator==(const buffer& right);
private:
	bool  expand(unsigned int size, bool copy = false);
};

typedef buffer buffer_t;

#endif //__BUFFER_H__
