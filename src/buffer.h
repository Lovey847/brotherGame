#ifndef BUFFER_H
#define BUFFER_H

#include "types.h"
#include "mem.h"
#include "log.h"

#include <string.h>

// T must have a default constructor and destructor
template<class T>
class buffer_t {
private:
	mem_t &m_m;
	
	T *m_buf;
	ubool *m_active; // Which objects are active and which aren't

	uptr m_cur, m_size;

	ubool resize(uptr newSize) {
		T *newBuf;

		try {
			newBuf = (T*)m_m.alloc(newSize*(sizeof(T) * sizeof(ubool)));
		} catch (const log_except_t &err) {
			log_warning("Failed to resize buffers: %s", err.str());
			return false;
		}
		
		ubool *newActive = (ubool*)(newBuf+newSize);

		memcpy((void*)newBuf, m_buf, m_size*sizeof(T));
		
		memcpy(newActive, m_active, m_size*sizeof(ubool));
		memset(newActive+m_size, 0, (newSize - m_size)*sizeof(ubool));

		m_m.free(m_buf);

		m_buf = newBuf;
		m_active = newActive;
		m_size = newSize;

		return true;
	}

	uptr cur() {
		uptr origCur = m_cur;

		// Mark current cursor as active
		m_active[m_cur] = true;

		// Advance cursor to next inactive object
		for (;;) {
			if (!m_active[m_cur]) return origCur;
			
			++m_cur;
			
			// Wrap around
			if (m_cur >= m_size) {
				m_cur = 0;
				break;
			}
		}

		while (m_active[m_cur]) {
			++m_cur;

			// Resize buffer if out of objects
			if (m_cur >= origCur) {
				m_cur = m_size;
				
				if (!resize(m_size*2))
					throw log_except("Out of objects!");
				
				return origCur;
			}
		}

		return origCur;
	}

public:
	buffer_t(mem_t &mem, uptr baseSize) : m_m(mem) {
		m_buf = (T*)m_m.alloc(baseSize*(sizeof(T) + sizeof(ubool)));
		m_active = (ubool*)(m_buf+baseSize);

		m_cur = 0;
		m_size = baseSize;

		memset(m_active, 0, baseSize*sizeof(ubool));
	}

	~buffer_t() {
		// Free current objects in object buffer
		// Objects should not be active in the buffer at time of destruction
		for (uptr obj = 0; obj < m_size; ++obj) {
			if (!m_active[obj]) continue;

			m_buf[obj].~T();
			log_warning("Object %u was active in buffer!", (u32)obj);
		}
		
		m_m.free(m_buf);
	}

	// Add object to buffer
	T &add() {
		const uptr c = cur();
		
		// Construct new object and mark it as active
		(void)new(m_buf+c) T();
		m_active[c] = true;

		// Return handle while advancing pointer
		return m_buf[c];
	}

	// Remove object from buffer
	void remove(T &obj) {
		// Get index of object in buffer
		uptr ind = &obj-m_buf;

		// Destruct object, and mark it as inactive
		obj.~T();
		m_active[ind] = false;
	}

	// Clear all items from buffer
	void clear() {
		// Free all objects in object buffer
		for (uptr obj = 0; obj < m_size; ++obj) {
			if (!m_active[obj]) continue;

			m_buf[obj].~T();
			m_active[obj] = false;
		}

		m_cur = 0;
	}
};

#endif //BUFFER_H
