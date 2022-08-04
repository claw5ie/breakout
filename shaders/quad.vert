#version 330

layout (location = 0) in vec2 vertex_pos;
layout (location = 1) in vec2 offset;
layout (location = 2) in vec2 scale;

void
main ()
{
  gl_Position = vec4 (scale * vertex_pos + offset, 0.0, 1.0);
}
