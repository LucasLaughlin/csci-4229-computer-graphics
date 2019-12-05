#version 430 core

// Per Pixel Lighting GL4

//  Transformation matrices
uniform mat4 ModelViewMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat3 NormalMatrix;
uniform mat4 ModelMatrix;

//  Vertex attributes (input)
//  Locations are hard wired to match nVidia fixed pipeline
//  This is a HORRIBLE kludge - do not use this in real code
//  Likely does not work on other hardware
//  You should construct your objects using glBuffer and
//  then use glBindAttribLocation to map to these attributes
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
   vec4 P = ModelViewMatrix * vertex_position;
   //  Light direction
   Light = vec3(ViewMatrix * vertex_position - P);
   //  Normal vector
   Norm = NormalMatrix * vertex_normal;
   //  Eye position
   View  = -P.xyz;
   //  Set diffuse to Color
   Kd = vertex_color;
   //  Texture
   TexCoord = vertex_texture;
   //  Set transformed vertex location
   gl_Position =  ProjectionMatrix * ViewMatrix * ModelMatrix vertex_position;
}