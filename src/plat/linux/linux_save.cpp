#include "types.h"
#include "save.h"
#include "endianUtil.h"
#include "util.h"
#include "log.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

static FINLINE void makeFilename(char *out, uptr id) {
	log_assert(id < 10, "ID too big!");

	memcpy(out, "save .dat", sizeof("save .dat"));
	out[4] = '0'+id;
}

ubool save_t::loadGlobal() {
	// 
}

ubool save_t::load(uptr id) {
	// 
}
