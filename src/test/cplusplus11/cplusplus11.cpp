// cplusplus11.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include "stdlib.h"
#include <utility>

#define FUNCTION	\
	do{\
		wchar_t buffer[256];\
		swprintf(buffer, L"%s(%d) function:%s is calling\n", _T(__FILE__), __LINE__, _T(__FUNCSIG__));\
		OutputDebugString(buffer);\
	}while(0);

#define NEWLINE(a)		\
	a.print();\
	OutputDebugString(_T("\n"));\
	

static int count = 0;
static int assign = 0;

class Base
{
public:
	Base() {
		FUNCTION;
		a = count++;
	}

	Base(const Base& right) {
		FUNCTION;
		a = count++;
	}

	Base(Base&& right) {
		FUNCTION;
		a = count++;
	}

	Base(const char* fmt) {
		FUNCTION;
		a = count++;
	}

	Base& operator=(const Base& right) {
		FUNCTION;
		as = assign++;
		return *this;
	}

	Base& operator=(const char* fmt) {
		FUNCTION;
		as = assign++;
		return *this;
	}

	Base& operator=(Base&& right) {
		FUNCTION;
		as = assign++;
		return *this;
	}

	Base& operator+=(int a) {
		FUNCTION;
		as = assign++;
		return *this;
	}

	void print() {
		wchar_t buffer[256]; 
		swprintf(buffer, L"con:%d/%d   assign:%d/%d\n", a, count, as, assign); 
		OutputDebugString(buffer); 
	}

	friend Base operator+(const Base& left, const Base& right) {
		FUNCTION;
		Base b;
		return  b;
	}

	friend Base operator+(Base&&left, Base&& right) {
		FUNCTION;
		return std::move(left);
	}

	int a;
	int as;
};

class A : public Base {
public:
	/*A() {
		FUNCTION;
	}
	A(const Base& right) {
		FUNCTION;
	}

	A(Base&& right) :
		Base(std::move(right))
	{
		FUNCTION;
	}*/
	using Base::Base;
	using Base::operator=;
	using Base::operator+=;
	A() {
		FUNCTION;
	}
	A(const Base& right) {
		FUNCTION;
	}

	A(Base&& right) {
		FUNCTION;
	}

	A& operator+=(float a) {
		FUNCTION;
		as = assign++;
		return *this;
	}

	A& operator=(int n) {
		FUNCTION;
		return *this;
	}
};

class B : public Base {
public:
	using Base::Base;
	/*B() {
		FUNCTION;
	}*/

	/*B(int a) {
		FUNCTION;
	}*/
	
};

Base getbase() {
	FUNCTION;
	Base b;
	return b;
}

template<class T, class A>
class Type
{
public:
	Type() { 
		FUNCTION; 
	}
	Type(const T* t){ 
		FUNCTION
	}

	Type(const T* t, A a) {
		FUNCTION;
	}

	template<class T2, class A2>
	Type(const Type<T2, A2>& r);
};

template<class T, class A>
template<class T2, class A2>
Type<T, A>::Type(const Type<T2, A2>& r) {
	if (sizeof(T2) == 2) {
		FUNCTION;
	}
	else if (sizeof(T2) == 1) {
		FUNCTION;
	}
	FUNCTION;
}



int main()
{
	// test A
	A a;   NEWLINE(a);
	A a1(""); NEWLINE(a1);
	A a2(a1); NEWLINE(a2);
	A a3(A("")); NEWLINE(a3);
	A a4(Base("")); NEWLINE(a4);

	A b(getbase()); NEWLINE(b);
	

	a += 10;
	a += (float)10.1;

	a = 10;
	a = "aa"; NEWLINE(a);
	a = Base(); NEWLINE(a);
	a = A(); NEWLINE(a);
	a = A("aa") + A() + getbase(); NEWLINE(a);

	// test B
	//B b(getbase);
	//b = getbase();

	Type<char, int> ta;
	Type<wchar_t, float> tb;

	Type<char, int> tc(tb);

    return 0;
}

