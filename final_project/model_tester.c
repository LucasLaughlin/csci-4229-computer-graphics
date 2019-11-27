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
int normals = 0; // Display normal vectors
int axes = 1;    // Display axes
double t = 0;    // Timer

/*    Camera/Eye position    */
int th = 30; //  Azimuth of view angle
int ph = 30; //  Elevation of view angle

/*    Projection model values    */
double asp = 1;   //  Aspect ratio
double dim = 3.0; //  Size of world
int fov = 55;     //  Field of view

/*    Light model values     */
int light = 1;     // Light Switch
int smooth = 1;    // Smooth/Flat shading
int emission = 0;  // Emission intensity (%)
int ambient = 30;  // Ambient intensity (%)
int diffuse = 100; // Diffuse intensity (%)
int specular = 0;  // Specular intensity (%)
int shininess = 0; // Shininess (power of two)
float shiny = 1;   // Shininess (value)

/*    Floating light positioning     */
int zh = 90;      // Light azimuth
float ylight = 0; // Elevation of light
int distance = 5; // Light distanced
int move = 0;     // Light ball movement type

/*    Texture variables for hat    */
double rep = 1;
int texMode = 0;
int ntex = 0;
unsigned int texture[3];

/*    Draw vertex in polar coordinates      */
static void Vertex(double th, double ph)
{
   glNormal3d(Sin(th) * Cos(ph), Cos(th) * Cos(ph), Sin(ph));
   glVertex3d(Sin(th) * Cos(ph), Cos(th) * Cos(ph), Sin(ph));
}

/* Draw a sphere
 *     at (x,y,z)
 *     radius (r)
 */
static void sphere(double x, double y, double z, double r)
{
   const int d = 15;
   int th, ph;
   float yellow[] = {1.0, 1.0, 0.0, 1.0};
   float Emission[] = {0.0, 0.0, 0.01 * emission, 1.0};

   //  Save transformation
   glPushMatrix();
   //  Offset and scale
   glTranslated(x, y, z);
   glScaled(r, r, r);

   glColor3f(1, 1, 1);
   glMaterialf(GL_FRONT, GL_SHININESS, shiny);
   glMaterialfv(GL_FRONT, GL_SPECULAR, yellow);
   glMaterialfv(GL_FRONT, GL_EMISSION, Emission);

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

static void Normal(double xn, double yn, double zn, double xp, double yp, double zp)
{
   glNormal3f(xn, yn, zn);
   if (normals)
   {
      glColor3f(1, 1, 1);
      int mag = sqrt(pow(xn, 2) + pow(yn, 2) + pow(zn, 2));
      
      glVertex3f(xp + xn / mag, yp + yn / mag, zp + zn / mag);
      glVertex3f(xp, yp, zp);
      glVertex3f(xp, yp, zp);
      glVertex3f(xp, yp, zp);
   }
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

   for (float r = 0; r <= 3; r += r_step)
   {
      glBegin(GL_QUAD_STRIP);

      int textFix = 0;
      for (int angle = 0; angle <= 360; angle += angle_step)
      {
         glColor3f(0.54, 0.27, 0.075);
         float xv = ((sqrt(2) * sqrt(0.5)) + r) * Cos(angle);
         float zv = r * Sin(angle);
         float yv = a * pow(sqrt(pow(xv, 2) / 2 + pow(zv, 2)), 4) + c * pow(sqrt(pow(xv, 2) / 2 + pow(zv, 2)), 2) + e;
         float yv_brim = 0.1 * pow(zv, 2) - 0.1;
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
         glColor3f(0.54, 0.27, 0.075 + yv / 12);
         glColor3f(0.54, 0.27, 0.075);
         glTexCoord2f(0.0, textFix ? 0.0 : rep);
         glVertex3f(xv, fmax(yv, yv_brim), zv);
         Normal(iv, jv, kv, xv, yv, zv);

         float xv_step = ((sqrt(2) * sqrt(0.5)) + (r + r_step)) * Cos(angle);
         float zv_step = (r + r_step) * Sin(angle);
         float yv_step = a * pow(sqrt(pow(xv_step, 2) / 2 + pow(zv_step, 2)), 4) + c * pow(sqrt(pow(xv_step, 2) / 2 + pow(zv_step, 2)), 2) + e;
         float yv_step_brim = 0.1 * pow(zv_step, 2) - 0.1;

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
         glColor3f(0.54, 0.27, 0.075 + yv_step / 12);
         
         glTexCoord2f(rep, textFix ? 0.0 : rep);
         glVertex3f(xv_step, fmax(yv_step, yv_step_brim), zv_step);
         Normal(iv, jv, kv, xv, yv, zv);
         textFix = !textFix;
         /* if ((angle/angle_step)%2=0){
            Normal(iv, jv, kv, xv, yv, zv);
         } */
      }
      glEnd();
   }
   float closure = -0.1;
   for (float r = 0; r <= 3; r += r_step)
   {

      glBegin(GL_QUAD_STRIP);

      int textFix = 0;
      for (int angle = 0; angle <= 360; angle += angle_step)
      {
         glColor3f(0.54, 0.27, 0.075);
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
         glColor3f(0.54, 0.27, 0.075 + yv / 12);
         
         glTexCoord2f(0.0, textFix ? 0.0 : rep);
         glVertex3f(xv, fmax(yv, yv_brim), zv);
         Normal(iv, jv, kv, xv, yv, zv);
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
         glColor3f(0.54, 0.27, 0.075 + yv_step / 12);
         
         glTexCoord2f(rep, textFix ? 0.0 : rep);
         glVertex3f(xv_step, fmax(yv_step, yv_step_brim), zv_step);
         textFix = !textFix;
         Normal(iv, jv, kv, xv, yv, zv);
      }
      glEnd();
   }
   glPopMatrix();
   //  Switch off textures so it doesn't apply to the rest
   glDisable(GL_TEXTURE_2D);
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
   Print("Angle=%d,%d | Dim=%.1f | FOV=%d", th, ph, dim, fov);
}

/*    OpenGL (GLUT) calls this routine to display the scene    */
void display()
{
   //  Erase the window and the depth buffer
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   //  Enable Z-buffering in OpenGL
   glEnable(GL_DEPTH_TEST);
   glDepthFunc(GL_LESS);
   //  Undo previous transformations
   glLoadIdentity();
   //  Perspective - set eye position
   double Ex = -2 * dim * Sin(th) * Cos(ph);
   double Ey = +2 * dim * Sin(ph);
   double Ez = +2 * dim * Cos(th) * Cos(ph);
   gluLookAt(Ex, Ey, Ez, 0, 0, 0, 0, Cos(ph), 0);

   //  Flat or smooth shading
   glShadeModel(smooth ? GL_SMOOTH : GL_FLAT);

   //  Light switch
   if (light)
   {
      //  Translate intensity to color vectors
      float Ambient[] = {0.01 * ambient, 0.01 * ambient, 0.01 * ambient, 1.0};
      float Diffuse[] = {0.01 * diffuse, 0.01 * diffuse, 0.01 * diffuse, 1.0};
      float Specular[] = {0.01 * specular, 0.01 * specular, 0.01 * specular, 1.0};
      //  Light position
      glColor3f(1, 1, 1);
      float Position[] = {distance * Cos(zh), ylight, distance * Sin(zh), 1.0};
      if (move == 0)
      {
         Position[2] = ylight;
         Position[1] = distance * Sin(zh);
      }
      sphere(Position[0], Position[1], Position[2], 0.1);

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
   }
   else
      glDisable(GL_LIGHTING);

   // Draw Hat
   Hat(0, 0, 0, 0.5);
   // Draw axes
   Axes();
   // Write info
   Info();
   //  Render the scene
   glFlush();
   //  Make the rendered scene visible
   glutSwapBuffers();
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

/*    GLUT calls this routine when there is nothing else to do    */
void idle()
{
   t = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
   zh = fmod(45 * t, 360.0);
   Project(fov, asp, dim);
   glutPostRedisplay();
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
   //  Light-Switch on/off
   else if (ch == 'l' || ch == 'L')
      light = !light;
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
   /* texture[0] =  */ LoadTexBMP("489.bmp");
   ErrCheck("init");
   //  Pass control to GLUT so it can interact with the user
   glutMainLoop();
   return 0;
}