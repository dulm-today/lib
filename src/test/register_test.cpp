#include "test_core.h"
#include "register.h"
#include <stdio.h>

class reg_test : public registe<int, reg_test*>
{
public:
	reg_test(int id, const char* name)
		: registe<int, reg_test*>(id, this),
		_id(id), _name(name)
	{}
	~reg_test(){}

	int 		_id;
	const char*	_name;
};

class reg_test2 : public registe<int, reg_test2*>
{
public:
	reg_test2(int id, const char* name)
		: registe<int, reg_test2*>(id, this),
		_id(id), _name(name)
	{}
	~reg_test2(){}

	int 		_id;
	const char*	_name;
};

static reg_test a(1, "aaaa");
static reg_test b(10, "bbbb");
static reg_test c(3, "cccc"); 
static reg_test d(11, "dddd");
static reg_test e(-1, "eeee");
static reg_test ee(-1, "e2e2e2");

static reg_test2 a2(1, "aaaa2");
static reg_test2 b2(10, "bbbb2");
static reg_test2 c2(3, "cccc2"); 
static reg_test2 d2(11, "dddd2");
static reg_test2 e2(-1, "eeee2");
static reg_test2 ee2(-1, "e2e2e22");


int register1_test()
{
	int i;
	int num = reg_test::num();

	for (i = 0; i < num; ++i){
		reg_test* reg = *reg_test::get(i);
		fprintf(stderr, "id: %3d   name: %-10s\n", reg->_id, reg->_name);
	}

	fprintf(stderr, "\nsearch:\n");
	for (i = -5; i < 20; ++i){
		reg_test** reg = reg_test::search(i);
		if (NULL == reg)
			continue;
		fprintf(stderr, "  id: %3d   name: %-10s\n", (*reg)->_id, (*reg)->_name);
		reg_test::del(i);
	}

	fprintf(stderr, "\nnumber: %d\n", reg_test::num());
	return 0;
}

int register2_test()
{
	int i;
	int num = reg_test2::num();

	for (i = 0; i < num; ++i){
		reg_test2* reg = *reg_test2::get(i);
		fprintf(stderr, "id: %3d   name: %-10s\n", reg->_id, reg->_name);
	}

	fprintf(stderr, "\nsearch:\n");
	for (i = -5; i < 20; ++i){
		reg_test2** reg = reg_test2::search(i);
		if (NULL == reg)
			continue;
		fprintf(stderr, "  id: %3d   name: %-10s\n", (*reg)->_id, (*reg)->_name);
		reg_test2::del(i);
	}

	fprintf(stderr, "\nnumber: %d\n", reg_test2::num());
	return 0;
}

int register_test()
{
	register1_test();
	register2_test();
	
	return 0;
}


Test register_test_item("register", register_test, 1);

