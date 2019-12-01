#version 330

//  Transformation matrices
uniform mat4 ModelViewMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat3 NormalMatrix;

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;

out VS_OUT {
    vec3 normal;
} vs_out;

void main()
{
  gl_Position =  ProjectionMatrix * ModelViewMatrix * vec4(vertex_position, 1.0);
  vs_out.normal = normalize(NormalMatrix * vertex_normal);
}