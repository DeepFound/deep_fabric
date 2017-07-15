///////////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 1995-2010, Thomas M. Hazel, txObject ATK (www.txobject.org)
//
//   All Rights Reserved. See LICENSE.txt for license definition
//
///////////////////////////////////////////////////////////////////////////////
 
#if !defined ( __TXTREE_H__ )
#define __TXTREE_H__

#include "txobject.h"
 
class txBtNode; // node
class txBtItem; // item
class txBtLeaf; // leaf
class txBtBranch; // branch

class txTree : public txObjectSequence
{
	TX_DEFINE_TYPE(txTree)

	private:
		txBtNode* _root_;
		unsigned long _flags_;

		int _entries_;
		int _leaf_low_water_;
		int _leaf_max_index_;
		int _branch_low_water_;
		int _branch_max_index_;

	private:
		void _initialize_ (int i);
		void _notifyRootFull_ (void);
		void _notifyRootEmpty_ (void);

		void _incrementEntries_ (void) { _entries_++; }
		void _decrementEntries_ (void) { _entries_--; }

	public:
		txTree (
			int order = 3,
			TX_AUTODEL_FLAG autodel = TX_AUTODEL_OFF,
			TX_COMPARE_FLAG compare = TX_COMPARE_V2K)
		{
			TX_SET_AUTODEL_FLAG(_flags_, autodel);
			TX_SET_COMPARE_FLAG(_flags_, compare);

			_initialize_(order);
		}

		~txTree (void)
		{
			if (TX_GET_AUTODEL_FLAG(_flags_))
			{
				clearAndDestroy();
			}
			else
			{
				clear();
			}
		}

		virtual txObject* remove (const txObject* obj);
		virtual const txObject* insert (txObject* obj);
		virtual const txObject* find (const txObject* obj) const;
		virtual void removeAndDestroy (const txObject* obj);

		const txObject* findBest (const txObject* obj) const;

		const txObject* last (void) const;
		const txObject* first (void) const;
		const txObject* at (int index) const;

		int index (const txObject* obj) const;
		virtual int entries (void) const { return _entries_; }

		virtual void clearAndDestroy (void);
		virtual void clear (void);

		void readObject (txIn& in);
		void writeObject (txOut& out) const;

	friend class txTreeIterator;
	friend class txBtBranch;
	friend class txBtLeaf;
	friend class txBtNode;
};

TX_DECLARE_STREAM_TYPE_OPER(txTree)

class txTreeIterator : public txObjectIterator
{
	TX_PERSIST

	private:
		int _cursor_;
		int _direction_;
		txTree* _tree_;

	public:
		txTreeIterator (const txTree& t, int direction = 1); // up

		~txTreeIterator (void);

		virtual txObject* remove (void);

		virtual const txObject* next (void);

		void reset (const txTree& t, int direction = 1); // up

		virtual void reset (void);

	friend class txTree;
};

class txBtNode : public txObject
{
	TX_PERSIST

	protected:
		txBtBranch* _parent_;
		txTree* _tree_;
		int _is_leaf_;
		int _last_;

	public:
		txBtNode (int isleaf, txBtBranch* p, txTree* t = 0);

		virtual ~txBtNode (void)
		{
		}

		virtual void insert (const txObject* obj, int index) = 0;
		virtual void remove (int index) = 0;

		virtual txObject* operator[] (int index) const = 0;

		virtual txObject* find (const txObject*, txBtNode**, int*) = 0;
		virtual txObject* findBest (const txObject*,txBtNode**,int*)=0;

		virtual int findIndex (const txObject* obj) const = 0;
		virtual int entries (void) const = 0;

		virtual txBtLeaf* firstLeafNode (void) = 0;
		virtual txBtLeaf* lastLeafNode (void) = 0;

		virtual void splitNode (void) = 0;

	friend class txBtBranch;
	friend class txBtLeaf;
	friend class txTree;
};

class txBtItem : public txObject
{
	TX_PERSIST

	private:
		txBtNode* _tree_;
		txObject* _key_;
		int _tree_entries_;

	public:
		txBtItem (void);
		txBtItem (txBtNode* n, txObject* o);
		txBtItem (txObject* o, txBtNode* n);

		~txBtItem (void);

	friend class txBtBranch;
};


class txBtBranch : public txBtNode
{
	TX_PERSIST

	private:
		txBtItem* _item_;

	public:
		txBtBranch (txBtBranch* p, txTree* t = 0);
		txBtBranch (txBtBranch* p, txTree* t, txBtNode* oldroot);

		~txBtBranch (void);

		void insert (const txObject* obj, int index);
		void insert (txBtItem &i, int index);
		void insert (int at, txObject* obj, txBtNode* n);
		void insertElement (txBtItem &itm, int at);
		void insertElement (int at, txObject* obj, txBtNode* n);

		void remove (int index);
		void removeItem (int index);

		txObject* operator[] (int index) const;
		txObject* find (const txObject*, txBtNode**, int*);
		txObject* findBest (const txObject*, txBtNode**, int*);

		int entries (int index) const;
		int entries (void) const;

		void setTree (int i, txBtNode* node)
		{
			_item_[i]._tree_ = node; node->_parent_ = this;
		}

		void setKey (int i, txObject* obj)
		{
			_item_[i]._key_ = obj;
		}

		void setItem (int i, txBtItem& itm)
		{
			_item_[i] = itm; itm._tree_->_parent_ = this;
		}

		void setItem (int i, txObject* obj, txBtNode* node)
		{
			setTree(i, node); setKey(i, obj);
		}

		void notifyParent (void);

		void incrementEntries (txBtNode* np);
		void decrementEntries (txBtNode* np);

		txBtLeaf* lastLeafNode (void);
		txBtLeaf* firstLeafNode (void);

		int getEntries (int i) const;
		void setEntries (int i, int r);

		int incrementEntries (int i, int n=1);
		int decrementEntries (int i, int n=1);

		int indexOf (const txBtNode* n) const;
		int findIndex (const txObject* obj) const;

		txBtItem& getItem (int i) const { return _item_[i]; }
		txBtNode* getTree (int i) const { return _item_[i]._tree_; }
		txObject* getKey (int i) const { return _item_[i]._key_; }

		void splitNode (void);
		void shiftLeft (int cnt);
		void splitNodeWith (txBtBranch* r, int index);

		void append (txBtItem &itm);
		void append (txObject* obj, txBtNode* n);
		void appendFrom (txBtBranch* src, int start, int stop);

		void mergeWithRight (txBtBranch* r, int index);

		void balanceWithLeft (txBtBranch* l, int index);
		void balanceWithRight (txBtBranch* r, int index);
		void balanceWith (txBtBranch* n, int index);

		void pushLeft (int cnt, txBtBranch* leftsib, int parentIdx);
		void pushRight (int cnt, txBtBranch* rightsib, int parentIdx);

		int vEntries (void) const;
		int pEntries (void) const { return _last_; }

		int maxIndex (void) const
		{
			return _tree_->_branch_max_index_;
		}

		int maxpEntries (void) const
		{
			return _tree_->_branch_max_index_;
		}

		void isFull (txBtNode* n);

		int isFull (void) const
		{
			return _last_ == maxIndex();
		}

		int isAlmtxtFull (void) const
		{
			return _last_ >= maxIndex()-1;
		}

		void isLow (txBtNode* n);

		int isLow (void) const
		{
			return _last_ < _tree_->_branch_low_water_;
		}
};


class txBtLeaf : public txBtNode
{
	TX_PERSIST

	private:
		txObject** _item_;

	public:
		txBtLeaf (txBtBranch*, const txObject* o=0, txTree* t=0);

		~txBtLeaf (void);

		void insert (const txObject* obj, int index);

		void remove (int index);
		void removeItem (int index) { remove(index); }

		txObject* operator[] (int index) const;
		txObject* find (const txObject*, txBtNode**, int*);
		txObject* findBest (const txObject*, txBtNode**, int*);

		int entries (void) const;
		int entries (int i) const;

		int indexOf (const txObject* obj) const;
		int findIndex (const txObject *obj) const;

		txObject* getKey (int index ) { return _item_[index]; }
		void setKey (int index, txObject* obj) { _item_[index] = obj; }

		txBtLeaf* firstLeafNode (void);
		txBtLeaf* lastLeafNode (void);

		void splitNode (void);
		void shiftLeft (int cnt);
		void splitNodeWith (txBtLeaf* r, int index);

		void append (txObject* obj);
		void appendFrom (txBtLeaf* src, int start, int stop);

		void mergeWithRight (txBtLeaf* r, int index);

		void balanceWith (txBtLeaf* n, int index);
		void balanceWithLeft (txBtLeaf* l, int index);
		void balanceWithRight (txBtLeaf* r, int index);

		void pushLeft (int cnt, txBtLeaf* l, int parentIndex);
		void pushRight (int cnt, txBtLeaf* r, int parentIndex);

		int vEntries (void) const;
		int pEntries (void) const { return _last_ + 1; }

		int maxIndex (void) const
		{
			return _tree_->_leaf_max_index_;
		}

		int maxpEntries (void) const
		{
			return _tree_->_leaf_max_index_+1;
		}

		int isFull (void) const
		{
			return _last_ == maxIndex();
		}

		int isAlmtxtFull (void) const
		{
			return _last_ >= maxIndex()-1;
		}

		int isLow (void) const
		{
			return _last_ < _tree_->_leaf_low_water_;
		}

	friend class txBtBranch;
};

inline const txObject* txTree::at (int i) const
{
	return _root_ ? (*_root_)[i] : 0;
}

inline const txObject* txTree::first (void) const
{
	return _root_ ? (*_root_)[0] : 0;
}

inline const txObject* txTree::last (void) const
{
	return _root_ ? (*_root_)[_entries_ - 1] : 0;
}

inline int txBtBranch::getEntries (int i) const
{
	return _item_[i]._tree_entries_;
}

inline int txBtBranch::entries (int index) const
{
	return getEntries(index);
}

inline void txBtBranch::setEntries (int i, int r)
{
	_item_[i]._tree_entries_ = r;
}

inline int txBtBranch::incrementEntries (int i, int n)
{
	return (_item_[i]._tree_entries_ += n);
}

inline int txBtBranch::decrementEntries (int i, int n)
{
	return (_item_[i]._tree_entries_ -= n);
}

inline int txBtBranch::vEntries (void) const
{
	return pEntries()+1;
}

inline txObject* txBtLeaf::operator[] (int i) const
{
	return _item_[i];
}

inline int txBtLeaf::vEntries (void) const
{
	return pEntries()+1;
}

#endif // __TXTREE_H__
