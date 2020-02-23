/*
 *  Thunderbirds
 *
 *  Draw complex objects from vertex list.
 *  Flight simulation demonstration.
 *
 *  Key bindings:
 *  l/L        Toggle lighting on/off
 *  f/F        Toggle flight/static mode
 *  KP 4/6     Roll (static mode)
 *  KP 2/8     Pitch (static mode)
 *  KP 1/3     Yaw (static mode)
 *  <>         Power (static mode)
 *  a          Toggle axes
 *  arrows     Change view angle
 *  PgDn/PgUp  Zoom in and out
 *  0          Reset view angle
 *  ESC        Exit
 */
#include "CSCIx229.h"
#include "F16.h"
int    fly=0;    //  Animated flight
int    axes=0;   //  Display axes
int    move=0;   //  Light movement
int    light=1;  //  Lighting
int    F16[3];   //  F16 display list
int    th=0;     //  Azimuth of view angle
int    ph=0;     //  Elevation of view angle
int    zh=0;     //  Azimuth of light
double Yl=2;     //  Elevation of light
double roll=0;   //  Roll angle
double pitch=0;  //  Pitch angle
double yaw=0;    //  Yaw angle
int    pwr=100;  //  Power setting (%)
int    fov=55;   //  Field of view (for perspective)
double asp=1;    //  Aspect ratio
double dim=3.0;  //  Size of world
int    box=1;    //  Draw sky
int    sky[2];   //  Sky textures
double X  = 0;   //  Location
double Y  = 0;   //  Location
double Z  = 0;   //  Location
double Dx = 1;   //  Direction
double Dy = 0;   //  Direction
double Dz = 0;   //  Direction
double Sx = 1;   //  Sideways
double Sy = 0;   //  Sideways
double Sz = 0;   //  Sideways
double Ux = 1;   //  Up
double Uy = 0;   //  Up
double Uz = 0;   //  Up
double Ox = 0;   //  LookAt
double Oy = 0;   //  LookAt
double Oz = 0;   //  LookAt
double Ex = 1;   //  Eye
double Ey = 1;   //  Eye
double Ez = 1;   //  Eye

/*
 *  Draw Set of Facets
 */
static void Facets(int k)
{
   int i,j;
   glBegin(GL_TRIANGLES);
   for (i=nFacet[k];i<nFacet[k+1];i++)
      for (j=0;j<3;j++)
      {
         glTexCoord2fv(Texture[Facet[i][2][j]]);
         glNormal3fv(Normal[Facet[i][1][j]]);
         glVertex3fv(Vertex[Facet[i][0][j]]);
      }
   glEnd();
}

/*
 *  Compile F16 display list
 */
static void CompileF16(void)
{
   float black[] = {0,0,0,1};
   int tex[2];

   //  Load textures
   tex[0] = LoadTexBMP("F16s.bmp");
   tex[1] = LoadTexBMP("F16t.bmp");

   //  Body list
   F16[0] = glGenLists(1);
   glNewList(F16[0],GL_COMPILE);
   glColor3f(1,1,1);
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D,tex[0]);
   Facets(0);
   glDisable(GL_TEXTURE_2D);
   glEndList();

   //  Engine list
   F16[1] = glGenLists(1);
   glNewList(F16[1],GL_COMPILE);
   glColor3f(1,1,1);
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D,tex[0]);
   Facets(1);
   glDisable(GL_TEXTURE_2D);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,black);
   glEndList();

   //  Canopy list
   F16[2] = glGenLists(1);
   glNewList(F16[2],GL_COMPILE);
   glColor4f(1,1,1,0.33);
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D,tex[1]);
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
   glDepthMask(0);
   Facets(2);
   glDepthMask(1);
   glDisable(GL_BLEND);
   glDisable(GL_TEXTURE_2D);
   glEndList();
}

/*
 *  Draw Flight
 */
static void DrawFlight(double x0,double y0,double z0,
                       double dx,double dy,double dz,
                       double ux,double uy,double uz)
{
   int i,k;
   //  Position of members
   double X[] = {-1.0,+1.0,+0.0,+0.0};
   double Y[] = {-0.3,+0.3,+0.0,+0.0};
   double Z[] = {+0.0,+0.0,-1.5,+1.5};
   //  Unit vector in direction of flght
   double D0 = sqrt(dx*dx+dy*dy+dz*dz);
   double X0 = dx/D0;
   double Y0 = dy/D0;
   double Z0 = dz/D0;
   //  Unit vector in "up" direction
   double D1 = sqrt(ux*ux+uy*uy+uz*uz);
   double X1 = ux/D1;
   double Y1 = uy/D1;
   double Z1 = uz/D1;
   //  Cross product gives the third vector
   double X2 = Y0*Z1-Y1*Z0;
   double Y2 = Z0*X1-Z1*X0;
   double Z2 = X0*Y1-X1*Y0;
   //  Rotation matrix
   double M[16] = {X0,Y0,Z0,0 , X1,Y1,Z1,0 , X2,Y2,Z2,0 , 0,0,0,1};

   //  Save current transforms
   glPushMatrix();
   //  Offset and rotate
   glTranslated(x0,y0,z0);
   glMultMatrixd(M);
   //  k=0  draw body
   //  k=1  draw engine
   //  k=2  draw canopy
   for (k=0;k<3;k++)
      //  Draw 4 F16s
      for (i=0;i<4;i++)
      {
         glPushMatrix();
         glTranslated(X[i],Y[i],Z[i]);
         glRotated(yaw,0,1,0);
         glRotated(pitch,0,0,1);
         glRotated(roll,1,0,0);
         if (k==1)
	 {
            float power[] = {0.01*pwr,0.01*pwr,0.01*pwr,1};
            glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,power);
	 }
         glCallList(F16[k]);
         glPopMatrix();
      }
   //  Undo transformations
   glPopMatrix();
}

/* 
 *  Draw sky box
 */
static void Sky(double D)
{
   glColor3f(1,1,1);
   glEnable(GL_TEXTURE_2D);

   //  Sides
   glBindTexture(GL_TEXTURE_2D,sky[0]);
   glBegin(GL_QUADS);
   glTexCoord2f(0.00,0); glVertex3f(-D,-D,-D);
   glTexCoord2f(0.25,0); glVertex3f(+D,-D,-D);
   glTexCoord2f(0.25,1); glVertex3f(+D,+D,-D);
   glTexCoord2f(0.00,1); glVertex3f(-D,+D,-D);

   glTexCoord2f(0.25,0); glVertex3f(+D,-D,-D);
   glTexCoord2f(0.50,0); glVertex3f(+D,-D,+D);
   glTexCoord2f(0.50,1); glVertex3f(+D,+D,+D);
   glTexCoord2f(0.25,1); glVertex3f(+D,+D,-D);

   glTexCoord2f(0.50,0); glVertex3f(+D,-D,+D);
   glTexCoord2f(0.75,0); glVertex3f(-D,-D,+D);
   glTexCoord2f(0.75,1); glVertex3f(-D,+D,+D);
   glTexCoord2f(0.50,1); glVertex3f(+D,+D,+D);

   glTexCoord2f(0.75,0); glVertex3f(-D,-D,+D);
   glTexCoord2f(1.00,0); glVertex3f(-D,-D,-D);
   glTexCoord2f(1.00,1); glVertex3f(-D,+D,-D);
   glTexCoord2f(0.75,1); glVertex3f(-D,+D,+D);
   glEnd();

   //  Top and bottom
   glBindTexture(GL_TEXTURE_2D,sky[1]);
   glBegin(GL_QUADS);
   glTexCoord2f(0.0,0); glVertex3f(+D,+D,-D);
   glTexCoord2f(0.5,0); glVertex3f(+D,+D,+D);
   glTexCoord2f(0.5,1); glVertex3f(-D,+D,+D);
   glTexCoord2f(0.0,1); glVertex3f(-D,+D,-D);

   glTexCoord2f(1.0,1); glVertex3f(-D,-D,+D);
   glTexCoord2f(0.5,1); glVertex3f(+D,-D,+D);
   glTexCoord2f(0.5,0); glVertex3f(+D,-D,-D);
   glTexCoord2f(1.0,0); glVertex3f(-D,-D,-D);
   glEnd();

   glDisable(GL_TEXTURE_2D);
}

/*
 *  Draw a ball
 *     at (x,y,z)
 *     radius r
 */
static void ball(double x,double y,double z,double r)
{
   //  Save transformation
   glPushMatrix();
   //  Offset, scale and rotate
   glTranslated(x,y,z);
   glScaled(r,r,r);
   //  White ball
   glColor3f(1,1,0);
   glutSolidSphere(1.0,16,16);
   //  Undo transofrmations
   glPopMatrix();
}

/*
 *  OpenGL (GLUT) calls this routine to display the scene
 */
void display()
{
   const double len=2.5;  //  Length of axes
   //  Erase the window and the depth buffer
   glClearColor(0,0.3,0.7,0);
   glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
   //  Enable Z-buffering in OpenGL
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_CULL_FACE);

   //  Undo previous transformations
   glLoadIdentity();
   //  Perspective - set eye position
   gluLookAt(Ex,Ey,Ez , Ox,Oy,Oz , Ux,Uy,Uz);

   //  Draw sky
   if (box) Sky(3.5*dim);

   //  Light switch
   if (light)
   {
      //  Translate intensity to color vectors
      float F = (light==2) ? 1 : 0.3;
      float Ambient[]   = {0.3*F,0.3*F,0.3*F,1};
      float Diffuse[]   = {0.5*F,0.5*F,0.5*F,1};
      float Specular[]  = {1.0*F,1.0*F,1.0*F,1};
      float white[]     = {1,1,1,1};
      //  Light direction
      float Position[]  = {5*Cos(zh),Yl,5*Sin(zh),1};
      //  Draw light position as ball (still no lighting here)
      ball(Position[0],Position[1],Position[2] , 0.1);
      //  Enable lighting with normalization
      glEnable(GL_LIGHTING);
      glEnable(GL_NORMALIZE);
      //  glColor sets ambient and diffuse color materials
      glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
      glEnable(GL_COLOR_MATERIAL);
      //  Enable light 0
      glEnable(GL_LIGHT0);
      glLightfv(GL_LIGHT0,GL_AMBIENT ,Ambient);
      glLightfv(GL_LIGHT0,GL_DIFFUSE ,Diffuse);
      glLightfv(GL_LIGHT0,GL_SPECULAR,Specular);
      glLightfv(GL_LIGHT0,GL_POSITION,Position);
      glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,32.0f);
      glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   }
   else
      glDisable(GL_LIGHTING);

   //  Draw flight of F16s
   DrawFlight(X,Y,Z , Dx,Dy,Dz , Ux,Uy,Uz);

   //  Draw axes
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
   Print("Angle=%d,%d  Dim=%.1f  Roll %.1f Pitch %.1f Yaw %.1f",th,ph,dim,roll,pitch,yaw);
   if (light) Print(" Light=%d,%.1f",zh,Yl);
   //  Render the scene and make it visible
   ErrCheck("display");
   glFlush();
   glutSwapBuffers();
}

/*
 *  GLUT calls this routine every 50ms
 */
void timer(int toggle)
{
   //  Toggle movement
   if (toggle>0)
      move = !move;
   //  Increment light position
   else
      zh = (zh+5)%360;
   //  Animate flight using Lorenz transform
   if (fly)
   {
      //  Lorenz integration parameters
      double dt = 0.003;
      double s = -1.7;
      double b = 2.66;
      double r = 50;
      //  Old vectors
      double D,Nx,Ny,Nz;
      double Dx0 = Dx;
      double Dy0 = Dy;
      double Dz0 = Dz;
      double Ux0 = Ux;
      double Uy0 = Uy;
      double Uz0 = Uz;
      //  Fix degenerate case
      if (X==0 && Y==0 && Z==0) Y = Z = 40;
      //  Update position
      Dx = s*(Y-X);
      Dy = X*(r-Z)-Y;
      Dz = X*Y - b*Z;
      X += dt*Dx;
      Y += dt*Dy;
      Z += dt*Dz;
      //  Normalize DX
      D = sqrt(Dx*Dx+Dy*Dy+Dz*Dz);
      Dx /= D;
      Dy /= D;
      Dz /= D;
      //  Calculate sideways
      Sx  = Dy0*Dz-Dz0*Dy;
      Sy  = Dz0*Dx-Dx0*Dz;
      Sz  = Dx0*Dy-Dy0*Dx;
      //  Calculate Up
      Ux  = Dz*Sy - Dy*Sz;
      Uy  = Dx*Sz - Dz*Sx;
      Uz  = Dy*Sx - Dx*Sy;
      //  Normalize Up
      D = sqrt(Ux*Ux+Uy*Uy+Uz*Uz);
      Ux /= D;
      Uy /= D;
      Uz /= D;
      //  Eye and lookat position
      Ex = X-7*Dx;
      Ey = Y-7*Dy;
      Ez = Z-7*Dz;
      Ox = X;
      Oy = Y;
      Oz = Z;
      //  Next DX
      Nx = s*(Y-X);
      Ny = X*(r-Z)-Y;
      Nz = X*Y - b*Z;
      //  Pitch angle
      pitch = 180*acos(Dx*Dx0+Dy*Dy0+Dz*Dz0);
      //  Roll angle
      D = (Ux*Ux0+Uy*Uy0+Uz*Uz0) / (Dx*Dx0+Dy*Dy0+Dz*Dz0);
      if (D>1) D = 1;
      roll = (Nx*Sx+Ny*Sy+Nz*Sz>0?+1:-1)*960*acos(D);
      //  Yaw angle
      yaw = 0;
      //  Power setting (0-1)
      if (Dy>0.8)
         pwr = 100;
      else if (Dy>-0.2)
	 pwr = 20+100*Dy;
      else
	 pwr = 0;
   }
   //  Static Roll/Pitch/Yaw
   else
   {
      Ex = -2*dim*Sin(th)*Cos(ph);
      Ey = +2*dim        *Sin(ph);
      Ez = +2*dim*Cos(th)*Cos(ph);
      Ox = Oy = Oz = 0;
      X = Y = Z = 0;
      Dx = 1; Dy = 0; Dz = 0;
      Ux = 0; Uy = 1; Uz = 0;
   }
   //  Set timer to go again
   if (move && toggle>=0) glutTimerFunc(50,timer,0);
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
   Project(fov,asp,dim);
   //  Update state
   timer(-1);
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
      roll = pitch = yaw = th = ph = 0;
   //  Toggle axes
   else if (ch == 'a' || ch == 'A')
      axes = 1-axes;
   //  Fly
   else if (ch == 'f' || ch == 'F')
      dim = (fly = !fly) ? 30 : 3;
   //  Toggle skybox
   else if (ch == 'b' || ch == 'B')
      box = 1-box;
   //  Cycle light
   else if (ch == 'l')
      light = (light+1)%3;
   else if (ch == 'L')
      light = (light+2)%3;
   //  Toggle light movement
   else if (ch == 's' || ch == 'S')
      timer(1);
   //  Roll
   else if (ch == '4')
      roll -= 10;
   else if (ch == '6')
      roll += 10;
   //  Pitch
   else if (ch == '8')
      pitch -= 1;
   else if (ch == '2')
      pitch += 1;
   //  Yaw
   else if (ch == '1')
      yaw -= 1;
   else if (ch == '3')
      yaw += 1;
   //  Power
   else if (ch=='<' && pwr>0)
      pwr -= 10;
   else if (ch=='>' && pwr<100)
      pwr += 10;
   //  Light azimuth
   else if (ch=='[')
      zh -= 1;
   else if (ch==']')
      zh += 1;
   //  Light elevation
   else if (ch=='-')
      Yl -= 0.1;
   else if (ch=='+')
      Yl += 0.1;
   //  Reproject
   Project(fov,asp,dim);
   //  Update state
   timer(-1);
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
   Project(fov,asp,dim);
}

/*
 *  Start up GLUT and tell it what to do
 */
int main(int argc,char* argv[])
{
   //  Initialize GLUT
   glutInit(&argc,argv);
   //  Request double buffered, true color window with Z buffering at 600x600
   glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
   glutInitWindowSize(600,600);
   glutCreateWindow("F16 Thunderbirds");
   //  Set callbacks
   glutDisplayFunc(display);
   glutReshapeFunc(reshape);
   glutSpecialFunc(special);
   glutKeyboardFunc(key);
   //  Load and Compile G16
   CompileF16();
   //  Load skybox texture
   sky[0] = LoadTexBMP("sky0.bmp");
   sky[1] = LoadTexBMP("sky1.bmp");
   //  Set timer
   timer(1);
   //  Pass control to GLUT so it can interact with the user
   ErrCheck("init");
   glutMainLoop();
   return 0;
}
