#include <cstdio>
#include <cstdlib>
#include <cassert>
#include "sys/stat.h"
#include "Utils.hpp"

void *
malloc_or_exit (size_t size)
{
  void *data = std::malloc (size);

  if (data == NULL)
    {
      std::fprintf (stderr,
                    "ERROR: failed to allocate %zu bytes.\n",
                    size);
      std::exit (EXIT_FAILURE);
    }

  return data;
}

char *
read_whole_file (const char *filepath, size_t *file_size_loc)
{
  FILE *const file = std::fopen (filepath, "r");

  if (file == NULL)
    {
      std::fprintf (stderr,
                    "ERROR: failed to open file \'%s\'.\n",
                    filepath);
      std::exit (EXIT_FAILURE);
    }

  size_t const file_size =
    [file]() -> size_t
    {
      struct stat stats;

      if (fstat (fileno (file), &stats) == -1)
        std::exit (EXIT_FAILURE);

      return stats.st_size;
    } ();

  char *file_data = (char *)malloc_or_exit (file_size + 1);

  if (std::fread (file_data, 1, file_size, file) < file_size)
    {
      std::fprintf (stderr,
                    "ERROR: failed to read the whole file \'%s\'.\n",
                    filepath);
      std::exit (EXIT_FAILURE);
    }

  file_data[file_size] = '\0';

  if (file_size_loc != NULL)
    *file_size_loc = file_size;

  std::fclose (file);

  return file_data;
}

gluint
create_shader (glenum shader_type, const char *filepath)
{
  assert (shader_type == GL_VERTEX_SHADER
          || shader_type == GL_FRAGMENT_SHADER);

  size_t file_size = 0;
  char *file_data = read_whole_file (filepath, &file_size);

  gluint shader = glCreateShader (shader_type);

  {
    glint len = file_size;
    glShaderSource (shader, 1, &file_data, &len);
    std::free (file_data);
  }

  glint is_ok;
  glCompileShader (shader);
  glGetShaderiv (shader, GL_COMPILE_STATUS, &is_ok);

  if (is_ok != GL_TRUE)
    {
      glint log_size = 0;
      glGetShaderiv (shader, GL_INFO_LOG_LENGTH, &log_size);
      char *error_message = (char *)malloc_or_exit (log_size + 1);
      glGetShaderInfoLog (shader, log_size, NULL, error_message);
      error_message[log_size] = '\0';
      std::fprintf (stderr,
                    "ERROR: failed to compile %s shader:\n%s",
                    shader_type == GL_VERTEX_SHADER ?
                      "vertex" : "fragment",
                    error_message);
      std::free (error_message);
      glDeleteShader (shader);
      std::exit (EXIT_FAILURE);
    }

  return shader;
}

gluint
create_program (gluint vertex_shader, gluint fragment_shader)
{
  gluint program = glCreateProgram ();

  glint is_ok;
  glAttachShader (program, vertex_shader);
  glAttachShader (program, fragment_shader);
  glLinkProgram (program);
  glGetProgramiv (program, GL_LINK_STATUS, &is_ok);

  if (is_ok != GL_TRUE)
    {
      glint log_size = 0;
      glGetProgramiv (program, GL_INFO_LOG_LENGTH, &log_size);
      char *error_message = (char *)malloc_or_exit (log_size + 1);
      glGetProgramInfoLog (program, log_size, NULL, error_message);
      error_message[log_size] = '\0';
      std::fprintf (stderr,
                    "ERROR: failed to link program:\n%s",
                    error_message);
      std::free (error_message);
      glDeleteProgram (program);
      std::exit (EXIT_FAILURE);
    }

  glDetachShader (program, vertex_shader);
  glDetachShader (program, fragment_shader);

  return program;
}
