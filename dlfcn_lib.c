/* Jordan Justen : gears3d is public domain */

#include "load_lib.h"
#include <dlfcn.h>

void *
open_library(const char *lib)
{
    return dlopen(lib, RTLD_NOW);
}

void *
get_library_symbol(void *lib, const char *fn)
{
    return dlsym(lib, fn);
}
