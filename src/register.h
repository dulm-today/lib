#ifndef __REGISTE_H__
#define __REGISTE_H__

#include <vector>
#include <algorithm>

template <class I, class V>
struct reg_data{
	I	_id;
	V	_value;

	bool operator<(const reg_data<I, V>& right){
		return _id < right._id;
	}
};

template <class I, class V>
class registe{
public:
	registe(I id, V v);
	~registe();
	I id() { return _id; }
	
private:
	I _id;

public:	
	static std::vector<reg_data<I, V>>* object();
	static void add(I id, V v);
	static void del(I id);
	static size_t index(I id);
	static V* search(I id);
	static size_t num();
	static V* get(size_t index);

	static std::vector<reg_data<I, V>>* list;
	static bool sort;
};

template <class I, class V>
std::vector<reg_data<I, V>>* registe<I,V>::list = NULL;

template <class I, class V>
bool registe<I,V>::sort = true;

template <class I, class V>
registe<I,V>::registe(I id, V v):_id(id){
	registe<I,V>::add(id, v);
}

template <class I, class V>
registe<I,V>::~registe(){
	registe<I,V>::del(_id);
}

template <class I, class V>
std::vector<reg_data<I, V>>* registe<I, V>::object(){
	if (NULL == registe<I, V>::list)
		registe<I, V>::list = new std::vector<reg_data<I, V>>;
	return registe<I, V>::list;
}

template <class I, class V>
void registe<I, V>::add(I id, V v){
	if (NULL == registe<I,V>::object())
		return;

	if ((size_t)-1 != registe<I, V>::index(id))
		return;

	reg_data<I, V> t;
	t._id = id; t._value = v;
	registe<I,V>::list->push_back(t);
	
	if (registe<I,V>::sort)
		std::sort(registe<I,V>::list->begin(), registe<I,V>::list->end());
}

template <class I, class V>
void registe<I, V>::del(I id){
	
	size_t i = registe<I, V>::index(id);
	if ((size_t)-1 != i){
		registe<I,V>::list->erase(registe<I,V>::list->begin()+i);
	}

	if (0 == registe<I,V>::list->size()){
		delete registe<I,V>::list;
		registe<I,V>::list = NULL;
	}
}

template <class I, class V>
size_t registe<I, V>::index(I id)
{
	if (NULL == registe<I,V>::object())
		return (size_t)-1;

	if (registe<I,V>::sort){
		size_t left = 0, right = registe<I,V>::list->size();
		size_t middle;

		while(left < right){
			middle = left + ((right-left) >> 1);
			reg_data<I,V>& ref = (*registe<I,V>::list)[middle];
			if (ref._id < id)
				left = middle+1;
			else if (ref._id > id)
				right = middle;
			else
				return middle;
		}
	}
	else{
		size_t size = registe<I,V>::list->size();
		for (size_t i = 0; i < size; ++i){
			reg_data<I,V>& ref = (*registe<I,V>::list)[i];
			if (ref._id == id)
				return i;
		}
	}
	return (size_t)-1;
}


template <class I, class V>
V* registe<I, V>::search(I id)
{
	size_t i = registe<I,V>::index(id);
	if ((size_t)-1 == i)
		return NULL;

	return &(*registe<I,V>::list)[i]._value;
}

template <class I, class V>
size_t registe<I, V>::num(){
	if (NULL == registe<I,V>::object())
		return 0;
	return registe<I,V>::list->size();
}

template <class I, class V>
V* registe<I, V>::get(size_t index){
	if (NULL == registe<I,V>::object())
		return NULL;

	if (index >= registe<I,V>::list->size())
		return NULL;
	
	return &(*registe<I,V>::list)[index]._value;
}


#endif //__REGISTE_H__

