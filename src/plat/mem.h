// Memory allocation handler
// All memory allocations are aligned to a 16-byte boundary

#ifndef MEM_H
#define MEM_H

#include "types.h"
#include "util.h"
#include "log.h"

// Placement new, usually this is defined in the C++ header new
// That means I have to make my own
FINLINE void *operator new(uptr size, void *addr) {
	(void)size;
	return addr;
}

class mem_t {
private:
	struct alignas(16) entry_t {
		entry_t *next, *prev;
		ubool active;
		
		FINLINE void *data() const {return (void*)(this+1);}
		FINLINE uptr entSize() const {return (u8*)next-(u8*)this;}
		FINLINE uptr size() const {return (u8*)next-(u8*)data();}
	};
	
	void *m_start, *m_end;
	
	// Functions for allocating/freeing block of memory
	// that are defined by the platform layer

	// Returned memory block is aligned to a 16-byte boundary
	void *allocBlock(uptr size); // Returns NULL on failure
	void freeBlock();

public:
	mem_t(uptr size);
	~mem_t();

	mem_t(const mem_t &other) = delete;
	
	// Allocate temporary memory, invalidated after
	// an allocation of any type
	void *allocTemp(uptr size) const;
	
	// Allocate basic memory
	void *alloc(uptr size);
	
	// Free allocation
	void free(void *addr);

	// Memory size
	FINLINE uptr size() const {return (u8*)m_end-(u8*)m_start;}
};

// Memory container for object, alocate memory on construction,
// free memory when out of scope
template<typename T>
class mem_container_t {
private:
	mem_t &m_m;

public:
	T *d;

	FINLINE mem_container_t(mem_t &m, uptr size) : m_m(m) {d = (T*)m.alloc(size);}
	FINLINE ~mem_container_t() {m_m.free(d);}
};

// Static memory container for object
template<typename T, uptr N = sizeof(T), uptr A = alignof(T)>
class mem_static_container_t {
private:
	alignas(A) u8 m_m[N];

public:
	FINLINE operator T*() {return (T*)m_m;}
	FINLINE operator T&() {return *(T*)m_m;}

	FINLINE operator const T*() const {return (const T*)m_m;}
	FINLINE operator const T&() const {return *(const T*)m_m;}

	template<typename O>
	FINLINE O *pcast() {return (O*)m_m;}
	template<typename O>
	FINLINE const O *pcast() const {return (const O*)m_m;}

	template<typename O>
	FINLINE O &rcast() {return *pcast<O>();}
	template<typename O>
	FINLINE const O &rcast() {return *pcast<O>();}
};

#endif //MEM_H
