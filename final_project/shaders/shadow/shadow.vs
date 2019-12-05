#version 430 core
layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec3 vertex_normal;
layout (location = 2) in vec3 vertex_color;
layout (location = 3) in vec2 vertex_texture;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
    vec3 Color;
} vs_out;

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ModelMatrix;
uniform mat4 LightSpaceMatrix;
uniform mat3 NormalMatrix;

void main()
{    
    vs_out.FragPos = vec3(ModelMatrix * vec4(vertex_position, 1.0));
    vs_out.Normal = NormalMatrix * vertex_normal;
    vs_out.TexCoords = vertex_texture;
    vs_out.FragPosLightSpace = LightSpaceMatrix * vec4(vs_out.FragPos, 1.0);
    vs_out.Color = vertex_color;
    gl_Position = ProjectionMatrix * ViewMatrix * ModelMatrix * vec4(vertex_position, 1.0);
}