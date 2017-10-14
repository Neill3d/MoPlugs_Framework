
// graph.h
/**
	bi-direction graph
    Copyright (C) 2010 Sergey Solokhin (neill3d), e-mail to: s@neill3d.com
	home page - neill3d.com

    GitHub page - https://github.com/Neill3d/MoPlugs_Framework
	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
*/

#pragma	once
/*
	graph holds bi-connected struct with edges and vertices
	edges are sorted by distance from point
	this is a graph of 3d points, so each vertex is a 3d point in space
	points cloud point or rigid body point

	T - element for graph vertex, such elements will be connected by edges
	and sorted in needed order
	when we add vertices, we have to make connections between them. These connection are stored in
	edges vars
	how we could quickly iterate through graph vertices and edges ?
		we could create connectors matrix and use this one for iterators... so each time data is changed,
		graph needs to be updated
	we need also additional table for the angles between edges
	vertex iterator - go through vertices,
		edge iterator - go through vertex edges, go through all avaliable edges
*/

#ifndef	UCHAR
#define UCHAR	unsigned char
#endif

template	<class T>
class	Graph {

public:
	//!	a constructor
	Graph<T>() {
		numVertices = numEdges = 0;
		vertices = NULL;
		connectivity = NULL;
		table_edges = NULL;
	}
	//! a destructor
	~Graph<T>() {
		Clear();
	}

	class Vertex {
		friend class Graph<T>;
	protected:
		T		_value;	// each graph vertex value
	};

	enum {
		EDGE_NONE,
		EDGE_EQUAL,
		EDGE_U_TO_U,
		EDGE_V_TO_U,
		EDGE_U_TO_V,
		EDGE_V_TO_V
	};

	class	Edge {
	public:

		char	_type;	// connection edge type
		int	_u;		// first vertex
		int	_v;		// second vertex
		Graph<T>	*_pointer;		// connection edge table

		Edge() {
			_u = 0;
			_v = -1;	// used in foreach algorithm
			_pointer = NULL;
		}
		Edge(const Graph<T> *const& pointer)
			: _u(0)
			, _v(-1)
		{
			_pointer = (Graph<T>*) pointer;
		}
		Edge(const int u, const int v, Graph<T> *pointer)
			: _u(u)
			, _v(v)
			, _pointer(pointer)
		{}
		Edge(const int u, const int v, const Graph<T> *const& pointer)
			: _u(u)
			, _v(v)
		{
			_pointer = (Graph<T>*) pointer;
		}
		Edge	&operator = (const Edge &edge)
		{
			_u = edge._u;
			_v = edge._v;
			_pointer = edge._pointer;
			return *this;
		}

		int	u() {
			return _u;
		}
		int u() const {
			return _u;
		}
		int v() {
			return _v;
		}
		int v() const {
			return _v;
		}

		const char GetType() {
			return _type;
		}

		static	Edge	Empty() {
			return Edge(0,-1, NULL);
		}

		bool operator==(const Edge	&_a) {return ((_u==_a._u)&&(_v==_a._v)&&(_pointer==_a._pointer));}
		bool operator!=(const Edge	&_a) {return ((_u!=_a._u)||(_v!=_a._v)||(_pointer!=_a._pointer));}
		friend bool operator==(const Edge	&_a, const Edge	&_b) {
			return ((_a._u == _b._u)&&(_a._v==_b._v)&&(_a._pointer==_b._pointer));}
		friend bool operator!=(const Edge	& _a, const Edge	&_b) {
			return ((_a._u != _b._u)||(_a._v!=_b._v)||(_a._pointer!=_b._pointer));}

		// classify two edges
		friend int operator	& (const Edge	&_a, const Edge &_b) {
			if (_a._u == _b._u)
			{
				if (_a._v == _b._v)	return	EDGE_EQUAL;
				else return EDGE_U_TO_U;
			}
			else if (_a._u == _b._v) return EDGE_U_TO_V;
			else if (_a._v == _b._v) return EDGE_V_TO_V;
			else if (_a._v == _b._u) return EDGE_V_TO_U;

			return EDGE_NONE;
		}
	};

	// iterator for all vertex edges
	class VertexEdgeIterator
	{
		friend class Graph<T>;
	protected:
		mutable	int		_u;			// vertex u for the edge
		mutable	int		_v;			// vertex v for the edge
		mutable Graph<T>	*_pointer;		// a connectivity table pointer

	public:
		VertexEdgeIterator()
			: _u(0)
			, _v(0)
			, _pointer(NULL) {}
		VertexEdgeIterator(const int u, const int v, const Graph<T> *pointer)
			: _u(u)
			, _v(v)
		{
			_pointer = (Graph<T>*) pointer;
		}
		VertexEdgeIterator(const VertexEdgeIterator& _a) {*this = _a;}
		VertexEdgeIterator(Edge *const& _edge) {
			_u = (int) _edge->_u;
			_v = (int) _edge->_v;
			_pointer = (Graph<T>*) _edge->_pointer;
		}
		//! a destructor
		~VertexEdgeIterator() {}

		const VertexEdgeIterator& operator=(const VertexEdgeIterator& _a) const {
			_u=_a._u; _v=_a._v;
			_pointer = _a._pointer;
			return *this;
		}
		const VertexEdgeIterator& operator=(Edge *const& _edge) const {
			_u = _edge->_u;	_v=_edge->_v;
			_pointer = _edge->_pointer;
			return *this;
		}
		const VertexEdgeIterator& operator=(Edge *& _edge) {
			_u = _edge->_u;	_v=_edge->_v;
			_pointer = _edge->_pointer;
			return *this;
		}

		const Edge operator*() const { Edge	edge(_u, _v, _pointer); return edge;}
		const Edge operator->() const { Edge edge(_u, _v, _pointer);  return edge;}
		Edge operator*() {Edge	edge(_u, _v, _pointer); return edge;}
		Edge operator->() {Edge	edge(_u, _v, _pointer); return edge;}

		const VertexEdgeIterator& operator++() const {
			if (!_pointer) return *this;
			_v = _pointer->nextVertexEdge(_u, _v);
			/*
			if (_v == -1) {
				_u = _v = 0;
				_pointer = NULL;
				return *this;
			}
			*/
			else return *this;
		}
		const VertexEdgeIterator operator++(int) const {VertexEdgeIterator _tmp(*this);++(*this);return _tmp;}
		VertexEdgeIterator& operator++() {
			if (!_pointer) return *this;
			_v = _pointer->nextVertexEdge(_u, _v);
			/*
			if (_v == -1) {
				_u = _v = 0;
				_pointer = NULL;
				return *this;
			}
			*/
			return *this;
		}
		VertexEdgeIterator operator++(int) {VertexEdgeIterator _tmp(*this);++(*this);return _tmp;}

		const VertexEdgeIterator& operator--() const {
			if (!_pointer) return *this;
			_v = _pointer->prevVertexEdge(_u, _v);
			/*
			if (_v == -1) {
				_u = _v = 0;
				_pointer = NULL;
				return *this;
			}
			*/
			return *this;
		}
		const VertexEdgeIterator operator--(int) const {VertexEdgeIterator _tmp(*this);--(*this);return _tmp;}
		VertexEdgeIterator& operator--() {
			if (!_pointer) return *this;
			_v = _pointer->prevVertexEdge(_u, _v);
			/*
			if (_v == -1) {
				_u = _v = 0;
				_pointer = NULL;
				return *this;
			}
			*/
			return *this;
		}
		VertexEdgeIterator operator--(int) {VertexEdgeIterator _tmp(*this);--(*this);return _tmp;}

		bool operator==(const VertexEdgeIterator& _a) const {return ( (_u == _a._u) && (_v == _a._v));}
		bool operator!=(const VertexEdgeIterator& _a) const {return ( (_u != _a._u) || (_v != _a._v));}
		/*
		bool operator==(const Edge*const& _a) const {return (_pointer == _a);}
		bool operator!=(const Edge*const& _a) const {return (_pointer != _a);}
		friend bool operator==(const Edge*const& _a, const iterator& _b) {return (_a == _b._pointer);}
		friend bool operator!=(const Edge*const& _a, const iterator& _b) {return (_a != _b._pointer);}
*/
		// edges intersect (in v vertex)
		friend bool	operator	& (const VertexEdgeIterator	&_a, const VertexEdgeIterator &_b) {
			return (_a._v == _b._v);
		}
	};

public:
	// create a bi-edge between u and v
	//! white edge value (length number from rb system)
	bool	Connect(long u, long v, const char val=1);
	//! break connection beetween u and v
	bool	Disconnect(long u, long v);
	//! break all connections between vertices
	bool	DisconnectAll();
	// is u and v connected ?
	inline bool	IsConnected(const long &u, const long &v) {
		//if (!connectivity) return false;
		return (connectivity[u * numVertices + v] > 0);
	}
	inline bool	IsConnected(const long &u, const long &v) const {
		//if (!connectivity) return false;
		return (connectivity[u * numVertices + v] > 0);
	}
	inline	char	ConnectType(const long &u, const long &v) {
		return (connectivity[u * numVertices + v]);
	}
	inline	char	ConnectType(const long &u, const long &v) const {
		return (connectivity[u * numVertices + v]);
	}
	inline	char	ConnectType(const Edge	&edge) {
		return (connectivity[edge._u * numVertices + edge._v]);
	}
	inline	char	ConnectType(const Edge	&edge) const {
		return (connectivity[edge._u * numVertices + edge._v]);
	}

	// find next edge in table
	bool	nextEdge(Edge &edge, UCHAR filter);
	bool	nextEdge(Edge &edge, UCHAR filter) const;

	// find next edge in table by diagonal index manner
	bool	nextEdge_diag(Edge &edge, UCHAR filter);
	// find next edge in table by diagonal index manner
	bool	nextEdge_diag(Edge &edge, UCHAR filter) const;

	int	nextVertexEdge(const int	&u,	const int &v);
	int	prevVertexEdge(const int	&u,	const int &v);

	// return edge list from connectivity table
	VertexEdgeIterator	beginVertexEdge(const int &u) {
		if (!connectivity) return VertexEdgeIterator(u, -1, this);
		int idx = 0;
		while(idx < numVertices) {
			if (IsConnected(u, idx) ) break;
			idx++;
		}
		if (idx < numVertices)
			return VertexEdgeIterator( u, idx, this );	// TODO missing pointer to the graph
		else
			return VertexEdgeIterator( u, -1, this);
	}
	VertexEdgeIterator	beginVertexEdge(const int &u) const {
		if (!connectivity) return VertexEdgeIterator(u,-1, this);
		int idx = 0;
		while(idx < numVertices) {
			if (IsConnected(u, idx) ) break;
			idx++;
		}
		if (idx < numVertices)
			return VertexEdgeIterator(u, idx, this );		// TODO		missing this pointer for the iterator
		else
			return VertexEdgeIterator(u,-1, this);
	}
	VertexEdgeIterator	endVertexEdge(const int &u) {
		return VertexEdgeIterator(u,-1, this);
	}
	VertexEdgeIterator	endVertexEdge(const int &u) const {
		return VertexEdgeIterator(u,-1, this);
	}

	// return iterator to the first edge
	VertexEdgeIterator	beginEdge() {
		if (!connectivity) return EdgeIterator();
	}
	VertexEdgeIterator	endEdge() {
		return EdgeIterator();
	}

	// go through each graph edge by VertexEdgeIterator
	Edge	begin() {
		return Edge(0,0, this);
	}
	Edge	begin() const {
		return Edge(0,0, this);
	}

	bool	foreach(Edge	&edge, const UCHAR filter=UCHAR_MAX) {
		return nextEdge(edge, filter);
	}
	bool	foreach(Edge	&edge, const UCHAR filter=UCHAR_MAX) const {
		return nextEdge(edge, filter);
	}
	// foreach edge by diagonal index manner
	bool	foreach_diag(Edge	&edge, const UCHAR filter=UCHAR_MAX) {
		return nextEdge_diag(edge, filter);
	}
	// foreach edge by diagonal index manner
	bool	foreach_diag(Edge	&edge, const UCHAR filter=UCHAR_MAX) const {
		return nextEdge_diag(edge, filter);
	}

	void Clear() {
		if (vertices) {
			delete [] vertices;
			vertices = NULL;
		}
		numVertices = 0;

		if (connectivity) {
			delete [] connectivity;
			connectivity = NULL;
		}
		numEdges = 0;

		if (table_edges) {
			delete [] table_edges;
			table_edges = NULL;
		}
	}

	// put in u,v table each edge index
	void	UpdateEdgesTable() {
		if (!connectivity || !table_edges) return;

		int n=0;
		for (int i=0; i<numVertices; i++)
			for (int j=0; j<numVertices; j++)
				if ( IsConnected(i, j) ) {
					table_edges[i*numVertices + j] = n;	// forw order
					table_edges[j*numVertices + i] = n;	// back order
					n++;
				}
	}
	inline int		GetEdgeIndex(const int u, const int v) {
		return table_edges[u * numVertices + v];
	}
	inline int		GetEdgeIndex(const int u, const int v) const {
		return table_edges[u * numVertices + v];
	}
	inline int		GetEdgeIndex(const Edge &edge) {
		return table_edges[edge.u() * numVertices + edge.v()];
	}
	inline int		GetEdgeIndex(const Edge &edge) const {
		return table_edges[edge.u() * numVertices + edge.v()];
	}

	bool	SetCount(int count) {
		if (!count) return false;
		Clear();
		// allocate graph vertices
		numVertices = count;
		vertices = new Vertex[count];
		if (!vertices) return false;

		// allocate connectivity table
		numEdges = 0;
		connectivity = new UCHAR[count*count];
		if (!connectivity) return false;
		memset( connectivity, 0, sizeof(UCHAR)*(count*count) );

		// allocate edges indices table
		table_edges = new int[count*count];
		if (!table_edges) return false;
		memset( table_edges, 0, sizeof(int)*(count*count) );
		return true;
	}
	int		GetCount() {
		return numVertices;
	}
	int		GetCount() const {
		return numVertices;
	}
	int		GetEdgesCount() {
		return numEdges;
	}
	int		GetEdgesCount() const {
		return numEdges;
	}
	void	SetVertex(int idx, T	value) {
		if (!vertices) return;
		vertices[idx]._value = value;
	}
	T		GetVertex(int idx) {
		return vertices[idx]._value;
	}
	T		GetVertex(int	idx) const {
		return vertices[idx]._value;
	}
	T	&operator [] (int idx) {
		return vertices[idx]._value;
	}

private:
	int					numVertices;
	Vertex			*vertices;		// graph vertices array
	int					numEdges;
	UCHAR				*connectivity;	// graph edges between each vertex (table of n*n)
	int					*table_edges;		// table for converting u,v to edge index
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////// IMPLEMENTION


// create a bi-edge between u and v
//! white edge value (length number from rb system)
template	<class T>
bool	Graph<T>::Connect(long u, long v, const char val=1) {
	if (!connectivity) return false;
	UCHAR *value = &connectivity[u * numVertices + v];
	UCHAR *value2 = &connectivity[v * numVertices + u];
	if ( (*value == 0) || (*value2 == 0)) {
		*value = val;
		*value2 = val;
		if (table_edges)	{
			table_edges[u * numVertices + v] = numEdges;	// hold edge index (forw order)
			table_edges[v * numVertices + u] = numEdges;	// back order
		}
		numEdges++;
	} else {
		// combine connections (with several type)
		*value = *value | val;
		*value2 = *value2 | val;
	}
	return true;
}
//! break connection beetween u and v
template	<class T>
bool	Graph<T>::Disconnect(long u, long v) {
	if (!connectivity) return false;
	UCHAR *value = &connectivity[u * numVertices + v];
	UCHAR *value2 = &connectivity[v * numVertices + u];
	if ( (*value > 0) || (*value2 > 0) ) {
		*value = 0;
		*value2 = 0;
		numEdges--;
	}
	return true;
}
//! break all connections between vertices
template	<class T>
bool	Graph<T>::DisconnectAll() {
	if (!connectivity) return false;
	memset(connectivity, 0, sizeof(UCHAR)*(numVertices * numVertices));
	return true;
}


// find next edge in table
template	<class T>
bool	Graph<T>::nextEdge(Edge &edge, UCHAR filter) {
	if (!connectivity)	return false;
	int u_idx = edge._u;
	int v_idx = edge._v+1;
	while (u_idx < numVertices) {
		while(v_idx < numVertices) {
			if (IsConnected(u_idx, v_idx) )	{
				char type = ConnectType(u_idx, v_idx);
				if ((type & filter) == type) {
					edge._u = u_idx;
					edge._v = v_idx;
					return true;
				}
			}
			v_idx++;
		}
		v_idx = 0;
		u_idx++;
	}
	edge._u = 0;
	edge._v = -1;
	return false;		// nothing avaliable
}

template	<class T>
bool	Graph<T>::nextEdge(Edge &edge, UCHAR filter) const {
	if (!connectivity)	return false;
	int u_idx = edge._u;
	int v_idx = edge._v+1;
	while (u_idx < numVertices) {
		while(v_idx < numVertices) {
			if (IsConnected(u_idx, v_idx) )	{
				char type = ConnectType(u_idx, v_idx);
				if ((type & filter) == type) {
					edge._u = u_idx;
					edge._v = v_idx;
					return true;
				}
			}
			v_idx++;
		}
		v_idx = 0;
		u_idx++;
	}
	edge._u = 0;
	edge._v = -1;
	return false;		// nothing avaliable
}

// find next edge in table by diagonal index manner
template	<class T>
bool	Graph<T>::nextEdge_diag(Edge &edge, UCHAR filter) {
	if (!connectivity)	return false;

	int u_idx = edge._u;
	int v_idx = edge._v;

	int b = u_idx + v_idx;
	int n = (numVertices-1) * 2 + 1;
	// for each diagonal
	while (b <= n)
	{
		// increase u, v
		if (v_idx == 0) {
			// go next diag line
			b+=1;
			u_idx = 0;
			//v_idx = (b < numEdges) ? b : (numEdges-1);	// crop to table ranges
			v_idx = (b < numVertices) ? b : numVertices-1;
		}
		else
		{
			u_idx++;
			v_idx--;
		}

		// check current u,v
		if (IsConnected(u_idx, v_idx) ) {
			char type = ConnectType(u_idx, v_idx);
			if ((type & filter) == type) {
				edge._u = u_idx;
				edge._v = v_idx;
				return true;
			}
		}
	}
	return false;		// nothing avaliable
}

// find next edge in table by diagonal index manner
template	<class T>
bool	Graph<T>::nextEdge_diag(Edge &edge, UCHAR filter) const {
	if (!connectivity)	return false;

	int u_idx = edge._u;
	int v_idx = edge._v;

	int b = u_idx + v_idx;
	int n = (numVertices-1) * 2 + 1;
	// for each diagonal
	while (b <= n)
	{
		// increase u, v
		if (v_idx == 0) {
			// go next diag line
			b+=1;
			u_idx = 0;
			//v_idx = (b < numEdges) ? b : (numEdges-1);	// crop to table ranges
			v_idx = numVertices-1;
		}
		else
		{
			u_idx++;
			v_idx--;
		}

		// check current u,v
		if (IsConnected(u_idx, v_idx) ) {
			char type = ConnectType(u_idx, v_idx);
			if ((type & filter) == type) {
				edge._u = u_idx;
				edge._v = v_idx;
				return true;
			}
		}
	}
	return false;		// nothing avaliable
}

template	<class T>
int	Graph<T>::nextVertexEdge(const int	&u,	const int &v)
{
	if (!connectivity)	return -1;
	int idx = v+1;
	while (idx < numVertices) {
		if (IsConnected(u, idx) )
			return idx;
		idx++;
	}
	return -1;		// nothing avaliable
}
template	<class T>
int	Graph<T>::prevVertexEdge(const int	&u,	const int &v)
{
	if (!connectivity)	return -1;
	int idx = v-1;
	while (idx >= 0) {
		if (IsConnected(u, idx) )
			return idx;
		idx--;
	}
	return -1;		// nothing avaliable
}
