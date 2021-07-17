#include "types.h"
#include "str.h"
#include "endianUtil.h"
#include "util.h"
#include "log.h"
#include "mem.h"
#include "file.h"
#include "save.h"
#include "stack.h"

#include <cstring>

// Save buffer to file
ubool save_t::saveFile(const char *filename, stack_t<save_var_t> &buf) const {
	// Open save file
	file_handle_t *f = m_f.open(filename, FILE_MODE_WRITE);
	if (!f) {
		log_warning("Failed to open %s!", filename);
		return false;
	}

	// Write header
	save_data_magic_t magic;
	magic.magic = SAVE_DATA1_MAGIC;

	if (f->write(&magic, sizeof(magic)) != (iptr)sizeof(magic)) {
		log_warning("Cannot write save magic to %s!", filename);
		f->close();
		return false;
	}

	// Padding for variable data
	const u8 zero[15] = {0};

	// Write variable list
	save_data1_var_t out;
	for (save_var_t *i = buf.ptr(); i != buf.end(); ++i) {
		out.size = i->size;
		out.name = i->name;

		if ((f->write(&out, sizeof(out)) < (iptr)sizeof(out)) ||
			(f->write(i->val, i->size)  < (iptr)i->size))
		{
			log_warning("Cannot write save variable to %s!", filename);
			f->close();
			return false;
		}

		// Add padding if necessary
		if (i->size&0xf) {
			const iptr zeros = 16-(i->size&0xf);
			if (f->write(zero, zeros) < zeros) {
				log_warning("Cannot write save variable padding to %s!", filename);

				f->close();
				return false;
			}
		}
	}

	// Write variable list terminator
	out.size = 0;
	if (f->write(&out, sizeof(out)) < (iptr)sizeof(out)) {
		log_warning("Cannot write variable list terminator to %s!", filename);
		f->close();
		return false;
	}
	
	f->close();

	return true;
}

ubool save_t::loadFile(const char *filename, stack_t<save_var_t> &buf) {
	// Clean variable memory
	cleanVarMem();

	// Save m_varMemCur
	u8 *varMemCur = m_varMemCur;
	
	// Open save file
	file_handle_t *f = m_f.open(filename, FILE_MODE_READ);
	if (!f) {
		log_warning("Cannot open save file %s!", filename);
		return false;
	}

	// Make sure this is a save file
	save_data_magic_t magic;
	if (f->read(&magic, sizeof(magic)) < (iptr)sizeof(magic)) {
		log_warning("Cannot read save file magic from %s!", filename);
		f->close();
		return false;
	}

	// Read differently depending on save file version
	switch (magic.magic) {

		// Save file format, version 1
	case SAVE_DATA1_MAGIC: {
		save_data1_var_t in;
		save_var_t *out;

		// Variable memory
		u8 varMem[1024];

		// Read variable list
		for (; !in.end();) {
			if (f->read(&in, sizeof(in)) < (iptr)sizeof(in)) {
				log_warning("Early end of file in %s!", filename);

				// Just assume that was the end
				f->close();
				return true;
			}

			// Add variable to memory
			if (f->read(varMem, in.size) < (iptr)in.size) {
				log_warning("Cannot read variable memory in %s!", filename);

				buf.clear();
				f->close();
				m_varMemCur = varMemCur;
				return false;
			}
			
			out = &buf.push();
			out->val = pushVarMem(varMem, in.size);
			out->size = in.size;
			out->name = in.name;

			// Was variable memory push successful?
			if (!out->val) {
				// TODO: Attempt to resize variable memory stack instead of throwing error!
				log_warning("Out of variable memory!");
				
				buf.clear();
				f->close();
				m_varMemCur = varMemCur;
				return false;
			}
		}

		// Return success
		return true;
	} break;

		// Unknown format
	default:
		magic.pad[0] = 0; // NULL terminator for "string"
		log_warning("Invalid save file format %s in %s!", (char*)&magic.magic);

		f->close();
		return false;
	}

	f->close();
	return true;
}

// Push variable memory to memory stack
void *save_t::pushVarMem(const u8 *ptr, uptr size, uptr realSize) {
	if (m_varMemCur+realSize > m_varMemEnd) {
		log_warning("Out of variable memory!");
		return NULL;
	}

	memcpy(m_varMemCur, ptr, size);
	void *ret = (void*)m_varMemCur;
	m_varMemCur += realSize;

	return ret;
}

// Clear loaded file from variable memory
void save_t::cleanVarMem() {
	// Reset variable memory cursor
	m_varMemCur = m_varMem;

	// Loop through m_gvars, adding just global variables to the variable memory stack
	for (save_var_t *i = m_gvars.ptr(); i != m_gvars.end(); ++i) {
		// Write variable to current pointer
		log_assert((i->size&0xf) == 0, "Misaligned global variable size!");

		// Considering we're working with the same variables, we shouldn't run out of memory
		log_assert(m_varMemCur+i->size < m_varMemEnd, "Out of variable copy memory!");

		// We shouldn't be reading data that's already written
		log_assert(m_varMemCur < (u8*)i->val, "Outran global variable data pointers!");

		// memcpy doesn't support memory area overlap, so use memmove
		memmove(m_varMemCur, i->val, i->size);
		i->val = m_varMemCur;
		m_varMemCur += util_alignUp<uptr>(i->size, 16);
	}
}

save_t::save_t(mem_t &m, file_system_t &f)
	: m_m(m), m_f(f),
	  m_gvars(m_m, 256), m_vars(m_m, 256)
{
	// Create save directory, if it doesn't exist
	if (!m_f.makeDir("save")) throw log_except("Cannot create save directory!");
	
	// Allocate variable memory
	constexpr uptr varMemSize = 32768;
	m_varMemCur = m_varMem = (u8*)m_m.alloc(varMemSize);
	m_varMemEnd = m_varMem+varMemSize;

	// Load global save file into memory
	loadFile("save/global.dat", m_gvars);

	// No save file is currently loaded
	m_curSave = -1;
}

save_t::~save_t() {
	// Free variable memory
	m_m.free(m_varMem);

	// Stacks will deconstruct by themselves
}

uptr save_t::get(str_hash_t name, void *out, uptr outSize, ubool global) const {
	const save_var_t *i, *end;

	// If global is true, search global variables
	// Otherwise, search loaded save variables
	if (global) {
		i = m_gvars.ptr();
		end = m_gvars.end();
	} else {
		i = m_vars.ptr();
		end = m_vars.end();
	}

	for (; i != end; ++i) {
		if (i->name == name) {
			memcpy(out, i->val, util_min(i->size, outSize));
			return i->size;
		}
	}

	return 0;
}

ubool save_t::set(str_hash_t name, const void *in, uptr inSize, ubool global) {
	save_var_t *i, *end;

	// If global is true, search global variables
	// Otherwise, search loaded save variables
	if (global) {
		i = m_gvars.ptr();
		end = m_gvars.end();
	} else {
		i = m_vars.ptr();
		end = m_vars.end();

		// Return false if there's no save file loaded,
		// don't wanna create a variable in an invalid save file
		if (m_curSave < 0) {
			log_warning("Setting variable in invalid save file!");
			return false;
		}
	}

	for (; i != end; ++i) {
		if (i->name == name) {
			if (inSize > i->size) {
				log_warning("%u is bigger than variable's size of %u!", (u32)inSize, (u32)i->size);
				return false;
			}

			// Write into data buffer
			memcpy(i->val, in, inSize);
			return true;
		}
	}

	// Variable not found, create it
	stack_t<save_var_t> &buf = global ? m_gvars : m_vars;
	save_var_t &v = buf.push();

	v.size = util_alignUp<uptr>(inSize, 16);
	v.val = pushVarMem((const u8*)in, inSize, v.size);
	v.name = name;

	// Did pushVarMem succeed?
	if (!v.val) {
		log_warning("Out of variable memory!");

		buf.pop();
		return false;
	}

	// pushVarMem succeeded, return true
	return true;
}

ubool save_t::load(u32 id) {
	char name[64];

	// Create save file name
	memcpy(name, "save/save", 9);

	strcat((name+9) + str_numstr(name+9, id), ".dat");

	// Load file
	return loadFile(name, m_vars);
}
