/*
 *  hw5
 *
 *  Lucas Laughlin
 * 
 * Portions of code were taken from proffesor Willem Schreuder's examples for CSCI 4229
 * 
 */


// TODO: Outsource heklper functions, PorjectL(0) in particular


#include "CSCIx229.h"

int th = 30;       //  Azimuth of view angle
int ph = 30;       //  Elevation of view angle
int fov = 55;      //  Field of view
double asp = 1;    //  Aspect ratio
double dim = 3.0;  //  Size of world
int view_mode = 0; //perspective mode, 0=orthogonal, 1=perspective
char view_mode_name[13];

int axes = 1;                  // Display axes

double t = 0;                  // time
int move = 0;
int light = 1;
int one = 1;       // Unit value
int distance = 5;  // Light distance
int inc = 10;      // Ball increment
int smooth = 1;    // Smooth/Flat shading
int local = 0;     // Local Viewer Model
int emission = 0;  // Emission intensity (%)
int ambient = 30;  // Ambient intensity (%)
int diffuse = 100; // Diffuse intensity (%)
int specular = 0;  // Specular intensity (%)
int shininess = 0; // Shininess (power of two)
float shiny = 1;   // Shininess (value)
int zh = 90;       // Light azimuth
float ylight = 0;  // Elevation of light

double rep=1;
int texMode = 0;
int ntex=0;
unsigned int texture[3]; 

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
 *  Draw vertex in polar coordinates
 */
static void Vertex(double th, double ph)
{
   glNormal3d(Sin(th) * Cos(ph), Cos(th) * Cos(ph), Sin(ph));
   glVertex3d(Sin(th) * Cos(ph), Cos(th) * Cos(ph), Sin(ph));
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
   float yellow[] = {1.0, 1.0, 0.0, 1.0};
   float Emission[] = {0.0, 0.0, 0.01 * emission, 1.0};

   //  Save transformation
   glPushMatrix();
   //  Offset and scale
   glTranslated(x, y, z);
   glScaled(r, r, r);

   glColor3f(1,1,1);
   glMaterialf(GL_FRONT,GL_SHININESS,shiny);
   glMaterialfv(GL_FRONT,GL_SPECULAR,yellow);
   glMaterialfv(GL_FRONT,GL_EMISSION,Emission);

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

/*
 *  Set projection
 */
static void ProjectL()
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
      glOrtho(-asp * dim, asp * dim, -dim, +dim, -dim, +dim); //asp modifies x to fit, if y is always dim
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

static void hat(double x, double y, double z, double r)
{
   //const int d = 15;
   //int th, ph;
   float white[] = {1, 1, 1, 1};
   float black[] = {0, 0, 0, 1};
   glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shiny);
   glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white);
   glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, black);
   glPushMatrix();
   //  Offset and scale
   glTranslated(x, y, z);
   glScaled(r, r, r);

 

   //constants for hat curvature:
   float a = -0.5; //x^4
   //float b = 0.0;  //x^3
   float c = 0.9;  //x^2
   //float d = 0.0;  //x
   float e = 1.5;  //constant

   float step = 0.1;

   int diff = 15;
   for (float r = 0; r <= 3; r += step)
   {
      glBegin(GL_QUAD_STRIP);
      
      int textFix = 0;
      for (int angle = 15; angle <= 375; angle += diff)
      {
         glColor3f(0.54, 0.27, 0.075);
         float xv = ((sqrt(2) * sqrt(0.5)) + r) * Cos(angle);
         float zv = r * Sin(angle);
         float yv = a * pow(sqrt(pow(xv, 2) / 2 + pow(zv, 2)), 4) + c * pow(sqrt(pow(xv, 2) / 2 + pow(zv, 2)), 2) + e;
         float yv_brim = 0.1 * pow(zv, 2) - 0.1;
         float iv, jv, kv, mag;
         if (yv >= yv_brim-1.5)
         {
            iv = -0.5 * pow(xv, 3) -  xv * pow(zv, 2) + 0.9 * xv;
            jv = -1;
            kv = -2 * pow(zv, 3) - zv * pow(xv, 2) + 1.8 * zv;
            mag = sqrt(pow(iv, 2) + pow(jv, 2) + pow(kv, 2));
         }
         else
         {
            iv = 0;
            jv = -1;
            kv = 0.2 * zv;
            mag = sqrt(pow(iv, 2) + pow(jv, 2) + pow(kv, 2));
         }
         glColor3f(0.54, 0.27, 0.075 + yv / 12);
         glNormal3f(iv / mag, jv / mag, kv / mag);
         /* glBegin(GL_LINES);
         glColor3f(1,1,1);
         glVertex3f(xv, fmax(yv, yv_brim), zv);
         glVertex3f(xv-(iv/mag)*0.5, fmax(yv, yv_brim)-(jv / mag)*0.5, zv-(kv / mag)*0.5);
         glEnd(); */
         glColor3f(0.54, 0.27, 0.075);
         glTexCoord2f(0.0,textFix?0.0:rep);
         glVertex3f(xv, fmax(yv, yv_brim), zv);

         float xv_step = ((sqrt(2) * sqrt(0.5)) + (r + step)) * Cos(angle);
         float zv_step = (r + step) * Sin(angle);
         float yv_step = a * pow(sqrt(pow(xv_step, 2) / 2 + pow(zv_step, 2)), 4) + c * pow(sqrt(pow(xv_step, 2) / 2 + pow(zv_step, 2)), 2) + e;
         float yv_step_brim = 0.1 * pow(zv_step, 2) - 0.1;

         if (yv_step >= yv_step_brim-1.5)
         {
            iv = -0.5 * pow(xv_step, 3) - xv_step * pow(zv_step, 2) + 0.9 * xv_step;
            jv = -1;
            kv = -2 * pow(zv_step, 3) -  zv_step * pow(xv_step, 2) + 1.8 * zv_step;
            mag = sqrt(pow(iv, 2) + pow(jv, 2) + pow(kv, 2));
         }
         else
         {
            iv = 0;
            jv = -1;
            kv = 0.2 * zv_step;
            mag = sqrt(pow(iv, 2) + pow(jv, 2) + pow(kv, 2));
         }
         glColor3f(0.54, 0.27, 0.075 + yv_step / 12);
         glNormal3f(iv / mag, jv / mag, kv / mag);
         /* glBegin(GL_LINES);
         glColor3f(1,1,1);
         glVertex3f(xv_step, fmax(yv_step, yv_step_brim), zv_step);
         glVertex3f(xv_step-(iv/mag)*0.5, fmax(yv_step, yv_step_brim)-(jv / mag)*0.5, zv_step-(kv / mag)*0.5);
         glEnd(); */
         glTexCoord2f(rep,textFix?0.0:rep);
         glVertex3f(xv_step, fmax(yv_step, yv_step_brim), zv_step);
         textFix=!textFix;
      }
      glEnd();
   }
   float closure = -0.1;
   for (float r = 0; r <= 3; r += step)
   {
      
      glBegin(GL_QUAD_STRIP);
      
      int textFix = 0;
      for (int angle = 15; angle <= 375; angle += diff)
      {
         glColor3f(0.54, 0.27, 0.075);
         closure = -0.1;
         float xv = ((sqrt(2) * sqrt(0.5)) + r) * Cos(angle);
         float zv = r * Sin(angle);
         float yv = a * pow(sqrt(pow(xv, 2) / 2 + pow(zv, 2)), 4) + c * pow(sqrt(pow(xv, 2) / 2 + pow(zv, 2)), 2) + e + closure;
         float yv_brim = 0.1 * pow(zv, 2) - 0.1 + closure;
         float iv, jv, kv, mag;
         if (yv >= yv_brim-1.5)
         {
            iv = -0.5 * pow(xv, 3) -  xv * pow(zv, 2) + 0.9 * xv;
            jv = -1;
            kv = -2 * pow(zv, 3) - zv * pow(xv, 2) + 1.8 * zv;
            mag = sqrt(pow(iv, 2) + pow(jv, 2) + pow(kv, 2));
         }
         else
         {
            iv = 0;
            jv = -1;
            kv = 0.2 * zv;
            mag = sqrt(pow(iv, 2) + pow(jv, 2) + pow(kv, 2));
         }
         glColor3f(0.54, 0.27, 0.075 + yv / 12);
         glNormal3f(iv / mag, jv / mag, kv / mag);
         glTexCoord2f(0.0,textFix?0.0:rep);
         glVertex3f(xv, fmax(yv, yv_brim), zv);

         if (r>2.9) {
            closure = 0.0;
         }
         float xv_step = ((sqrt(2) * sqrt(0.5)) + (r + step)) * Cos(angle);
         float zv_step = (r + step) * Sin(angle);
         float yv_step = a * pow(sqrt(pow(xv_step, 2) / 2 + pow(zv_step, 2)), 4) + c * pow(sqrt(pow(xv_step, 2) / 2 + pow(zv_step, 2)), 2) + e + closure;
         float yv_step_brim = 0.1 * pow(zv_step, 2) - 0.1 + closure;

         if (yv_step >= yv_step_brim-1.5)
         {
            iv = -0.5 * pow(xv_step, 3) - xv_step * pow(zv_step, 2) + 0.9 * xv_step;
            jv = -1;
            kv = -2 * pow(zv_step, 3) - zv_step * pow(xv_step, 2) + 1.8 * zv_step;
            mag = sqrt(pow(iv, 2) + pow(jv, 2) + pow(kv, 2));
         }
         else
         {
            iv = 0;
            jv = -1;
            kv = 0.2 * zv_step;
            mag = sqrt(pow(iv, 2) + pow(jv, 2) + pow(kv, 2));
         }
         glColor3f(0.54, 0.27, 0.075 + yv_step / 12);
         glNormal3f(iv / mag, jv / mag, kv / mag);
         glTexCoord2f(rep,textFix?0.0:rep);
         glVertex3f(xv_step, fmax(yv_step, yv_step_brim), zv_step);
         textFix=!textFix;
      }
      glEnd();
   }
   //glDisable(GL_TEXTURE_2D);
   glPopMatrix();
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
   glDepthFunc(GL_LESS); 
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
   }
   

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
      if (move==0)
      {
         Position[2]=ylight;
         Position[1]=distance * Sin(zh);
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

   
   
   
   glEnable(GL_TEXTURE_2D);
   glTexEnvi(GL_TEXTURE_ENV , GL_TEXTURE_ENV_MODE , GL_MODULATE/* texMode?GL_REPLACE:GL_MODULATE */);
   /* glBindTexture(GL_TEXTURE_2D,texture[ntex]); */
   hat(0, 0, 0, 0.5);
   //  Switch off textures so it doesn't apply to the rest
   glDisable(GL_TEXTURE_2D);
   
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

   //  Print the text string
   switch (view_mode)
   {
   case 0:
      strcpy(view_mode_name, "Orthogonal  ");
      break;
   case 1:
      strcpy(view_mode_name, "Perspective ");
      break;
   }
   glWindowPos2i(5,50);
      Print("Ambient=%d  Diffuse=%d Specular=%d Emission=%d",ambient,diffuse,specular,emission,shiny);
   glWindowPos2i(5, 25);
   Print("Angle=%d,%d | Dim=%.1f | FOV=%d | View Mode=%s", th, ph, dim, fov, view_mode_name);
   //  Render the scene
   glFlush();
   //  Make the rendered scene visible
   glutSwapBuffers();
}

/*
 *  GLUT calls this routine when an arrow key is pressed
 */
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
   //  Update projection
   ProjectL();
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/*
 *  GLUT calls this toutine when there is nothing else to do
 */
void idle()
{
   t = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
   zh = fmod(45 * t, 360.0);
   ProjectL();
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
   else if (ch == 'x' || ch == 'X')
      axes = 1 - axes;
   else if (ch == 't' || ch == 'T')
      smooth = 1-smooth;
   //  Switch object mode
   else if (ch == 'm' || ch == 'M')
      move = (move+1)%3;
   else if (ch == 'l' || ch == 'L')
      light = 1-light;
   //switch View Mode
   else if (ch == 'v' || ch == 'V')
      view_mode = (view_mode + 1) % 2;
   //increase amplitude
   else if (ch == 'w' || ch == 'W')
      axes=axes;
   //decrease amplitude
  else if (ch=='a' && ambient>0)
      ambient -= 5;
   else if (ch=='A' && ambient<100)
      ambient += 5;
   //  Diffuse level
   else if (ch=='d' && diffuse>0)
      diffuse -= 5;
   else if (ch=='D' && diffuse<100)
      diffuse += 5;
   //  Specular level
   else if (ch=='s' && specular>0)
      specular -= 5;
   else if (ch=='S' && specular<100)
      specular += 5;
   else if (ch=='e' && emission>0)
      emission -= 5;
   else if (ch=='E' && emission<100)
      emission += 5;   
   else if (ch == 'r' || ch == 'R')
      fov--;
   else if (ch == 'f' || ch == 'F')
      fov++;

   //  Reproject
   ProjectL();
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();

   glutIdleFunc((move!=2)?idle:NULL);

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
   ProjectL();
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
   glutCreateWindow("Lucas Laughlin - homework 5");
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
   /* texture[0] =  */LoadTexBMP("489.bmp");
   ErrCheck("init");
   //  Pass control to GLUT so it can interact with the user
   glutMainLoop();
   return 0;
}