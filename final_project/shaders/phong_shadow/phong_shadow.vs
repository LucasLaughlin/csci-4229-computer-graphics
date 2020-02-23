#version 330 core
//  Vertex Data
layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec3 vertex_normal;
layout (location = 2) in vec3 vertex_color;
layout (location = 3) in vec2 vertex_texture;

//  Transformation Matrix Data
uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ModelMatrix;
uniform mat3 NormalMatrix;
uniform vec3 ViewPosition;

//  Light Matrices
uniform mat4 LightSpaceMatrix;
uniform vec3 LightPosition;

//  Data passed to Fragment Shader
out vec3 Normal;
out vec3 Color;
out vec2 TexCoord;
out vec3 LightDir;
out vec3 ViewDir;
out vec3 ReflectDir;
out vec4 LightSpacePosition;
out float Visibility;

const float density = 0.04;
const float gradient = 1.5;

void main()
{
    // Pass normalize normal matrix and color 
    Normal = normalize(NormalMatrix * vertex_normal);
    Color = vertex_color;
    TexCoord = vertex_texture;

    //Fragment position in model Space
    vec3 FragPosition = vec3(ModelMatrix * vec4(vertex_position, 1.0));
    vec4 PositionRelativeCamera = ViewMatrix * vec4(FragPosition, 1.0);

    //  Light direction to the Fragment Position
    LightDir = normalize(LightPosition - FragPosition);

    //  View direction + reflection direction for fragment position 
    ViewDir = normalize(ViewPosition - FragPosition);
    ReflectDir  = reflect(-LightDir, Normal);

    //  Fragment Position from light POV
    LightSpacePosition = LightSpaceMatrix * vec4(FragPosition, 1.0);

    float distance = length(PositionRelativeCamera.xyz);
    Visibility = exp(-pow((distance * density), gradient));
    Visibility = clamp(Visibility, 0.0, 1.0);
    gl_Position = ProjectionMatrix * PositionRelativeCamera;

}