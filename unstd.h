#ifndef UNSTD_H
#define UNSTD_H


#include <memory>
#include <vector>
#include <string>
#include <sstream>
#include <type_traits>


namespace unstd
{
	struct Exception : std::exception
	{
	private:
		Exception* m_parent = nullptr;
		std::string m_message;
	
	
	private:
		template<typename T>
		void AppendMessage(std::ostringstream& target, T &t)
		{
			target << t;
		}
		
		template<typename T, typename ... K>
		void AppendMessage(std::ostringstream& target, T t, K ... k)
		{
			target << t;
			AppendMessage(target, k...);
		}
		
		
	public:
		~Exception() override
		{
			delete m_parent;
		}
		
		Exception() = delete;
		Exception& operator=(Exception&& other) = delete;
		Exception& operator=(const Exception& other) = delete;
		Exception(const Exception& e): m_message(e.m_message) {}
		Exception(Exception && e) noexcept : m_message(std::move(e.m_message)) {}
		
		explicit Exception(const Exception* e): m_message(e->m_message) {}
		explicit Exception(std::string && message): m_message(message) {}
		explicit Exception(char const* message): m_message(message) {}
		
		Exception(const Exception& parent, char const* message): m_parent(new Exception(parent)), m_message(message) {}
		
		
		template <typename ... T>
		explicit Exception(const Exception& parent, T... t): 
			m_parent(new Exception(parent))
		{
			std::ostringstream s;
			AppendMessage(s, t...);
			m_message = s.str();
		}
		
		template <typename ... T>
		explicit Exception(T... t)
		{
			std::ostringstream s;
			AppendMessage(s, t...);
			m_message = s.str();
		}
		
		
	public:
		inline bool HasParent() const noexcept { return m_parent != nullptr; }
		inline Exception* Parent() const noexcept { return m_parent; }
		std::string Message() const noexcept 
		{
			if (m_parent == nullptr)
			{
				return m_message;
			}
			else
			{
				return m_message + " " + m_parent->m_message;
			}
		}
		
		
	public:
		char const* what() const noexcept override { return m_message.c_str(); }
	};
	
	
	// Types
	
	typedef char				byte;
	typedef unsigned char		ubyte;
	typedef unsigned int		uint;
	
	typedef int16_t				int16;
	typedef int32_t				int32;
	typedef int64_t				int64;
	
	typedef uint16_t			uint16;
	typedef uint32_t			uint32;
	typedef uint64_t			uint64;
	
	
	template <typename ... T>
	using callable = void (*)(T...);
	
	template <typename R, typename ... T>
	using lambda = R (*)(T...);
	
	template <typename I, typename ... T>
	using instance_callable = void (I::*)(T...);

	
	template <class T>
	using sptr = std::shared_ptr<T>;
	
	template <class T>
	using uptr = std::unique_ptr<T>;
	
	template <class T>
	using wptr = std::weak_ptr<T>;
	
	template <class T>
	using v = std::vector<T>;
	
	
	template<class T, class U>
	inline wptr<T> static_pointer_cast(wptr<U> const& r)
	{
		return std::static_pointer_cast<T>(std::shared_ptr<U>(r));
	}
	
	template <class T, class ...PRMS>
	inline sptr<T> shared(PRMS... prms)
	{
		return std::make_shared<T>(prms...);
	}
	
	template <class T, class ...PRMS>
	inline uptr<T> unique(PRMS... prms)
	{
		return std::make_unique<T>(prms...);
	}
	
	
	template <class T>
	class mptr;
	
	template <class T>
	class wmptr;
	
	
	template <class T>
	class wmptr
	{
	private:
		wptr<T>	m_wptr { nullptr };
		sptr<T>	m_sptr { nullptr };
		T*		m_ptr { nullptr };
	
	public:
		wmptr() = default;
		
		virtual ~wmptr() = default;
		wmptr(wmptr&&) noexcept = default;
		wmptr(const wmptr&) = default;
		wmptr& operator=(const wmptr& other) = default;
		wmptr& operator=(wmptr&& other) noexcept = default;
		
		
		explicit wmptr(wptr<T> ptr):
			m_wptr{ptr},
			m_sptr{nullptr},
			m_ptr(nullptr) {}
		
		explicit wmptr(sptr<T> sptr):
			m_wptr{nullptr},
			m_sptr(sptr),
			m_ptr(sptr.get()) {}
		
		explicit wmptr(T* ptr):
			m_wptr{nullptr},
			m_sptr{nullptr},
			m_ptr(ptr) {}
		
		
		mptr<T> Lock()
		{
			if (m_sptr) return mptr<T>(m_sptr);
			else if (m_ptr) return mptr<T>(m_ptr);
			else return mptr<T>(m_wptr.lock());
		}
		
		
		inline explicit operator bool() 
		{
			return (m_ptr || (bool)(m_wptr.lock()));
		}
		
		inline explicit operator mptr<T>()
		{
			return Lock();
		}
	};
	
	template <class T>
	class mptr
	{
		template<typename K>
		friend class mptr;
	
		
	private:
		T*		m_ptr { nullptr };
		sptr<T>	m_sptr { nullptr };
		
	public:
		mptr() = default;
		
		virtual ~mptr() = default;
		mptr(mptr&&) noexcept = default;
		mptr(const mptr&) = default;
		mptr& operator=(const mptr& other) = default;
		mptr& operator=(mptr&& other) noexcept = default;
		
		
		explicit mptr(sptr<T> sptr):
			m_sptr(sptr),
			m_ptr(sptr.get()) {}
		
		explicit mptr(T* ptr):
			m_sptr{nullptr},
			m_ptr(ptr) {}
			
			
		template<class K, class = typename std::enable_if<std::is_base_of<T, K>::value>::type>
		explicit mptr(const mptr<K>& m):
			m_sptr(m.m_sptr ? sptr<T>(m.m_sptr) : nullptr),
			m_ptr((T*)m.m_ptr) {}
		
		
		T* operator->() const
		{
			if (m_ptr == nullptr)
				throw Exception("mptr should not contain null pointers");
			
			return m_ptr;
		}
		
		T& operator*()
		{
			if (m_ptr == nullptr)
				throw Exception("mptr should not contain null pointers");
			
			return *m_ptr;
		}
		
		T* Get()
		{
			if (m_ptr == nullptr)
				throw Exception("mptr should not contain null pointers");
			
			return m_ptr;
		}
		
			
		explicit operator wmptr<T>()
		{
			if (m_sptr) return wmptr<T>(m_sptr);
			else return wmptr<T>(m_ptr);
		}
		
		
		template<class K, class = typename std::enable_if<std::is_base_of<T, K>::value>::type>
		explicit operator wmptr<K>()
		{
			if (m_sptr) return wmptr<K>(static_cast<K>(m_sptr));
			else return wmptr<K>((K*)m_ptr);
		}
		
		inline explicit operator bool() const
		{
			return m_ptr != nullptr;
		}
		
		
		template<class ...PRMS>
		inline static mptr<T> Shared(PRMS ... p)
		{
			return mptr<T>(shared<T>(p...));
		}
	};
	
	
	template <typename I, typename ... T>
	inline void invoke_callback(I& i, instance_callable<I, T...> c, T... t)
	{
		(i.*c)(t...);
	}
	
	template <typename I, typename ... T>
	inline void invoke_callback(I* i, instance_callable<I, T...> c, T... t)
	{
		(i->*c)(t...);
	}
	
	template <typename I, typename ... T>
	inline void invoke_callback(sptr<I> i, instance_callable<I, T...> c, T... t)
	{
		(i->*c)(t...);
	}
	
	template <typename I, typename ... T>
	inline bool  invoke_callback(wptr<I> i, instance_callable<I, T...> c, T... t)
	{
		auto s = i.lock();
		
		if (s)
		{
			(s->*c)(t...);
			return true;
		}
		
		return false;
	}
	
	template <typename I, typename ... T>
	inline void invoke_callback(mptr<I> i, instance_callable<I, T...> c, T... t)
	{
		(i->*c)(t...);
	}
	
	template <typename I, typename ... T>
	inline bool invoke_callback(wmptr<I> i, instance_callable<I, T...> c, T... t)
	{
		auto s = i.Lock();
		
		if (s)
		{
			(s->*c)(t...);
			return true;
		}
		
		return false;
	}
}


#endif