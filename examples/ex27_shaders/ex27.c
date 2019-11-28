/*
 *  Shaders
 *
 *  Demonstrate shaders:
 *    Simple shaders
 *    Vertex lighting
 *    Procedural textures
 *    Toon shader
 *    Pixel lighting
 *    Texture lookup
 *    Pixel lighting with texture
 *  If OpenGL 4 is supported:
 *    Pixel lighting OpenGL 4 style but using
 *    a HORRIBLE kludge to set attributes
 *
 *  Key bindings:
 *  m/M        Cycle through shaders
 *  x/X        Increase/decrease mandelbrot X-value
 *  y/Y        Increase/decrease mandelbrot Y-value
 *  z/Z        Zoom in/out of mandelbrot
 *  o/O        Cycle through objects
 *  p/P        Toggle between orthogonal & perspective projections
 *  s/S        Start/stop light movement
 *  -/+        Decrease/increase light elevation
 *  a          Toggle axes
 *  arrows     Change view angle
 *  PgDn/PgUp  Zoom in and out
 *  0          Reset view angle
 *  ESC        Exit
 */
#include "CSCIx229.h"
int axes=1;       //  Display axes
int mode=0;       //  Shader mode
int move=1;       //  Move light
int roll=1;       //  Rolling brick texture
int proj=1;       //  Projection type
int obj=1;        //  Object
int th=0;         //  Azimuth of view angle
int ph=0;         //  Elevation of view angle
int fov=55;       //  Field of view (for perspective)
double asp=1;     //  Aspect ratio
double dim=3.0;   //  Size of world
int zh=90;        //  Light azimuth
int suza=0;       //  Object
float Ylight=2;   //  Light elevation
int shader[] = {0,0,0,0,0,0,0,0,0,0,0}; //  Shader programs
char* text[] = {"No Shader","Constant Color","Lighting","Brick","Mandelbrot Set","Mandelbrot Hole","Toon Shader","Pixel Lighting","Textures","Pixel Lighting and Texture","OpenGL 4"};
#ifdef GL_VERSION_4_0
#define MODE 11
#else
#define MODE 10
#endif
float X=0,Y=0,Z=1; //  Mandelbrot X,Y,Z

/*
 *  Draw a cube
 *     at (x,y,z)
 *     dimensions (dx,dy,dz)
 *     rotated th about the y axis
 */
static void Cube(double x,double y,double z,
                 double dx,double dy,double dz,
                 double th)
{
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);
   //  Cube
   //  Front
   glColor3f(1,0,0);
   glBegin(GL_QUADS);
   glNormal3f( 0, 0,+1);
   glTexCoord2f(0,0); glVertex3f(-1,-1,+1);
   glTexCoord2f(1,0); glVertex3f(+1,-1,+1);
   glTexCoord2f(1,1); glVertex3f(+1,+1,+1);
   glTexCoord2f(0,1); glVertex3f(-1,+1,+1);
   glEnd();
   //  Back
   glColor3f(0,0,1);
   glBegin(GL_QUADS);
   glNormal3f( 0, 0,-1);
   glTexCoord2f(0,0); glVertex3f(+1,-1,-1);
   glTexCoord2f(1,0); glVertex3f(-1,-1,-1);
   glTexCoord2f(1,1); glVertex3f(-1,+1,-1);
   glTexCoord2f(0,1); glVertex3f(+1,+1,-1);
   glEnd();
   //  Right
   glColor3f(1,1,0);
   glBegin(GL_QUADS);
   glNormal3f(+1, 0, 0);
   glTexCoord2f(0,0); glVertex3f(+1,-1,+1);
   glTexCoord2f(1,0); glVertex3f(+1,-1,-1);
   glTexCoord2f(1,1); glVertex3f(+1,+1,-1);
   glTexCoord2f(0,1); glVertex3f(+1,+1,+1);
   glEnd();
   //  Left
   glColor3f(0,1,0);
   glBegin(GL_QUADS);
   glNormal3f(-1, 0, 0);
   glTexCoord2f(0,0); glVertex3f(-1,-1,-1);
   glTexCoord2f(1,0); glVertex3f(-1,-1,+1);
   glTexCoord2f(1,1); glVertex3f(-1,+1,+1);
   glTexCoord2f(0,1); glVertex3f(-1,+1,-1);
   glEnd();
   //  Top
   glColor3f(0,1,1);
   glBegin(GL_QUADS);
   glNormal3f( 0,+1, 0);
   glTexCoord2f(0,0); glVertex3f(-1,+1,+1);
   glTexCoord2f(1,0); glVertex3f(+1,+1,+1);
   glTexCoord2f(1,1); glVertex3f(+1,+1,-1);
   glTexCoord2f(0,1); glVertex3f(-1,+1,-1);
   glEnd();
   //  Bottom
   glColor3f(1,0,1);
   glBegin(GL_QUADS);
   glNormal3f( 0,-1, 0);
   glTexCoord2f(0,0); glVertex3f(-1,-1,-1);
   glTexCoord2f(1,0); glVertex3f(+1,-1,-1);
   glTexCoord2f(1,1); glVertex3f(+1,-1,+1);
   glTexCoord2f(0,1); glVertex3f(-1,-1,+1);
   glEnd();
   //  Undo transofrmations
   glPopMatrix();
}

/*
 *  OpenGL (GLUT) calls this routine to display the scene
 */
void display()
{
   const double len=2.0;  //  Length of axes
   //  Light position and colors
   float Emission[]  = {0.0,0.0,0.0,1.0};
   float Ambient[]   = {0.3,0.3,0.3,1.0};
   float Diffuse[]   = {1.0,1.0,1.0,1.0};
   float Specular[]  = {1.0,1.0,1.0,1.0};
   float Position[]  = {2*Cos(zh),Ylight,2*Sin(zh),1.0};
   float Shinyness[] = {16};

   //  Erase the window and the depth buffer
   glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

   //  Enable Z-buffering in OpenGL
   glEnable(GL_DEPTH_TEST);
   //  Undo previous transformations
   glLoadIdentity();
   //  Perspective - set eye position
   if (proj)
   {
      double Ex = -2*dim*Sin(th)*Cos(ph);
      double Ey = +2*dim        *Sin(ph);
      double Ez = +2*dim*Cos(th)*Cos(ph);
      gluLookAt(Ex,Ey,Ez , 0,0,0 , 0,Cos(ph),0);
   }
   //  Orthogonal - set world orientation
   else
   {
      glRotatef(ph,1,0,0);
      glRotatef(th,0,1,0);
   }

   //  Draw light position as sphere (still no lighting here)
   glColor3f(1,1,1);
   glPushMatrix();
   glTranslated(Position[0],Position[1],Position[2]);
   glutSolidSphere(0.03,10,10);
   glPopMatrix();
   //  OpenGL should normalize normal vectors
   glEnable(GL_NORMALIZE);
   //  Enable lighting
   glEnable(GL_LIGHTING);
   //  glColor sets ambient and diffuse color materials
   glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
   glEnable(GL_COLOR_MATERIAL);
   //  Enable light 0
   glEnable(GL_LIGHT0);
   //  Set ambient, diffuse, specular components and position of light 0
   glLightfv(GL_LIGHT0,GL_AMBIENT ,Ambient);
   glLightfv(GL_LIGHT0,GL_DIFFUSE ,Diffuse);
   glLightfv(GL_LIGHT0,GL_SPECULAR,Specular);
   glLightfv(GL_LIGHT0,GL_POSITION,Position);
   //  Set materials
   glMaterialfv(GL_FRONT_AND_BACK,GL_SHININESS,Shinyness);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,Specular);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);

   //
   //  Draw scene
   //
   //  Select shader (0 => no shader)
   glUseProgram(shader[mode]);
   //  For brick shader set "uniform" variables
   if (mode>0)
   {
      int id;
      float time = roll ? 0.001*glutGet(GLUT_ELAPSED_TIME) : 0;
      id = glGetUniformLocation(shader[mode],"Xcenter");
      if (id>=0) glUniform1f(id,X);
      id = glGetUniformLocation(shader[mode],"Ycenter");
      if (id>=0) glUniform1f(id,Y);
      id = glGetUniformLocation(shader[mode],"Zoom");
      if (id>=0) glUniform1f(id,Z);
      id = glGetUniformLocation(shader[mode],"time");
      if (id>=0) glUniform1f(id,time);
#ifdef GL_VERSION_4_0
      if (mode==10)
      {
         //  Steal the modelview and projection matrixes from OpenGL
         //  Proper replacement would be GLM or QMatrix4x4, etc.
         float ViewMatrix[16],ModelViewMatrix[16],ProjectionMatrix[16],NormalMatrix[9];
         glGetFloatv(GL_PROJECTION_MATRIX,ProjectionMatrix);
         glGetFloatv(GL_MODELVIEW_MATRIX,ViewMatrix);
         // Mark Kilgard's original implementation of
         // glutSolidTeapot adds transformations
         // Temporarily reconstruct it here
         if (obj==1)
         {
            glPushMatrix();
            glRotatef(-90,1,0,0);
            glScalef(0.5,0.5,0.5);
            glTranslatef(0,0,-1.5);
            glGetFloatv(GL_MODELVIEW_MATRIX,ModelViewMatrix);
            glPopMatrix();
         }
         // For Suzanne and Cube, the ModelViewMatrix = ViewMatrix
         else
            glGetFloatv(GL_MODELVIEW_MATRIX,ModelViewMatrix);

         // This is a cheat!!!
         // We get away with it because the transforms are just translation, rotation and symmetric scaling
         // We really should compute the inverse-transpose and then take the upper 3x3 submatrix
         // GLM and QMatrix4x4 have methods to do this the right way.
         NormalMatrix[0] = ModelViewMatrix[0];   NormalMatrix[3] = ModelViewMatrix[4];   NormalMatrix[6] = ModelViewMatrix[8];
         NormalMatrix[1] = ModelViewMatrix[1];   NormalMatrix[4] = ModelViewMatrix[5];   NormalMatrix[7] = ModelViewMatrix[9];
         NormalMatrix[2] = ModelViewMatrix[2];   NormalMatrix[5] = ModelViewMatrix[6];   NormalMatrix[8] = ModelViewMatrix[10];
         //  Set matrixes
         id = glGetUniformLocation(shader[mode],"ModelViewMatrix");
         if (id>=0) glUniformMatrix4fv(id,1,0,ModelViewMatrix);
         id = glGetUniformLocation(shader[mode],"ViewMatrix");
         if (id>=0) glUniformMatrix4fv(id,1,0,ViewMatrix);
         id = glGetUniformLocation(shader[mode],"ProjectionMatrix");
         if (id>=0) glUniformMatrix4fv(id,1,0,ProjectionMatrix);
         id = glGetUniformLocation(shader[mode],"NormalMatrix");
         if (id>=0) glUniformMatrix3fv(id,1,0,NormalMatrix);
         //  Set lighting parameters
         id = glGetUniformLocation(shader[mode],"Position");
         if (id>=0) glUniform4fv(id,1,Position);
         id = glGetUniformLocation(shader[mode],"Ambient");
         if (id>=0) glUniform4fv(id,1,Ambient);
         id = glGetUniformLocation(shader[mode],"Diffuse");
         if (id>=0) glUniform4fv(id,1,Diffuse);
         id = glGetUniformLocation(shader[mode],"Specular");
         if (id>=0) glUniform4fv(id,1,Specular);
         //  Set material properties
         id = glGetUniformLocation(shader[mode],"Ks");
         if (id>=0) glUniform4fv(id,1,Specular);
         id = glGetUniformLocation(shader[mode],"Ke");
         if (id>=0) glUniform4fv(id,1,Emission);
         id = glGetUniformLocation(shader[mode],"Shinyness");
         if (id>=0) glUniform1f(id,Shinyness[0]);
      }
#endif
   }
   //  Draw the teapot or cube
   glColor3f(0,1,1);
   glEnable(GL_TEXTURE_2D);
   if (obj==2)
      glCallList(suza);
   else if (obj==1)
      glutSolidTeapot(1.0);
   else
      Cube(0,0,0,1,1,1,0);
   glDisable(GL_TEXTURE_2D);
   //  No shader for what follows
   glUseProgram(0);

   //  Draw axes - no lighting from here on
   glDisable(GL_LIGHTING);
   glColor3f(1,1,1);
   if (axes)
   {
      glBegin(GL_LINES);
      glVertex3d(0.0,0.0,0.0);
      glVertex3d(len,0.0,0.0);
      glVertex3d(0.0,0.0,0.0);
      glVertex3d(0.0,len,0.0);
      glVertex3d(0.0,0.0,0.0);
      glVertex3d(0.0,0.0,len);
      glEnd();
      //  Label axes
      glRasterPos3d(len,0.0,0.0);
      Print("X");
      glRasterPos3d(0.0,len,0.0);
      Print("Y");
      glRasterPos3d(0.0,0.0,len);
      Print("Z");
   }
   //  Display parameters
   glWindowPos2i(5,5);
   Print("Angle=%d,%d  Dim=%.1f Projection=%s Mode=%s",
     th,ph,dim,proj?"Perpective":"Orthogonal",text[mode]);
   if (mode==4)
   {
      glWindowPos2i(5,25);
      Print("X=%f Y=%f Z=%f Mag=%f",X,Y,Z,1/Z);
   }
   //  Render the scene and make it visible
   ErrCheck("display");
   glFlush();
   glutSwapBuffers();
}

/*
 *  GLUT calls this routine when the window is resized
 */
void idle()
{
   //  Elapsed time in seconds
   double t = glutGet(GLUT_ELAPSED_TIME)/1000.0;
   if (move) zh = fmod(90*t,360.0);
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when an arrow key is pressed
 */
void special(int key,int x,int y)
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
   else if (key == GLUT_KEY_PAGE_DOWN)
      dim += 0.1;
   //  PageDown key - decrease dim
   else if (key == GLUT_KEY_PAGE_UP && dim>1)
      dim -= 0.1;
   //  Keep angles to +/-360 degrees
   th %= 360;
   ph %= 360;
   //  Update projection
   Project(proj?fov:0,asp,dim);
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when a key is pressed
 */
void key(unsigned char ch,int x,int y)
{
   //  Exit on ESC
   if (ch == 27)
      exit(0);
   //  Reset view angle
   else if (ch == '0')
      th = ph = 0;
   //  Toggle axes
   else if (ch == 'a' || ch == 'A')
      axes = 1-axes;
   //  Toggle projection type
   else if (ch == 'p' || ch == 'P')
      proj = 1-proj;
   //  Toggle light movement
   else if (ch == 's' || ch == 'S')
      move = 1-move;
   //  Toggle brick movement
   else if (ch == 'b' || ch == 'B')
      roll = 1-roll;
   //  Toggle objects
   else if (ch == 'o' || ch == 'O')
      obj = (obj+1)%3;
   //  Cycle modes
   else if (ch == 'm')
      mode = (mode+1)%MODE;
   else if (ch == 'M')
      mode = (mode+MODE-1)%MODE;
   //  Light elevation
   else if (ch == '+')
      Ylight += 0.1;
   else if (ch == '-')
      Ylight -= 0.1;
   //  Mandelbrot
   else if (ch == 'x')
      X += 0.01*Z;
   else if (ch == 'X')
      X -= 0.01*Z;
   else if (ch == 'y')
      Y += 0.01*Z;
   else if (ch == 'Y')
      Y -= 0.01*Z;
   else if (ch == 'z')
      Z *= 0.9;
   else if (ch == 'Z')
      Z *= 1.1;
   else if (ch == 'r' || ch=='R')
   {
      X = Y = 0;
      Z = 1;
   }
   //  Reproject
   Project(proj?fov:0,asp,dim);
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when the window is resized
 */
void reshape(int width,int height)
{
   //  Ratio of the width to the height of the window
   asp = (height>0) ? (double)width/height : 1;
   //  Set the viewport to the entire window
   glViewport(0,0, width,height);
   //  Set projection
   Project(proj?fov:0,asp,dim);
}

/*
 *  Read text file
 */
char* ReadText(char *file)
{
   int   n;
   char* buffer;
   //  Open file
   FILE* f = fopen(file,"rt");
   if (!f) Fatal("Cannot open text file %s\n",file);
   //  Seek to end to determine size, then rewind
   fseek(f,0,SEEK_END);
   n = ftell(f);
   rewind(f);
   //  Allocate memory for the whole file
   buffer = (char*)malloc(n+1);
   if (!buffer) Fatal("Cannot allocate %d bytes for text file %s\n",n+1,file);
   //  Snarf the file
   if (fread(buffer,n,1,f)!=1) Fatal("Cannot read %d bytes for text file %s\n",n,file);
   buffer[n] = 0;
   //  Close and return
   fclose(f);
   return buffer;
}

/*
 *  Print Shader Log
 */
void PrintShaderLog(int obj,char* file)
{
   int len=0;
   glGetShaderiv(obj,GL_INFO_LOG_LENGTH,&len);
   if (len>1)
   {
      int n=0;
      char* buffer = (char *)malloc(len);
      if (!buffer) Fatal("Cannot allocate %d bytes of text for shader log\n",len);
      glGetShaderInfoLog(obj,len,&n,buffer);
      fprintf(stderr,"%s:\n%s\n",file,buffer);
      free(buffer);
   }
   glGetShaderiv(obj,GL_COMPILE_STATUS,&len);
   if (!len) Fatal("Error compiling %s\n",file);
}

/*
 *  Print Program Log
 */
void PrintProgramLog(int obj)
{
   int len=0;
   glGetProgramiv(obj,GL_INFO_LOG_LENGTH,&len);
   if (len>1)
   {
      int n=0;
      char* buffer = (char *)malloc(len);
      if (!buffer) Fatal("Cannot allocate %d bytes of text for program log\n",len);
      glGetProgramInfoLog(obj,len,&n,buffer);
      fprintf(stderr,"%s\n",buffer);
   }
   glGetProgramiv(obj,GL_LINK_STATUS,&len);
   if (!len) Fatal("Error linking program\n");
}

/*
 *  Create Shader
 */
int CreateShader(GLenum type,char* file)
{
   //  Create the shader
   int shader = glCreateShader(type);
   //  Load source code from file
   char* source = ReadText(file);
   glShaderSource(shader,1,(const char**)&source,NULL);
   free(source);
   //  Compile the shader
   fprintf(stderr,"Compile %s\n",file);
   glCompileShader(shader);
   //  Check for errors
   PrintShaderLog(shader,file);
   //  Return name
   return shader;
}

/*
 *  Create Shader Program
 */
int CreateShaderProg(char* VertFile,char* FragFile)
{
   //  Create program
   int prog = glCreateProgram();
   //  Create and compile vertex shader
   int vert = CreateShader(GL_VERTEX_SHADER  ,VertFile);
   //  Create and compile fragment shader
   int frag = CreateShader(GL_FRAGMENT_SHADER,FragFile);
   //  Attach vertex shader
   glAttachShader(prog,vert);
   //  Attach fragment shader
   glAttachShader(prog,frag);
   //  Link program
   glLinkProgram(prog);
   //  Check for errors
   PrintProgramLog(prog);
   //  Return name
   return prog;
}

/*
 *  Start up GLUT and tell it what to do
 */
int main(int argc,char* argv[])
{
   //  Initialize GLUT
   glutInit(&argc,argv);
   //  Request double buffered, true color window with Z buffering at 600x600
   glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
   glutInitWindowSize(600,600);
   glutCreateWindow("Shaders");
#ifdef USEGLEW
   //  Initialize GLEW
   if (glewInit()!=GLEW_OK) Fatal("Error initializing GLEW\n");
#endif
   //  Set callbacks
   glutDisplayFunc(display);
   glutReshapeFunc(reshape);
   glutSpecialFunc(special);
   glutKeyboardFunc(key);
   glutIdleFunc(idle);
   //  Load object
   suza = LoadOBJ("suzanne.obj");
   //  Load texture
   LoadTexBMP("pi.bmp");
   //  Create Shader Programs
   shader[1] = CreateShaderProg("simple.vert","simple.frag");
   shader[2] = CreateShaderProg("phong.vert","phong.frag");
   shader[3] = CreateShaderProg("model.vert","brick.frag");
   shader[4] = CreateShaderProg("model.vert","mandel.frag");
   shader[5] = CreateShaderProg("model.vert","manhole.frag");
   shader[6] = CreateShaderProg("toon.vert","toon.frag");
   shader[7] = CreateShaderProg("pixlight.vert","pixlight.frag");
   shader[8] = CreateShaderProg("texture.vert","texture.frag");
   shader[9] = CreateShaderProg("pixtex.vert","pixtex.frag");
#ifdef GL_VERSION_4_0
   shader[10] = CreateShaderProg("gl4.vert","gl4.frag");
#endif
   //  Pass control to GLUT so it can interact with the user
   ErrCheck("init");
   glutMainLoop();
   return 0;
}
