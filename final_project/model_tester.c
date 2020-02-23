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
#include "CSCIx229.h"
#include "cglm/cglm.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
int normals = 0; // Display normal vectors
int axes = 0;    // Display axes
double t = 0;    // Timer
double daytime = 0;
double dayCycleDuration = 24;
int wall = 1;    // Floor/wall boolean
float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame
int SCR_WIDTH = 600, SCR_HEIGHT = 600;

/*    Camera/Eye position    */
int th = 30;      //  Azimuth of view angle
int ph = 30;      //  Elevation of view angle
vec3 cameraPos;
vec3 cameraUp = {0.0f, 1.0f, 0.0f};
vec3 cameraFront; 
int camera_mode=1;
float cameraSpeed = 0.01f;
float lastX = 600/2, lastY=600/2;
float pitch = 0, yaw = 0;
float sensitivity = 0.3;

/*    Projection model values    */
double asp = 1;   //  Aspect ratio
double dim = 6.0; //  Size of world
int fov = 55;     //  Field of view

/*    Light model values     */
int emission = 0;  // Emission intensity (%)
int light_ambient = 20;  // Ambient intensity (%)
int light_diffuse = 50; // Diffuse intensity (%)
int light_specular = 100;  // Specular intensity (%)
float shiny = 32;   // Shininess (value)
float Diffuse[4], Ambient[4], Specular[4], Emission[4];

/*    Floating light positioning     */
float zh = 90;       // Light azimuth
float ylight = 3;  // Elevation of light
int distance = 20;  // Light distance
int light_move_mode = 0;      // Light ball movement type
float Light_Pos[4]; // Light Position
vec3 lightUp = {0,1,0};

/*    Texture variables for hat    */
double rep = 1;
int texMode = 0;
int ntex = 0;
unsigned int texture[3];
int tex = 0;
GLuint grassTex, fernTex, treeTex, pathTex, flowerTex, mudTex, blendMap;;

/*    Skybox texture    */
char *dayBoxTex[6];
char *nightBoxTex[6];
GLuint DayBox;
GLuint NightBox;
vec3 skyColor = {0.5, 0.5, 0.5};
float skyBlendFactor = 0;

/*    Shader variables     */
int shader_mode = 3;                            //  Current active shader
int shader[] = {0, 0, 0, 0};                       //  Array to hold shader programs
int num_shaders = sizeof(shader) / sizeof(int); //  number of shaders
int drawArrayShaders = 0;

/*    Shaddow Map Variables      */
int shadowdim;             // Size of shadow map textures
unsigned int framebuf = 0; // Frame buffer id
GLuint depthMapFBO;
GLuint depthMap;
int depthShader, normalShader, shadowMapDebug, skyShader, terrainShader;
const GLuint SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

/*    VAOs     */
GLuint hatVao = 0;
GLuint wallVao = 0;
GLuint skyBoxVao = 0;

unsigned int quadVAO = 0;
unsigned int quadVBO;

/*    Objects     */
int fern;
int tree;

/*    Test variables     */
int init = 1;
mat4 gModelMatrix, gViewMatrix;
float near_plane = 0.0f, far_plane = 35.0f;
int debug = 0;

/*    Fill specified buffer with vertex info at given index   */
static void FillBuffer(int index, float bufName[], float x, float y, float z)
{
   bufName[index] = x;
   bufName[index + 1] = y;
   if (z != NAN)
      bufName[index + 2] = z;
   return;
}

/*    Draw Skybox    */
static void Skybox(){
   if (skyBoxVao ==0){
      float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };
      glGenVertexArrays(1, &skyBoxVao);
      glBindVertexArray(skyBoxVao);
      GLuint position_vbo;
      glGenBuffers(1, &position_vbo);
      glBindBuffer(GL_ARRAY_BUFFER, position_vbo);
      glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
      glEnableVertexAttribArray(0);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
   }
   glBindVertexArray(skyBoxVao);
   glDrawArrays(GL_TRIANGLE_STRIP, 0, 36);
   glBindVertexArray(0);
}

/*    Draw a wall    */
static void Wall(float St)
{
   int n = 8;
   int num_verteces = (2 * n + 1) * (2 * n) * 2;
   if (wallVao == 0)
   {
      int i, j;
      float s = 1.0 / n;
      float t = 0.5 * St / n;
      float position[num_verteces * 3];
      float normals[num_verteces * 3];
      float colors[num_verteces * 3];
      float textCoords[num_verteces * 2];
      int bufIndexCount = 0;
      //  Draw walls
      for (j = -n; j < n; j++)
      {
         for (i = -n; i <= n; i++)
         {
            FillBuffer(bufIndexCount, position, i * s, j * s, 0);
            FillBuffer(bufIndexCount + 3, position, i * s, (j + 1) * s, 0);
            FillBuffer(bufIndexCount, normals, 0, 0, 1);
            FillBuffer(bufIndexCount + 3, normals, 0, 0, 1);
            FillBuffer(bufIndexCount, colors, 0.5,0.5,0.5);
            FillBuffer(bufIndexCount + 3, colors, 0.5,0.5,0.5);
            FillBuffer(bufIndexCount / 3 * 2, textCoords, (i + n) * t, (j + n) * t, NAN);
            FillBuffer((bufIndexCount + 3) / 3 * 2, textCoords, (i + n) * t, (j + 1 + n) * t, NAN);
            bufIndexCount += 6;
         }
      }
      glGenVertexArrays(1, &wallVao);
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
   }

   glEnable(GL_DEPTH_TEST);
   glDepthMask(GL_TRUE);
   glDepthFunc(GL_LEQUAL);
   glDepthRange(0.0f, 1.0f);

   glBindVertexArray(wallVao);
   glDrawArrays(GL_TRIANGLE_STRIP, 0, num_verteces);
   glBindVertexArray(0);
}

/*    Draw cowboy hat   */
static void Hat()
{
   /*    Hat fidelity step    */
   float r_step = 0.1;  // Hat radius fidelity
   int angle_step = 15; // Hat angle fidelity

   /*    Hat max dimensions    */
   int r_max = 3;
   int angle_max = 360;

   /*    Set up Buffer arays for Shaders     */
   int num_verteces = ((r_max / r_step) + 1) * ((angle_max / angle_step) + 1) * 4 * 2; //(number of r steps + 1 for 0 index) * (number of angle steps + 1 for 0 index) * 4 points drawn per point on hat * 2 hats

   if (hatVao == 0)
   {
      float position[num_verteces * 3];
      float normals[num_verteces * 3];
      float colors[num_verteces * 3];
      float textCoords[num_verteces * 2];
      int bufIndexCount = 0;

      /*    Constants for hat curvature    */
      float a = -0.5; // x^4
      //float b = 0.0;  // x^3
      float c = 0.9; // x^2
      //float d = 0.0;  // x
      float e = 1.5; // constant

      /*    Loop Twice: Top/bottom surface of hat     */
      for (int i = 0; i < 2; i++)
      {
         int flip_norm=(i==0)? -1:-1;
         /*    iterate over total radius     */
         for (float r = 0; r <= r_max; r += r_step)
         {
            
            int textFix = 0;
            /*    iterate over 360 degrees     */
            for (int angle = 0; angle <= angle_max; angle += angle_step)
            {
               float closure;

               for (float j = 0; j<=r_step; j+= r_step){
               if (r > 2.9 && j>0)
               {
                  closure = 0.0;
               }
               else closure = -0.1 * i;
               float radius = r + j;
               float xv = ((sqrt(2) * sqrt(0.5)) + radius) * Cos(angle);
               float zv = radius * Sin(angle);
               float yv = a * pow(sqrt(pow(xv, 2) / 2 + pow(zv, 2)), 4) + c * pow(sqrt(pow(xv, 2) / 2 + pow(zv, 2)), 2) + e + closure;
               float yv_brim = 0.1 * pow(zv, 2) - 0.1 + closure;
               float iv, jv, kv;
               if (yv >= yv_brim - 1.5)
               {
                  iv = flip_norm * (-0.5 * pow(xv, 3) - xv * pow(zv, 2) + 0.9 * xv);
                  jv = flip_norm * (-1);
                  kv = flip_norm * (-2 * pow(zv, 3) - zv * pow(xv, 2) + 1.8 * zv);
               }
               else
               {
                  iv = 0;
                  jv = flip_norm * (-1);
                  kv = flip_norm * (0.2 * zv);
               }

               FillBuffer(bufIndexCount, position, xv, fmax(yv, yv_brim), zv);
               FillBuffer(bufIndexCount, normals, iv, jv, kv);
               FillBuffer(bufIndexCount, colors, 0.54, 0.27, 0.075 + yv / 12);
               FillBuffer(bufIndexCount / 3 * 2, textCoords, (j==0)? 0.0:1.0, textFix ? 0.0 : rep, NAN);

               bufIndexCount += 3;
               if(j>0) textFix = !textFix; 
               }
            }
         }
      }

      glGenVertexArrays(1, &hatVao);
      glBindVertexArray(hatVao);
      GLuint position_vbo;
      glGenBuffers(1, &position_vbo);
      glBindBuffer(GL_ARRAY_BUFFER, position_vbo);
      glBufferData(GL_ARRAY_BUFFER, sizeof(position), position, GL_STATIC_DRAW);
      glEnableVertexAttribArray(0);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

      GLuint normals_vbo;
      glGenBuffers(1, &normals_vbo);
      glBindBuffer(GL_ARRAY_BUFFER, normals_vbo);
      glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);
      glEnableVertexAttribArray(1);
      glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

      GLuint colors_vbo;
      glGenBuffers(1, &colors_vbo);
      glBindBuffer(GL_ARRAY_BUFFER, colors_vbo);
      glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
      glEnableVertexAttribArray(2);
      glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

      GLuint tex_vbo;
      glGenBuffers(1, &tex_vbo);
      glBindBuffer(GL_ARRAY_BUFFER, tex_vbo);
      glBufferData(GL_ARRAY_BUFFER, sizeof(textCoords), textCoords, GL_STATIC_DRAW);
      glEnableVertexAttribArray(3);
      glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, NULL);
      //printf("|   %d   |   %d   |    ", bufIndexCount/6, num_verteces/4);
   }

   glBindVertexArray(hatVao);
   /* GlBinf */
   //Draw Top Hat
   glDrawArrays(GL_TRIANGLE_STRIP, 0, num_verteces / 4);
   //Draw Bottom Hat
   glDrawArrays(GL_TRIANGLE_STRIP, num_verteces / 4, num_verteces / 4);
   glBindVertexArray(0);
}

/*    Flat Quad for rendering Shadow Map     */
void renderQuad()
{
   if (quadVAO == 0)
   {
      float quadVertices[] = {
          // positions        // texture Coords
          -1.0f,
          1.0f,
          0.0f,
          0.0f,
          1.0f,
          -1.0f,
          -1.0f,
          0.0f,
          0.0f,
          0.0f,
          1.0f,
          1.0f,
          0.0f,
          1.0f,
          1.0f,
          1.0f,
          -1.0f,
          0.0f,
          1.0f,
          0.0f,
      };
      // setup plane VAO
      glGenVertexArrays(1, &quadVAO);
      glGenBuffers(1, &quadVBO);
      glBindVertexArray(quadVAO);
      glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
      glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
      glEnableVertexAttribArray(0);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
      glEnableVertexAttribArray(1);
      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
   }
   glBindVertexArray(quadVAO);
   glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
   glBindVertexArray(0);
}

/*    Draw axes      */
static void Axes()
{
   const double len = 1.5; //  Length of axes
   //  Draw axes
   if (axes && camera_mode==0)
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

/*    Draw Simulation Info to Screen      */
static void Info()
{
   glColor3f(1, 1, 1);
   glWindowPos2i(5, 75);
   Print("Ambient=%d  Diffuse=%d Specular=%d", light_ambient, light_diffuse, light_specular);
   glWindowPos2i(5, 50);
   Print("Angle=%d,%d | Dim=%.1f | FOV=%d | light mode:%d | normals:%d", th, ph, dim, fov, light_move_mode, normals);
   glWindowPos2i(5, 25);
   Print("Camera (%f, %f, %f) | camera mode: %d", cameraPos[0], cameraPos[1], cameraPos[2], camera_mode);
}

/*    Set light Position   */
static void Light()
{
   //  Set light position
   switch (light_move_mode)
   {
      case 0:
         Light_Pos[0] = distance * Cos(zh);
         Light_Pos[1] = ylight;
         Light_Pos[2] = distance * Sin(zh);
         Light_Pos[3] = 1;
         lightUp[1]=1.0;
         break;
      case 1:
         Light_Pos[0] = distance * Cos(zh)+cameraPos[0];
         Light_Pos[1] = distance * Sin(zh);
         Light_Pos[2] = 0+cameraPos[2];
         Light_Pos[3] = 1;
         lightUp[1]=Cos(zh);
         break;
      default:
         Light_Pos[0] = 0.5;
         Light_Pos[1] = distance*2;
         Light_Pos[2] = 0.5;
         Light_Pos[3] = 1;
   }
}

/*    Set Camera/eye position    */
void Camera()
{
   switch (camera_mode){
      case 0: 
         cameraPos[0] = -2 * dim * Sin(th) * Cos(ph);
         cameraPos[1] = +2 * dim * Sin(ph);
         cameraPos[2] = +2 * dim * Cos(th) * Cos(ph);
         glm_vec3_scale(cameraPos, -1, cameraFront);
         cameraUp[1] = Cos(ph);
         break;
      case 1:
         cameraFront[0] = cos(glm_rad(pitch)) * cos(glm_rad(yaw));
         cameraFront[1] = sin(glm_rad(pitch));
         cameraFront[2] = cos(glm_rad(pitch)) * sin(glm_rad(yaw));
         glm_normalize(cameraFront);
         cameraUp[1] = 1;
         cameraPos[1]=1;
         break;
      case 2:
      cameraFront[0] = cos(glm_rad(pitch)) * cos(glm_rad(yaw));
         cameraFront[1] = sin(glm_rad(pitch));
         cameraFront[2] = cos(glm_rad(pitch)) * sin(glm_rad(yaw));
         glm_normalize(cameraFront);
         cameraUp[1] = 1;
         break;   
      }
}

/*    Set the View and Projection Matrices      */
void SetViewProjectionMatrices(int shader)
{
   float ProjectionMatrix[16];
   glGetFloatv(GL_PROJECTION_MATRIX, ProjectionMatrix);

   mat4 ViewMatrix;
   vec3 focus;

   Camera();
   glm_vec3_add(cameraPos, cameraFront, focus);
   
   glm_lookat(cameraPos,
              focus,
              cameraUp,
              ViewMatrix);
   glm_mat4_copy(ViewMatrix, gViewMatrix);

   if (shader == skyShader){
      mat3 upperLeftView;
      glm_mat4_pick3(ViewMatrix, upperLeftView);
      glm_mat4_identity(ViewMatrix);
      glm_mat4_ins3(upperLeftView, ViewMatrix);
   }
   
   glUniformMatrix4fv(glGetUniformLocation(shader, "ViewMatrix"), 1, 0, (float *)ViewMatrix);
   glUniformMatrix4fv(glGetUniformLocation(shader, "ProjectionMatrix"), 1, 0, ProjectionMatrix);
   glUniform3fv(glGetUniformLocation(shader, "ViewPosition"), 1, cameraPos);
}

/*    Set the ambient, diffuse, specular, shinyness of material   */
void SetMaterial(int shader, vec3 ambient, vec3 diffuse, vec3 specular, float shinyness){
   glUniform3fv(glGetUniformLocation(shader, "Material.Ambient"), 1, ambient);
   glUniform3fv(glGetUniformLocation(shader, "Material.Diffuse"), 1, diffuse);
   glUniform3fv(glGetUniformLocation(shader, "Material.Specular"), 1, specular);
   glUniform1f(glGetUniformLocation(shader, "Material.Shinyness"), shinyness);   
}

/*    Set the Model, ModelView and Normal Matrices      */
void setModelViewNormalMatrices(int shader, float x_trans, float y_trans, float z_trans, float x_rot, float y_rot, float z_rot, float x_scale, float y_scale, float z_scale)
{
   if (!drawArrayShaders)
   {
      glMatrixMode(GL_MODELVIEW);
      glRotatef(x_rot, 1, 0, 0);
      glRotatef(y_rot, 0, 1, 0);
      glRotatef(z_rot, 0, 0, 1);
      glTranslatef(x_trans, y_trans, z_trans);
      glScalef(x_scale, y_scale, z_scale);
   }

   mat4 ModelMatrix;
   mat4 ModelViewMatrix;
   mat3 NormalMatrix;
   glm_mat4_identity(ModelMatrix);
   glm_translate(ModelMatrix, (vec3){x_trans, y_trans, z_trans});
   glm_rotate_y(ModelMatrix, y_rot * 3.1415926 / 180, ModelMatrix);
   glm_rotate_x(ModelMatrix, x_rot * 3.1415926 / 180, ModelMatrix);
   glm_rotate_z(ModelMatrix, z_rot * 3.1415926 / 180, ModelMatrix);
   glm_scale(ModelMatrix, (vec3){x_scale, y_scale, z_scale});

   glm_mat4_mul(gViewMatrix, ModelMatrix, ModelViewMatrix);
   glm_mat4_pick3(ModelViewMatrix, NormalMatrix);
   glm_mat3_inv(NormalMatrix, NormalMatrix);
   glm_mat3_transpose(NormalMatrix);

   glUniformMatrix4fv(glGetUniformLocation(shader, "ModelMatrix"), 1, 0, (float *)ModelMatrix);
   glUniformMatrix3fv(glGetUniformLocation(shader, "NormalMatrix"), 1, 0, (float *)NormalMatrix);
   glUniformMatrix3fv(glGetUniformLocation(shader, "ModelViewMatrix"), 1, 0, (float *)ModelViewMatrix);
}

 void Terrain(GLuint shader, int size)
{
   srand(1);
   //glUseProgram(terrainShader);
   /* SetViewProjectionMatrices(terrainShader);
   SetMaterial(terrainShader, (vec3){1.0f,1.0f,1.0f}, (vec3){1.0f,1.0f,1.0f}, (vec3){ 1.0f,1.0f,1.0f}, 16.0f );
   glUniform1i(glGetUniformLocation(terrainShader, "ShadowMap"), 1);
   glUniform1i(glGetUniformLocation(terrainShader, "DiffuseTexture"), 0);
   glUniform1i(glGetUniformLocation(terrainShader, "rTexture"), 2);
   glUniform1i(glGetUniformLocation(terrainShader, "gTexture"), 3);
   glUniform1i(glGetUniformLocation(terrainShader, "bTexture"), 4);
   glUniform1i(glGetUniformLocation(terrainShader, "blendMap"), 5);
   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, grassTex);
   glActiveTexture(GL_TEXTURE1);
   glBindTexture(GL_TEXTURE_2D,depthMap);
   glActiveTexture(GL_TEXTURE2);
   glBindTexture(GL_TEXTURE_2D,pathTex);
   glActiveTexture(GL_TEXTURE3);
   glBindTexture(GL_TEXTURE_2D,flowerTex);
   glActiveTexture(GL_TEXTURE4);
   glBindTexture(GL_TEXTURE_2D,grassTex);
   glActiveTexture(GL_TEXTURE5);
   glBindTexture(GL_TEXTURE_2D,blendMap); */
   SetMaterial(terrainShader, (vec3){1.0f,1.0f,1.0f}, (vec3){1.0f,1.0f,1.0f}, (vec3){ 1.0f,1.0f,1.0f}, 8.0f );
   for(int i = -size/2;i<size/2;i++){
      for(int j = -size/2;j<size/2;j++){
         setModelViewNormalMatrices(shader, i, 0, j,-90, 0, 0, 1, 1, 1);
         Wall(4);
         int random = rand();
         if (random <10000000){
            setModelViewNormalMatrices(shader, i, 0, j,0, 0, 0, 0.1, 0.1, 0.1);
            glEnable(GL_TEXTURE_2D);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, fernTex);
            glColor3f(1.0,1.0,1.0);
            glCallList(fern);
            glBindTexture(GL_TEXTURE_2D, grassTex);
         }
         else if (random < 20000000){
            setModelViewNormalMatrices(shader, i, 0, j,0, 0, 0, 1, 1, 1);
            glEnable(GL_TEXTURE_2D);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, treeTex);
            glCallList(tree);
            glBindTexture(GL_TEXTURE_2D, grassTex);
         }
      }
   }
   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, 0);
}

/*    Set Ambient, Diffuse, Specular of light, and skyColor   */
void SetLightColor(int shader, vec3 ambient, vec3 diffuse, vec3 specular){
   glUniform3fv(glGetUniformLocation(shader, "Light.Ambient"), 1, ambient);
   glUniform3fv(glGetUniformLocation(shader, "Light.Diffuse"), 1, diffuse);
   glUniform3fv(glGetUniformLocation(shader, "Light.Specular"), 1, specular);
   glUniform3fv(glGetUniformLocation(shader, "SkyColor"), 1, skyColor);
      
}
void setSkyBoxUniforms()
{
   glUniform1i(glGetUniformLocation(skyShader, "dayBox"), 0);
   glUniform1i(glGetUniformLocation(skyShader, "nightBox"), 1);
   glUniform3fv(glGetUniformLocation(skyShader, "SkyColor"), 1, skyColor);

   
   float quarterDay = dayCycleDuration/4;
   if(daytime >= 0 && daytime < quarterDay){
      skyBlendFactor = 0.0;
		}
   else if(daytime >= quarterDay && daytime < 2*quarterDay){
			skyBlendFactor = (daytime-quarterDay)/quarterDay;
		}
   else if(daytime >= 2*quarterDay && daytime < 3 * quarterDay){
         skyBlendFactor=1;
		}
   else{
         skyBlendFactor= 1-(daytime-quarterDay*3)/(quarterDay);
		}


   glUniform1f(glGetUniformLocation(skyShader, "BlendFactor"), skyBlendFactor);
   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_CUBE_MAP, DayBox);
   glActiveTexture(GL_TEXTURE1);
   glBindTexture(GL_TEXTURE_CUBE_MAP, NightBox);
   
}

/*    Render the Objects in the Scene      */
void Scene(int shader)
{
   setModelViewNormalMatrices(shader, 0, 2, 0, 0, 0, 0, 1, 1, 1);
   SetMaterial(shader, (vec3){1.0f,1.0f,1.0f}, (vec3){1.0f,1.0f,1.0f}, (vec3){ 1.0f,1.0f,1.0f}, 16.0f );
   Hat();

   
   if (wall && shader )
   {
      setModelViewNormalMatrices(shader, 0, 0, 0, -90, 0, 0, 1, 1, 1);
      SetMaterial(shader, (vec3){0.05f,0.05f,0.05f}, (vec3){0.5f,0.5f,0.5f,}, (vec3){ 0, 0, 0}, 10.0f );
      Terrain(shader, 80);
   }

   glDepthFunc(GL_LEQUAL);
   glUseProgram(skyShader);
   SetViewProjectionMatrices(skyShader);
   setModelViewNormalMatrices(skyShader, 0, 0, 0, 0, t, 0, 45.0, 45.0, 45.0);
   setSkyBoxUniforms();
   Skybox();
   glDepthFunc(GL_LESS);
   glBindVertexArray(0);
}

/*    Configure the uniforms for Depth Map: LighSpaceMatrix, LightPosition      */
void ConfigureLightPovMatrices(int shader)
{
   /* float near_plane = 1.0f, far_plane = 7.5f; */
   mat4 LightProjection, LightView, LightSpaceMatrix;
   glm_ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane, LightProjection);
   //vec3 LightPos={0.0f, 4.0f, 0.0f};
   vec3 LightPos = {Light_Pos[0], Light_Pos[1], Light_Pos[2]};
   vec3 lookat = {0.0, 0.0, 0.0};
   if (camera_mode==1){
      lookat[0]=cameraPos[0];
      lookat[2]=cameraPos[2];

   }
   glm_lookat(LightPos,
              lookat,
              lightUp,
              LightView);

   glm_mat4_mul(LightProjection, LightView, LightSpaceMatrix);

   glUniformMatrix4fv(glGetUniformLocation(shader, "LightSpaceMatrix"), 1, 0, (float *)LightSpaceMatrix);
   glUniform3fv(glGetUniformLocation(shader, "LightPosition"), 1, LightPos);
}

/*    Configure the textures amd eye position for shadow mapping      */
void ConfigureShadowShader(int shader)
{
   glUniform1i(glGetUniformLocation(shader, "ShadowMap"), 1);
   glUniform1i(glGetUniformLocation(shader, "DiffuseTexture"), 0);
   glUniform3fv(glGetUniformLocation(shader, "ViewPos"), 1, cameraPos);
}

/*    Render Depth Map and set up Shadow map shader   */
void ShadowMap(int shader)
{
   // 1. first render to depth map
   glUseProgram(depthShader);
   ConfigureLightPovMatrices(depthShader);

   glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
   glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
   glClear(GL_DEPTH_BUFFER_BIT);
   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, tex);
   glCullFace(GL_FRONT);
   Scene(depthShader);
   glCullFace(GL_BACK);
   glBindFramebuffer(GL_FRAMEBUFFER, 0);

   // 2. then render scene as normal with shadow mapping (using depth map)
   glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glUseProgram(shader);
   ConfigureLightPovMatrices(shader);
   ConfigureShadowShader(shader);
   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, tex);
   glActiveTexture(GL_TEXTURE1);
   glBindTexture(GL_TEXTURE_2D, depthMap);
   SetViewProjectionMatrices(shader);
   /* Scene(shader[3]); */
}

/*    Initiate depth map buffer for shadow mapping    */
void InitMap()
{
   glGenFramebuffers(1, &depthMapFBO);
   glBindVertexArray(0);
   //SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

   glGenTextures(1, &depthMap);
   glBindTexture(GL_TEXTURE_2D, depthMap);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
   float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
   glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

   glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
   glDrawBuffer(GL_NONE);
   glReadBuffer(GL_NONE);
   if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      Fatal("Error setting up frame buffer\n");
   glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLuint InitSkybox(char *faces[])
{
   GLuint cubemap;
   glGenTextures(1, &cubemap);
   glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
   int width, height, nrChannels;
    for (unsigned int i = 0; i < 6; i++)
    {
        unsigned char *data = stbi_load(faces[i], &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            printf("Cubemap texture failed to load at path:  %s", faces[i] );
            stbi_image_free(data);
        }
    }
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
   return cubemap;
} 

/*    OpenGL (GLUT) calls this routine to display the scene    */
void display()
{
   //  Erase the window and the depth buffer
   glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   //  Enable Z-buffering in OpenGL
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_NORMALIZE);

   Light();
   if (shader_mode == 3 || shader_mode == 2 || debug)
      ShadowMap(shader[shader_mode]);
   else
   {
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, tex);
      glUseProgram(shader[shader_mode]);
   }
   glEnable(GL_DEPTH_TEST);
   SetViewProjectionMatrices(shader[shader_mode]);
   SetLightColor(shader[shader_mode], (vec3){light_ambient*0.01, light_ambient*0.01, light_ambient*0.01}, (vec3){light_diffuse*0.01, light_diffuse*0.01, light_diffuse*0.01}, (vec3){light_specular*0.01, light_specular*0.01, light_specular*0.01});
   // Draw Hat
   if (!debug)
   {
      Scene(shader[shader_mode]);
   }
   else
   {
      glUseProgram(shadowMapDebug);
      glUniform1fv(glGetUniformLocation(shadowMapDebug, "near_plane"), 1, &near_plane);
      glUniform1fv(glGetUniformLocation(shadowMapDebug, "near_plane"), 1, &far_plane);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, depthMap);
      renderQuad();
   }

   glBindFramebuffer(GL_FRAMEBUFFER, 0);

   if (normals)
   {
      glUseProgram(normalShader);
      SetViewProjectionMatrices(normalShader);
      Scene(normalShader);
   }
   
   glUseProgram(shader[0]);
   glPushMatrix();
   glLoadIdentity();
   glEnable(GL_NORMALIZE);
   glDisable(GL_TEXTURE_2D);
   glDisable(GL_LIGHTING);
   glDisable(GL_COLOR_MATERIAL);
   gluLookAt(cameraPos[0], cameraPos[1], cameraPos[2], cameraPos[0]+cameraFront[0], cameraPos[1]+cameraFront[1], cameraPos[2]+cameraFront[2], 0, cameraUp[1], 0);
   glColor3f(1, 1, 1);
   //  Draw light Light_Pos as sphere
   glPushMatrix();
   glColor3f(1,0.99,0.45);
   glTranslated(Light_Pos[0], Light_Pos[1], Light_Pos[2]);
   glutSolidSphere(0.1, 10, 10);
   glPopMatrix();
   glColor3f(1,1,1);
   // Draw axes
   Axes();
   // Write info
   Info();
   glPopMatrix();
   //  Render the scene
   glFlush();
   //  Make the rendered scene visible
   glutSwapBuffers();
}

/*    Read text file    */
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

/*    Mouse Movement    */
void mouseMovement(int x, int y){
   float dlta_x = x - lastX;
   float dlta_y = y - lastY;
   lastX = x;
   lastY = y;

   dlta_x *= sensitivity;
   dlta_y *= sensitivity;

   pitch -= dlta_y;
   yaw += dlta_x;

   while (pitch>360)pitch-=360;
   while (yaw>360)yaw-=360;

   if(pitch > 89.0f)
      pitch =  89.0f;
   if(pitch < -89.0f)
      pitch = -89.0f;
   
   
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/*    GLUT calls this routine when an arrow key is pressed     */
void special(int key, int x, int y)
{
   float cur_cam_vel = cameraSpeed * deltaTime;

   if (camera_mode ==0 ){
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
   }
   else if (camera_mode == 1){
      vec3 displacement;
      //  Right arrow key - increase angle by 5 degrees
      if (key == GLUT_KEY_RIGHT){
         glm_vec3_cross(cameraFront, cameraUp, displacement);
         glm_vec3_normalize(displacement);
         glm_vec3_scale(displacement, cur_cam_vel, displacement);
         glm_vec3_add(cameraPos, displacement, cameraPos ); 
      }
      //  Left arrow key - decrease angle by 5 degrees
      else if (key == GLUT_KEY_LEFT){
         glm_vec3_cross(cameraFront, cameraUp, displacement);
         glm_vec3_normalize(displacement);
         glm_vec3_scale(displacement, cur_cam_vel, displacement);
         glm_vec3_sub(cameraPos, displacement, cameraPos ); 
      }
      //  Up arrow key - increase elevation by 5 degrees
      else if (key == GLUT_KEY_UP){
         glm_vec3_scale(cameraFront, cur_cam_vel, displacement);
         glm_vec3_add(cameraPos, displacement, cameraPos );
      }
      //  Down arrow key - decrease elevation by 5 degrees
      else if (key == GLUT_KEY_DOWN){
         glm_vec3_scale(cameraFront, cur_cam_vel, displacement);
         glm_vec3_sub(cameraPos, displacement, cameraPos );
      }
   }
   else if (camera_mode == 2){
      vec3 displacement;
      //  Right arrow key - increase angle by 5 degrees
      if (key == GLUT_KEY_RIGHT){
         glm_vec3_cross(cameraFront, cameraUp, displacement);
         glm_vec3_normalize(displacement);
         glm_vec3_scale(displacement, cur_cam_vel, displacement);
         glm_vec3_add(cameraPos,displacement, cameraPos ); 
      }
      //  Left arrow key - decrease angle by 5 degrees
      else if (key == GLUT_KEY_LEFT){
         glm_vec3_cross(cameraFront, cameraUp, displacement);
         glm_vec3_normalize(displacement);
         glm_vec3_scale(displacement, cur_cam_vel, displacement);
         glm_vec3_sub(cameraPos,displacement, cameraPos ); 
      }
      //  Up arrow key - increase elevation by 5 degrees
      else if (key == GLUT_KEY_UP){
         glm_vec3_scale(cameraFront, cur_cam_vel, displacement);
         glm_vec3_add(cameraPos,displacement, cameraPos );
      }
      //  Down arrow key - decrease elevation by 5 degrees
      else if (key == GLUT_KEY_DOWN){
         glm_vec3_scale(cameraFront, cur_cam_vel, displacement);
         glm_vec3_sub(cameraPos,displacement, cameraPos );
      }
      else if (key == GLUT_KEY_PAGE_UP)
         cameraPos[1]+=cur_cam_vel;
      //  PageDown key - decrease dim
      else if (key == GLUT_KEY_PAGE_DOWN && dim > 1)
         cameraPos[1]-=cur_cam_vel;
   }

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
   // Camera Mode
   else if (ch == 'v' || ch == 'V')
      camera_mode = (camera_mode + 1) % 3;
   //  Light movement mode
   else if (ch == 'm' || ch == 'M')
      light_move_mode = (light_move_mode + 1) % 2;
   //  Ambient level
   else if (ch == 'a' && light_ambient > 0)
      light_ambient -= 5;
   else if (ch == 'A' && light_ambient < 100)
      light_ambient += 5;
   //  Diffuse level
   else if (ch == 'd' && light_diffuse > 0)
      light_diffuse -= 5;
   else if (ch == 'D' && light_diffuse < 100)
      light_diffuse += 5;
   //  Specular level
   else if (ch == 's' && light_specular > 0)
      light_specular -= 5;
   else if (ch == 'S' && light_specular < 100)
      light_specular += 5;
   //  FOV
   else if (ch == 'f')
      fov--;
   else if (ch == 'F')
      fov++;
   // Toggle Shadow Map
   else if (ch == 'Z' || ch == 'z')
      debug = !debug;
   //  Shader selector
/*    else if (ch == '<' || ch == ',')
      shader_mode = (shader_mode + num_shaders - 1) % num_shaders;
   else if (ch == '>' || ch == '.')
      shader_mode = (shader_mode + 1) % num_shaders; */
}

/*    GLUT calls this routine when the window is resized    */
void reshape(int width, int height)
{
   //  Ratio of the width to the height of the window
   asp = (height > 0) ? (double)width / height : 1;

   SCR_WIDTH = width;
   SCR_HEIGHT = height;
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
   daytime = fmod(t, dayCycleDuration);
   float currentFrame = glutGet(GLUT_ELAPSED_TIME);
   deltaTime = currentFrame - lastFrame;
   lastFrame = currentFrame;
   //zh = fmod(45 * t, 360.0);
   zh = (360 * daytime)/dayCycleDuration;
   Project(fov, asp, dim);
   glutPostRedisplay();
}

GLuint loadPng(char *faces){
   GLuint texture;
   glGenTextures(1, &texture);
   glBindTexture(GL_TEXTURE_2D, texture);
   int width, height, nrChannels;
   //GLuint* image =  stbi_load(faces, &width, &height, &nrChannels, STBI_rgb_alpha);
   unsigned char *image = stbi_load(faces, &width, &height, &nrChannels, STBI_rgb_alpha);

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

   glBindTexture(GL_TEXTURE_2D, 0);

   stbi_image_free(image);
   return texture;
}

/* GLuint LoadObject(char *file){
   FILE* f = fopen(file,"r");
   if (!f) Fatal("Cannot open file %s\n",file);

   //float verteces[];

} */

/*    Start up GLUT and tell it what to do     */
int main(int argc, char *argv[])
{
   //  Initialize GLUT and process user parameters
   glutInit(&argc, argv);
   //  Request double buffered, true color window with Z buffering at 600x600
   glutInitWindowSize(SCR_WIDTH, SCR_HEIGHT);
   glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
   //  Create the window
   glutCreateWindow("Model viewer");

   //  Tell GLUT to call "idle" when there is nothing else to do
   glutIdleFunc(idle);
   //  Tell GLUT to call "display" when the scene should be drawn
   glutDisplayFunc(display);
   //  Tell GLUT to call "reshape" when the window is resized
   glutReshapeFunc(reshape);
   //  Tell GLUT to call "special" when an arrow key is pressed
   glutSpecialFunc(special);
   //  Tell GLUT to call "key" when a key is presse
   glutKeyboardFunc(key);
   // Tell Glut wha tot call when mouse moves
   glutPassiveMotionFunc(mouseMovement);

   LoadTexBMP("./textures/490.bmp");
   grassTex = LoadTexBMP("./textures/grass.bmp");
   tex = LoadTexBMP("./textures/490.bmp");
   pathTex = LoadTexBMP("./textures/path.bmp");
   mudTex = LoadTexBMP("./textures/mud.bmp");
   flowerTex = LoadTexBMP("./textures/flowers.bmp");
   blendMap = loadPng("./textures/blendMap.png");
   
   dayBoxTex[0] = "./textures/blueSkybox/bluecloud_rt.bmp";
   dayBoxTex[1] = "./textures/blueSkybox/bluecloud_lf.bmp";
   dayBoxTex[2] = "./textures/blueSkybox/bluecloud_up.bmp";
   dayBoxTex[3] = "./textures/blueSkybox/bluecloud_dn.bmp";
   dayBoxTex[4] = "./textures/blueSkybox/bluecloud_bk.bmp";
   dayBoxTex[5] = "./textures/blueSkybox/bluecloud_ft.bmp";
   DayBox = InitSkybox(dayBoxTex);

   nightBoxTex[0] = "./textures/nightSkyBox/nightRight.png";
   nightBoxTex[1] = "./textures/nightSkyBox/nightLeft.png";
   nightBoxTex[2] = "./textures/nightSkyBox/nightTop.png";
   nightBoxTex[3] = "./textures/nightSkyBox/nightBottom.png";
   nightBoxTex[4] = "./textures/nightSkyBox/nightBack.png";
   nightBoxTex[5] = "./textures/nightSkyBox/nightFront.png";
   NightBox = InitSkybox(nightBoxTex);

   fernTex = loadPng("./textures/fern.png");
   treeTex = loadPng("./textures/tree.png");
   //Objects
   fern = LoadOBJ("objects/fern.obj", fernTex);
   tree = LoadOBJ("objects/tree.obj", treeTex);
   
   
   srand(1);
   //  Shaders
   shader[3] = CreateShaderProg("./shaders/phong_shadow/phong_shadow.vs", "./shaders/phong_shadow/phong_shadow.fs", NULL);
   depthShader = CreateShaderProg("./shaders/lightPOV/lightPOV.vs", "./shaders/lightPOV/lightPOV.fs", NULL);
   normalShader = CreateShaderProg("./shaders/normals/normals.vs", "./shaders/normals/normals.fs", "./shaders/normals/normals.gs");
   shadowMapDebug = CreateShaderProg("./shaders/shadowMapDebug/shadowMapDebug.vs", "./shaders/shadowMapDebug/shadowMapDebug.fs", NULL);
   skyShader = CreateShaderProg("./shaders/skyShader/skyShader.vs", "./shaders/skyShader/skyShader.fs",  NULL);
   terrainShader = CreateShaderProg("./shaders/terrain/terrain.vs", "./shaders/terrain/terrain.fs", NULL);
   InitMap();
   ErrCheck("init");
   //  Pass control to GLUT so it can interact with the user
   glutMainLoop();
   return 0;
}