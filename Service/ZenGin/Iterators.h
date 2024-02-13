namespace GOTHIC_NAMESPACE
{	
	template <class T, class TList>
	class ListIterator
	{
	public:
		typedef ListIterator self_type;
		typedef T* value_type;
		typedef T*& reference;
		typedef T** pointer;
		typedef std::forward_iterator_tag iterator_category;
		typedef int difference_type;

		inline ListIterator();
		inline ListIterator(TList& list);
		inline ListIterator(const ListIterator<T, TList>& right) = default;
		inline ListIterator& operator=(const ListIterator<T, TList>& right) = default;

		inline self_type& operator++();
		inline self_type operator++(int junk);
		inline reference operator*() const;
		inline bool operator==(const self_type& right) const;

	private:
		TList* node;
	};

	template <typename T>
	class TreeIterator
	{
	public:
		typedef TreeIterator self_type;
		typedef T* value_type;
		typedef T*& reference;
		typedef T** pointer;
		typedef std::bidirectional_iterator_tag iterator_category;

		inline TreeIterator();
		inline TreeIterator(zCTree<T>& tree);
		inline TreeIterator(const TreeIterator&) = default;
		inline TreeIterator& operator=(const TreeIterator&) = default;

		inline self_type& operator++();
		inline self_type operator++(int);
		inline self_type& operator--();
		inline self_type operator--(int);
		inline value_type operator*();
		inline bool operator==(const self_type& right);

	private:
		zCTree<T>* node;
	};

	class InventoryIterator
	{
	public:
		typedef InventoryIterator self_type;
		typedef oCItem* value_type;
		typedef oCItem*& reference;
		typedef oCItem** pointer;
		typedef std::forward_iterator_tag iterator_category;

		inline InventoryIterator();
		inline InventoryIterator(oCNpcInventory& inventory);
		inline InventoryIterator(const InventoryIterator&) = default;
		inline InventoryIterator& operator=(const InventoryIterator&) = default;

		inline self_type& operator++();
		inline self_type operator++(int);
		inline reference operator*() const;
		inline bool operator==(const self_type& right) const;

	private:
		zCListSort<oCItem>* node;

#if ENGINE <= Engine_G1A
		oCNpcInventory* inventory;
		int inventoryNr;
#endif
	};

	template <typename T>
	inline T* begin(zCArray<T>& array);

	template <typename T>
	inline T* end(zCArray<T>& array);

	template <typename T>
	inline const T* begin(const zCArray<T>& array);

	template <typename T>
	inline const T* end(const zCArray<T>& array);

	template <typename T>
	inline T* begin(zCArraySort<T>& array);

	template <typename T>
	inline T* end(zCArraySort<T>& array);

	template <typename T>
	inline const T* begin(const zCArraySort<T>& array);

	template <typename T>
	inline const T* end(const zCArraySort<T>& array);

	template <typename T>
	inline ListIterator<T, zCList<T>> begin(zCList<T>& list);

	template <typename T>
	inline ListIterator<T, zCList<T>> end(zCList<T>& list);

	template <typename T>
	inline ListIterator<T, zCList<T>> begin(zCList<T>* list);

	template <typename T>
	inline ListIterator<T, zCList<T>> end(zCList<T>* list);

	template <typename T>
	inline ListIterator<T, zCListSort<T>> begin(zCListSort<T>& list);

	template <typename T>
	inline ListIterator<T, zCListSort<T>> end(zCListSort<T>& list);

	template <typename T>
	inline ListIterator<T, zCListSort<T>> begin(zCListSort<T>* list);

	template <typename T>
	inline ListIterator<T, zCListSort<T>> end(zCListSort<T>* list);

	template <typename T>
	inline TreeIterator<T> begin(zCTree<T>& root);

	template <typename T>
	inline TreeIterator<T> end(zCTree<T>& root);

	template <typename T>
	inline TreeIterator<T> begin(zCTree<T>* root);

	template <typename T>
	inline TreeIterator<T> end(zCTree<T>* root);

	inline InventoryIterator begin(oCNpcInventory& inv);
	inline InventoryIterator end(oCNpcInventory& inv);

	inline const char* begin(const zSTRING& text);
	inline const char* end(const zSTRING& text);

	inline char* begin(zSTRING& text);
	inline char* end(zSTRING& text);

#pragma region Implementation

#pragma push_macro("TEMPLATE")
#pragma push_macro("ITERATOR")
#undef TEMPLATE
#undef ITERATOR
#define TEMPLATE template <typename T, typename TList>
#define ITERATOR ListIterator<T, TList>

	TEMPLATE ITERATOR::ListIterator() :
		node{}
	{

	}

	TEMPLATE ITERATOR::ListIterator(TList& list) :
		node{ list.next }
	{

	}

	TEMPLATE typename ITERATOR::self_type& ITERATOR::operator++()
	{
		node = node->next;
		return *this;
	}

	TEMPLATE ITERATOR::self_type ITERATOR::operator++(int)
	{
		const self_type backup{ *this };
		++*this;
		return backup;
	}

	TEMPLATE ITERATOR::reference ITERATOR::operator*() const
	{
		return node->data;
	}

	TEMPLATE bool ITERATOR::operator==(const self_type& right) const
	{
		return node == right.node;
	}

#undef TEMPLATE
#undef ITERATOR
#define TEMPLATE template <typename T>
#define ITERATOR TreeIterator<T>

	TEMPLATE ITERATOR::TreeIterator() :
		node{ nullptr }
	{

	}

	TEMPLATE ITERATOR::TreeIterator(zCTree<T>& tree) :
		node{ &tree }
	{

	}

	TEMPLATE typename ITERATOR::self_type& ITERATOR::operator++()
	{
		if (node->GetFirstChild())
		{
			node = node->GetFirstChild();
			return *this;
		}

		if (node->GetNextChild())
		{
			node = node->GetNextChild();
			return *this;
		}

		while (node = node->GetParent())
			if (node->GetNextChild())
			{
				node = node->GetNextChild();
				return *this;
			}

		return *this;
	}

	TEMPLATE ITERATOR::self_type ITERATOR::operator++(int)
	{
		const ITERATOR backup{ *this };
		++*this;
		return backup;
	}

	TEMPLATE typename ITERATOR::self_type& ITERATOR::operator--()
	{
		if (!node->GetPrevChild())
		{
			node = node->GetParent();
			return *this;
		}

		node = node->GetPrevChild();

		while (node->GetFirstChild())
		{
			node = node->GetFirstChild();

			while (node->GetNextChild())
				node = node->GetNextChild();
		}

		return *this;
	}

	TEMPLATE ITERATOR::self_type ITERATOR::operator--(int)
	{
		const ITERATOR backup{ *this };
		--*this;
		return backup;
	}

	TEMPLATE ITERATOR::value_type ITERATOR::operator*()
	{
		return node->GetData();
	}

	TEMPLATE bool ITERATOR::operator==(const self_type& right)
	{
		return node == right.node;
	}

	InventoryIterator::InventoryIterator() :
		node{}

#if ENGINE <= Engine_G1A
		, inventory{}
		, inventoryNr{}
#endif

	{

	}

	InventoryIterator::InventoryIterator(oCNpcInventory& inventory) :
		node{}

#if ENGINE <= Engine_G1A
		, inventory{ &inventory }
		, inventoryNr{}
#endif

	{
		if (inventory.packAbility)
			inventory.UnpackAllItems();

#if ENGINE <= Engine_G1A
		
		for (inventoryNr = 0; inventoryNr < INV_MAX; inventoryNr++)
			if (node = inventory.inventory[inventoryNr].GetNextInList())
				return;
#else
		node = inventory.inventory.GetNextInList();
#endif

	}

	InventoryIterator::self_type& InventoryIterator::operator++()
	{
		node = node->GetNextInList();

#if ENGINE <= Engine_G1A
		while (!node && inventoryNr < INV_MAX - 1)
			node = inventory->inventory[++inventoryNr].GetNextInList();
#endif

		return *this;
	}

	InventoryIterator::self_type InventoryIterator::operator++(int)
	{
		const self_type backup{ *this };
		++*this;
		return backup;
	}

	InventoryIterator::reference InventoryIterator::operator*() const
	{
		return node->data;
	}

	bool InventoryIterator::operator==(const self_type& right) const
	{
		return node == right.node;
	}

#undef ITERATOR
#pragma push_macro("CONTAINER")
#undef CONTAINER
#define CONTAINER zCArray<T>
	
	TEMPLATE T* begin(CONTAINER& array)
	{
		return array.GetArray();
	}

	TEMPLATE T* end(CONTAINER& array)
	{
		return array.GetArray() + array.GetNum();
	}

	TEMPLATE const T* begin(const CONTAINER& array)
	{
		return array.GetArray();
	}

	TEMPLATE const T* end(const CONTAINER& array)
	{
		return array.GetArray() + array.GetNum();
	}

#undef CONTAINER
#define CONTAINER zCArraySort<T>

	TEMPLATE T* begin(CONTAINER& array)
	{
		return array.GetArray();
	}

	TEMPLATE T* end(CONTAINER& array)
	{
		return array.GetArray() + array.GetNum();
	}

	TEMPLATE const T* begin(const CONTAINER& array)
	{
		return array.GetArray();
	}

	TEMPLATE const T* end(const CONTAINER& array)
	{
		return array.GetArray() + array.GetNum();
	}

#undef CONTAINER
#define CONTAINER zCList<T>
#define ITERATOR ListIterator<T, CONTAINER>

	TEMPLATE ITERATOR begin(CONTAINER& list)
	{
		return ITERATOR{ list };
	}

	TEMPLATE ITERATOR end(CONTAINER& list)
	{
		return ITERATOR{};
	}

	TEMPLATE ITERATOR begin(CONTAINER* list)
	{
		return list ? ITERATOR{ *list } : ITERATOR{};
	}

	TEMPLATE ITERATOR end(CONTAINER* list)
	{
		return ITERATOR(nullptr);
	}

#undef CONTAINER
#undef ITERATOR
#define CONTAINER zCListSort<T>
#define ITERATOR ListIterator<T, CONTAINER>

	TEMPLATE ITERATOR begin(CONTAINER& list)
	{
		return ITERATOR{ list };
	}

	TEMPLATE ITERATOR end(CONTAINER& list)
	{
		return ITERATOR{};
	}

	TEMPLATE ITERATOR begin(CONTAINER* list)
	{
		return list ? ITERATOR{ *list } : ITERATOR{};
	}

	TEMPLATE ITERATOR end(CONTAINER* list)
	{
		return ITERATOR{};
	}

#undef CONTAINER
#undef ITERATOR
#define CONTAINER zCTree<T>
#define ITERATOR TreeIterator<T>

	TEMPLATE ITERATOR begin(CONTAINER& root)
	{
		return ITERATOR{ root };
	}

	TEMPLATE ITERATOR end(CONTAINER& root)
	{
		if (root.GetNextChild())
			return ITERATOR{ root.GetNextChild() };

		CONTAINER* node = &root;

		while (node = node->GetParent())
			if (node->GetNextChild())
				return ITERATOR{ node->GetNextChild() };

		return ITERATOR{};
	}

	TEMPLATE ITERATOR begin(CONTAINER* root)
	{
		return ITERATOR{ root };
	}

	TEMPLATE ITERATOR end(CONTAINER* root)
	{
		return root ? end(*root) : ITERATOR{};
	}

#pragma pop_macro("TEMPLATE")
#pragma pop_macro("ITERATOR")
#pragma pop_macro("CONTAINER")

	InventoryIterator begin(oCNpcInventory& inv)
	{
		return InventoryIterator(inv);
	}

	InventoryIterator end(oCNpcInventory& inv)
	{
		return InventoryIterator{};
	}

	const char* begin(const zSTRING& text)
	{
		return text.IsEmpty() ? nullptr : text.ToChar();
	}

	const char* end(const zSTRING& text)
	{
		return text.IsEmpty() ? nullptr : text.ToChar() + text.Length();
	}

	char* begin(zSTRING& text)
	{
		return text.IsEmpty() ? nullptr : text.ToChar();
	}

	char* end(zSTRING& text)
	{
		return text.IsEmpty() ? nullptr : text.ToChar() + text.Length();
	}

#pragma endregion

}
