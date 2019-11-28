#version 330 core

//  Transformation matrices
uniform mat4 ModelViewMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat3 NormalMatrix;

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec3 vertex_color;
layout(location = 3) in vec2 vertex_texture;


//  Light properties
uniform vec4 Position;

//  Output to next shader
out vec3 View;
out vec3 Light;
out vec3 Norm;
out vec2 TexCoord;
out vec4 Kd;

void main() 
{
  //  Vertex location in modelview coordinates
  vec4 P = ModelViewMatrix * vec4(vertex_position, 1.0);
  //  Light direction
  Light = vec3(ViewMatrix * Position - P);
  //  Normal vector
  Norm = NormalMatrix * vertex_normal;
  //  Eye position
  View  = -P.xyz;
  //  Set diffuse to Color
  Kd = vec4(vertex_color, 1.0);
  //  Texture
  TexCoord = vertex_texture;
  //  Set transformed vertex location
  gl_Position =  ProjectionMatrix * ModelViewMatrix * vec4(vertex_position, 1.0);
}



