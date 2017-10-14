/**
	list
    Copyright (C) 2010 Sergey Solokhin (neill3d), e-mail to: s@neill3d.com
	home page - neill3d.com

        GitHub page - https://github.com/Neill3d/MoPlugs_Framework
	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
*/
#ifndef	LIST_H_
#define	LIST_H_

#include <stdio.h>
#include <assert.h>

// template class for a list structure

// one way list and bi-directional list classes implemention

class	ListNode
{
public:
	ListNode()
		: _next(NULL)
		, _prev(NULL)
	{}

	const	ListNode *GetLeft() {
		return _prev;
	}
	const ListNode *GetRight() {
		return _next;
	}
	void	Disconnect() {
		if (_next)
			_next->_prev = NULL;
		_next = NULL;
		if (_prev)
			_prev->_next = NULL;
		_prev = NULL;
	}
	void	ConnectLeft(ListNode *left) {
		_prev = left;
		left->_next = this;
	}
	void	ConnectRight(ListNode *right) {
		_next = right;
		right->_prev = this;
	}

protected:
	ListNode	*_next;
	ListNode	*_prev;
};

// bi-direction list
template <class _t>
class bilist
{
	struct _node
	{
		_t _value;
		_node* _next;
		_node* _prev;

		_node() {}
		_node(const _node& _a) {*this = _a;}
		const _node& operator=(const _node& _a) const {_value=_a._value;_next=_a._next; _prev=_a._prev;}
		~_node() {}

		void	disconnect() {
			_value->Disconnect();
		}
		void  update() {
			_value->Disconnect();
			if (_prev)
				_value->ConnectLeft(_prev->_value);
			if (_next)
				_value->ConnectRight(_next->_value);
		}
	};

	_node* _front;
	_node* _back;
	unsigned int _size;

public:
	bilist<_t>(): _front(NULL), _back(NULL), _size(0) {}
	bilist<_t>(const bilist<_t>& _a): _front(NULL), _back(NULL), _size(0) {*this = _a;}
	bilist<_t>& operator=(const bilist<_t>& _a){
		clear();
		_node* _tmp = _a._front;
		while (true){
			if (_tmp == NULL) return *this;
			push_back(_tmp->_value);
			_tmp = _tmp->_next;}}
	~bilist<_t>() {clear();}

	class iterator
	{
		friend class bilist<_t>;
	protected:
		mutable _node* _pointer;

	public:
		iterator(): _pointer(NULL) {}
		iterator(const iterator& _a) {*this = _a;}
		iterator(_node*const& _a): _pointer(_a) {}
		~iterator() {}

		const iterator& operator=(const iterator& _a) const {_pointer = _a._pointer;return *this;}
		const iterator& operator=(const _node*const& _a) const {_pointer = (_node*)_a;return *this;}
		const iterator& operator=(_node*& _a) {_pointer = _a;return *this;}

		const _t& operator*() const {return _pointer->_value;}
		const _t* operator->() const {return &_pointer->_value;}
		_t& operator*() {return _pointer->_value;}
		_t* operator->() {return &_pointer->_value;}

		const iterator& operator++() const {_pointer = _pointer->_next;return *this;}
		const iterator operator++(int) const {iterator _tmp(*this);++(*this);return _tmp;}
		iterator& operator++() {_pointer = _pointer->_next;return *this;}
		iterator operator++(int) {iterator _tmp(*this);++(*this);return _tmp;}

		const iterator& operator--() const {_pointer = _pointer->_prev;return *this;}
		const iterator operator--(int) const {iterator _tmp(*this);--(*this);return _tmp;}
		iterator& operator--() {_pointer = _pointer->_prev;return *this;}
		iterator operator--(int) {iterator _tmp(*this);--(*this);return _tmp;}

		bool operator==(const iterator& _a) const {return (_pointer == _a._pointer);}
		bool operator!=(const iterator& _a) const {return (_pointer != _a._pointer);}
		bool operator==(const _node*const& _a) const {return (_pointer == _a);}
		bool operator!=(const _node*const& _a) const {return (_pointer != _a);}
		friend bool operator==(const _node*const& _a, const iterator& _b) {return (_a == _b._pointer);}
		friend bool operator!=(const _node*const& _a, const iterator& _b) {return (_a != _b._pointer);}
	};

	void push_front(const _t& _a);//put value at front
	void push_back(const _t& _a);//put value at back
	void pop_front();//remove front element
	void pop_back();//remove back element
	const iterator begin() const {return iterator(_front);}//constant iterator
	iterator begin() {return iterator(_front);}//non-constant iterator
	const _t& front() const {return _front->_value;}//consant reference
	_t& front() {return _front->_value;}//non-constant reference
	const iterator end() const {return /*iterator(_back)++*/ iterator(NULL);}//constant iterator	(return next to _back)
	iterator end() {return /*iterator(_back)++*/ iterator(NULL);}//non-constant iterator	(return next to _back)
	const _t& back() const {return _back->_value;}//constant reference
	_t& back() {return _back->_value;}//non-constant reference
	unsigned int size() const {return _size;}//size of list
	bool empty() const {return (!_size);}//is list empty?
	void clear() {while (!empty()) pop_front();}//erase all the contents of the list
	void remove(const _t& _a);//remove all elements with values equal to argument
	void erase(iterator &_a); //erases iterator from list

	iterator	find(const _t& _a) // find iterator from list
	{
		iterator node_it = begin();
		while( node_it != end() )
		{
			if( (*node_it) == _a)
			{
				return node_it;
			}
			node_it++;
		}
		return end();
	}
	void insert(iterator &_a, const _t& _b);//insert _b in front of _a
	void insert_after(iterator& _a, const _t& _b);//insert _b after _a
	void swap(bilist<_t>& _a){//swaps this list with argument
		_node* _tmp;_tmp = _front;_front = _a._front;_a._front = _tmp;
		_tmp = _back;_back = _a._back;_a._back = _tmp;
		unsigned int _tmp2 = _size;_size = _a._size;_a._size = _tmp2;}
	void reverse(){//reverses order of elements
		bilist<_t> _new;
		while (!empty()){
			_new.push_front(front());
			pop_front();}
		swap(_new);}
};



// one way list
template <class _t>
class SimpleList
{
	struct _node
	{
		_t _value;
		_node* _next;

		_node() {}
		_node(const _node& _a) {*this = _a;}
		const _node& operator=(const _node& _a) const {_value=_a._value;_next=_a._next;}
		~_node() {}
	};

	_node* _front;
	_node* _back;
	unsigned int _size;

public:
	SimpleList<_t>(): _front(NULL), _back(NULL), _size(0) {}
	SimpleList<_t>(const SimpleList<_t>& _a): _front(NULL), _back(NULL), _size(0) {*this = _a;}
	SimpleList<_t>& operator=(const SimpleList<_t>& _a){
		clear();
		_node* _tmp = _a._front;
		while (true){
			if (_tmp == NULL) return *this;
			push_back(_tmp->_value);
			_tmp = _tmp->_next;}}
	~SimpleList<_t>() {clear();}

	class iterator
	{
		friend class SimpleList<_t>;
	protected:
		mutable _node* _pointer;

	public:
		iterator(): _pointer(NULL) {}
		iterator(const iterator& _a) {*this = _a;}
		iterator(_node*const& _a): _pointer(_a) {}
		~iterator() {}

		const iterator& operator=(const iterator& _a) const {_pointer = _a._pointer;return *this;}
		const iterator& operator=(const _node*const& _a) const {_pointer = (_node*)_a;return *this;}
		const iterator& operator=(_node*& _a) {_pointer = _a;return *this;}

		const _t& operator*() const {return _pointer->_value;}
		const _t* operator->() const {return &_pointer->_value;}
		_t& operator*() {return _pointer->_value;}
		_t* operator->() {return &_pointer->_value;}

		const iterator& operator++() const {_pointer = _pointer->_next;return *this;}
		const iterator operator++(int) const {iterator _tmp(*this);++(*this);return _tmp;}
		iterator& operator++() {_pointer = _pointer->_next;return *this;}
		iterator operator++(int) {iterator _tmp(*this);++(*this);return _tmp;}
/*
		const iterator& operator--() const {_pointer = _pointer->_prev;return *this;}
		const iterator operator--(int) const {iterator _tmp(*this);--(*this);return _tmp;}
		iterator& operator--() {_pointer = _pointer->_prev;return *this;}
		iterator operator--(int) {iterator _tmp(*this);--(*this);return _tmp;}
*/
		bool operator==(const iterator& _a) const {return (_pointer == _a._pointer);}
		bool operator!=(const iterator& _a) const {return (_pointer != _a._pointer);}
		bool operator==(const _node*const& _a) const {return (_pointer == _a);}
		bool operator!=(const _node*const& _a) const {return (_pointer != _a);}
		friend bool operator==(const _node*const& _a, const iterator& _b) {return (_a == _b._pointer);}
		friend bool operator!=(const _node*const& _a, const iterator& _b) {return (_a != _b._pointer);}
	};

	void push_front(const _t& _a);//put value at front
	void push_back(const _t& _a);//put value at back
	void pop_front();//remove front element
	void pop_back();//remove back element
	const iterator begin() const {return iterator(_front);}//constant iterator
	iterator begin() {return iterator(_front);}//non-constant iterator
	const _t& front() const {return _front->_value;}//consant reference
	_t& front() {return _front->_value;}//non-constant reference
	const iterator end() const {return /*iterator(_back)++*/ iterator(NULL);}//constant iterator	(return next to _back)
	iterator end() {return /*iterator(_back)++*/ iterator(NULL);}//non-constant iterator	(return next to _back)
	const _t& back() const {return _back->_value;}//constant reference
	_t& back() {return _back->_value;}//non-constant reference
	unsigned int size() const {return _size;}//size of list
	bool empty() const {return (!_size);}//is list empty?
	void clear() {while (!empty()) pop_front();}//erase all the contents of the list
	void remove(const _t& _a);//remove all elements with values equal to argument
	void erase(iterator &_a);//erases iterator from list

	iterator	find(const _t& _a) // find iterator from list
	{
		iterator node_it = begin();
		while( node_it != end() )
		{
			if( (*node_it) == _a)
			{
				return node_it;
			}
			node_it++;
		}
		return end();
	}
	void insert(iterator &_a, const _t& _b);//insert _b in front of _a
	void insert_after(iterator& _a, const _t& _b);//insert _b after _a
	void swap(SimpleList<_t>& _a){//swaps this list with argument
		_node* _tmp;_tmp = _front;_front = _a._front;_a._front = _tmp;
		_tmp = _back;_back = _a._back;_a._back = _tmp;
		unsigned int _tmp2 = _size;_size = _a._size;_a._size = _tmp2;}
	void reverse(){//reverses order of elements
		SimpleList<_t> _new;
		while (!empty()){
			_new.push_front(front());
			pop_front();}
		swap(_new);}
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////// IMPLEMENTION

// BI-DIRECTIONAL LIST

template <class _t>
void bilist<_t>::push_front(const _t& _a)
{
	if (!empty())
	{
		_node* _new = new _node;
		_new->_next = _front;
		_front->_prev = _new;
		_new->_prev = NULL;
		_front = _new;
	}
	else
	{
		_front = new _node;
		_front->_next = NULL;
		_front->_prev = NULL;
		_back = _front;
	}
	_front->_value = _a;
	_front->update();	// update block
	_size++;
}

template <class _t>
void bilist<_t>::push_back(const _t& _a)
{
	if (!empty())
	{
		_back->_next = new _node;
		_back->_next->_prev = _back;
		_back = _back->_next;
	}
	else
	{
		_front = new _node;
		_back = _front;
		_back->_prev = NULL;
	}
	_back->_value = _a;
	_back->_next = NULL;
	_back->update();		// update block
	_size++;
}

template <class _t>
void bilist<_t>::pop_front()
{
	if (empty()) return;
	if (_front == _back)
	{
		delete _front;
		_front = _back = NULL;
	}
	else
	{
		_node* _tmp = _front->_next;
		delete _front;
		_front = _tmp;
		_front->_prev = NULL;
		_front->update();	// update block
	}
	_size--;
}

template <class _t>
void bilist<_t>::pop_back()
{
	if (empty()) return;
	if (_front == _back)
	{
		delete _front;
		_front = _back = NULL;
	}
	else
	{
		_node* _tmp = _back->_prev;
		delete _back;
		_back = _tmp;
		_back->_next = NULL;
		_back->update();	// update block
	}
	_size--;
}

template <class _t>
void bilist<_t>::remove(const _t& _a)
{
	_node* _tmp = _front;
	_node* _tmp2 = _tmp;
	while (true)
	{
		if (_tmp == NULL) return;
		if (_tmp->_value == _a)
		{
			if (_tmp == _front)
			{
				pop_front();
				if (empty()) return;
				_tmp = _tmp2 = _front;
				_tmp = _tmp->_next;
			}
			else
			{
				_node *_tmp3 = _tmp->_next;
				_tmp2->_next = _tmp3;
				if (_tmp3) _tmp3._prev = _tmp2;
				if (_tmp == _back) _back = _tmp2;
				delete _tmp;
				_tmp = _tmp2->_next;
				_size--;
			}
			continue;
		}
		_tmp2 = _tmp;
		_tmp = _tmp->_next;
	}
}

template <class _t>
void bilist<_t>::erase(iterator &_a)
{
	if (_front == NULL) return;
	if (_a == _front)
	{
		pop_front();
		_a = _front;
	}
	else if (_a == _back)
	{
		pop_back();
		_a = NULL;	// set to end()
	}
	else
	{
		_node* _tmp = _a._pointer;
		_node* _tmp2 = _tmp->_prev;
		_tmp = _tmp->_next;

		_tmp2->_next = _tmp;
		_tmp->_prev = _tmp2;

		_tmp2 = _a._pointer;
		delete _tmp2;
		_a = _tmp;
		if (_a != end() )	(_a._pointer)->update();	// update block
		_size--;
	}
}

template <class _t>
void bilist<_t>::insert(iterator &_a, const _t& _b)
{
	if (_front == NULL) {
		_ASSERT( _a == end() );
		push_back(_b);
	}
	else
	if (_a == _front)
		push_front(_b);
	else
	{
		_ASSERT( _a != end() );
		_node *_tmp = _a._pointer;
		_tmp = _tmp->_prev;
		_node *_tmp2 = _a._pointer;
		_tmp2->_prev = new _node;
		_tmp2 = _tmp2->_prev;
		_tmp->_next = _tmp2;
		_tmp2->_next = _a._pointer;
		_tmp2->_prev = _tmp;
		_tmp2->_value = _b;
		_tmp2->update();
		_size++;
	}
}

template <class _t>
void bilist<_t>::insert_after(iterator& _a, const _t& _b)
{
	if (_front == NULL) {
		_ASSERT( _a == end() );
		push_back(_b);
	}
	else
	if (_a == _back)
		push_back(_b);
	else
	{
		_ASSERT( _a != end() );
		_node *tmp = _a._pointer;
		tmp = tmp->_next;
		_node *tmp2 = _a._pointer;
		tmp2->_next = new _node;
		tmp2 = tmp2->_next;
		tmp->_prev = tmp2;
		tmp2->_next = tmp;
		tmp2->_prev = _a._pointer;
		tmp2->_value = _b;
		tmp2->update();
		_size++;
	}
}


//
// one way list
//


template <class _t>
void SimpleList<_t>::push_front(const _t& _a)
{
	if (!empty())
	{
		_node* _new = new _node;
		_new->_next = _front;
		_front = _new;
	}
	else
	{
		_front = new _node;
		_front->_next = NULL;
		_back = _front;
	}
	_front->_value = _a;
	_size++;
}

template <class _t>
void SimpleList<_t>::push_back(const _t& _a)
{
	if (!empty())
	{
		_back->_next = new _node;
		_back = _back->_next;
	}
	else
	{
		_front = new _node;
		_back = _front;
	}
	_back->_value = _a;
	_back->_next = NULL;
	_size++;
}

template <class _t>
void SimpleList<_t>::pop_front()
{
	if (empty()) return;
	if (_front == _back)
	{
		delete _front;
		_front = _back = NULL;
	}
	else
	{
		_node* _tmp = _front->_next;
		delete _front;
		_front = _tmp;
	}
	_size--;
}

template <class _t>
void SimpleList<_t>::pop_back()
{
	if (empty()) return;
	if (_front == _back)
	{
		delete _front;
		_front = _back = NULL;
	}
	else
	{
		_node* _tmp = _front;
		while (_tmp->_next != _back)
			_tmp = _tmp->_next;

		delete _back;
		_back = _tmp;
		_back->_next = NULL;
	}
	_size--;
}

template <class _t>
void SimpleList<_t>::remove(const _t& _a)
{
	_node* _tmp = _front;
	_node* _tmp2 = _tmp;
	while (true)
	{
		if (_tmp == NULL) return;
		if (_tmp->_value == _a)
		{
			if (_tmp == _front)
			{
				pop_front();
				if (empty()) return;
				_tmp = _tmp2 = _front;
				_tmp = _tmp->_next;
			}
			else
			{
				_node *_tmp3 = _tmp->_next;
				_tmp2->_next = _tmp3;
				if (_tmp3) _tmp3._prev = _tmp2;
				if (_tmp == _back) _back = _tmp2;
				delete _tmp;
				_tmp = _tmp2->_next;
				_size--;
			}
			continue;
		}
		_tmp2 = _tmp;
		_tmp = _tmp->_next;
	}
}

template <class _t>
void SimpleList<_t>::erase(iterator &_a)
{
	if (_front == NULL) return;
	if (_a == _front)
	{
		pop_front();
		_a = _front;
	}
	else if (_a == _back)
	{
		pop_back();
		_a = NULL;	// set to end()
	}
	else
	{

		_node* _tmp = _front;
		while (_tmp->_next != _a)
			_tmp = _tmp->_next;

		_node* _tmp2 = _tmp->_next;
		_tmp->_next = _tmp2->_next;
		delete _tmp2;
		_a = _tmp->_next;
		_size--;
	}
}

// insert _b before _a
template <class _t>
void SimpleList<_t>::insert(iterator &_a, const _t& _b)
{
	if (_front == NULL) {
		assert( _a == end() );
		push_back(_b);
	}
	else
	if (_a == _front)
		push_front(_b);
	else
	{
		_node *_tmp = _front;
		while (_tmp->_next != _a)
			_tmp = _tmp->_next;

		_node	*_tmp2 = new _node;
		_tmp->_next = _tmp2;
		_tmp2->_next = _a._pointer;
		_tmp2->_value = _b;
		_size++;
	}
}

// insert after _a, this is prefer for a SimpleList
template <class _t>
void SimpleList<_t>::insert_after(iterator& _a, const _t& _b)
{
	if (_front == NULL) {
		_ASSERT( _a == end() );
		push_back(_b);
	}
	else
	if (_a == _back)
		push_back(_b);
	else
	{
		_node *_tmp = new _node;
		_tmp._value = _b;
		_node *_tmp2 = _a;
		_node *_tmp3 = _tmp2->_next;
		_tmp2->_next = _tmp;
		_tmp->_next = _tmp3;
		_size++;
	}
}



#endif
