#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstddef>
#include "gl_types.hpp"

void *
malloc_or_exit (size_t size);

char *
read_whole_file (const char *filepath, size_t *file_size_loc);

gluint
create_shader (glenum shader_type, const char *filepath);

gluint
create_program (gluint vertex_shader, gluint fragment_shader);

#endif // UTILS_HPP
