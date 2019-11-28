#version 430 core

//  Transformation matrices
uniform mat4 ModelViewMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec3 vertex_color;
layout(location = 3) in vec3 vertex_tex;


out vec3 color;

void main() {
  color = vertex_color;
  gl_Position = ProjectionMatrix * ModelViewMatrix * vec4(vertex_position, 1.0);
}



