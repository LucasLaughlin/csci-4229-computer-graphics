/*
 *  final-project
 *
 *  Lucas Laughlin
 * 
 * Portions of code were taken from proffesor Willem Schreuder's examples for CSCI 4229
 * 
 */

/* TODO: 
 *    make a Normal function: draw normals if normal enabled. 
 *    figure out textFix
 *    figure out specular, emission light etc
 *    refactor Hat function
 */
#include "CSCI_helpers/CSCIx229.h"
#include "cglm/cglm.h"
int      normals = 0;   // Display normal vectors
int      axes = 1;      // Display axes
double   t = 0;         // Timer
int      wall = 1;      // Floor/wall boolean

/*    Camera/Eye position    */
int      th = 30;       //  Azimuth of view angle
int      ph = 30;       //  Elevation of view angle
float    Ex, Ey, Ez;    //  Eye/Camera position

/*    Projection model values    */
double   asp = 1;       //  Aspect ratio
double   dim = 4.0;     //  Size of world
int      fov = 55;      //  Field of view

/*    Light model values     */
int      emission = 0;  // Emission intensity (%)
int      ambient = 30;  // Ambient intensity (%)
int      diffuse = 100; // Diffuse intensity (%)
int      specular = 0;  // Specular intensity (%)
float    shiny = 1;     // Shininess (value)
float Diffuse[4], Ambient[4], Specular[4], Emission[4];

/*    Floating light positioning     */
int      zh = 90;       // Light azimuth
float    ylight = 3;    // Elevation of light
int      distance = 5;  // Light distance
int      move = 0;      // Light ball movement type
float    Position[4];   // Light Position

/*    Texture variables for hat    */
double rep = 1;
int texMode = 0;
int ntex = 0;
unsigned int texture[3];
int tex = 0;

/*    Shader variables     */
int shader_mode = 3;                               //  Current active shader
int shader[] = {0, 0, 0, 0};                 //  Array to hold shader programs
int num_shaders = sizeof(shader) / sizeof(int);    //  number of shaders
int drawArrayShaders = 0;

/*    Shaddow Map Variables      */
int shadowdim;          // Size of shadow map textures
unsigned int framebuf=0;// Frame buffer id
GLuint       depthMapFBO;
GLuint       depthMap;
int shadowMapShader, normalShader; 
const GLuint SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
int SCR_WIDTH = 600, SCR_HEIGHT = 600;


/*    VAOs     */
GLuint      hatVao;
GLuint      wallVao;
GLuint      shadowVao;

/*    Test variables     */
int init = 1;
mat4 gModelMatrix, gViewMatrix;

static void FillBuffer(int index, float bufName[], float x, float y, float z)
{
   bufName[index] = x;
   bufName[index + 1] = y;
   if (z != NAN)
      bufName[index + 2] = z;
   return;
}

/* Draw a wall
 */
static void Wall( float St)
{
   int n=8;
   int   i,j;
   float s=1.0/n;
   float t=0.5*St/n;
   
   int bufferLen = (3*n)*(3*n);
   float position[bufferLen * 3];
   float normals[bufferLen * 3];
   float textCoords[bufferLen * 2];
   int bufIndexCount=0;
   //  Draw walls
   for (j=-n;j<n;j++)
   {
      for (i=-n;i<=n;i++)
      {
         FillBuffer(bufIndexCount, position, i*s,    j*s,-1);
         FillBuffer(bufIndexCount+3, position, i*s,(j+1)*s,-1);
         FillBuffer(bufIndexCount, normals, 0, 0, 1);
         FillBuffer(bufIndexCount+3, normals, 0, 0, 1);
         FillBuffer(bufIndexCount / 3 * 2, textCoords, (i+n)*t,(j  +n)*t, NAN);
         FillBuffer((bufIndexCount+3) / 3 * 2, textCoords,(i+n)*t,(j+1+n)*t, NAN);

         bufIndexCount+=6;
      }
   }
   
   glBindVertexArray(wallVao);

   GLuint position_vbo;
   glGenBuffers(1, &position_vbo);
   glBindBuffer(GL_ARRAY_BUFFER, position_vbo);
   glBufferData(GL_ARRAY_BUFFER, sizeof(position), position, GL_STATIC_DRAW);
   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

   GLuint normals_vbo;
   glGenBuffers(1, &normals_vbo);
   glBindBuffer(GL_ARRAY_BUFFER, normals_vbo);
   glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);
   glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

   GLuint tex_vbo;
   glGenBuffers(1, &tex_vbo);
   glBindBuffer(GL_ARRAY_BUFFER, tex_vbo);
   glBufferData(GL_ARRAY_BUFFER, sizeof(textCoords), textCoords, GL_STATIC_DRAW);
   glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, NULL);

   glEnableVertexAttribArray(0);
   glEnableVertexAttribArray(1);
   glEnableVertexAttribArray(3);
   
   glDrawArrays(GL_TRIANGLE_STRIP, 0, bufIndexCount / 3);
   
   glDeleteBuffers(1, &position_vbo);
   glDeleteBuffers(1, &normals_vbo);
   glDeleteBuffers(1, &tex_vbo);

   glDisable(GL_TEXTURE_2D);
}

/* Draw cowboy hat 
 *    at (x,y,z)
 *    radius (r)

*/
static void Hat()
{
   glEnable(GL_TEXTURE_2D);
   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

   /*    Constants for hat curvature    */
   float a = -0.5; // x^4
   //float b = 0.0;  // x^3
   float c = 0.9; // x^2
   //float d = 0.0;  // x
   float e = 1.5; // constant

   /*    Hat fidelity step    */
   float r_step = 0.1;  // Hat radius fidelity
   int angle_step = 15; // Hat angle fidelity

   /*    Hat max dimensions    */
   int r_max = 3;
   int angle_max = 360;

   /*    Set up Buffer arays for Shaders     */
   int bufferLen = ((r_max / r_step) + 1) * ((angle_max / angle_step) + 1) * 4 * 2; //(number of r steps + 1 for 0 index) * (number of angle steps + 1 for 0 index) * 4 points drawn per point on hat * 2 hats
   float position[bufferLen * 3];
   float normals[bufferLen * 3];
   float colors[bufferLen * 3];
   float textCoords[bufferLen * 2];
   int bufIndexCount = 0;

   for (float r = 0; r <= r_max; r += r_step)
   {

      glBegin(GL_TRIANGLE_STRIP);

      int textFix = 0;
      for (int angle = 0; angle <= angle_max; angle += angle_step)
      {
         float xv = ((sqrt(2) * sqrt(0.5)) + r) * Cos(angle);
         float zv = r * Sin(angle);
         float yv = a * pow(sqrt(pow(xv, 2) / 2 + pow(zv, 2)), 4) + c * pow(sqrt(pow(xv, 2) / 2 + pow(zv, 2)), 2) + e;
         float yv_brim = 0.1 * pow(zv, 2) - 0.1;
         float iv, jv, kv;
         if (yv >= yv_brim - 1.5)
         {
            iv = -1*(-0.5 * pow(xv, 3) - xv * pow(zv, 2) + 0.9 * xv);
            jv = 1;
            kv = -1*(-2 * pow(zv, 3) - zv * pow(xv, 2) + 1.8 * zv);
         }
         else
         {
            iv = 0;
            jv = 1;
            kv = -0.2 * zv;
         }

         if (shader_mode <= 1)
         {
            glNormal3f(iv, jv, kv);
            glColor3f(0.54, 0.27, 0.075 + yv / 12);
            glTexCoord2f(0.0, textFix ? 0.0 : rep);
            glVertex3f(xv, fmax(yv, yv_brim), zv);
         }
         else
         {
            FillBuffer(bufIndexCount, position, xv, fmax(yv, yv_brim), zv);
            FillBuffer(bufIndexCount, normals, iv, jv, kv);
            FillBuffer(bufIndexCount, colors, 0.54, 0.27, 0.075 + yv / 12);
            FillBuffer(bufIndexCount / 3 * 2, textCoords, 0.0, textFix ? 0.0 : rep, NAN);
         }
         bufIndexCount += 3;

         float xv_step = ((sqrt(2) * sqrt(0.5)) + (r + r_step)) * Cos(angle);
         float zv_step = (r + r_step) * Sin(angle);
         float yv_step = a * pow(sqrt(pow(xv_step, 2) / 2 + pow(zv_step, 2)), 4) + c * pow(sqrt(pow(xv_step, 2) / 2 + pow(zv_step, 2)), 2) + e;
         float yv_step_brim = 0.1 * pow(zv_step, 2) - 0.1;

         if (yv_step >= yv_step_brim - 1.5)
         {
            iv = -1*(-0.5 * pow(xv_step, 3) - xv_step * pow(zv_step, 2) + 0.9 * xv_step);
            jv = 1;
            kv = -1*(-2 * pow(zv_step, 3) - zv_step * pow(xv_step, 2) + 1.8 * zv_step);
         }
         else
         {
            iv = 0;
            jv = 1;
            kv = -0.2 * zv_step;
         }
         if (shader_mode <= 1)
         {
            glNormal3f(iv, jv, kv);
            glColor3f(0.54, 0.27, 0.075 + yv_step / 12);
            glTexCoord2f(rep, textFix ? 0.0 : rep);
            glVertex3f(xv_step, fmax(yv_step, yv_step_brim), zv_step);
         }
         else
         {
            FillBuffer(bufIndexCount, position, xv_step, fmax(yv_step, yv_step_brim), zv_step);
            FillBuffer(bufIndexCount, normals, iv, jv, kv);
            FillBuffer(bufIndexCount, colors, 0.54, 0.27, 0.075 + yv_step / 12);
            FillBuffer(bufIndexCount / 3 * 2, textCoords, rep, textFix ? 0.0 : rep, NAN);
         }
         bufIndexCount += 3;
         textFix = !textFix;
      }
      glEnd();
   }
   float closure = -0.1;
   for (float r = 0; r <= r_max; r += r_step)
   {

      glBegin(GL_TRIANGLE_STRIP);

      int textFix = 0;
      for (int angle = 0; angle <= angle_max; angle += angle_step)
      {
         closure = -0.1;
         float xv = ((sqrt(2) * sqrt(0.5)) + r) * Cos(angle);
         float zv = r * Sin(angle);
         float yv = a * pow(sqrt(pow(xv, 2) / 2 + pow(zv, 2)), 4) + c * pow(sqrt(pow(xv, 2) / 2 + pow(zv, 2)), 2) + e + closure;
         float yv_brim = 0.1 * pow(zv, 2) - 0.1 + closure;
         float iv, jv, kv;
         if (yv >= yv_brim - 1.5)
         {
            iv = -0.5 * pow(xv, 3) - xv * pow(zv, 2) + 0.9 * xv;
            jv = -1;
            kv = -2 * pow(zv, 3) - zv * pow(xv, 2) + 1.8 * zv;
         }
         else
         {
            iv = 0;
            jv = -1;
            kv = 0.2 * zv;
         }

         if (shader_mode <= 1)
         {
            glNormal3f(iv, jv, kv);
            glColor3f(0.54, 0.27, 0.075 + yv / 12);
            glTexCoord2f(0.0, textFix ? 0.0 : rep);
            glVertex3f(xv, fmax(yv, yv_brim), zv);
         }
         else
         {
            FillBuffer(bufIndexCount, position, xv, fmax(yv, yv_brim), zv);
            FillBuffer(bufIndexCount, normals, iv, jv, kv);
            FillBuffer(bufIndexCount, colors, 0.54, 0.27, 0.075 + yv / 12);
            FillBuffer(bufIndexCount / 3 * 2, textCoords, 0.0, textFix ? 0.0 : rep, NAN);
         }
         bufIndexCount += 3;
         if (r > 2.9)
         {
            closure = 0.0;
         }
         float xv_step = ((sqrt(2) * sqrt(0.5)) + (r + r_step)) * Cos(angle);
         float zv_step = (r + r_step) * Sin(angle);
         float yv_step = a * pow(sqrt(pow(xv_step, 2) / 2 + pow(zv_step, 2)), 4) + c * pow(sqrt(pow(xv_step, 2) / 2 + pow(zv_step, 2)), 2) + e + closure;
         float yv_step_brim = 0.1 * pow(zv_step, 2) - 0.1 + closure;

         if (yv_step >= yv_step_brim - 1.5)
         {
            iv = -0.5 * pow(xv_step, 3) - xv_step * pow(zv_step, 2) + 0.9 * xv_step;
            jv = -1;
            kv = -2 * pow(zv_step, 3) - zv_step * pow(xv_step, 2) + 1.8 * zv_step;
         }
         else
         {
            iv = 0;
            jv = -1;
            kv = 0.2 * zv_step;
         }
         if (shader_mode <= 1)
         {
            glNormal3f(iv, jv, kv);
            glColor3f(0.54, 0.27, 0.075 + yv_step / 12);
            glTexCoord2f(rep, textFix ? 0.0 : rep);
            glVertex3f(xv_step, fmax(yv_step, yv_step_brim), zv_step);
         }
         else
         {
            FillBuffer(bufIndexCount, position, xv_step, fmax(yv_step, yv_step_brim), zv_step);
            FillBuffer(bufIndexCount, normals, iv, jv, kv);
            FillBuffer(bufIndexCount, colors, 0.54, 0.27, 0.075 + yv_step / 12);
            FillBuffer(bufIndexCount / 3 * 2, textCoords, rep, textFix ? 0.0 : rep, NAN);
         }
         bufIndexCount += 3;
         textFix = !textFix;
      }
      glEnd();
   }
   glBindVertexArray(hatVao);
   GLuint position_vbo;
   glGenBuffers(1, &position_vbo);
   glBindBuffer(GL_ARRAY_BUFFER, position_vbo);
   glBufferData(GL_ARRAY_BUFFER, sizeof(position), position, GL_STATIC_DRAW);
   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

   GLuint normals_vbo;
   glGenBuffers(1, &normals_vbo);
   glBindBuffer(GL_ARRAY_BUFFER, normals_vbo);
   glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);
   glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

   GLuint colors_vbo;
   glGenBuffers(1, &colors_vbo);
   glBindBuffer(GL_ARRAY_BUFFER, colors_vbo);
   glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
   glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

   GLuint tex_vbo;
   glGenBuffers(1, &tex_vbo);
   glBindBuffer(GL_ARRAY_BUFFER, tex_vbo);
   glBufferData(GL_ARRAY_BUFFER, sizeof(textCoords), textCoords, GL_STATIC_DRAW);
   glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, NULL);

   glEnableVertexAttribArray(0);
   glEnableVertexAttribArray(1);
   glEnableVertexAttribArray(2);
   glEnableVertexAttribArray(3);

   
   //  Switch off textures so it doesn't apply to the rest
   glDisable(GL_TEXTURE_2D);
   //Draw Top Hat
   glDrawArrays(GL_TRIANGLE_STRIP, 0, bufIndexCount / 6);
   //Draw Bottom Hat
   glDrawArrays(GL_TRIANGLE_STRIP, bufIndexCount / 6, bufIndexCount / 6);
   glDeleteBuffers(1, &position_vbo);
   glDeleteBuffers(1, &normals_vbo);
   glDeleteBuffers(1, &tex_vbo);
   glDeleteBuffers(1, &colors_vbo);
}

/* Draw axes */
static void Axes()
{
   const double len = 1.5; //  Length of axes
   glDisable(GL_LIGHTING);
   //  White
   glColor3f(1, 1, 1);
   //  Draw axes
   if (axes)
   {
      glBegin(GL_LINES);
      glVertex3d(0.0, 0.0, 0.0);
      glVertex3d(len, 0.0, 0.0);
      glVertex3d(0.0, 0.0, 0.0);
      glVertex3d(0.0, len, 0.0);
      glVertex3d(0.0, 0.0, 0.0);
      glVertex3d(0.0, 0.0, len);
      glEnd();
      //  Label axes
      glRasterPos3d(len, 0.0, 0.0);
      Print("X");
      glRasterPos3d(0.0, len, 0.0);
      Print("Y");
      glRasterPos3d(0.0, 0.0, len);
      Print("Z");
   }
}
/* Draw Info */
static void Info()
{
   glWindowPos2i(5, 75);
   Print("Ambient=%d  Diffuse=%d Specular=%d Emission=%d", ambient, diffuse, specular, emission, shiny);
   glWindowPos2i(5, 50);
   Print("Angle=%d,%d | Dim=%.1f | FOV=%d | shader:%d | normals:%d", th, ph, dim, fov, shader_mode, normals);
   glWindowPos2i(5, 25);
   Print("Camera (%f, %f, %f)", Ex, Ey, Ez);
}

/*
 *  Set light
 *    light>0 bright
 *    light<0 dim
 *    light=0 off
 */
static void Light(int light)
{
   //  Set light position
   Position[0] = distance*Cos(zh);
   Position[1] = ylight;
   Position[2] = distance*Sin(zh);
   Position[3] = 1;

   //  Enable lighting
   if (light)
   {
      //  Enable lighting with normalization
      glEnable(GL_LIGHTING);
      glEnable(GL_NORMALIZE);
      //  glColor sets ambient and diffuse color materials
      glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
      glEnable(GL_COLOR_MATERIAL);
      //  Enable light 0
      glEnable(GL_LIGHT0);
   }
   else
   {
      glDisable(GL_LIGHTING);
      glDisable(GL_COLOR_MATERIAL);
      glDisable(GL_NORMALIZE);
   }
}

void Camera(){
   Ex = -2 * dim * Sin(th) * Cos(ph);
   Ey = +2 * dim * Sin(ph);
   Ez = +2 * dim * Cos(th) * Cos(ph);
}

void SetViewProjectionMatrices(int shader)
{
   float ProjectionMatrix[16];
   glGetFloatv(GL_PROJECTION_MATRIX, ProjectionMatrix);

   mat4 ViewMatrix;
   Camera();
   vec3 ViewPos={Ex, Ey, Ez};
   glm_lookat(ViewPos, 
              (vec3){ 0.0f, 0.0f,  0.0f}, 
              (vec3){ 0.0f, Cos(ph),  0.0f}, 
              ViewMatrix);
   glm_mat4_copy(ViewMatrix, gViewMatrix);

   glUniformMatrix4fv(glGetUniformLocation(shader, "ViewMatrix"), 1, 0, (float *)ViewMatrix);
   glUniformMatrix4fv(glGetUniformLocation(shader, "ProjectionMatrix"), 1, 0, ProjectionMatrix);
}

void setModelViewNormalMatrices(int shader, float x_trans, float  y_trans, float  z_trans, float x_rot, float y_rot, float z_rot, float x_scale, float y_scale, float z_scale) 
{
   if (!drawArrayShaders){
      glMatrixMode(GL_MODELVIEW);
      glRotatef(x_rot,1,0,0);
      glRotatef(y_rot,0,1,0);
      glRotatef(z_rot,0,0,1);
      glTranslatef(x_trans, y_trans, z_trans);
      glScalef(x_scale, y_scale, z_scale);
   }

   mat4 ModelMatrix;
   mat4 ModelViewMatrix;
   mat3 NormalMatrix;
   glm_mat4_identity(ModelMatrix);
   glm_scale(ModelMatrix, (vec3){x_scale, y_scale, z_scale});
   glm_translate(ModelMatrix, (vec3){x_trans, y_trans, z_trans});
   glm_rotate_y(ModelMatrix, y_rot*3.1415926/180, ModelMatrix);
   glm_rotate_x(ModelMatrix, x_rot*3.1415926/180, ModelMatrix);
   glm_rotate_z(ModelMatrix, z_rot*3.1415926/180, ModelMatrix);

   glm_mat4_mul(gViewMatrix, ModelMatrix, ModelViewMatrix);
   glm_mat4_pick3(ModelViewMatrix, NormalMatrix);
   glm_mat3_inv(NormalMatrix, NormalMatrix);
   glm_mat3_transpose(NormalMatrix);

   glUniformMatrix4fv(glGetUniformLocation(shader, "ModelMatrix"), 1, 0, (float *)ModelMatrix);
   glUniformMatrix3fv(glGetUniformLocation(shader, "NormalMatrix"), 1, 0, (float *)NormalMatrix);
   glUniformMatrix3fv(glGetUniformLocation(shader, "ModelViewMatrix"), 1, 0, (float *)ModelViewMatrix);
}

void Scene(int shader){
   glPushMatrix();
   setModelViewNormalMatrices(shader, 0, 0, 0, 0, 0, 0, 1, 1, 1);
   Hat();
   glPopMatrix();

   if (wall){
      glPushMatrix();
      setModelViewNormalMatrices(shader, 0, 0.8, 0, -90, 0, 0, 8, 8, 8);
      Wall(4);
      glPopMatrix();
   }
}

void ConfigureLightPovMatrices(int shader){
   float near_plane = 1.0f, far_plane = 7.5f;
   mat4 LightProjection, LightView, LightSpaceMatrix;
   glm_ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane, LightProjection);
   //vec3 LightPos={0.0f, 4.0f, 0.0f};
   vec3 LightPos={Position[0], Position[1], Position[2]};
   glm_lookat(LightPos, 
              (vec3){ 0.0f, 0.0f,  0.0f}, 
              (vec3){ 0.0f, 1.0f,  0.0f}, 
              LightView);

   glm_mat4_mul(LightProjection, LightView, LightSpaceMatrix);

   mat4 test; 
   glm_mat4_identity(test);
   glUniformMatrix4fv(glGetUniformLocation(shader, "test"), 1, 0, (float *)test);
   
   glUniformMatrix4fv(glGetUniformLocation(shader, "LightSpaceMatrix"), 1, 0, (float *)LightSpaceMatrix);
   glUniform4fv(glGetUniformLocation(shader, "lightPos"), 1, LightPos);

}

void ConfigureShadowShader(){

   vec3 ViewPos={Ex, Ey, Ez};
   glUniform1i(glGetUniformLocation(shader[3], "shadowMap"), depthMapFBO);
   glUniform1i(glGetUniformLocation(shader[3], "diffuseTexture"), tex);
   glUniform4fv(glGetUniformLocation(shader[3], "ViewPos"), 1, ViewPos);
         
}

void ShadowMap(){

   glBindVertexArray(shadowVao);
   glEnable(GL_NORMALIZE);
   // 1. first render to depth map
   glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
   glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
   glClear(GL_DEPTH_BUFFER_BIT);
   glUseProgram(shadowMapShader);
   ConfigureLightPovMatrices(shadowMapShader);
   glCullFace(GL_FRONT);
   Scene(shadowMapShader);
   glCullFace(GL_BACK);
   glBindFramebuffer(GL_FRAMEBUFFER, 0);


   // 2. then render scene as normal with shadow mapping (using depth map)
   glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glUseProgram(shader[3]);
   ConfigureLightPovMatrices(shader[3]);
   ConfigureShadowShader();
   
   glBindTexture(GL_TEXTURE_2D, depthMap);
}

void InitMap(){
   glGenFramebuffers(1, &depthMapFBO);

   //SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

   glGenTextures(1, &depthMap);
   glBindTexture(GL_TEXTURE_2D, depthMap);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); 
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
   float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
   glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);  

   glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
   glDrawBuffer(GL_NONE);
   glReadBuffer(GL_NONE);
   if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) Fatal("Error setting up frame buffer\n");
   glBindFramebuffer(GL_FRAMEBUFFER, 0); 
}


/*    OpenGL (GLUT) calls this routine to display the scene    */
void display()
{
   //  Erase the window and the depth buffer
   glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   //  Enable Z-buffering in OpenGL
   glEnable(GL_DEPTH_TEST);
   //  OpenGL should normalize normal vectors
   glEnable(GL_NORMALIZE);
   //  Undo previous transformations
   glLoadIdentity();

   //  Perspective - set eye position
   Camera();
   gluLookAt(Ex, Ey, Ez, 0, 0, 0, 0, Cos(ph), 0);

   //  Translate intensity to color vectors
   float Emission[]  = {0.1  * emission, 0.1  * emission, 0.1  * emission, 1.0};
   float Ambient[]   = {0.01 * ambient,  0.01 * ambient,  0.01 * ambient,  1.0};
   float Diffuse[]   = {0.01 * diffuse,  0.01 * diffuse,  0.01 * diffuse,  1.0};
   float Specular[]  = {0.01 * specular, 0.01 * specular, 0.01 * specular, 1.0};

   Light(1);
   //  Set ambient, diffuse, specular components and position of light 0
   glLightfv(GL_LIGHT0, GL_AMBIENT, Ambient);
   glLightfv(GL_LIGHT0, GL_DIFFUSE, Diffuse);
   glLightfv(GL_LIGHT0, GL_SPECULAR, Specular);
   glLightfv(GL_LIGHT0, GL_POSITION, Position);
   
   if(shader_mode==3){
      ShadowMap();
      SetViewProjectionMatrices(shader[shader_mode]);
      glBindTexture(GL_TEXTURE_2D, tex);
      Scene(shader[shader_mode]);
   }
   else{
      glUseProgram(shader[shader_mode]);
      SetViewProjectionMatrices(shader[shader_mode]);
      int id;
      //  Set lighting parameters
      id = glGetUniformLocation(shader[shader_mode], "Position");
      if (id >= 0)
         glUniform4fv(id, 1, Position);
      id = glGetUniformLocation(shader[shader_mode], "Ambient");
      if (id >= 0)
         glUniform4fv(id, 1, Ambient);
      id = glGetUniformLocation(shader[shader_mode], "Diffuse");
      if (id >= 0)
         glUniform4fv(id, 1, Diffuse);
      id = glGetUniformLocation(shader[shader_mode], "Specular");
      if (id >= 0)
         glUniform4fv(id, 1, Specular);
      //  Set material properties
      id = glGetUniformLocation(shader[shader_mode], "Ks");
      if (id >= 0)
         glUniform4fv(id, 1, Specular);
      id = glGetUniformLocation(shader[shader_mode], "Ke");
      if (id >= 0)
         glUniform4fv(id, 1, Emission);
      id = glGetUniformLocation(shader[shader_mode], "Shinyness");
      if (id >= 0)
         glUniform1f(id, shiny);

      glBindTexture(GL_TEXTURE_2D, tex);
      // Draw Hat
      Scene(shader[shader_mode]);
      
   }
   if(normals){
      glUseProgram(normalShader);
      SetViewProjectionMatrices(normalShader);
      Scene(normalShader);
   }
   glUseProgram(shader[0]);
   //  Draw light position as sphere 
   glPushMatrix();
   glColor3f(1, 1, 1);
   glTranslated(Position[0], Position[1], Position[2]);
   glutSolidSphere(0.3, 10, 10);
   glPopMatrix();
   // Draw axes
   Axes();
   // Write info
   Info();
   //  Render the scene
   glFlush();
   //  Make the rendered scene visible
   glutSwapBuffers();
}

/* Read text file
 */
char *ReadText(char *file)
{
   int n;
   char *buffer;
   //  Open file
   FILE *f = fopen(file, "rt");
   if (!f)
      Fatal("Cannot open text file %s\n", file);
   //  Seek to end to determine size, then rewind
   fseek(f, 0, SEEK_END);
   n = ftell(f);
   rewind(f);
   //  Allocate memory for the whole file
   buffer = (char *)malloc(n + 1);
   if (!buffer)
      Fatal("Cannot allocate %d bytes for text file %s\n", n + 1, file);
   //  Snarf the file
   if (fread(buffer, n, 1, f) != 1)
      Fatal("Cannot read %d bytes for text file %s\n", n, file);
   buffer[n] = 0;
   //  Close and return
   fclose(f);
   return buffer;
}

/*    Print Shader Log     */
void PrintShaderLog(int obj, char *file)
{
   int len = 0;
   glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &len);
   if (len > 1)
   {
      int n = 0;
      char *buffer = (char *)malloc(len);
      if (!buffer)
         Fatal("Cannot allocate %d bytes of text for shader log\n", len);
      glGetShaderInfoLog(obj, len, &n, buffer);
      fprintf(stderr, "%s:\n%s\n", file, buffer);
      free(buffer);
   }
   glGetShaderiv(obj, GL_COMPILE_STATUS, &len);
   if (!len)
      Fatal("Error compiling %s\n", file);
}

/*    Print Program Log    */
void PrintProgramLog(int obj)
{
   int len = 0;
   glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &len);
   if (len > 1)
   {
      int n = 0;
      char *buffer = (char *)malloc(len);
      if (!buffer)
         Fatal("Cannot allocate %d bytes of text for program log\n", len);
      glGetProgramInfoLog(obj, len, &n, buffer);
      fprintf(stderr, "%s\n", buffer);
   }
   glGetProgramiv(obj, GL_LINK_STATUS, &len);
   if (!len)
      Fatal("Error linking program\n");
}

/*    Create Shader     */
int CreateShader(GLenum type, char *file)
{
   //  Create the shader
   int shader = glCreateShader(type);
   //  Load source code from file
   char *source = ReadText(file);
   glShaderSource(shader, 1, (const char **)&source, NULL);
   free(source);
   //  Compile the shader
   fprintf(stderr, "Compile %s\n", file);
   glCompileShader(shader);
   //  Check for errors
   PrintShaderLog(shader, file);
   //  Return name
   return shader;
}

/*    Create Shader Program      */
int CreateShaderProg(char *VertFile, char *FragFile, char *GeoFile)
{
   //  Create program
   int prog = glCreateProgram();
   //  Create and compile vertex shader
   int vert = CreateShader(GL_VERTEX_SHADER, VertFile);
   //  Attach vertex shader
   glAttachShader(prog, vert);
   if (GeoFile != NULL)
   {
      int geo = CreateShader(GL_GEOMETRY_SHADER, GeoFile);
      glAttachShader(prog, geo);
      printf("success");
   }
   //  Create and compile fragment shader
   int frag = CreateShader(GL_FRAGMENT_SHADER, FragFile);
   //  Attach fragment shader
   glAttachShader(prog, frag);
   //  Link program
   glLinkProgram(prog);
   //  Check for errors
   PrintProgramLog(prog);
   //  Return name
   return prog;
}

/*    GLUT calls this routine when an arrow key is pressed     */
void special(int key, int x, int y)
{

   //  Right arrow key - increase angle by 5 degrees
   if (key == GLUT_KEY_RIGHT)
      th += 5;
   //  Left arrow key - decrease angle by 5 degrees
   else if (key == GLUT_KEY_LEFT)
      th -= 5;
   //  Up arrow key - increase elevation by 5 degrees
   else if (key == GLUT_KEY_UP)
      ph += 5;
   //  Down arrow key - decrease elevation by 5 degrees
   else if (key == GLUT_KEY_DOWN)
      ph -= 5;
   //  PageUp key - increase dim
   else if (key == GLUT_KEY_PAGE_UP)
      dim += 0.1;
   //  PageDown key - decrease dim
   else if (key == GLUT_KEY_PAGE_DOWN && dim > 1)
      dim -= 0.1;

   //  Keep angles to +/-360 degrees
   th %= 360;
   ph %= 360;
}

/*    GLUT calls this routine when a key is pressed      */
void key(unsigned char ch, int x, int y)
{

   //  Exit on ESC
   if (ch == 27)
      exit(0);
   //  Reset view angle
   else if (ch == '0')
      th = ph = 0;
   //  Toggle Normals
   else if (ch == 'n' || ch == 'N')
      normals = !normals;
   //  Toggle axes
   else if (ch == 'x' || ch == 'X')
      axes = !axes;
   //  Light movement mode
   else if (ch == 'm' || ch == 'M')
      move = (move + 1) % 3;
   //  Ambient level
   else if (ch == 'a' && ambient > 0)
      ambient -= 5;
   else if (ch == 'A' && ambient < 100)
      ambient += 5;
   //  Diffuse level
   else if (ch == 'd' && diffuse > 0)
      diffuse -= 5;
   else if (ch == 'D' && diffuse < 100)
      diffuse += 5;
   //  Specular level
   else if (ch == 's' && specular > 0)
      specular -= 5;
   else if (ch == 'S' && specular < 100)
      specular += 5;
   //  Emission level
   else if (ch == 'e' && emission > 0)
      emission -= 5;
   else if (ch == 'E' && emission < 100)
      emission += 5;
   //  FOV
   else if (ch == 'f')
      fov--;
   else if (ch == 'F')
      fov++;
   //  Shader selector
   else if (ch == '<' || ch == ',')
      shader_mode = (shader_mode + num_shaders - 1) % num_shaders;
   else if (ch == '>' || ch == '.')
      shader_mode = (shader_mode + 1) % num_shaders;
}

/*    GLUT calls this routine when the window is resized    */
void reshape(int width, int height)
{
   //  Ratio of the width to the height of the window
   asp = (height > 0) ? (double)width / height : 1;

   SCR_WIDTH=width;
   SCR_HEIGHT=height;
   //  Set the viewport to the entire window
   glViewport(0, 0, width, height);
   //  Set projection
   Project(fov, asp, dim);
   glutPostRedisplay();
}

/*    GLUT calls this routine when there is nothing else to do    */
void idle()
{
   t = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
   zh = fmod(45 * t, 360.0);
   Project(fov, asp, dim);
   glutPostRedisplay();
}


/*    Start up GLUT and tell it what to do     */
int main(int argc, char *argv[])
{
   //  Initialize GLUT and process user parameters
   glutInit(&argc, argv);
   //  Request double buffered, true color window with Z buffering at 600x600
   glutInitWindowSize(600, 600);
   glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
   //  Create the window
   glutCreateWindow("Model viewer");

   // Create vertex array object once window has been generated, before any other openGl call - http://www.opengl-tutorial.org/beginners-tutorials/tutorial-2-the-first-triangle/
   glGenVertexArrays(1, &hatVao);
   glGenVertexArrays(1, &wallVao);
   glGenVertexArrays(1, &shadowVao);

   //  Tell GLUT to call "idle" when there is nothing else to do
   glutIdleFunc(idle);
   //  Tell GLUT to call "display" when the scene should be drawn
   glutDisplayFunc(display);
   //  Tell GLUT to call "reshape" when the window is resized
   glutReshapeFunc(reshape);
   //  Tell GLUT to call "special" when an arrow key is pressed
   glutSpecialFunc(special);
   //  Tell GLUT to call "key" when a key is pressed
   glutKeyboardFunc(key);
   LoadTexBMP("./textures/490.bmp");
   tex=LoadTexBMP("./textures/490.bmp");
   //  Shaders
   shader[1] = CreateShaderProg("./shaders/pixtex/pixtex.vs", "./shaders/pixtex/pixtex.fs", NULL);
   shader[2] = CreateShaderProg("./shaders/simple/simple.vs", "./shaders/simple/simple.fs", NULL);
   shader[3] = CreateShaderProg("./shaders/shadow/shadow.vs", "./shaders/shadow/shadow.fs", NULL);
   shadowMapShader = CreateShaderProg("./shaders/lightPOV/lightPOV.vs", "./shaders/lightPOV/lightPOV.fs", NULL);
   normalShader = CreateShaderProg("./shaders/normals/normals.vs", "./shaders/normals/normals.fs", "./shaders/normals/normals.gs");
   InitMap();
   ErrCheck("init");
   //  Pass control to GLUT so it can interact with the user
   glutMainLoop();
   return 0;
}