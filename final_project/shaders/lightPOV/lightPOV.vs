#version 330 core
layout (location = 0) in vec3 vertex_position;

uniform mat4 LightSpaceMatrix;
uniform mat4 ModelMatrix;
uniform mat4 test;

void main()
{
    gl_Position = LightSpaceMatrix * test * vec4(vertex_position, 1.0);
}  