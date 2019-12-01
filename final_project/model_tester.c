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

/*    Camera/Eye position    */
int      th = 30;       //  Azimuth of view angle
int      ph = 30;       //  Elevation of view angle

/*    Projection model values    */
double   asp = 1;       //  Aspect ratio
double   dim = 4.0;     //  Size of world
int      fov = 65;      //  Field of view

/*    Light model values     */
int      smooth = 1;    // Smooth/Flat shading
int      emission = 0;  // Emission intensity (%)
int      ambient = 30;  // Ambient intensity (%)
int      diffuse = 100; // Diffuse intensity (%)
int      specular = 0;  // Specular intensity (%)
int      shininess = 0; // Shininess (power of two)
float    shiny = 1;     // Shininess (value)

/*    Floating light positioning     */
int      zh = 90;       // Light azimuth
float    ylight = 0;    // Elevation of light
int      distance = 5;  // Light distance
int      move = 0;      // Light ball movement type
float    Position[4];

/*    Texture variables for hat    */
double rep = 1;
int texMode = 0;
int ntex = 0;
unsigned int texture[3];

/*    Shader variables     */
int shader_mode = 2;                               //  Current active shader
int shader[] = {0, 0, 0, 0};                       //  Array to hold shader programs
int num_shaders = sizeof(shader) / sizeof(int);    //  number of shaders

/*    Shaddow Map Variables      */
int shadowdim;          // Size of shadow map textures
unsigned int framebuf=0;// Frame buffer id
double       Svec[4];   // Texture planes S
double       Tvec[4];   // Texture planes T
double       Rvec[4];   // Texture planes R
double       Qvec[4];   // Texture planes Q

/*    Test variables     */
int init = 1;

static void FillBuffer(int index, float bufName[], float x, float y, float z)
{
   bufName[index] = x;
   bufName[index + 1] = y;
   if (z != NAN)
      bufName[index + 2] = z;
   return;
}

/* Draw cowboy hat 
 *    at (x,y,z)
 *    radius (r)

*/
static void Hat(double x, double y, double z, double r)
{
   glEnable(GL_TEXTURE_2D);
   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

   float white[] = {1, 1, 1, 1};
   float black[] = {0, 0, 0, 1};
   glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shiny);
   glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white);
   glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, black);
   glPushMatrix();

   //  Offset and scale
   glTranslated(x, y, z);
   glScaled(r, r, r);

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

   //93006200
   //9300 - bufIndexCount
   //6200 - bufferLen

   glPopMatrix();
   //  Switch off textures so it doesn't apply to the rest
   glDisable(GL_TEXTURE_2D);
   //Draw Top Hat
   glDrawArrays(GL_TRIANGLE_STRIP, 0, bufIndexCount / 6);
   //Draw Bottom Hat
   glDrawArrays(GL_TRIANGLE_STRIP, bufIndexCount / 6, bufIndexCount / 6);
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
   glWindowPos2i(5, 50);
   Print("Ambient=%d  Diffuse=%d Specular=%d Emission=%d", ambient, diffuse, specular, emission, shiny);
   glWindowPos2i(5, 25);
   Print("Angle=%d,%d | Dim=%.1f | FOV=%d | shader:%d | normals:%d", th, ph, dim, fov, shader_mode, normals);
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
   Position[0] = 2*Cos(zh);
   Position[1] = ylight;
   Position[2] = 2*Sin(zh);
   Position[3] = 1;

   //  Enable lighting
   if (light)
   {
      float Med[]  = {0.3,0.3,0.3,1.0};
      float High[] = {1.0,1.0,1.0,1.0};
      //  Enable lighting with normalization
      glEnable(GL_LIGHTING);
      glEnable(GL_NORMALIZE);
      //  glColor sets ambient and diffuse color materials
      glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
      glEnable(GL_COLOR_MATERIAL);
      //  Enable light 0
      glEnable(GL_LIGHT0);
      glLightfv(GL_LIGHT0,GL_POSITION,Position);
      glLightfv(GL_LIGHT0,GL_AMBIENT,Med);
      glLightfv(GL_LIGHT0,GL_DIFFUSE,High);
   }
   else
   {
      glDisable(GL_LIGHTING);
      glDisable(GL_COLOR_MATERIAL);
      glDisable(GL_NORMALIZE);
   }
}

void SetUniformMatrices(int shader_index)
{
   float ViewMatrix[16], ModelViewMatrix[16], ProjectionMatrix[16];
   glGetFloatv(GL_PROJECTION_MATRIX, ProjectionMatrix);
   glGetFloatv(GL_MODELVIEW_MATRIX, ViewMatrix);
   glGetFloatv(GL_MODELVIEW_MATRIX, ModelViewMatrix);
   /* printf("%f", ProjectionMatrix[10]) */

   mat3 inverse, NormalMatrix;

   mat3 submatrix = {
       {ModelViewMatrix[0], ModelViewMatrix[1], ModelViewMatrix[2]},
       {ModelViewMatrix[4], ModelViewMatrix[5], ModelViewMatrix[6]},
       {ModelViewMatrix[8], ModelViewMatrix[9], ModelViewMatrix[10]}};

   glm_mat3_inv(submatrix, inverse);
   glm_mat3_transpose_to(inverse, NormalMatrix);

   int id;
   //  Set matrixes
   id = glGetUniformLocation(shader[shader_index], "ModelViewMatrix");
   if (id >= 0)
      glUniformMatrix4fv(id, 1, 0, ModelViewMatrix);
   id = glGetUniformLocation(shader[shader_index], "ViewMatrix");
   if (id >= 0)
      glUniformMatrix4fv(id, 1, 0, ViewMatrix);
   id = glGetUniformLocation(shader[shader_index], "ProjectionMatrix");
   if (id >= 0)
      glUniformMatrix4fv(id, 1, 0, ProjectionMatrix);
   id = glGetUniformLocation(shader[shader_index], "NormalMatrix");
   if (id >= 0)
      glUniformMatrix3fv(id, 1, 0, (float *)NormalMatrix);
}

/*    OpenGL (GLUT) calls this routine to display the scene    */
void display()
{
   //  Erase the window and the depth buffer
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   //  Enable Z-buffering in OpenGL
   glEnable(GL_DEPTH_TEST);
   /* glDepthFunc(GL_LEQUAL);
   glFrontFace(GL_CCW);   
   glEnable(GL_CULL_FACE);  */ 
   //  Undo previous transformations
   glLoadIdentity();

   //  Perspective - set eye position
   double Ex = -2 * dim * Sin(th) * Cos(ph);
   double Ey = +2 * dim * Sin(ph);
   double Ez = +2 * dim * Cos(th) * Cos(ph);
   gluLookAt(Ex, Ey, Ez, 0, 0, 0, 0, Cos(ph), 0);

   //  Flat or smooth shading
   glShadeModel(smooth ? GL_SMOOTH : GL_FLAT);

   //  Translate intensity to color vectors
   float Emission[] = {0.1 * emission, 0.1 * emission, 0.1 * emission, 1.0};
   float Ambient[] = {0.01 * ambient, 0.01 * ambient, 0.01 * ambient, 1.0};
   float Diffuse[] = {0.01 * diffuse, 0.01 * diffuse, 0.01 * diffuse, 1.0};
   float Specular[] = {0.01 * specular, 0.01 * specular, 0.01 * specular, 1.0};
   float Shinyness[] = {16};

   //  Light position
   Position[0] = distance * Cos(zh);
   Position[1] = ylight;
   Position[2] = distance * Sin(zh);
   Position[3] = 1.0;
   //  Draw light position as sphere (still no lighting here)
   glPushMatrix();
   glColor3f(1, 1, 1);
   glTranslated(Position[0], Position[1], Position[2]);
   glutSolidSphere(0.03, 10, 10);
   glPopMatrix();

   //  OpenGL should normalize normal vectors
   glEnable(GL_NORMALIZE);
   //  Enable lighting
   glEnable(GL_LIGHTING);
   //  Location of viewer for specular calculations
   glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
   //  glColor sets ambient and diffuse color materials
   glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
   glEnable(GL_COLOR_MATERIAL);
   //  Enable light 0
   glEnable(GL_LIGHT0);
   //  Set ambient, diffuse, specular components and position of light 0
   glLightfv(GL_LIGHT0, GL_AMBIENT, Ambient);
   glLightfv(GL_LIGHT0, GL_DIFFUSE, Diffuse);
   glLightfv(GL_LIGHT0, GL_SPECULAR, Specular);
   glLightfv(GL_LIGHT0, GL_POSITION, Position);
   
   if(normals){
      glUseProgram(shader[3]);
      SetUniformMatrices(3);
      Hat(0, 0, 0, 1);
   }

   glUseProgram(shader[shader_mode]);
   SetUniformMatrices(shader_mode);
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
      glUniform1f(id, Shinyness[0]);

   // Draw Hat
   Hat(0, 0, 0, 1);

   glUseProgram(shader[0]);
   // Draw axes
   Axes();
   // Write info
   Info();
   //  Render the scene
   glFlush();
   //  Make the rendered scene visible
   glutSwapBuffers();
}

/*
 *  Read text file
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
   //  Toggle shading mode
   else if (ch == 't' || ch == 'T')
      smooth = !smooth;
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


/*
 *  Build Shadow Map
 */
void ShadowMap(void)
{
   double Lmodel[16];  //  Light modelview matrix
   double Lproj[16];   //  Light projection matrix
   double Tproj[16];   //  Texture projection matrix
   double Dim=2.0;     //  Bounding radius of scene
   double Ldist;       //  Distance from light to scene center

   //  Save transforms and modes
   glPushMatrix();
   glPushAttrib(GL_TRANSFORM_BIT|GL_ENABLE_BIT);
   //  No write to color buffer and no smoothing
   glShadeModel(GL_FLAT);
   glColorMask(0,0,0,0);
   // Overcome imprecision
   glEnable(GL_POLYGON_OFFSET_FILL);

   //  Turn off lighting and set light position
   Light(0);

   //  Light distance
   Ldist = sqrt(Position[0]*Position[0] + Position[1]*Position[1] + Position[2]*Position[2]);
   if (Ldist<1.1*Dim) Ldist = 1.1*Dim;

   //  Set perspective view from light position
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective(114.6*atan(Dim/Ldist),1,Ldist-Dim,Ldist+Dim);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   gluLookAt(Position[0],Position[1],Position[2] , 0,0,0 , 0,1,0);
   //  Size viewport to desired dimensions
   glViewport(0,0,shadowdim,shadowdim);

   // Redirect traffic to the frame buffer
   glBindFramebuffer(GL_FRAMEBUFFER,framebuf);

   // Clear the depth buffer
   glClear(GL_DEPTH_BUFFER_BIT);
   // Draw all objects that can cast a shadow
   Scene(0);

   //  Retrieve light projection and modelview matrices
   glGetDoublev(GL_PROJECTION_MATRIX,Lproj);
   glGetDoublev(GL_MODELVIEW_MATRIX,Lmodel);

   // Set up texture matrix for shadow map projection,
   // which will be rolled into the eye linear
   // texture coordinate generation plane equations
   glLoadIdentity();
   glTranslated(0.5,0.5,0.5);
   glScaled(0.5,0.5,0.5);
   glMultMatrixd(Lproj);
   glMultMatrixd(Lmodel);

   // Retrieve result and transpose to get the s, t, r, and q rows for plane equations
   glGetDoublev(GL_MODELVIEW_MATRIX,Tproj);
   Svec[0] = Tproj[0];    Tvec[0] = Tproj[1];    Rvec[0] = Tproj[2];    Qvec[0] = Tproj[3];
   Svec[1] = Tproj[4];    Tvec[1] = Tproj[5];    Rvec[1] = Tproj[6];    Qvec[1] = Tproj[7];
   Svec[2] = Tproj[8];    Tvec[2] = Tproj[9];    Rvec[2] = Tproj[10];   Qvec[2] = Tproj[11];
   Svec[3] = Tproj[12];   Tvec[3] = Tproj[13];   Rvec[3] = Tproj[14];   Qvec[3] = Tproj[15];

   // Restore normal drawing state
   glShadeModel(GL_SMOOTH);
   glColorMask(1,1,1,1);
   glDisable(GL_POLYGON_OFFSET_FILL);
   glPopAttrib();
   glPopMatrix();
   glBindFramebuffer(GL_FRAMEBUFFER,0);

   //  Check if something went wrong
   ErrCheck("ShadowMap");
}

/*
 *
 */
void InitMap()
{
   unsigned int shadowtex; //  Shadow buffer texture id
   int n;

   //  Make sure multi-textures are supported
   glGetIntegerv(GL_MAX_TEXTURE_UNITS,&n);
   if (n<2) Fatal("Multiple textures not supported\n");

   //  Get maximum texture buffer size
   glGetIntegerv(GL_MAX_TEXTURE_SIZE,&shadowdim);
   //  Limit texture size to maximum buffer size
   glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE,&n);
   if (shadowdim>n) shadowdim = n;
   //  Limit texture size to 2048 for performance
   if (shadowdim>2048) shadowdim = 2048;
   if (shadowdim<512) Fatal("Shadow map dimensions too small %d\n",shadowdim);

   //  Do Shadow textures in MultiTexture 1
   glActiveTexture(GL_TEXTURE1);

   //  Allocate and bind shadow texture
   glGenTextures(1,&shadowtex);
   glBindTexture(GL_TEXTURE_2D,shadowtex);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, shadowdim, shadowdim, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

   //  Map single depth value to RGBA (this is called intensity)
   glTexParameteri(GL_TEXTURE_2D,GL_DEPTH_TEXTURE_MODE,GL_INTENSITY);

   //  Set texture mapping to clamp and linear interpolation
   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

   //  Set automatic texture generation mode to Eye Linear
   glTexGeni(GL_S,GL_TEXTURE_GEN_MODE,GL_EYE_LINEAR);
   glTexGeni(GL_T,GL_TEXTURE_GEN_MODE,GL_EYE_LINEAR);
   glTexGeni(GL_R,GL_TEXTURE_GEN_MODE,GL_EYE_LINEAR);
   glTexGeni(GL_Q,GL_TEXTURE_GEN_MODE,GL_EYE_LINEAR);

   // Switch back to default textures
   glActiveTexture(GL_TEXTURE0);

   // Attach shadow texture to frame buffer
   glGenFramebuffers(1,&framebuf);
   glBindFramebuffer(GL_FRAMEBUFFER,framebuf);
   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowtex, 0);
   //  Don't write or read to visible color buffer
   glDrawBuffer(GL_NONE);
   glReadBuffer(GL_NONE);
   //  Make sure this all worked
   if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) Fatal("Error setting up frame buffer\n");
   glBindFramebuffer(GL_FRAMEBUFFER,0);

   //  Check if something went wrong
   ErrCheck("InitMap");

   //  Create shadow map
   ShadowMap();
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
   glutCreateWindow("model viewer");

   // Create vertex array object once window has been generated, before any other openGl call - http://www.opengl-tutorial.org/beginners-tutorials/tutorial-2-the-first-triangle/
   GLuint vao;
   glGenVertexArrays(1, &vao);
   glBindVertexArray(vao);

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
   LoadTexBMP("./textures/489.bmp");
   //  Shaders
   shader[1] = CreateShaderProg("./shaders/pixtex/pixtex.vert", "./shaders/pixtex/pixtex.frag", NULL);
   shader[2] = CreateShaderProg("./shaders/simple/simple.vert", "./shaders/simple/simple.frag", NULL);
   shader[3] = CreateShaderProg("./shaders/normals/normals.vert", "./shaders/normals/normals.frag", "./shaders/normals/normals.geo");

   ErrCheck("init");
   //  Pass control to GLUT so it can interact with the user
   glutMainLoop();
   return 0;
}