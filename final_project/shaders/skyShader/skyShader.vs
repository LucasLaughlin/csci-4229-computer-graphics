#version 330 core
layout (location = 0) in vec3 vertex_position;

out vec3 TexCoords;

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ModelMatrix;

void main()
{
    TexCoords = vertex_position;
    vec4 pos = ProjectionMatrix * ViewMatrix * ModelMatrix* vec4(vertex_position, 1.0);
    gl_Position = pos.xyzw;
}  