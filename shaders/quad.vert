#version 330

layout (location = 0) in vec2 vertex_pos;
layout (location = 1) in vec4 aabb;

void
main ()
{
  gl_Position = vec4 (aabb.zw * vertex_pos + aabb.xy, 0.0, 1.0);
}
