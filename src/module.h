#ifndef MODULE_H
#define MODULE_H

#include "types.h"
#include "log.h"

// Forward-declaration of module struct
struct modules_t;

// Base class for modules, includes overrides for operator new and operator delete
//
//   maxSize: Maximum size of derived class module
template<uptr maxSize>
class module_base_t {
public:
	static FINLINE void *operator new(uptr size, void *store) {
		log_assert(size <= maxSize,
				   "Module datastore is too big! (%u > %u)",
				   (u32)size, (u32)maxSize);
		return store;
	}

	// Used to call destructor of base and derived classes
	static FINLINE void operator delete(void *ptr) {(void)ptr;}
};

// Constructor callback for modules, used in module_inst_t::set
// Constructs the class for the given backend
//
// Function should report an error if backend constructor failed
template<class T, typename initType>
using module_constructProc_t = ubool (*)(T*, initType&);

// Container class for modules, includes data store
//
//   T: Contained class
//   backendType: Enum type for module backend
//   backendCount: Number of backends available for module
//   initType: Type for initializing module
//   maxSize: Maximum data sizse of module backend
//   constructTable: Table containing function pointers for constructing backends
template<class T,
		 typename backendType, backendType backendCount,
		 typename initType, uptr maxSize,
		 const module_constructProc_t<T, initType> *constructTable>
class module_inst_t {
private:
	alignas(16) u8 m_store[maxSize];
	ubool m_active = false;

public:
	FINLINE void destroy() {
		if (m_active) {
			delete (T*)m_store;
			m_active = false;
		}
	}

	FINLINE ~module_inst_t() {
		destroy();
	}

	FINLINE operator T*() {return (T*)m_store;}
	FINLINE T *operator->() {return (T*)m_store;}
	
	FINLINE ubool set(backendType type, initType &args) {
		log_assert(type < backendCount, "Backend type out of bounds! (%d, %d)", (i32)type, (i32)backendCount);
		
		// Make sure module is destroyed
		destroy();

		// Construct following backend, return false if construction failed
		// Module will be inactive if construction failed, true otherwise
		return m_active = constructTable[(uptr)type]((T*)m_store, args);
	}

	// Same as set, but if construction fails, fallback to valid backends
	// Returns backendCount on error
	backendType setFallback(backendType type, initType &args) {
		log_assert(type < backendCount, "Backend type out of bounds! (%d, %d)", (i32)type, (i32)backendCount);
		
		// Make sure the module is destroyed
		destroy();

		// Loop through all backends, trying to construct one
		ubool first = true;
		for (uptr i = (uptr)type; first || (i != (uptr)type); i = (i+1)%(uptr)backendCount) {
			first = false;

			if (i >= (uptr)backendCount) i = 0;
			if ((m_active = constructTable[i]((T*)m_store, args))) return (backendType)i;
		}

		log_warning("Cannot construct module!");
		return backendCount;
	}

	FINLINE ubool active() const {return m_active;}
};

#endif //MODULE_H
