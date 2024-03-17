namespace Service
{
	template <typename T>
	struct zReleaser
	{
		inline void operator()(T* object) const;
	};
	
	template <typename T>
	using zPtr = std::unique_ptr<T, zReleaser<T>>;
		
	template <typename T>
	inline zPtr<T> zOwn(T* object);
}

namespace Service
{
	template <typename T>
	void zReleaser<T>::operator()(T* object) const
	{
		object->Release();
	};
	
	template <typename T>
	zPtr<T> zOwn(T* object)
	{
		if (object)
			object->AddRef();
		
		return zPtr<T>{ object };
	}
}
