//  Shadow Vertex shader

varying vec3 View;
varying vec3 Light;
varying vec3 Normal;
varying vec4 Ambient;

void main()
{
   //
   //  Lighting values needed by fragment shader
   //
   //  Vertex location in modelview coordinates
   vec3 P = vec3(gl_ModelViewMatrix * gl_Vertex);
   //  Light position
   Light  = vec3(gl_LightSource[0].position) - P;
   //  Normal
   Normal = gl_NormalMatrix * gl_Normal;
   //  Eye position
   View  = -P;
   //  Ambient color
   Ambient = gl_FrontMaterial.emission + gl_FrontLightProduct[0].ambient + gl_LightModel.ambient*gl_FrontMaterial.ambient;

   //  Texture coordinate for fragment shader
   gl_TexCoord[0] = gl_MultiTexCoord0;
   //  Generate eye position coordinates
   vec4 X = gl_ModelViewMatrix*gl_Vertex;
   gl_TexCoord[1].s = dot(gl_EyePlaneS[1],X);
   gl_TexCoord[1].t = dot(gl_EyePlaneT[1],X);
   gl_TexCoord[1].p = dot(gl_EyePlaneR[1],X);
   gl_TexCoord[1].q = dot(gl_EyePlaneQ[1],X);
   gl_TexCoord[1] /= gl_TexCoord[1].q;

   //  Set vertex position
   gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
