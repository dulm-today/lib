#ifndef __TSTRING_H_INCLUDE
#define __TSTRING_H_INCLUDE

#include <string>
#include <algorithm>
#include <cctype>
#include <sstream>
#include "printf_safe.h"

_STD_BEGIN

#ifndef INLINE
#define INLINE inline
#endif


template <class _Elem> struct ex_format {};
template <> struct ex_format<char>{
	static INLINE int ex_format_length(const char* fmt, va_list ap){
		return vsnprintf_safe(NULL, 0, fmt, ap);
	}
	static INLINE int ex_format_do(char* buffer, size_t numOfElem,
				const char* fmt, va_list ap){
		return vsnprintf_safe(buffer, numOfElem, fmt, ap);				
	}
};

template <> struct ex_format<wchar_t>{
	static INLINE int ex_format_length(const wchar_t* fmt, va_list ap){
		return vsnwprintf_safe(NULL, 0, fmt, ap);
	}
	static INLINE int ex_format_do(wchar_t* buffer, size_t numOfElem,
				const wchar_t* fmt, va_list ap){
		return vsnwprintf_safe(buffer, numOfElem, fmt, ap);				
	}
};

template<class _Elem,
		class _Traits,
		class _Alloc,
		class _ExFormat>
class basic_exstring;

// tostring
template<class _Elem,
	class _Traits,
	class _Alloc,
	class _ExFormat,
	typename T>
basic_exstring<_Elem, _Traits, _Alloc, _ExFormat>
ex_tostring(const T& v, int cnt = 0, bool left = false, int precision = -1){
	basic_stringstream<_Elem, _Traits, _Alloc> ss;
	ss.width(cnt);
	ss.precision(precision);
	if (left)
		ss.setf(ios_base::left);
	ss << v;
	return ss.str();
}

typedef int(*case_fn)(int);

template<class _Elem> INLINE
case_fn ex_tolower() {
	int(*_func[4])(int) = { ::tolower, (case_fn)::towlower, (case_fn)::towlower, (case_fn)::towlower };
	return _func[sizeof(_Elem) - 1];
}

template<class _Elem> INLINE
case_fn ex_toupper() {
	int(*_func[4])(int) = { ::toupper, (case_fn)::towupper, (case_fn)::towupper, (case_fn)::towupper };
	return _func[sizeof(_Elem) - 1];
}

template<class _Elem,
	class _Traits,
	class _Alloc,
	class _ExFormat = ex_format<_Elem> >
	class basic_exstring
		: public basic_string<_Elem, _Traits, _Alloc>
{
public:
	typedef basic_exstring<_Elem, _Traits, _Alloc, _ExFormat>	_Myt;
	typedef basic_string<_Elem, _Traits, _Alloc> _Mybase;
	typedef typename _Elem _Elemtype;
	
	// use basic's constructor
	using basic_string<_Elem, _Traits, _Alloc>::basic_string;

	// use basic's operator=
	using basic_string<_Elem, _Traits, _Alloc>::operator=;

	// use basic's operator+=
	using basic_string<_Elem, _Traits, _Alloc>::operator+=;

	// we have a new replace, and also use the basic's
	using basic_string<_Elem, _Traits, _Alloc>::replace; 
	
	// we need a no paramet constructor, using basic's not have this. but why ???
	basic_exstring() : _Mybase() {}

	// we need copy constructor with paramet of basic_string
	basic_exstring(const _Mybase& _right) : _Mybase(_right){}
	basic_exstring(_Mybase&& _right) : _Mybase(_STD move(_right)){}
	
	// wcs -> mbs
	template<class _Elem2,
			class _T = typename enable_if<
				is_same<_Elem, char>::value
			&&  is_same<_Elem2, wchar_t>::value>::type >
	basic_exstring(const _Elem2* _right) {
		_Tidy();
		size_type size = wcstombs(NULL, _right, 0);
		if ((size_type)-1 == size)
			_Xlen();
		if (_Grow(size + 1)) {
			size = wcstombs(_Myptr(), _right, size + 1);
			_Eos(size+1);
		}
	}

	// mbs -> wcs
	template<class _Elem2,
			class = typename enable_if<
				is_same<_Elem, wchar_t>::value
			&&  is_same<_Elem2, char>::value>::type >
	basic_exstring(const _Elem2* _right, int _v = 0)
	{
		_Tidy();
		size_type size = mbstowcs(NULL, _right, 0);
		if ((size_type)-1 == size)
			_Xlen();
		if (_Grow(size + 1)) {
			size = mbstowcs(_Myptr(), _right, size + 1);
			_Eos(size+1);
		}
	}

	// 
	_Elem* getbuffer(size_t _size = 0){
		if (_Myres() >= _size)
			return _Myptr();
		if (_Grow(_size))
			return _Myptr();
		return NULL;
	}

	void releasebuffer(size_t _size) {
		if (_size > _Myres())
			_Xran();
		_Eos(_size);
	}

	/**** these type conversion function is very dangerous!!! ****/
	//operator const _Elem* () const { return _Myptr(); }
	//operator _Elem* () { return _Myptr(); }
	
	_Myt& format(const _Elem* fmt, ...){
		va_list ap;
		va_start(ap, fmt);
		formatv(fmt, ap);
		va_end(ap);
		return (*this);
	}

	_Myt& formatv(const _Elem* fmt, va_list ap){
		int size = _ExFormat::ex_format_length(fmt, ap);
		if (-1 == size)
			_Xlen();
		if (_Grow(size+1)){
			size = _ExFormat::ex_format_do(_Myptr(), _Myres(), fmt, ap);
			_Eos(size);
		}
		return (*this);
	}
		
	_Myt& toupper(){
		transform(begin(), end(), begin(), ex_toupper<_Elem>());
		return (*this);
	}
		
	_Myt toupper() const{
		_Myt s;
		transform(begin(), end(), s.begin(), ex_toupper<_Elem>());
		return s;
	}
		
	_Myt& tolower(){
		transform(begin(), end(), begin(), ex_tolower<_Elem>());
		return (*this);
	}
	
	_Myt tolower() const{
		_Myt s;
		transform(begin(), end(), s.begin(), ex_tolower<_Elem>());
		return s;
	}

	_Myt& replace(const _Myt& oldstr, const _Myt& newstr) {
		if (empty() || oldstr.empty())
			return (*this);
		size_type index;
		while ((index = find(oldstr)) != npos)
			replace(index, oldstr.length(), newstr);
		return (*this);
	}
	
	_Myt& trimLeft(const _Myt& target){
		erase(0, find_first_not_of(target));
		return (*this);
	}
	
	_Myt& trimRight(const _Myt& target){
		erase(find_last_not_of(target)+1);
		return (*this);
	}

	// operator+=
	_Myt& operator+=(const _Myt& _right) {
		_Mybase::operator+=(_right);
		return (*this);
	}

	_Myt& operator+=(_Myt&& _right) {
		(*this) = _STD move(*this) + _STD move(_right);
		return (*this);
	}

	_Myt& operator+=(_Mybase&& _right) {
		(*this) = _STD move(*this) + _STD move(_right);
		return (*this);
	}

	// operator+= arithmetic type, eg: int float double
	template<typename _T,
			class = typename _STD enable_if<
				_STD is_arithmetic<_T>::value>::type >
	_Myt& operator+=(const _T& t) {
		(*this) += ex_tostring<_Elem, _Traits, _Alloc, _ExFormat>(t);
		return (*this);
	}

	/*** operator+ ***/
	template<
		class _T,
		class = typename _STD enable_if<
			_STD is_arithmetic<_T>::value
			&& !_STD is_same<_T, char>::value  // this is so stupid, is_same<_T, _Elem> not work
			&& !_STD is_same<_T, wchar_t>::value, void>::type>
	friend INLINE
	basic_exstring<_Elem, _Traits, _Alloc, _ExFormat> operator+ (
		const basic_exstring<_Elem, _Traits, _Alloc, _ExFormat>& _left,
		const _T& _right)
	{
		basic_exstring<_Elem, _Traits, _Alloc, _ExFormat> _Ans;
		basic_exstring<_Elem, _Traits, _Alloc, _ExFormat> _Tmp
			= ex_tostring<_Elem, _Traits, _Alloc, _ExFormat>(_right);
		_Ans.reserve(_left.size() + _Tmp.size());
		_Ans += _left;
		_Ans += _Tmp;
		return _Ans;
	}

	template<
		class _T,
		class = typename _STD enable_if<
			_STD is_arithmetic<_T>::value
			&& !_STD is_same<_T, char>::value
			&& !_STD is_same<_T, wchar_t>::value, void>::type>
	friend INLINE
	basic_exstring<_Elem, _Traits, _Alloc, _ExFormat> operator+ (
		const _T& _left,
		const basic_exstring<_Elem, _Traits, _Alloc, _ExFormat>& _right)
	{
		basic_exstring<_Elem, _Traits, _Alloc, _ExFormat> _Ans;
		basic_exstring<_Elem, _Traits, _Alloc, _ExFormat> _Tmp
			= ex_tostring<_Elem, _Traits, _Alloc, _ExFormat>(_left);
		_Ans.reserve(_right.size() + _Tmp.size());
		_Ans += _Tmp;
		_Ans += _right;
		return _Ans;
	}

	template<
		class _T,
		class = typename _STD enable_if<
			_STD is_arithmetic<_T>::value
			&& !_STD is_same<_T, char>::value
			&& !_STD is_same<_T, wchar_t>::value, void>::type>
	friend INLINE
	basic_exstring<_Elem, _Traits, _Alloc, _ExFormat> operator+ (
		basic_exstring<_Elem, _Traits, _Alloc, _ExFormat>&& _left,
		const _T& _right)
	{
		return (_STD move(_STD move(_left)
			+ ex_tostring<_Elem, _Traits, _Alloc, _ExFormat>(_right)));
	}

	template<
		class _T,
		class = typename _STD enable_if<
			_STD is_arithmetic<_T>::value
			&& !_STD is_same<_T, char>::value
			&& !_STD is_same<_T, wchar_t>::value, void>::type>
	friend INLINE
	basic_exstring<_Elem, _Traits, _Alloc, _ExFormat> operator+ (
		const _T& _left,
		basic_exstring<_Elem, _Traits, _Alloc, _ExFormat>&& _right)
	{
		return (_STD move(ex_tostring<_Elem, _Traits, _Alloc, _ExFormat>(_left)
			+ _STD move(_right)));
	}

}; // basic_exstring end

// exstring
typedef basic_exstring<char, char_traits<char>,
	allocator<char>, ex_format<char> > exstring;

typedef basic_exstring<wchar_t, char_traits<wchar_t>,
	allocator<wchar_t>, ex_format<wchar_t> > exwstring;

#if defined(UNICODE) || defined(_UNICODE)
typedef extwstring	tstring;
#else
typedef exstring	tstring;
#endif

_STD_END

#endif //__TSTRING_H_INCLUDE
