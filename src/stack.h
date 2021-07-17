#ifndef STACK_H
#define STACK_H

#include "types.h"
#include "mem.h"
#include "log.h"

#include <string.h>

// A stack where you can push and pop items
// T must have a default constructor and destructor
template<typename T>
class stack_t {
private:
	mem_t &m_m;

	T *m_buf, *m_cur;
	uptr m_count, m_size;

	ubool resize(uptr newSize) {
		T *newBuf;
		
		try {
			newBuf = (T*)m_m.alloc(newSize*sizeof(T));
		} catch (const log_except_t &err) {
			log_warning("Cannot resize stack: %s", err.str());
			return false;
		}

		memcpy((void*)newBuf, m_buf, m_count*sizeof(T));

		m_cur = m_cur-m_buf+newBuf;
		
		m_m.free(m_buf);
		m_buf = newBuf;

		return true;
	}

public:
	FINLINE stack_t(mem_t &m, uptr baseSize) : m_m(m) {
		m_size = baseSize;
		m_count = 0;

		m_cur = m_buf = (T*)m_m.alloc(m_size*sizeof(T));
	}

	// Index stack
	FINLINE T &operator[](int i) {
		log_assert(i < m_count, "Index out of bounds!");
		return m_buf[i];
	}

	// Push new object onto stack
	T &push() {
		// Resize buffer if needed
		if (++m_count > m_size) {
			if (!resize(m_size*2)) throw log_except("Out of memory!");
		}
		
		// Construct and return object
		(void)new((void*)m_cur) T;
		++m_count;
		
		return *m_cur++;
	}

	// Pop object off top of stack
	FINLINE void pop() {
		log_assert(m_count, "Popping with no objects on stack!");
		
		// Deconstruct object and go back
		// (this would be unreadable without parenthesis)
		(--m_cur)->~T();
		--m_count;
	}

	FINLINE uptr size() {return m_count;}
	
	FINLINE T *ptr() {return m_buf;}
	FINLINE const T *ptr() const {return m_buf;}
	
	FINLINE T *end() {return m_cur;}
	FINLINE const T *end() const {return m_cur;}

	// Clear stack contents
	FINLINE void clear() {
		// Pop every object off the stack
		while (m_count) pop();
	}
};

#endif //STACK_H
