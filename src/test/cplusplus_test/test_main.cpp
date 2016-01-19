#include <stdio.h>
#include <vector>
#include <memory>

class A;
class B;

class A
{
public:
	A(){}
	~A() { 
		printf( "A::~A()\n" );
		unreg();
	}

	void print(const char* str){
		printf( "A::printf: %s\n", str );
	}

	void reg(std::shared_ptr<B>& b){
		list.push_back(b);
	}

	void unreg(){
		list.clear();
	}

	std::vector<std::shared_ptr<B>> list;
};

class B
{
public:
	B(std::shared_ptr<A>& a, int id)
		:_ref_a(a),
		_id(id)
	{
	}

	~B(){ 
		printf( "B::~B() %d\n", _id );
	}

	std::weak_ptr<A> _ref_a;
	int _id;
};

struct ref{

	ref(std::shared_ptr<A>& r): _r(r){}
	std::shared_ptr<A>& _r;
};

int main(int argc, const char** argv)
{
	{
		std::shared_ptr<A> a(new A);

		for (int i = 0; i < 5; ++i){
			std::shared_ptr<B> b(new B(a, i));
			a.get()->reg(b);
		}
	}

	{
		std::shared_ptr<A> a;
		ref b(a);
		b._r.reset(new A);

		if (a.get())
			a->print("a");
		if (b._r.get())
			b._r->print("b");
	}

	system("pause");
	return 0;
}