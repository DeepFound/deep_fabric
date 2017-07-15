///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////

#include "txtree.h"
#include "txstring.h"

TX_DEFINE_PARENT_TYPE(txTree,txObject)

void txTree::_initialize_ (int order)
{
	if (order < 3)
	{
		order = 3;
	}

	_root_ = 0;
	_entries_ = 0;
	_branch_max_index_ = order;
	_branch_low_water_ = (order - 1) / 2;
	_leaf_max_index_ = (2 * (order + 1)) - 1;
	_leaf_low_water_ = ((_leaf_max_index_ + 1) - 1) / 2 - 1;
}

void txTree::_notifyRootFull_ (void)
{
	txBtNode* oldroot = _root_;

	#if !defined OSTORE_SUPPORT
		_root_ = new txBtBranch(0, this, oldroot);
	#else
		if (TX_IS_OBJECT_PERSISTENT(this))
		{
			_root_ = new(TX_GET_OBJECT_CLUSTER(this),
				txBtBranch::get_os_typespec())
					txBtBranch(0, this, oldroot);
		}
		else
		{
			_root_ = new txBtBranch(0, this, oldroot);
		}
	#endif

	oldroot->splitNode();
}

void txTree::_notifyRootEmpty_ (void)
{
	if (_root_->_is_leaf_)
	{
		txBtLeaf* lroot = (txBtLeaf*) _root_;
		delete lroot;
		_root_ = 0;
	}
	else
	{
		txBtBranch* iroot = (txBtBranch*) _root_;
		_root_ = iroot->getTree(0);
		_root_->_parent_ = 0;
		delete iroot;
	}
}

const txObject* txTree::insert (txObject* obj)
{
	if (!_root_)
	{
		#if !defined OSTORE_SUPPORT
			_root_ = new txBtLeaf(0, obj, this);
		#else
			if (TX_IS_OBJECT_PERSISTENT(this))
			{
				_root_ = new(TX_GET_OBJECT_CLUSTER(this),
					txBtLeaf::get_os_typespec())
						txBtLeaf(0, obj, this);
			}
			else
			{
				_root_ = new txBtLeaf(0, obj, this);
			}
		#endif

		_incrementEntries_();
	}
	else
	{
		int index;
		txBtNode* n;

		_root_->find(obj, &n, &index);

		n->insert(obj, index);
	}

	return obj;
}

const txObject* txTree::find (const txObject* obj) const
{
	if (!_root_)
	{
		return 0;
	}
	else
	{
		int index;
		txBtNode* n;
		return _root_->find(obj, &n, &index);
	}
}

const txObject* txTree::findBest (const txObject* obj) const
{
	if (!_root_)
	{
		return 0;
	}
	else
	{
		int index;
		txBtNode* n;
		return _root_->findBest(obj, &n, &index);
	}
}

int txTree::index (const txObject* obj) const
{
	if (!_root_)
	{
		return -1;
	}
	else
	{
		return _root_->findIndex(obj);
	}
}

txObject* txTree::remove (const txObject* obj)
{
	if (!_root_)
	{
		return 0;
	}

	int index;
	txBtNode* n;
	txObject* obj_found = _root_->find(obj, &n, &index);

	if (!obj_found)
	{
		return 0;
	}

	n->remove(index);

	if (entries() == 0)
	{
		clear();
	}

	return obj_found;
}

void txTree::removeAndDestroy (const txObject* obj)
{
	txObject* obj_found = remove(obj);

	delete obj_found; obj_found = 0;
}

void txTree::clear (void)
{
	delete _root_; _root_ = 0; _entries_ = 0;
}

void txTree::clearAndDestroy (void)
{
	int items = entries();

	for (int i = 0; i < items; i++)
	{
		delete (*_root_)[i];
	}

	clear();
}

void txTree::readObject (txIn& in)
{
	const char* end = in.cursor() + in.objectLength();

	signed short int flag; in >> flag;

	TX_SET_AUTODEL_FLAG(_flags_, (TX_AUTODEL_FLAG) flag);

	txObject* o;

	while (in.cursor() < end)
	{
		in >> o;

		insert(o);
	}
}

void txTree::writeObject (txOut& out) const
{
	txObject* node;
	unsigned int hdr = out.length();

	out.putNull();

	out << (signed short int) TX_GET_AUTODEL_FLAG(_flags_);

	txTreeIterator iter((txTree&) *this);

	while (node = (txObject*) iter.next())
	{
		out.put(node);
	}

	out.writeHeader(hdr, out.length() - (hdr + 4), TX_TREE);
}

txBtItem::txBtItem (void) : 
	_tree_entries_(0), _tree_(0), _key_(0)
{
}

txBtItem::txBtItem (txBtNode* n, txObject* obj)
{
	_tree_entries_ = n->entries() + 1;
	_tree_ = n;
	_key_ = obj;
}

txBtItem::txBtItem (txObject* obj, txBtNode* n)
{
	_tree_entries_ = n->entries() + 1;
	_tree_ = n;
	_key_ = obj;
}

txBtItem::~txBtItem (void)
{
}

txBtNode::txBtNode (int isleaf, txBtBranch* p, txTree* t)
{
	_last_ = -1;
	_is_leaf_ = isleaf;
	_parent_ = p;

	if (p == 0)
	{
		_tree_ = t;
	}
	else
	{
		_tree_ = p->_tree_;
	}
}

txTreeIterator::txTreeIterator (const txTree& t, int direction) :
	_tree_(&(txTree&) t), _cursor_(0), _direction_(direction)
{
	reset();
}

txTreeIterator::~txTreeIterator (void)
{
}

txObject* txTreeIterator::remove (void)
{
	int index;

	if (_direction_ == 1)
	{
		index = _cursor_ - 1; _cursor_--;
	}
	else
	{
		index = _cursor_ + 1; _cursor_++;
	}

	return _tree_ ? _tree_->remove(_tree_->at(index)) : 0;
}

void txTreeIterator::reset (const txTree& t, int direction)
{
	_tree_ = &(txTree&) t; _cursor_ = 0; _direction_ = direction;

	reset();
}

void txTreeIterator::reset (void)
{
	if (!_tree_) return;

	if (_direction_ == 1)
	{
		_cursor_ = 0;
	}
	else
	{
		_cursor_ = _tree_->entries() - 1;
	}
}

const txObject* txTreeIterator::next (void)
{
	if (!_tree_) return 0;

	if (_direction_ == 1)
	{
		if (_cursor_ < _tree_->entries())
		{
			return _tree_->at(_cursor_++);
		}
	}
	else
	{
		if (_cursor_ >= 0)
		{
			return _tree_->at(_cursor_--);
		}
	}

	return 0;
}

txBtBranch::txBtBranch (txBtBranch* prnt, txTree* tree) :
	txBtNode(0, prnt, tree)
{
	#if !defined OSTORE_SUPPORT
		_item_ = new txBtItem[maxIndex() + 1];
	#else
	if ((tree && TX_IS_OBJECT_PERSISTENT(tree)) ||
		(prnt && prnt->_tree_ && TX_IS_OBJECT_PERSISTENT(prnt->_tree_)))
	{
		_item_ = new(TX_GET_OBJECT_CLUSTER(this),
			txBtItem::get_os_typespec(), maxIndex() + 1)
				txBtItem[maxIndex() + 1];
	}
	else
	{
		_item_ = new txBtItem[maxIndex() + 1];
	}
	#endif
}

txBtBranch::txBtBranch (txBtBranch* prnt, txTree* tree, txBtNode* oldroot) :
	txBtNode(0, prnt, tree)
{
	#if !defined OSTORE_SUPPORT
		_item_ = new txBtItem[maxIndex() + 1];
	#else
	if ((tree && TX_IS_OBJECT_PERSISTENT(tree)) ||
		(prnt && prnt->_tree_ && TX_IS_OBJECT_PERSISTENT(prnt->_tree_)))
	{
		_item_ = new(TX_GET_OBJECT_CLUSTER(this),
			txBtItem::get_os_typespec(), maxIndex() + 1)
				txBtItem[maxIndex() + 1];
	}
	else
	{
		_item_ = new txBtItem[maxIndex() + 1];
	}
	#endif

	append(0, oldroot);
}

txBtBranch::~txBtBranch (void)
{
	if (_last_ > 0)
	{
		delete _item_[0]._tree_;
	}

	for (int i = 1; i <= _last_; i++)
	{
		delete _item_[i]._tree_;
	}

	delete [] _item_; _item_ = 0;
}

void txBtBranch::insert (const txObject* obj, int index)
{
	txBtLeaf* ln = getTree(index - 1)->lastLeafNode();

	ln->insert(obj, ln->_last_ + 1);
}

void txBtBranch::insertElement (txBtItem& itm, int at)
{
	for (int i = _last_ + 1; i > at; i--)
	{
		getItem(i) = getItem(i - 1);
	}

	setItem(at, itm);

	_last_++;
}

void txBtBranch::insertElement (int at, txObject* k, txBtNode* t)
{
	txBtItem newitem(k, t);

	insertElement(newitem, at);
}

void txBtBranch::insert (txBtItem& itm, int at)
{
	insertElement(itm, at);

	if (isFull())
	{
		notifyParent();
	}
}

void txBtBranch::insert (int at, txObject* k, txBtNode* t)
{
	txBtItem newitem(k, t);

	insert(newitem, at);
}

void txBtBranch::appendFrom (txBtBranch* src, int start, int stop)
{
	if (start > stop)
	{
		return;
	}

	for (int i = start; i <= stop; i++)
	{
		setItem(++_last_, src->getItem(i));
	}
}

void txBtBranch::append (txObject* d, txBtNode* n)
{
	setItem(++_last_, d, n);
}

void txBtBranch::append (txBtItem& itm)
{
	setItem(++_last_, itm);
}

void txBtBranch::balanceWithLeft (txBtBranch* l_node, int p_index)
{
	int new_this_size = (vEntries() + l_node->pEntries())/2;
	int num_frm_this = pEntries() - new_this_size;

	pushLeft(num_frm_this, l_node, p_index);
}

void txBtBranch::balanceWithRight (txBtBranch* r_node, int p_index)
{
	int new_this_size = (pEntries() + r_node->vEntries())/2;
	int num_frm_this = pEntries() - new_this_size;

	pushRight(num_frm_this, r_node, p_index);
}

void txBtBranch::balanceWith (txBtBranch* r_node, int pindx)
{
	if (pEntries() < r_node->vEntries())
	{
		r_node->balanceWithLeft(this, pindx);
	}
	else
	{
		balanceWithRight(r_node, pindx);
	}
}

void txBtBranch::decrementEntries (txBtNode* that)
{
	int i = indexOf(that);

	_item_[i]._tree_entries_--;

	if (_parent_ != 0)
	{
		_parent_->decrementEntries(this);
	}
	else
	{
		_tree_->_decrementEntries_();
	}
}

int txBtBranch::findIndex (const txObject* what) const
{
	if (what->compare(getKey(1)) == -1)
	{
		return getTree(0)->findIndex(what);
	}

	int sum = getEntries(0);

	for (int i = 1; i < _last_; i++)
	{
		if (what->compare(getKey(i)) == 0)
		{
			return sum;
		}

		sum++;

		if (what->compare(getKey(i + 1)) == -1)
		{
			return sum + getTree(i)->findIndex(what);
		}

		sum += getEntries(i);
	}

	if (what->compare(getKey(_last_)) == 0)
	{
		return sum;
	}

	sum++;

	return sum + getTree(_last_)->findIndex(what);
}

txBtLeaf* txBtBranch::firstLeafNode (void)
{
	return getTree(0)->firstLeafNode();
}

txObject* txBtBranch::find (const txObject* what, txBtNode** which, int* where)
{
	for (int i = 1 ; i <= _last_; i++)
	{
		int weight = getKey(i)->compare(what);

		if (weight == 0)
		{
			*which = this;
			*where = i;
			return getKey(i);
		}

		if (weight == 1)
		{
			return getTree(i - 1)->find(what, which, where);
		}
	}

	return getTree(_last_)->find(what, which, where);
}

txObject* txBtBranch::findBest (const txObject* what, txBtNode** which,
	int* where)
{
	for (int i = 1 ; i <= _last_; i++)
	{
		int weight = getKey(i)->compare(what);

		if (weight == 0)
		{
			*which = this;
			*where = i;
			return getKey(i);
		}

		if (weight == 1)
		{
			return getTree(i - 1)->findBest(what, which, where);
		}
	}

	return getTree(_last_)->findBest(what, which, where);
}

void txBtBranch::incrementEntries (txBtNode* that)
{
	int i = indexOf(that);

	_item_[i]._tree_entries_++;

	if (_parent_ != 0)
	{
		_parent_->incrementEntries(this);
	}
	else
	{
		_tree_->_incrementEntries_();
	}
}

int txBtBranch::indexOf (const txBtNode* that) const
{
	for (int i = 0; i <= _last_; i++)
	{
		if (getTree(i) == that)
		{
			return i;
		}
	}

	return 0;
}

void txBtBranch::notifyParent (void)
{
	if (_parent_ == 0)
	{
		_tree_->_notifyRootFull_();
	}
	else
	{
		_parent_->isFull(this);
	}
}

void txBtBranch::isFull (txBtNode* that)
{
	if (that->_is_leaf_)
	{
		txBtLeaf* leaf = (txBtLeaf*) that;
		txBtLeaf* left = 0;
		txBtLeaf* right= 0;

		int leafindex = indexOf(leaf);

		int hasRightNode = (leafindex < _last_) &&
			((right = (txBtLeaf*)getTree(leafindex + 1)) != 0);

		int hasLeftNode = (leafindex > 0) &&
			((left = (txBtLeaf*)getTree(leafindex - 1)) != 0);

		int rightNodeFull = (hasRightNode && right->isAlmtxtFull());
		int leftNodeFull = (hasLeftNode && left->isAlmtxtFull());

		if (rightNodeFull)
		{
			if (leftNodeFull)
			{
				left->splitNodeWith(leaf, leafindex);
			}
			else if (hasLeftNode)
			{
				leaf->balanceWithLeft(left, leafindex);
			}
			else
			{
				leaf->splitNodeWith(right, leafindex + 1);
			}
		}
		else if (hasRightNode)
		{
			leaf->balanceWithRight(right, leafindex + 1);
		}
		else if (leftNodeFull)
		{
			left->splitNodeWith(leaf, leafindex);
		}
		else if (hasLeftNode)
		{
			leaf->balanceWithLeft(left, leafindex);
		}
	}
	else
	{
		txBtBranch* inner = (txBtBranch*) that;
		int innerindex = indexOf(inner);
		txBtBranch* left = 0;
		txBtBranch* right= 0;

		int hasRightNode = (innerindex < _last_) &&
			((right = (txBtBranch*)getTree(innerindex + 1)) != 0);

		int hasLeftNode = (innerindex > 0) &&
			((left=(txBtBranch*)getTree(innerindex - 1)) != 0);

		int rightNodeFull = (hasRightNode && right->isAlmtxtFull());
		int leftNodeFull = (hasLeftNode && left->isAlmtxtFull());

		if (rightNodeFull)
		{
			if (leftNodeFull)
			{
				left->splitNodeWith(inner, innerindex);
			}
			else if (hasLeftNode)
			{
				inner->balanceWithLeft(left, innerindex);
			}
			else
			{
				inner->splitNodeWith(right, innerindex + 1);
			}
		}
		else if (hasRightNode)
		{
			inner->balanceWithRight(right, innerindex + 1);
		}
		else if (leftNodeFull)
		{
			left->splitNodeWith(inner, innerindex);
		}
		else if (hasLeftNode)
		{
			inner->balanceWithLeft(left, innerindex);
		}
	}
}

void txBtBranch::isLow (txBtNode* that)
{
	if (that->_is_leaf_)
	{
		txBtLeaf* leaf = (txBtLeaf*) that;
		txBtLeaf* left = 0;
		txBtLeaf* right= 0;

		int leafindex = indexOf(leaf);

		int hasRightNode = (leafindex < _last_) &&
			((right = (txBtLeaf*)getTree(leafindex + 1)) != 0);

		int hasLeftNode = (leafindex > 0) &&
			((left = (txBtLeaf*)getTree(leafindex - 1)) != 0);

		if (hasRightNode && (leaf->pEntries() +
			right->vEntries()) >= leaf->maxpEntries())
		{
			leaf->balanceWith(right, leafindex + 1);
		}
		else if (hasLeftNode && (leaf->vEntries() +
			left->pEntries()) >= leaf->maxpEntries())
		{
			left->balanceWith(leaf, leafindex);
		}
		else if (hasLeftNode)
		{
			left->mergeWithRight(leaf, leafindex);
		}
		else if (hasRightNode)
		{
			leaf->mergeWithRight(right, leafindex + 1);
		}
	}
	else
	{
		txBtBranch* inner = (txBtBranch*) that;

		int innerindex = indexOf(inner);
		txBtBranch* left = 0;
		txBtBranch* right= 0;

		int hasRightNode = (innerindex < _last_) &&
			((right = (txBtBranch*)getTree(innerindex + 1)) != 0);

		int hasLeftNode = (innerindex > 0) &&
			((left = (txBtBranch*)getTree(innerindex - 1)) != 0);

		if (hasRightNode && (inner->pEntries() +
			right->vEntries()) >= inner->maxpEntries())
		{
			inner->balanceWith(right, innerindex + 1);
		}
		else if (hasLeftNode && (inner->vEntries() +
			left->pEntries()) >= inner->maxpEntries())
		{
			left->balanceWith(inner, innerindex);
		}
		else if (hasLeftNode)
		{
			left->mergeWithRight(inner, innerindex);
		}
		else if (hasRightNode)
		{
			inner->mergeWithRight(right, innerindex + 1);
		}
	}
}

txBtLeaf* txBtBranch::lastLeafNode (void)
{
	return getTree(_last_)->lastLeafNode();
}

void txBtBranch::mergeWithRight (txBtBranch* r_node, int p_index)
{
	if (r_node->pEntries() > 0)
	{
		r_node->pushLeft(r_node->pEntries(), this, p_index);
	}

	r_node->setKey(0, _parent_->getKey(p_index));

	appendFrom(r_node, 0, 0);

	_parent_->incrementEntries(p_index - 1, r_node->getEntries(0) + 1);
	_parent_->removeItem(p_index);

	delete r_node; r_node = 0;
}

int txBtBranch::entries (void) const
{
	int sum = 0;

	for (int i = 0; i <= _last_; i++)
	{
		sum += getEntries(i);
	}

	return sum + pEntries();
}

txObject* txBtBranch::operator[] (int index) const
{
	for (int j = 0; j <= _last_; j++)
	{
		int r;

		if (index < (r = getEntries(j)))
		{
			return (*getTree(j))[index];
		}

		if (index == r)
		{
			if (j == _last_)
			{
				return 0;
			}
			else
			{
				return getKey(j + 1);
			}
		}

		index -= r + 1;
	}

	return 0;
}

void txBtBranch::pushLeft (int num_frm_this, txBtBranch* l_node, int p_index)
{
	setKey(0, _parent_->getKey(p_index));

	l_node->appendFrom(this, 0, num_frm_this - 1);

	shiftLeft(num_frm_this);

	_parent_->setKey(p_index, getKey(0));
	_parent_->setEntries(p_index - 1, l_node->entries());
	_parent_->setEntries(p_index, entries());
}

void txBtBranch::pushRight (int num_frm_this, txBtBranch* r_node, int p_index)
{
	int start = _last_ - num_frm_this + 1;
	int tgt = r_node->_last_ + num_frm_this;
	int src = r_node->_last_;

	r_node->_last_ = tgt;
	r_node->setKey(0, _parent_->getKey(p_index)); incrementEntries(0);

	while (src >= 0)
	{
		r_node->getItem(tgt--) = r_node->getItem(src--);
	}

	for (int i = _last_; i >= start; i-- )
	{
		r_node->setItem(tgt--, getItem(i));
	}

	_parent_->setKey(p_index, r_node->getKey(0));

	decrementEntries(0);

	_last_ -= num_frm_this;

	_parent_->setEntries(p_index - 1, entries());
	_parent_->setEntries(p_index, r_node->entries());
}

void txBtBranch::remove (int index)
{
	txBtLeaf* lf = getTree(index)->firstLeafNode();

	setKey(index, lf->_item_[0]);

	lf->removeItem(0);
}

void txBtBranch::removeItem (int index)
{
	for (int to = index; to < _last_; to++)
	{
		_item_[to] = _item_[to + 1];
		_item_[to + 1]._tree_ = 0;
		_item_[to + 1]._key_ = 0;
	}

	_item_[_last_]._tree_ = 0;
	_item_[_last_]._key_ = 0;

	_last_--;

	if (isLow())
	{
		if (_parent_ == 0)
		{
			if (pEntries() == 0)
			{
				_tree_->_notifyRootEmpty_();
			}
		}
		else
		{
			_parent_->isLow(this);
		}
	}
}

void txBtBranch::shiftLeft (int cnt)
{
	if (cnt <= 0)
	{
		return;
	}

	for (int i = cnt; i <= _last_; i++)
	{
		getItem(i-cnt) = getItem(i);
	}

	_last_ -= cnt;
}

void txBtBranch::splitNode (void)
{
	txBtBranch* newnode;

	#if !defined OSTORE_SUPPORT
		newnode = new txBtBranch(_parent_);
	#else
		if (TX_IS_OBJECT_PERSISTENT(_tree_))
		{
			newnode = new(TX_GET_OBJECT_CLUSTER(this),
				txBtBranch::get_os_typespec())
					txBtBranch(_parent_);
		}
		else
		{
			newnode = new txBtBranch(_parent_);
		}
	#endif

	_parent_->append(getKey(_last_), newnode);

	newnode->appendFrom(this, _last_, _last_);

	_last_--;

	_parent_->incrementEntries(1, newnode->getEntries(0));
	_parent_->decrementEntries(0, newnode->getEntries(0));

	balanceWithRight(newnode, 1);
}

void txBtBranch::splitNodeWith (txBtBranch* r_node, int key_index)
{
	r_node->setKey(0, _parent_->getKey(key_index));

	int num_of_keys = pEntries() + r_node->vEntries();

	int new_size_This = num_of_keys / 3;
	int new_size_New = (num_of_keys - new_size_This) / 2;
	int new_size_Node = (num_of_keys - new_size_This - new_size_New);

	int num_frm_this = pEntries() - new_size_This;
	int num_from_node = r_node->vEntries() - new_size_Node;

	txBtBranch* newnode;

	#if !defined OSTORE_SUPPORT
		newnode = new txBtBranch(_parent_);
	#else
		if (TX_IS_OBJECT_PERSISTENT(_tree_))
		{
			newnode = new(TX_GET_OBJECT_CLUSTER(this),
				txBtBranch::get_os_typespec())
					txBtBranch(_parent_);
		}
		else
		{
			newnode = new txBtBranch(_parent_);
		}
	#endif

	if (num_frm_this > 0)
	{
		newnode->append(getItem(_last_));
		_parent_->insertElement(key_index, getKey(_last_--), newnode);

		if (num_frm_this > 2)
		{
			this->pushRight(num_frm_this - 1, newnode, key_index);
		}

		r_node->pushLeft(num_from_node, newnode, key_index + 1);
	}
	else
	{
		newnode->append(r_node->getItem(0));

		_parent_->insertElement(
			key_index + 1, r_node->getKey(1), r_node);

		r_node->shiftLeft(1);
		_parent_->setTree(key_index, newnode);
		r_node->pushLeft(num_from_node - 1, newnode, key_index + 1);
	}

	_parent_->setEntries(key_index - 1, this->entries());
	_parent_->setEntries(key_index, newnode->entries());
	_parent_->setEntries(key_index + 1, r_node->entries());

	if (_parent_->isFull())
	{
		_parent_->notifyParent();
	}
}

txBtLeaf::txBtLeaf (txBtBranch* p, const txObject* obj, txTree* t) :
	txBtNode(1, p, t)
{
	#if !defined OSTORE_SUPPORT
		_item_ = new txObject*[maxIndex() + 1];
	#else
		if (TX_IS_OBJECT_PERSISTENT(_tree_))
		{
			_item_ = new(TX_GET_OBJECT_CLUSTER(this),
				os_typespec::get_pointer(), maxIndex() + 1)
					txObject*[maxIndex() + 1];
		}
		else
		{
			_item_ = new txObject*[maxIndex() + 1];
		}
	#endif

	memset(_item_, 0, (maxIndex()+1) * sizeof(txObject*));

	if (obj != 0)
	{
		_item_[++_last_] = (txObject*) obj;
	}
}

txBtLeaf::~txBtLeaf (void)
{
	delete [] _item_; _item_ = 0;
}

void txBtLeaf::insert (const txObject* obj, int index)
{
	for (int i = _last_ + 1; i > index ; i--)
	{
		_item_[i] = _item_[i - 1]; _item_[i - 1] = 0;
	}

	_item_[index] = (txObject*) obj;

	_last_++;

	if (_parent_ == 0)
	{
		_tree_->_incrementEntries_();
	}
	else
	{
		_parent_->incrementEntries(this);
	}

	if (isFull())
	{
		if (_parent_ == 0)
		{
			_tree_->_notifyRootFull_();
		}
		else
		{
			_parent_->isFull(this);
		}
	}
}

void txBtLeaf::appendFrom (txBtLeaf* src, int start, int stop)
{
	if (start > stop)
	{
		return;
	}

	for (int i = start; i <= stop; i++)
	{
		_item_[++_last_] = src->_item_[i]; src->_item_[i] = 0;
	}
}

void txBtLeaf::append (txObject* obj)
{
	_item_[++_last_] = obj;
}

void txBtLeaf::balanceWithLeft (txBtLeaf* l_node, int p_index)
{
	int new_this_size = (vEntries() + l_node->pEntries())/2;
	int num_frm_this = pEntries() - new_this_size;

	pushLeft(num_frm_this, l_node, p_index);
}

void txBtLeaf::balanceWithRight (txBtLeaf* r_node, int p_index)
{
	int new_this_size = (pEntries() + r_node->vEntries())/2;
	int num_frm_this = pEntries() - new_this_size;

	pushRight(num_frm_this, r_node, p_index);
}

void txBtLeaf::balanceWith (txBtLeaf* r_node, int p_index)
{
	if (pEntries() < r_node->vEntries())
	{
		r_node->balanceWithLeft(this, p_index);
	}
	else
	{
		balanceWithRight(r_node, p_index);
	}
}

int txBtLeaf::findIndex (const txObject* what) const
{
	for (int i = 0; i <= _last_; i++)
	{
		int weight = _item_[i]->compare(what);

		if (weight == 0)
		{
			return i;
		}

		if (weight == 1)
		{
			return -1;
		}
	}

	return -1;
}

txBtLeaf* txBtLeaf::firstLeafNode (void)
{
	return this;
}

txObject* txBtLeaf::find (const txObject* what, txBtNode** which,
	int* where)
{
	for (int i = 0; i <= _last_; i++)
	{
		int weight = _item_[i]->compare(what);

		if (weight == 0)
		{
			*which = this;
			*where = i;
			return _item_[i];
		}

		if (weight == 1)
		{
			*which = this;
			*where = i;
			return 0;
		}
	}

	*which = this;
	*where = _last_ + 1;
	return 0;
}

txObject* txBtLeaf::findBest (const txObject* what, txBtNode** which, int* where)
{
	int i = 0;

	for (; i <= _last_; i++)
	{
		int weight = _item_[i]->compare(what);

		if (weight == -1 || weight == 1)
		{
			*which = this;
			*where = i;
			return _item_[i];
		}
	}

	*which = this;
	*where = _last_ + 1;
	return _item_[--i];
}

int txBtLeaf::indexOf (const txObject* that) const
{
	for (int i = 0; i <= _last_; i++)
	{
		if (_item_[i] == that)
		{
			return i;
		}
	}

	return -1;
}

txBtLeaf* txBtLeaf::lastLeafNode (void)
{
	return this;
}

void txBtLeaf::mergeWithRight (txBtLeaf* r_node, int p_index)
{
	r_node->pushLeft(r_node->pEntries(), this, p_index);

	append(_parent_->getKey(p_index));

	_parent_->setEntries(p_index - 1, entries());
	_parent_->removeItem(p_index);

	delete r_node; r_node = 0;
}

int txBtLeaf::entries (int) const
{
	return 1;
}

int txBtLeaf::entries (void) const
{
	return pEntries();
}

void txBtLeaf::pushLeft (int num_frm_this, txBtLeaf* l_node, int p_index)
{
	l_node->append(_parent_->getKey(p_index));

	if (num_frm_this > 1)
	{
		l_node->appendFrom(this, 0, num_frm_this - 2);
	}

	_parent_->setKey(p_index, _item_[num_frm_this - 1]);

	shiftLeft(num_frm_this);

	_parent_->setEntries(p_index - 1, l_node->entries());
	_parent_->setEntries(p_index, entries());
}

void txBtLeaf::pushRight (int num_frm_this, txBtLeaf* r_node, int p_index)
{
	int start = _last_ - num_frm_this + 1;
	int tgt = r_node->_last_ + num_frm_this;
	int src = r_node->_last_;

	r_node->_last_ = tgt;

	while (src >= 0)
	{
		r_node->_item_[tgt--] = r_node->_item_[src--];

		r_node->_item_[src + 1] = 0;
	}

	r_node->_item_[tgt--] = _parent_->getKey(p_index);

	for (int i = _last_; i > start; i--)
	{
		r_node->_item_[tgt--] = _item_[i]; _item_[i] = 0;
	}

	_parent_->setKey(p_index, _item_[start]);

	_last_ -= num_frm_this;

	_parent_->setEntries(p_index - 1, entries());
	_parent_->setEntries(p_index, r_node->entries());
}

void txBtLeaf::remove (int index)
{
	for (int to = index; to < _last_; to++)
	{
		_item_[to] = _item_[to + 1]; _item_[to + 1] = 0;
	}

	_item_[_last_] = 0;

	_last_--;

	if (_parent_ == 0)
	{
		_tree_->_decrementEntries_();
	}
	else
	{
		_parent_->decrementEntries(this);
	}

	if (isLow())
	{
		if (_parent_ == 0)
		{
			if (pEntries() == 0)
			{
				_tree_->_notifyRootEmpty_();
			}
		}
		else
		{
			_parent_->isLow(this);
		}
	}
}

void txBtLeaf::shiftLeft (int cnt)
{
	if (cnt <= 0)
	{
		return;
	}

	for (int i = cnt; i <= _last_; i++)
	{
		_item_[i-cnt] = _item_[i]; _item_[i] = 0;
	}

	_last_ -= cnt;
}

void txBtLeaf::splitNode (void)
{
	txBtLeaf* newnode;

	#if !defined OSTORE_SUPPORT
		newnode = new txBtLeaf(_parent_);
	#else
		if (TX_IS_OBJECT_PERSISTENT(_tree_))
		{
			newnode = new(TX_GET_OBJECT_CLUSTER(this),
				txBtLeaf::get_os_typespec())
					txBtLeaf(_parent_);
		}
		else
		{
			newnode = new txBtLeaf(_parent_);
		}
	#endif

	_parent_->append(_item_[_last_--], newnode);

	_parent_->setEntries(0, _parent_->getTree(0)->entries());
	_parent_->setEntries(1, _parent_->getTree(1)->entries());

	balanceWithRight(newnode, 1);
}

void txBtLeaf::splitNodeWith (txBtLeaf* r_node, int key_index)
{
	int num_of_keys = pEntries() + r_node->vEntries();

	int new_size_This = num_of_keys / 3;
	int new_size_New = (num_of_keys - new_size_This) / 2;
	int new_size_Node = (num_of_keys - new_size_This - new_size_New);

	int num_frm_this = pEntries() - new_size_This;
	int num_from_node = r_node->vEntries() - new_size_Node;

	txBtLeaf* newnode;

	#if !defined OSTORE_SUPPORT
		newnode = new txBtLeaf(_parent_);
	#else
		if (TX_IS_OBJECT_PERSISTENT(_tree_))
		{
			newnode = new(TX_GET_OBJECT_CLUSTER(this),
				txBtLeaf::get_os_typespec())
					txBtLeaf(_parent_);
		}
		else
		{
			newnode = new txBtLeaf(_parent_);
		}
	#endif

	_parent_->insertElement(key_index, _item_[_last_--], newnode);

	_parent_->setEntries(key_index, 0);
	_parent_->decrementEntries(key_index - 1);

	this->pushRight(num_frm_this - 1, newnode, key_index);
	r_node->pushLeft(num_from_node, newnode, key_index + 1);

	if (_parent_->isFull())
	{
		_parent_->notifyParent();
	}
}

