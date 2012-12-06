
#pragma once

template <typename Key, typename Value>
class BinarySearchTree
{
private:
	class Node
	{
	public:
		Key key;
		Value Data;
		Node *Left;
		Node *Right;

		Node() {}
		Node(Value NewData)
			: Data(NewData), Left(nullptr), Right(nullptr)
		{}

		Value &RecFind(const Key &FindKey)
		{
			if (key == FindKey)
				return Data;
			else if (FindKey < key)
				return Left->RecFind(FindKey);
			else if (FindKey > key)
				return Right->RecFind(FindKey);
		}

		void RecInsert(const Key &NewKey, Value &NewValue)
		{
			if (key == NewKey)
				Data = NewValue;
			else if (NewKey < key)
			{
				if (Left == nullptr)
				{
					Left = new Node(NewValue);
					Left->key = NewKey;
				}
				else
					Left->Data = NewValue;
			}
			else if (NewKey > key)
			{
				if (Right == nullptr)
				{
					Right = new Node(NewValue);
					Right->key = NewKey;
				}
				else
					Right->Data = NewValue;
			}
		}
	};

	Node *root;

public:
	BinarySearchTree()
		: root(nullptr)
	{}

	Value &operator[](const Key &key)
	{
		return root->RecFind(key);
	}

	void Insert(const Key &NewKey, Value &NewValue)
	{
		if (root == nullptr)
		{
			root = new Node(NewValue);
			root->key = NewKey;
		}
	}
};