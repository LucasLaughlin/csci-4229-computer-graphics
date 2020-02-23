#version 330 core
out vec4 FragColor;

//  Material
struct material{
    vec3 Ambient;
    vec3 Diffuse;
    vec3 Specular;
    float Shinyness;
};
uniform material Material;

struct light {
    vec3 Ambient;
    vec3 Diffuse;
    vec3 Specular;
};
uniform light Light; 

uniform vec3 SkyColor;
//  Textures
uniform sampler2D DiffuseTexture;
uniform sampler2D ShadowMap;

uniform sampler2D rTexture;
uniform sampler2D gTexture;
uniform sampler2D bTexture;
uniform sampler2D blendMap;
//  Data passed from vertex shader
in vec3 Normal;
in vec3 Color;
in vec2 TexCoord;
in vec3 LightDir;
in vec3 ViewDir;
in vec3 ReflectDir;
in vec4 LightSpacePosition;
in float Visibility;

float ShadowCalculation(vec4 LightSpacePosition)
{
    float bias = max(0.05 * (1.0 - dot(Normal, LightDir)), 0.05);
    // perform perspective divide
    vec3 projCoords = LightSpacePosition.xyz / LightSpacePosition.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(ShadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
     
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(ShadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(ShadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    if(projCoords.z > 1.0)
        shadow = 0.0;
 
    return shadow;
}

void main()
{
    vec4 blendMapColor = texture(blendMap, vec2(TexCoord.x/40, TexCoord.y/40));

    float backTextureAmount =  1 - (blendMapColor.r + blendMapColor.g + blendMapColor.b);
    vec2 tiledCoords = TexCoord;
    vec4 backgroundTextureColor = texture(DiffuseTexture, tiledCoords) * backTextureAmount;
    vec4 rtextureColor = texture(rTexture, tiledCoords) * blendMapColor.r;
    vec4 gtextureColor = texture(gTexture, tiledCoords) * blendMapColor.g;
    vec4 btextureColor = texture(bTexture, tiledCoords) * blendMapColor.b;

    vec4 totalColor = backgroundTextureColor + rtextureColor + gtextureColor + btextureColor;
    // ambient
    vec3 Ambient = Light.Ambient * Material.Ambient;
  	
    // diffuse 
    float diff = max(dot(LightDir, Normal), 0.0);
    vec3 Diffuse =  Light.Diffuse * (diff * Material.Diffuse);
    
    // specular 
    float spec = (diff>0.0) ? pow(max(dot(ViewDir, ReflectDir), 0.0), Material.Shinyness):0;
    vec3 Specular = Light.Specular * (spec * Material.Specular);  
        
    //shadow
    float Shadow = ShadowCalculation(LightSpacePosition);

    vec4 Texture = texture2D(DiffuseTexture, TexCoord);
    //if texture.a

    vec3 result = (Ambient + (1.0 - Shadow) * (Diffuse + Specular));
    FragColor = mix(vec4(SkyColor, 1.0), vec4(result, 1.0) * Texture, Visibility);
    /* FragColor = vec4(result, 1.0) * texture2D(DiffuseTexture, TexCoord);
    FragColor=vec4(Color* (Ambient +Specular+Diffuse), 1.0); */
} 