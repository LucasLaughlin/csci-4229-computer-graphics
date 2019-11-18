/*
 *  hw3
 *
 *  Lucas Laughlin
 *
 *  Key bindings:
 *  m/M        Cycle through different sets of objects
 *  a          Toggle axes
 *  arrows     Change view angle
 *  0          Reset view angle
 *  ESC        Exit
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
//  OpenGL with prototypes for glext
#define GL_GLEXT_PROTOTYPES
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

int th = 30;       //  Azimuth of view angle
int ph = 30;       //  Elevation of view angle
int fov = 55;      //  Field of view
double asp = 1;    //  Aspect ratio
double dim = 3.0;  //  Size of world
int view_mode = 2; //perspective mode, 0=orthogonal, 1=perspective, 2=first person
char view_mode_name[13];

/*First Person variables*/
float Exfp = 0.;
float Eyfp = -3.;
float Ezfp = -3.0;
float lastx = 0;
float lasty = 0;

float pitch = 45.0f; //xrot - rotation around x axis - up and down
float yaw = 0.0f;   //yrot - rotation aroun dy axis - left and right
float angle = 0.0;
float yawrotrad = 0.;
float pitchrotrad = 0.;

int axes = 1;                  //  Display axes
int mode = 2;                  //  What to display
double plane_array[20][20][3]; //shape array coordinates
double xOffset = 0;            //offset to center shape array
double zOffset = 0;            //offset to center shape array
double t = 0;                  //time
double amp = 0.5;              //amplitude of wave functions
double period = 2;             //period modifier of wave functions
int waveMode = 0;              //active wave function

//  Cosine and Sine in degrees
#define Cos(x) (cos((x)*3.1415927 / 180))
#define Sin(x) (sin((x)*3.1415927 / 180))
#define Acos(x) (acos((x)) * 180 / 3.1415927)
#define Asin(x) (asin((x)) * 180 / 3.1415927)

/*
 *  Convenience routine to output raster text
 *  Use VARARGS to make this more flexible
 */
#define LEN 8192 //  Maximum length of text string
void Print(const char *format, ...)
{
   char buf[LEN];
   char *ch = buf;
   va_list args;
   //  Turn the parameters into a character string
   va_start(args, format);
   vsnprintf(buf, LEN, format, args);
   va_end(args);
   //  Display the characters one at a time at the current raster position
   while (*ch)
      glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *ch++);
}

/*
 *  Draw a cube
 *     at (x,y,z)
 *     dimensions (dx,dy,dz)
 *     rotated th about the y axis
 */
static void cube(double x, double y, double z,
                 double dx, double dy, double dz,
                 double th)
{
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x, y, z);
   glRotated(th, 0, 1, 0);
   glScaled(dx, dy, dz);
   //  Cube
   glBegin(GL_QUADS);
   //  Front
   glColor3f(1, 0, 0);
   glVertex3f(-1, -1, 1);
   glVertex3f(+1, -1, 1);
   glVertex3f(+1, +1, 1);
   glVertex3f(-1, +1, 1);
   //  Back
   glColor3f(0, 0, 1);
   glVertex3f(+1, -1, -1);
   glVertex3f(-1, -1, -1);
   glVertex3f(-1, +1, -1);
   glVertex3f(+1, +1, -1);
   //  Right
   glColor3f(1, 1, 0);
   glVertex3f(+1, -1, +1);
   glVertex3f(+1, -1, -1);
   glVertex3f(+1, +1, -1);
   glVertex3f(+1, +1, +1);
   //  Left
   glColor3f(0, 1, 0);
   glVertex3f(-1, -1, -1);
   glVertex3f(-1, -1, +1);
   glVertex3f(-1, +1, +1);
   glVertex3f(-1, +1, -1);
   //  Top
   glColor3f(0, 1, 1);
   glVertex3f(-1, +1, +1);
   glVertex3f(+1, +1, +1);
   glVertex3f(+1, +1, -1);
   glVertex3f(-1, +1, -1);
   //  Bottom
   glColor3f(1, 0, 1);
   glVertex3f(-1, -1, -1);
   glVertex3f(+1, -1, -1);
   glVertex3f(+1, -1, +1);
   glVertex3f(-1, -1, +1);
   //  End
   glEnd();
   //  Undo transformations
   glPopMatrix();
}

/*
 *  Draw vertex in polar coordinates
 */
static void Vertex(double th, double ph)
{
   glColor3f(Cos(th) * Cos(th), Sin(ph) * Sin(ph), Sin(th) * Sin(th));
   glVertex3d(Sin(th) * Cos(ph), Sin(ph), Cos(th) * Cos(ph));
}

/*
 *  Draw a sphere (version 2)
 *     at (x,y,z)
 *     radius (r)
 */
static void sphere(double x, double y, double z, double r)
{
   const int d = 15;
   int th, ph;

   //  Save transformation
   glPushMatrix();
   //  Offset and scale
   glTranslated(x, y, z);
   glScaled(r, r, r);

   //  Latitude bands
   for (ph = -90; ph < 90; ph += d)
   {
      glBegin(GL_QUAD_STRIP);
      for (th = 0; th <= 360; th += d)
      {
         Vertex(th, ph);
         Vertex(th, ph + d);
      }
      glEnd();
   }

   //  Undo transformations
   glPopMatrix();
}

static double wavetype(double x, double z)
{
   switch (waveMode)
   {
   case 0:
      return amp * cos(sqrt(pow(x, 2) + pow(z, 2)) * period - t);
      break;
   case 1:
      return amp * sin(x * period + t);
      break;
   case 2:
      return amp * cos(z * period + t);
      break;
   case 3:
      return amp * sin(x * period + t) + amp * sin(z * period + t);
      break;
   case 4:
      return amp * sin(x * period + t) + amp * cos(z * period + t);
      break;
   case 5:
      return amp * tan(x * period + t);
      break;
   }
   return 0;
}

static void renderMetaShape(int metaMode)
{
   switch (metaMode)
   {
   // plane
   case 0:
      xOffset = (sizeof(plane_array) / sizeof(plane_array[0]) + 1) / 2;
      zOffset = (sizeof(plane_array[0]) / sizeof(plane_array[0][0]) + 1) / 2;
      for (int i = 0; i < (sizeof(plane_array) / sizeof(plane_array[0])); i++)
      {
         for (int j = 0; j < sizeof(plane_array[0]) / sizeof(plane_array[0][0]); j++)
         {
            plane_array[i][j][0] = (i - xOffset) * 0.2;
            plane_array[i][j][2] = (j - zOffset) * 0.2;
            plane_array[i][j][1] = wavetype(plane_array[i][j][0], plane_array[i][j][2]);
         }
      }
   }
}

/*
 *  Set projection
 */
static void Project()
{
   //  Tell OpenGL we want to manipulate the projection matrix
   glMatrixMode(GL_PROJECTION);
   //  Undo previous transformations
   glLoadIdentity();
   //  Perspective transformation
   switch (view_mode)
   {
   //  Orthogonal projection
   case 0:
      //glOrtho(x_min, x_max, y_min, y_max, z_min, z_max)
      glOrtho(-asp * dim, +asp * dim, -dim, +dim, -dim, +dim); //asp modifies x to fit, if y is always dim
      break;
   //  Perspective & First Person
   case 1:
   case 2:
      //gluPerspective(fov,aspect,z_near,z_far) z_near: z_far = 1:16 good ratio
      gluPerspective(fov, asp, dim / 4, 4 * dim);
      break;
   }
   //  Switch to manipulating the model matrix
   glMatrixMode(GL_MODELVIEW);
   //  Undo previous transformations
   glLoadIdentity();
}

void camera()
{
   glRotatef(pitch, 1.0, 0.0, 0.0);
   glRotatef(yaw, 0.0, 1.0, 0.0);
   glTranslated(Exfp, Eyfp, Ezfp);
}

/*
 *  OpenGL (GLUT) calls this routine to display the scene
 */
void display()
{
   const double len = 1.5; //  Length of axes
   //  Erase the window and the depth buffer
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   //  Enable Z-buffering in OpenGL
   glEnable(GL_DEPTH_TEST);
   //  Undo previous transformations
   glLoadIdentity();
   switch (view_mode)
   {
   case 0:
      //  Orthogonal - Set view angle
      glRotatef(ph, 1, 0, 0);
      glRotatef(th, 0, 1, 0);
      break;
   case 1:;
      //  Perspective - set eye position
      double Ex = -2 * dim * Sin(th) * Cos(ph);
      double Ey = +2 * dim * Sin(ph);
      double Ez = +2 * dim * Cos(th) * Cos(ph);
      gluLookAt(Ex, Ey, Ez, 0, 0, 0, 0, Cos(ph), 0);
      break;
   case 2:;
      camera();
      break;
   }

   //  Decide what to draw
   switch (mode)
   {
   //  Draw sphere
   case 0:
      renderMetaShape(0);
      for (int i = 0; i < (sizeof(plane_array) / sizeof(plane_array[0])); i++)
      {
         for (int j = 0; j < sizeof(plane_array[i]) / sizeof(plane_array[0][0]); j++)
         {
            sphere(plane_array[i][j][0], plane_array[i][j][1], plane_array[i][j][2], 0.05);
         }
      }
      break;
   //  Draw cubes
   case 1:
      renderMetaShape(0);
      for (int i = 0; i < (sizeof(plane_array) / sizeof(plane_array[0])); i++)
      {
         for (int j = 0; j < sizeof(plane_array[i]) / sizeof(plane_array[0][0]); j++)
         {
            cube(plane_array[i][j][0], plane_array[i][j][1], plane_array[i][j][2], 0.05, 0.05, 0.05, 0);
         }
      }
      break;
   //  squares
   case 2:
      renderMetaShape(0);
      glPushMatrix();
      for (int i = 0; i < (sizeof(plane_array) / sizeof(plane_array[0])) - 1; i++)
      {
         glBegin(GL_QUAD_STRIP);
         for (int j = 0; j < (sizeof(plane_array[i]) / sizeof(plane_array[0][0])); j++)
         {
            glColor3f((plane_array[i][j][1] + amp) / (2 * amp), (plane_array[i][j][1] + amp) / (2 * amp), (plane_array[i][j][1] + amp) / (2 * amp));
            glVertex3d(plane_array[i][j][0], plane_array[i][j][1], plane_array[i][j][2]);
            glColor3f((plane_array[i + 1][j][1] + amp) / (2 * amp), (plane_array[i + 1][j][1] + amp) / (2 * amp), (plane_array[i + 1][j][1] + amp) / (2 * amp));
            glVertex3d(plane_array[i + 1][j][0], plane_array[i + 1][j][1], plane_array[i + 1][j][2]);
         }
         glEnd();
      }
      glPopMatrix();
      break;
   }
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
   
   if (view_mode==2){
      glWindowPos2i(5, 75);
      Print("Camera coordiantes: (%.1f, %.1f, %.1f) | pitch: %1.f | yaw: %1.f ", -1*Exfp, -1*Eyfp, Ezfp, -1*pitch, yaw);
   }
   //  Five pixels from the lower left corner of the window
   glWindowPos2i(5, 50);
   //  Print the text string
   switch (view_mode)
   {
   case 0:
      strcpy(view_mode_name, "Orthogonal  ");
      break;
   case 1:
      strcpy(view_mode_name, "Perspective ");
      break;
   case 2:
      strcpy(view_mode_name, "First Person");
      break;
   }
   Print("amplitude=%.1f | period=%.1f | wave mode:%d | object mode:%d", amp, (6.283185) / period, waveMode, mode);
   glWindowPos2i(5, 25);
   Print("Angle=%d,%d | Dim=%.1f | FOV=%d | View Mode=%s", th, ph, dim, fov, view_mode_name);
   //  Render the scene
   glFlush();
   //  Make the rendered scene visible
   glutSwapBuffers();
}

/*
 *  GLUT calls this routine when mouse moves
 */
void mouseMovement(int x, int y)
{
   int dlta_x = x - lastx;
   int dlta_y = y - lasty;
   lastx = x;
   lasty = y;

   pitch += (float)dlta_y;
   yaw += (float)dlta_x;

   while (pitch>360)pitch-=360;
   while (yaw>360)yaw-=360;
   

   //  Update projection
   Project();
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when an arrow key is pressed
 */
void special(int key, int x, int y)
{
   if (view_mode == 2)
   {
      if (key == GLUT_KEY_RIGHT)
      {
         yawrotrad = (yaw / 180 * 3.141592654f);
         Exfp -= (float)(cos(yawrotrad)) *0.2;
         Ezfp -= (float)(sin(yawrotrad)) * 0.2;
      }
      else if (key == GLUT_KEY_LEFT)
      {
         yawrotrad = (yaw / 180 * 3.141592654f);
         Exfp += (float)(cos(yawrotrad)) * 0.2;
         Ezfp += (float)(sin(yawrotrad)) * 0.2;
      }
      else if (key == GLUT_KEY_UP)
      {
         pitchrotrad = (pitch / 180 * 3.141592654f);
         yawrotrad = (yaw / 180 * 3.141592654f);
         Exfp -= (float)sin(yawrotrad);
         Ezfp += (float)cos(yawrotrad);
         Eyfp += (float)sin(pitchrotrad);
      }
      else if (key == GLUT_KEY_DOWN){
         pitchrotrad = (pitch / 180 * 3.141592654f);
         yawrotrad = (yaw / 180 * 3.141592654f);
         Exfp += (float)sin(yawrotrad);
         Ezfp -= (float)cos(yawrotrad);
         Eyfp -= (float)sin(pitchrotrad);
      }
      else if (key == GLUT_KEY_PAGE_UP)
         Eyfp -= 0.5;
      //  PageDown key - decrease dim
      else if (key == GLUT_KEY_PAGE_DOWN && dim > 1)
         Eyfp += 0.5;
   }
   else
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
   }
   th %= 360;
   ph %= 360;
   //  Update projection
   Project();
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when a key is pressed
 */
void key(unsigned char ch, int x, int y)
{

   //  Exit on ESC
   if (ch == 27)
      exit(0);
   //  Reset view angle
   else if (ch == '0')
      th = ph = 0;
   //  Toggle axes
   else if (ch == 'a' || ch == 'A')
      axes = 1 - axes;
   //  Switch object mode
   else if (ch == 'm' || ch == 'M')
      mode = (mode + 1) % 3;
   //switch waveform
   else if (ch == 'q' || ch == 'Q')
      waveMode = (waveMode + 1) % 6;
   //switch View Mode
   else if (ch == 'v' || ch == 'V')
   {
      view_mode = (view_mode + 1) % 3;
   }
   //increase amplitude
   else if (ch == 'w' || ch == 'W')
      amp += 0.1;
   //decrease amplitude
   else if (ch == 's' || ch == 'S')
      amp -= 0.1;
   //increase period
   else if (ch == 'd' || ch == 'D')
      period += 0.1;
   //decrease period
   else if (ch == 'E' || ch == 'e')
      period -= 0.1;
   else if (ch == 'r' || ch == 'R')
      fov--;
   else if (ch == 'f' || ch == 'F')
      fov++;

   //  Reproject
   Project();
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when the window is resized
 */
void reshape(int width, int height)
{
   //  Ratio of the width to the height of the window
   asp = (height > 0) ? (double)width / height : 1;
   //  Set the viewport to the entire window
   glViewport(0, 0, width, height);
   //  Set projection
   Project();
   glutPostRedisplay();
}

/*
 *  GLUT calls this toutine when there is nothing else to do
 */
void idle()
{
   t = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
   Project();
   glutPostRedisplay();
}

/*
 *  Start up GLUT and tell it what to do
 */
int main(int argc, char *argv[])
{
   //  Initialize GLUT and process user parameters
   glutInit(&argc, argv);
   //  Request double buffered, true color window with Z buffering at 600x600
   glutInitWindowSize(600, 600);
   glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
   //  Create the window
   glutCreateWindow("Lucas Laughlin - Homework 5");
   //  Tell GLUT to call "idle" when there is nothing else to do
   glutIdleFunc(idle);
   //  Tell GLUT to call "display" when the scene should be drawn
   glutDisplayFunc(display);
   //  Tell GLUT to call "reshape" when the window is resized
   glutReshapeFunc(reshape);
   //  Tell GLUT to call "special" when an arrow key is pressed
   glutSpecialFunc(special);
   // Tell Glut wha tot call when mouse moves
   glutPassiveMotionFunc(mouseMovement);
   //  Tell GLUT to call "key" when a key is pressed
   glutKeyboardFunc(key);
   //  Pass control to GLUT so it can interact with the user
   glutMainLoop();
   return 0;
}
