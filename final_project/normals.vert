
#version 330 core

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 modelMatrix;

void main()
{	
  vec4 world_pos = modelMatrix * gl_Vertex;
  vec4 view_pos = viewMatrix * world_pos;
  gl_Position = projectionMatrix * view_pos;
}