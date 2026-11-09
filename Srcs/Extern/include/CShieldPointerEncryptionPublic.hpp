#pragma once

#include <windows.h>
#include <atomic>

#ifdef CShield_EXPORTS
#define CShield_API __declspec(dllexport)
#else
#define CShield_API __declspec(dllimport)
#endif

namespace PointerCrypto
{
	CShield_API DWORD_PTR EncryptPointer(const void* ptr);
	CShield_API void* DecryptPointer(DWORD_PTR encrypted);
	CShield_API bool ValidatePointer(const void* ptr);
	CShield_API DWORD CalculateChecksum(DWORD_PTR encrypted);
}

//////////////////////////////////////////////////////////////////////////
// EncryptedPtr - Template wrapper for encrypted pointer storage
//////////////////////////////////////////////////////////////////////////
template <typename T>
class EncryptedPtr
{
public:
	// Constructors
	EncryptedPtr()
		: m_EncryptedValue(0)
		, m_Checksum(0)
	{
	}

	EncryptedPtr(T* ptr)
		: m_EncryptedValue(0)
		, m_Checksum(0)
	{
		Set(ptr);
	}

	EncryptedPtr(const EncryptedPtr& other)
		: m_EncryptedValue(other.m_EncryptedValue)
		, m_Checksum(other.m_Checksum)
	{
#ifdef DEBUG_MODE
		static std::atomic<uint32_t> copyCount{ 0 };
		if ((copyCount.fetch_add(1, std::memory_order_relaxed) % 1000) == 0)
		{
			printf("[EncryptedPtr] Copy constructor: Transferred encrypted state (encrypted=0x%p, count=%u)\n ",
				(void*)m_EncryptedValue, copyCount.load());
		}
#endif
	}

	// Move constructor - transfers encrypted value without re-encryption
	EncryptedPtr(EncryptedPtr&& other) noexcept
		: m_EncryptedValue(other.m_EncryptedValue)
		, m_Checksum(other.m_Checksum)
	{
		// Reset the moved-from object to a valid empty state
		other.m_EncryptedValue = 0;
		other.m_Checksum = 0;
	}

	// Assignment operators
	EncryptedPtr& operator=(T* ptr)
	{
		Set(ptr);
		return *this;
	}

	EncryptedPtr& operator=(const EncryptedPtr& other)
	{
		if (this != &other)
		{
			m_EncryptedValue = other.m_EncryptedValue;
			m_Checksum = other.m_Checksum;

#ifdef DEBUG_MODE
			static std::atomic<uint32_t> assignCount{ 0 };
			if ((assignCount.fetch_add(1, std::memory_order_relaxed) % 1000) == 0)
			{
				printf("[EncryptedPtr] Copy assignment: Transferred encrypted state (encrypted=0x%p, count=%u)\n",
					(void*)m_EncryptedValue, assignCount.load());
			}
#endif
		}
		return *this;
	}

	// Move assignment operator - transfers encrypted value without re-encryption
	EncryptedPtr& operator=(EncryptedPtr&& other) noexcept
	{
		if (this != &other)
		{
			m_EncryptedValue = other.m_EncryptedValue;
			m_Checksum = other.m_Checksum;

			// Reset the moved-from object to a valid empty state
			other.m_EncryptedValue = 0;
			other.m_Checksum = 0;
		}
		return *this;
	}

	// Getter with validation
	T* Get() const
	{
		// nullptr check
		if (m_EncryptedValue == 0)
		{
#ifdef DEBUG_MODE
			if (m_Checksum != 0)
			{
				static std::atomic<uint32_t> nullptrInconsistencyCount{ 0 };
				if ((nullptrInconsistencyCount.fetch_add(1, std::memory_order_relaxed) % 100) == 0)
				{
					printf("[EncryptedPtr][ERROR] Get: nullptr with non-zero checksum! checksum=0x%08X\n",
						m_Checksum);
				}
			}
#endif
			return nullptr;
		}

		DWORD computedChecksum = PointerCrypto::CalculateChecksum(m_EncryptedValue);

		// Verify checksum (detects bit-level corruption)
		if (m_Checksum != computedChecksum)
		{
#ifdef DEBUG_MODE
			printf("[EncryptedPtr][CRITICAL] Get: Checksum verification failed! encrypted=0x%p, stored_checksum=0x%08X, computed=0x%08X\n",
				(void*)m_EncryptedValue, m_Checksum, computedChecksum);
#endif
			return nullptr;
		}

		void* decrypted = PointerCrypto::DecryptPointer(m_EncryptedValue);
		T* ptr = static_cast<T*>(decrypted);

		// Validate decrypted pointer
		if (!PointerCrypto::ValidatePointer(ptr))
		{
#ifdef DEBUG_MODE
			printf("[EncryptedPtr][CRITICAL] Get: Pointer validation failed! encrypted=0x%p, decrypted=0x%p\n",
				(void*)m_EncryptedValue, decrypted);
#endif
			return nullptr;
		}

#ifdef DEBUG_MODE
		static std::atomic<uint32_t> successCount{ 0 };
		if ((successCount.fetch_add(1, std::memory_order_relaxed) % 1000000) == 0)
		{
			printf("[EncryptedPtr] Get success: encrypted=0x%p, decrypted=0x%p\n",
				(void*)m_EncryptedValue, decrypted);
		}
#endif

		return ptr;
	}

	// Setter with encryption
	void Set(T* ptr)
	{
#ifdef DEBUG_MODE
		static std::atomic<uint32_t> setCount{ 0 };
		if ((setCount.fetch_add(1, std::memory_order_relaxed) % 10000) == 0)
		{
			printf("[EncryptedPtr] Set: ptr=0x%p (this=0x%p)\n",
				ptr, (void*)this);
		}
#endif

		m_EncryptedValue = PointerCrypto::EncryptPointer(ptr);
		m_Checksum = PointerCrypto::CalculateChecksum(m_EncryptedValue);

#ifdef DEBUG_MODE
		if (ptr != nullptr)
		{
			void* testDecrypt = PointerCrypto::DecryptPointer(m_EncryptedValue);
			if (testDecrypt != ptr)
			{
				printf("[EncryptedPtr][CRITICAL] Set: Round-trip verification FAILED! ptr=0x%p, encrypted=0x%p, decrypted=0x%p\n",
					ptr, (void*)m_EncryptedValue, testDecrypt);
			}
		}
#endif
	}

	// Dereference operators
	T* operator->() const
	{
		T* ptr = Get();
		return ptr;
	}

	T& operator*() const
	{
		T* ptr = Get();
		return *ptr;
	}

	// Comparison operators
	bool operator==(const T* ptr) const
	{
		return Get() == ptr;
	}

	bool operator!=(const T* ptr) const
	{
		return Get() != ptr;
	}

	bool operator==(const EncryptedPtr& other) const
	{
		return Get() == other.Get();
	}

	bool operator!=(const EncryptedPtr& other) const
	{
		return Get() != other.Get();
	}

	// Boolean conversion for null checks
	operator bool() const
	{
		return Get() != nullptr;
	}

	bool operator!() const
	{
		return Get() == nullptr;
	}

	// Implicit conversion to raw pointer (for compatibility)
	operator T* () const
	{
		return Get();
	}

	// Explicit conversion to raw pointer
	T* ToRawPtr() const
	{
		return Get();
	}

	// Check if pointer is valid
	bool IsValid() const
	{
		return PointerCrypto::ValidatePointer(Get());
	}

	// Reset to nullptr
	void Reset()
	{
		Set(nullptr);
	}

	// Swap function for STL algorithm support (sort, etc.)
	void Swap(EncryptedPtr& other) noexcept
	{
		std::swap(m_EncryptedValue, other.m_EncryptedValue);
		std::swap(m_Checksum, other.m_Checksum);
	}

	// Friend swap for ADL (Argument-Dependent Lookup) - used by std::sort, etc.
	friend void swap(EncryptedPtr& a, EncryptedPtr& b) noexcept
	{
		a.Swap(b);
	}

 private:
	DWORD_PTR m_EncryptedValue;  // Encrypted pointer value
	DWORD m_Checksum;             // Integrity checksum
};

//////////////////////////////////////////////////////////////////////////
// EncryptedStaticPtr - For static/global pointers with singleton pattern
//////////////////////////////////////////////////////////////////////////
template <typename T>
class EncryptedStaticPtr
{
public:
	static void Set(T* ptr)
	{
		Get_Internal().Set(ptr);
	}

	static T* Get()
	{
		return Get_Internal().Get();
	}

	static bool IsValid()
	{
		return Get_Internal().IsValid();
	}

	static void Reset()
	{
		Get_Internal().Reset();
	}

private:
	static EncryptedPtr<T>& Get_Internal()
	{
		static EncryptedPtr<T> s_Instance;
		return s_Instance;
	}
};

// Define an encrypted pointer member
#define CSHIELD_ENCRYPTED_PTR(type, name) EncryptedPtr<type> name

// Define an encrypted static pointer
#define CSHIELD_ENCRYPTED_STATIC_PTR(type, name) \
	static type* Get##name() { return EncryptedStaticPtr<type>::Get(); } \
	static void Set##name(type* ptr) { EncryptedStaticPtr<type>::Set(ptr); }

// Access encrypted pointer (automatically decrypts)
#define CSHIELD_ACCESS_ENCRYPTED(ptr) ((ptr).Get())

// Set encrypted pointer value
#define CSHIELD_SET_ENCRYPTED(ptr, value) ((ptr).Set(value))
