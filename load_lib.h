/* Jordan Justen : gears3d is public domain */

#ifndef __load_lib_h_included__
#define __load_lib_h_included__

void * open_library(const char *lib);
void * get_library_symbol(void *lib, const char *fn);

#endif
