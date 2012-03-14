//	smart_pointer.h
//	created by Sebastian Reiter
//
// this code was originally created for a private project of mine,
// may however be freely used in ug.

#ifndef __SMART_POINTER__
#define __SMART_POINTER__

#include <functional>
#include <cstring>
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//	Policies

////////////////////////////////////////////////////////////////////////
//	FreeDelete
template <typename T>
class FreeDelete
{
	public:
		static void free(const T* data)	{delete data;}
};

////////////////////////////////////////////////////////////////////////
//	FreeDelete
template <typename T>
class FreeArrayDelete
{
	public:
		static void free(const T* data)	{delete[] data;}
};

////////////////////////////////////////////////////////////////////////
//	FreeRelease
template <typename T>
class FreeRelease
{
	public:
		static void free(const T* data)	{data->Release;}
};


////////////////////////////////////////////////////////////////////////
//	PREDECLARATIONS
template <class T, template <class TT> class FreePolicy = FreeDelete> class SmartPtr;
template <class T, template <class TT> class FreePolicy = FreeDelete> class ConstSmartPtr;

////////////////////////////////////////////////////////////////////////
//	SmartPtr
/**
 * A Smart Pointer encapsulates a pointer. With each copy
 * of the SmartPointer a shared reference count is increased and with
 * each destructor call to a SmartPointer this shared reference count is
 * decreased. If the shared reference count reaches 0 free as specified in
 * the FreePolicy is executed on the pointer.
 *
 * The FreePolicy has to feature the method free().
 *
 * If a const smart pointer is required, use ConstSmartPtr
 * \{
 */
template <typename T, template <class TT> class FreePolicy>
class SmartPtr
{
	friend class ConstSmartPtr<T, FreePolicy>;
	friend class SmartPtr<void>;
	friend class ConstSmartPtr<void>;

	public:
		SmartPtr() : m_ptr(0), m_refCount(0)	{}
		SmartPtr(T* ptr) : m_ptr(ptr), m_refCount(0)	{if(ptr) m_refCount = new int(1);}
		SmartPtr(const SmartPtr& sp) : m_ptr(sp.m_ptr), m_refCount(sp.m_refCount)
		{
			if(m_refCount) (*m_refCount)++;
		}

	/**	this template method allows to assign smart-pointers that encapsulate
	 *	derivates of T. Make sure that the pointer-type of TSmartPtr is castable
	 *	to T*.*/
		template <class TPtr>
		SmartPtr(const SmartPtr<TPtr, FreePolicy>& sp) :
			m_ptr(static_cast<T*>(sp.get_nonconst_impl())),
			m_refCount(sp.get_refcount_ptr())
		{
			if(m_refCount) (*m_refCount)++;
		}

		~SmartPtr() {release();}
/*
	///	copies a SmartPtr and applies a reinterpret_cast on its impl.
		template <class TIn>
		static SmartPtr<T> from_reinterpret_cast(SmartPtr<TIn>& in)	{
			return SmartPtr<T>(reinterpret_cast<T*>(in.m_ptr),
								in.get_refcount_ptr());
		}
*/
		T* operator->() 			{return m_ptr;}
		const T* operator->() const	{return m_ptr;}

		T& operator*()				{return *m_ptr;}
		const T& operator*() const	{return *m_ptr;}

		SmartPtr& operator=(const SmartPtr& sp)	{
			if(m_ptr)
				release();
			m_ptr = sp.m_ptr;
			m_refCount = sp.m_refCount;
			if(m_refCount)
				(*m_refCount)++;
			return *this;
		}

		template <class TIn>
		SmartPtr<T, FreePolicy>& operator=(const SmartPtr<TIn, FreePolicy>& sp)	{
			if(m_ptr)
				release();
			m_ptr = sp.get_nonconst_impl();
			m_refCount = sp.get_refcount_ptr();
			if(m_refCount)
				(*m_refCount)++;
			return *this;
		}

		bool operator==(const SmartPtr& sp){
			return (this->get_impl() == sp.get_impl());
		}

		bool operator!=(const SmartPtr& sp){
			return !(this->operator==(sp));
		}

	///	returns encapsulated pointer
		T* get_impl()				{return m_ptr;}

	///	returns encapsulated pointer
		const T* get_impl() const	{return m_ptr;}

	///	returns refcount
		int get_refcount() const {if(m_refCount) return *m_refCount; return 0;}

	///	returns true if the pointer is valid, false if not.
		inline bool is_valid() const	{return m_ptr != NULL;}
		
	///	preforms a dynamic cast
		template <class TDest>
		SmartPtr<TDest, FreePolicy> cast_dynamic() const{
			TDest* p = dynamic_cast<TDest*>(m_ptr);
			if(p) return SmartPtr<TDest, FreePolicy>(p, m_refCount);
			else return SmartPtr<TDest, FreePolicy>(NULL);
		}

	///	performs a static cast
		template <class TDest>
		SmartPtr<TDest, FreePolicy> cast_static() const{
			TDest* p = static_cast<TDest*>(m_ptr);
			if(p) return SmartPtr<TDest, FreePolicy>(p, m_refCount);
			else return SmartPtr<TDest, FreePolicy>(NULL);
		}

	///	performs a reinterpret cast
		template <class TDest>
		SmartPtr<TDest, FreePolicy> cast_reinterpret() const{
			TDest* p = reinterpret_cast<TDest*>(m_ptr);
			if(p) return SmartPtr<TDest, FreePolicy>(p, m_refCount);
			else return SmartPtr<TDest, FreePolicy>(NULL);
		}

	///	performs a const cast
		ConstSmartPtr<T, FreePolicy> cast_const() const;

	///	WARNING: this method is DANGEROUS!
	/**	You should only use this constructor if you really know what you're doing!
	 *	The following methods are required for SmartPtr<void> and casts */
		SmartPtr(T* ptr, int* refCount) : m_ptr(ptr), m_refCount(refCount)
		{
			if(m_refCount)
				(*m_refCount)++;
		}

	///	WARNING: this method is DANGEROUS!
	/**	This method should never be used since it may be removed in future
	 *	versions of the SmartPtr class.
	 *	It is featured in order to allow to implement a template-constructor
	 *	that casts element-pointers of a smart pointer.
	 *	\{*/
		int* get_refcount_ptr() const 	{return m_refCount;}

		T* get_nonconst_impl() const	{return m_ptr;}
	/**	\}	*/

	private:
	///	decrements the refCount and frees the encapsulated pointer if required.
		void release() {
			if(m_refCount)
			{
				(*m_refCount)--;
				if((*m_refCount) < 1)
				{
					delete m_refCount;
					//delete m_ptr;
					FreePolicy<T>::free(m_ptr);
				}
			}
		}

	///	this release method is required by SmartPtr<void>
	/**	const void is not really correct here, of course...*/
		static void free_void_ptr(const void* ptr){
			FreePolicy<T>::free(reinterpret_cast<const T*>(ptr));
		}

	private:
		T*		m_ptr;
		int*	m_refCount;
};

template <typename T, template <class TT> class FreePolicy>
class ConstSmartPtr
{
	friend class ConstSmartPtr<void>;

	public:
		ConstSmartPtr() : m_ptr(0), m_refCount(0)	{}
		ConstSmartPtr(const T* ptr) : m_ptr(ptr), m_refCount(0)	{if(ptr) m_refCount = new int(1);}
		ConstSmartPtr(const ConstSmartPtr& sp) : m_ptr(sp.m_ptr), m_refCount(sp.m_refCount)
		{
			if(m_refCount) (*m_refCount)++;
		}

	/**	this template method allows to assign smart-pointers that encapsulate
	 *	derivates of T. Make sure that the pointer-type of TSmartPtr is castable
	 *	to T*.*/
		template <class TPtr>
		ConstSmartPtr(const SmartPtr<TPtr, FreePolicy>& sp) :
			m_ptr(static_cast<const T*>(sp.get_impl())),
			m_refCount(sp.get_refcount_ptr())
		{
			if(m_refCount) (*m_refCount)++;
		}

		template <class TPtr>
		ConstSmartPtr(const ConstSmartPtr<TPtr, FreePolicy>& sp) :
			m_ptr(static_cast<const T*>(sp.get_impl())),
			m_refCount(sp.get_refcount_ptr())
		{
			if(m_refCount) (*m_refCount)++;
		}

		~ConstSmartPtr() {release();}
/*
	///	copies a SmartPtr and applies a reinterpret_cast on its impl.
		template <class TIn>
		static SmartPtr<T> from_reinterpret_cast(SmartPtr<TIn>& in)	{
			return SmartPtr<T>(reinterpret_cast<T*>(in.m_ptr),
								in.get_refcount_ptr());
		}
*/
		const T* operator->() const	{return m_ptr;}

		const T& operator*() const	{return *m_ptr;}

		ConstSmartPtr& operator=(const SmartPtr<T, FreePolicy>& sp)	{
			if(m_ptr)
				release();
			m_ptr = sp.m_ptr;
			m_refCount = sp.m_refCount;
			if(m_refCount)
				(*m_refCount)++;
			return *this;
		}

		template <class TIn>
		ConstSmartPtr<T, FreePolicy>& operator=(const SmartPtr<TIn, FreePolicy>& sp)	{
			if(m_ptr)
				release();
			m_ptr = sp.get_impl();
			m_refCount = sp.get_refcount_ptr();
			if(m_refCount)
				(*m_refCount)++;
			return *this;
		}

		ConstSmartPtr& operator=(const ConstSmartPtr& sp)	{
			if(m_ptr)
				release();
			m_ptr = sp.m_ptr;
			m_refCount = sp.m_refCount;
			if(m_refCount)
				(*m_refCount)++;
			return *this;
		}

		template <class TIn>
		ConstSmartPtr<T, FreePolicy>& operator=(const ConstSmartPtr<TIn, FreePolicy>& sp)	{
			if(m_ptr)
				release();
			m_ptr = sp.get_impl();
			m_refCount = sp.get_refcount_ptr();
			if(m_refCount)
				(*m_refCount)++;
			return *this;
		}

		bool operator==(const ConstSmartPtr& sp){
			return (this->get_impl() == sp.get_impl());
		}

		bool operator!=(const ConstSmartPtr& sp){
			return !(this->operator==(sp));
		}

		const T* get_impl() const	{return m_ptr;}

		int get_refcount() const {if(m_refCount) return *m_refCount; return 0;}

	///	returns true if the pointer is valid, false if not.
		inline bool is_valid() const	{return m_ptr != NULL;}

	///	preforms a dynamic cast
		template <class TDest>
		ConstSmartPtr<TDest, FreePolicy> cast_dynamic() const{
			const TDest* p = dynamic_cast<const TDest*>(m_ptr);
			if(p) return ConstSmartPtr<TDest, FreePolicy>(p, m_refCount);
			else return ConstSmartPtr<TDest, FreePolicy>(NULL);
		}

	///	performs a static cast
		template <class TDest>
		ConstSmartPtr<TDest, FreePolicy> cast_static() const{
			const TDest* p = static_cast<const TDest*>(m_ptr);
			if(p) return ConstSmartPtr<TDest, FreePolicy>(p, m_refCount);
			else return ConstSmartPtr<TDest, FreePolicy>(NULL);
		}

	///	performs a static cast
		template <class TDest>
		ConstSmartPtr<TDest, FreePolicy> cast_reinterpret() const{
			const TDest* p = reinterpret_cast<const TDest*>(m_ptr);
			if(p) return ConstSmartPtr<TDest, FreePolicy>(p, m_refCount);
			else return ConstSmartPtr<TDest, FreePolicy>(NULL);
		}

	///	performs a const cast
		SmartPtr<T, FreePolicy> cast_const() const{
			return SmartPtr<T, FreePolicy>(const_cast<T*>(m_ptr), m_refCount);
		}

	///	WARNING: this method is DANGEROUS!
	/**	You should only use this constructor if you really know what you're doing!
	 *	The following methods are required for SmartPtr<void> and casts */
		ConstSmartPtr(const T* ptr, int* refCount) : m_ptr(ptr), m_refCount(refCount)
		{
			if(m_refCount)
				(*m_refCount)++;
		}

	///	WARNING: this method is dangerous!
	/**	This method should never be used since it may be removed in future
	 *	versions of the SmartPtr class.
	 *	It is featured in order to allow to implement a template-constructor
	 *	that casts element-pointers of a smart pointer.*/
		int* get_refcount_ptr() const {return m_refCount;}

	private:
	///	decrements the refCount and frees the encapsulated pointer if required.
		void release() {
			if(m_refCount)
			{
				(*m_refCount)--;
				if((*m_refCount) < 1)
				{
					delete m_refCount;
					//delete m_ptr;
					FreePolicy<T>::free(m_ptr);
				}
			}
		}

	//	this release method is required by SmartPtr<void>
		static void free_void_ptr(void* ptr){
			FreePolicy<T>::free(reinterpret_cast<T*>(ptr));
		}

	private:
		const T*	m_ptr;
		int*		m_refCount;
};
/**	\} */


///	performs a const cast

template <typename T, template <class TT> class FreePolicy>
inline ConstSmartPtr<T, FreePolicy> SmartPtr<T, FreePolicy>::cast_const() const{
	return ConstSmartPtr<T, FreePolicy>(*this);
}


/**	The SmartPtr<void> is a specialization of the SmartPtr class.
 * It can only be constructed from an existing SmartPtr or from an
 * existing SmartPtr<void>. This is crucial to guarantee save
 * release methods.
 *
 * In contrary to the original SmartPtr class, the void specialization
 * does not feature the -> and * operators. One also can't directly access
 * the encapsulated pointer.
 *
 * If you need a const smart pointer use ConstSmartPtr<void>.
 *
 * \todo	add to_smart_ptr_dynamic
 *
 * \{
 */
template <>
class SmartPtr<void>
{
	friend class ConstSmartPtr<void>;

	public:
		SmartPtr() : m_ptr(0), m_refCountPtr(0), m_freeFunc(0) {}

		SmartPtr(const SmartPtr<void>& sp) :
			m_ptr(sp.m_ptr),
			m_refCountPtr(sp.m_refCountPtr),
			m_freeFunc(sp.m_freeFunc)
		{
			if(m_refCountPtr) (*m_refCountPtr)++;
		}

		SmartPtr(void* ptr, void (*freeFunc)(const void*)) :
			m_ptr(ptr),
			m_refCountPtr(0),
			m_freeFunc(freeFunc)
		{
			if(ptr) m_refCountPtr = new int(1);
		}

		template <class T>
		SmartPtr(const SmartPtr<T>& sp) :
			m_ptr((void*)sp.m_ptr),
			m_refCountPtr(sp.m_refCount),
			m_freeFunc(&SmartPtr<T>::free_void_ptr)
		{
			if(m_refCountPtr) (*m_refCountPtr)++;
		}

		~SmartPtr() {release();}

		SmartPtr<void>& operator=(const SmartPtr<void>& sp)
		{
			if(m_ptr)
				release();
			m_ptr = sp.m_ptr;
			m_refCountPtr = sp.m_refCountPtr;
			if(m_refCountPtr)
				(*m_refCountPtr)++;
			m_freeFunc = sp.m_freeFunc;
			return *this;
		}

		template <class T>
		SmartPtr<void>& operator=(const SmartPtr<T>& sp)
		{
			if(m_ptr)
				release();
			m_ptr = sp.m_ptr;
			m_refCountPtr = sp.m_refCount;
			if(m_refCountPtr)
				(*m_refCountPtr)++;
			m_freeFunc = &SmartPtr<T>::free_void_ptr;
			return *this;
		}

	///	Returns a SmartPtr with the specified type and shared reference counting.
	/**	USE WITH CARE! ONLY COMPATIBLE TYPES SHOULD BE USED*/
		template <class T,  template <class TPtr> class TFreePolicy>
		SmartPtr<T, TFreePolicy> cast_reinterpret() const {
			return SmartPtr<T, TFreePolicy>(reinterpret_cast<T*>(m_ptr), m_refCountPtr);
		}

	///	sets the void* to a different location correspoding to a cast to a new type T
	/**	!!! WARNING: THIS METHOD IS DANDGEROUS: DO NOT USE IT UNLESS YOU REALLY
	 *               KNOW WHAT YOU ARE DOING !!!
	 */
		template <class T, template <class TPtr> class TFreePolicy>
		void set_impl(void* ptr)
		{
			m_ptr = ptr;
			m_freeFunc = &SmartPtr<T, TFreePolicy>::free_void_ptr;
		}

		inline bool is_valid() const	{return m_ptr != NULL;}
		void invalidate()				{if(is_valid())	release(); m_ptr = NULL;}

		void* get_impl()				{return m_ptr;}
		const void* get_impl() const	{return m_ptr;}

		int get_refcount() const {if(m_refCountPtr) return *m_refCountPtr; return 0;}

	private:
		void release() {
			if(m_refCountPtr)
			{
				(*m_refCountPtr)--;
				if((*m_refCountPtr) < 1)
				{
					delete m_refCountPtr;
					m_freeFunc(m_ptr);
				}
			}
		}

		void* m_ptr;
		int* m_refCountPtr;
		void (*m_freeFunc)(const void*);
};

template <>
class ConstSmartPtr<void>
{
	public:
		ConstSmartPtr() : m_ptr(0), m_refCountPtr(0), m_freeFunc(0) {}

		ConstSmartPtr(void* ptr, void (*freeFunc)(const void*)) :
			m_ptr(ptr),
			m_refCountPtr(0),
			m_freeFunc(freeFunc)
		{
			if(ptr) m_refCountPtr = new int(1);
		}

		ConstSmartPtr(const SmartPtr<void>& sp) :
			m_ptr(sp.m_ptr),
			m_refCountPtr(sp.m_refCountPtr),
			m_freeFunc(sp.m_freeFunc)
		{
			if(m_refCountPtr) (*m_refCountPtr)++;
		}

		ConstSmartPtr(const ConstSmartPtr<void>& sp) :
			m_ptr(sp.m_ptr),
			m_refCountPtr(sp.m_refCountPtr),
			m_freeFunc(sp.m_freeFunc)
		{
			if(m_refCountPtr) (*m_refCountPtr)++;
		}

		template <class T, template <class TPtr> class TFreePolicy>
		ConstSmartPtr(const SmartPtr<T, TFreePolicy>& sp) :
			m_ptr((void*)sp.m_ptr),
			m_refCountPtr(sp.m_refCount),
			m_freeFunc(&SmartPtr<T, TFreePolicy>::free_void_ptr)
		{
			if(m_refCountPtr) (*m_refCountPtr)++;
		}

		template <class T, template <class TPtr> class TFreePolicy>
		ConstSmartPtr(const ConstSmartPtr<T, TFreePolicy>& sp) :
			m_ptr((void*)sp.m_ptr),
			m_refCountPtr(sp.m_refCount),
			m_freeFunc(&SmartPtr<T, TFreePolicy>::free_void_ptr)
		{
			if(m_refCountPtr) (*m_refCountPtr)++;
		}

		~ConstSmartPtr() {release();}

		ConstSmartPtr<void>& operator=(const SmartPtr<void>& sp)
		{
			if(m_ptr)
				release();
			m_ptr = sp.m_ptr;
			m_refCountPtr = sp.m_refCountPtr;
			if(m_refCountPtr)
				(*m_refCountPtr)++;
			m_freeFunc = sp.m_freeFunc;
			return *this;
		}

		ConstSmartPtr<void>& operator=(const ConstSmartPtr<void>& sp)
		{
			if(m_ptr)
				release();
			m_ptr = sp.m_ptr;
			m_refCountPtr = sp.m_refCountPtr;
			if(m_refCountPtr)
				(*m_refCountPtr)++;
			m_freeFunc = sp.m_freeFunc;
			return *this;
		}

		template <class T, template <class TPtr> class TFreePolicy>
		ConstSmartPtr<void>& operator=(const SmartPtr<T, TFreePolicy>& sp)
		{
			if(m_ptr)
				release();
			m_ptr = sp.m_ptr;
			m_refCountPtr = sp.m_refCount;
			if(m_refCountPtr)
				(*m_refCountPtr)++;
			m_freeFunc = &SmartPtr<T, TFreePolicy>::free_void_ptr;
			return *this;
		}

		template <class T, template <class TPtr> class TFreePolicy>
		ConstSmartPtr<void>& operator=(const ConstSmartPtr<T, TFreePolicy>& sp)
		{
			if(m_ptr)
				release();
			m_ptr = sp.m_ptr;
			m_refCountPtr = sp.m_refCount;
			if(m_refCountPtr)
				(*m_refCountPtr)++;
			m_freeFunc = &SmartPtr<T, TFreePolicy>::free_void_ptr;
			return *this;
		}

	///	Returns a SmartPtr with the specified type and shared reference counting.
	/**	USE WITH CARE! ONLY COMPATIBLE TYPES SHOULD BE USED*/
		template <class T, template <class TPtr> class TFreePolicy>
		ConstSmartPtr<T, TFreePolicy> cast_reinterpret() const{
			return ConstSmartPtr<T, TFreePolicy>(reinterpret_cast<const T*>(m_ptr), m_refCountPtr);
		}

	///	sets the void* to a different location correspoding to a cast to a new type T
	/**	!!! WARNING: THIS METHOD IS DANDGEROUS: DO NOT USE IT UNLESS YOU REALLY
	 *               KNOW WHAT YOU ARE DOING !!!
	 */
		template <class T, template <class TPtr> class TFreePolicy>
		void set_impl(const void* ptr)
		{
			m_ptr = ptr;
			m_freeFunc = &SmartPtr<T, TFreePolicy>::free_void_ptr;
		}

		inline bool is_valid() const	{return m_ptr != NULL;}
		void invalidate()				{if(is_valid())	release(); m_ptr = NULL;}

		const void* get_impl() const	{return m_ptr;}

		int get_refcount() const {if(m_refCountPtr) return *m_refCountPtr; return 0;}

	private:
		void release() {
			if(m_refCountPtr)
			{
				(*m_refCountPtr)--;
				if((*m_refCountPtr) < 1)
				{
					delete m_refCountPtr;
					m_freeFunc(const_cast<void*>(m_ptr));
				}
			}
		}

		const void* m_ptr;
		int* m_refCountPtr;
		void (*m_freeFunc)(const void*);
};



namespace std
{
	template <class T, template <class TPtr> class TFreePolicy>
	struct less<SmartPtr<T, TFreePolicy> > :
		public binary_function<SmartPtr<T, TFreePolicy>, SmartPtr<T, TFreePolicy>, bool>
	{
		bool operator()(const SmartPtr<T, TFreePolicy>& lhs,
						const SmartPtr<T, TFreePolicy>& rhs) const
		{
			return less<T*>()(lhs.get_impl(), rhs.get_impl());
		}
	};
}

#endif
