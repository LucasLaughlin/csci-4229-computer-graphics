#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube dayBox;
uniform samplerCube nightBox;
uniform float BlendFactor;
uniform vec3 SkyColor;

const float lowerLimit = 0.0;
const float upperLimit =0.4;
void main()
{   float blender = clamp(BlendFactor, 0, 1.0);
    vec4 dayTex = texture(dayBox, TexCoords);
    vec4 nightTex = texture(nightBox, TexCoords);
    vec4 textColor = mix(dayTex, nightTex, blender);

    float factor = (TexCoords.y - lowerLimit)/ (upperLimit - lowerLimit);
    factor = clamp(factor, 0.0, 1.0);
    FragColor = mix(vec4(SkyColor, 1.0), textColor, factor);
}