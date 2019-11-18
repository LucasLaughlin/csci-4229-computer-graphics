/*
 *  Lighting with Simple Shadows
 *
 *  Demonstrate shadows on a flat surface.
 *
 *  Key bindings:
 *  p/P        Toggle between orthogonal/perspective projection
 *  s/S        Start/stop light movement
 *  +/-        Lower/rise light
 *  x          Toggle axes
 *  arrows     Change view angle
 *  PgDn/PgUp  Zoom in and out
 *  0          Reset view angle
 *  ESC        Exit
 */
#include "CSCIx229.h"
int axes=1;       //  Display axes
int move=1;       //  Move light
int mode=0;       //  Shadow mode
int proj=1;       //  Projection type
int th=0;         //  Azimuth of view angle
int ph=0;         //  Elevation of view angle
int fov=55;       //  Field of view (for perspective)
double asp=1;     //  Aspect ratio
double dim=3.0;   //  Size of world
// Light values
int zh        =  90;  // Light azimuth
float Ylight  =   2;  // Elevation of light

#define Dfloor  8
#define Yfloor -2
float N[] = {0, -1, 0}; // Normal vector for the plane
float E[] = {0, Yfloor, 0 }; // Point of the plane

//  Modes
#define MODE 6
char* text[]={"No shadows",
              "Draw flattened scene",
              "Flattened scene with lighting disabled",
              "Flattened scene with color grey color",
              "Blended shadows",
              "Blended shadows with Z-buffer read-only",
              "Blended shadows with stencil buffer",
             };
/*[doc] examples $ vi ex31.c
[doc] examples $ 


 * Multiply the current ModelView-Matrix with a shadow-projetion matrix.
 *
 * L is the position of the light source
 * E is a point within the plane on which the shadow is to be projected.  
 * N is the normal vector of the plane.
 *
 * Everything that is drawn after this call is "squashed" down to the plane.
 */
void ShadowProjection(float L[4], float E[4], float N[4])
{
   float mat[16];
   float e = E[0]*N[0] + E[1]*N[1] + E[2]*N[2];
   float l = L[0]*N[0] + L[1]*N[1] + L[2]*N[2];
   float c = e - l;
   //  Create the matrix.
   mat[0] = N[0]*L[0]+c; mat[4] = N[1]*L[0];   mat[8]  = N[2]*L[0];   mat[12] = -e*L[0];
   mat[1] = N[0]*L[1];   mat[5] = N[1]*L[1]+c; mat[9]  = N[2]*L[1];   mat[13] = -e*L[1];
   mat[2] = N[0]*L[2];   mat[6] = N[1]*L[2];   mat[10] = N[2]*L[2]+c; mat[14] = -e*L[2];
   mat[3] = N[0];        mat[7] = N[1];        mat[11] = N[2];        mat[15] = -l;
   //  Multiply modelview matrix
   glMultMatrixf(mat);
}

/*
 *  Draw scene
 */
void scene()
{
   int i;
   //  Draw two teapots
   for (i=-1;i<=1;i+=2)
   {
      glPushMatrix();
      glTranslated(0.5*i,0.5*i,0);
      glutSolidTeapot(0.5);
      glPopMatrix();
   }
}

/*
 *  OpenGL (GLUT) calls this routine to display the scene
 */
void display()
{
   int i,j;
   const double len=2.0;  //  Length of axes
   //  Light position and colors
   float Ambient[]  = {0.3,0.3,0.3,1.0};
   float Diffuse[]  = {1.0,1.0,1.0,1.0};
   float Position[] = {Cos(zh),Ylight,Sin(zh),1.0};

   //  Erase the window and the depth buffer
   glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);

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
   glLightfv(GL_LIGHT0,GL_POSITION,Position);

   //  Draw floor
   glEnable(GL_TEXTURE_2D);
   glEnable(GL_POLYGON_OFFSET_FILL);
   glPolygonOffset(1,1);
   glColor3f(1,1,1);
   glNormal3f(0,1,0);
   for (j=-Dfloor;j<Dfloor;j++)
   {
      glBegin(GL_QUAD_STRIP);
      for (i=-Dfloor;i<=Dfloor;i++)
      {
         glTexCoord2f(i,j); glVertex3f(i,Yfloor,j);
         glTexCoord2f(i,j+1); glVertex3f(i,Yfloor,j+1);
      }
      glEnd();
   }
   glDisable(GL_POLYGON_OFFSET_FILL);
   glDisable(GL_TEXTURE_2D);

   //  Draw scene
   glColor3f(1,1,0);
   scene();

   //  Save what is glEnabled
   glPushAttrib(GL_ENABLE_BIT);
   //  Draw shadow
   switch (mode)
   {
      //  No shadow
      case 0:
         break;
      //  Draw flattened scene
      case 1:
         glPushMatrix();
         ShadowProjection(Position,E,N);
         scene();
         glPopMatrix();
         break;
      //  Transformation with lighting disabled
      case 2:
         glDisable(GL_LIGHTING);
         //  Draw flattened scene
         glPushMatrix();
         ShadowProjection(Position,E,N);
         scene();
         glPopMatrix();
         break;
      //  Set shadow color
      case 3:
         glDisable(GL_LIGHTING);
         glColor3f(0.3,0.3,0.3);
         //  Draw flattened scene
         glPushMatrix();
         ShadowProjection(Position,E,N);
         scene();
         glPopMatrix();
         break;
      //  Blended shadows
      case 4:
         glDisable(GL_LIGHTING);
         //  Blended color
         glEnable(GL_BLEND);
         glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
         glColor4f(0,0,0,0.4);
         //  Draw flattened scene
         glPushMatrix();
         ShadowProjection(Position,E,N);
         scene();
         glPopMatrix();
         break;
      //  Blended shadows Z-buffer masked
      case 5:
         glDisable(GL_LIGHTING);
         //  Draw blended 
         glEnable(GL_BLEND);
         glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
         glColor4f(0,0,0,0.4);
         //  Make Z-buffer read-only
         glDepthMask(0);
         //  Draw flattened scene
         glPushMatrix();
         ShadowProjection(Position,E,N);
         scene();
         glPopMatrix();
         //  Make Z-buffer read-write
         glDepthMask(1);
         break;
      //  Blended with stencil buffer
      case 6:
         glDisable(GL_LIGHTING);
         //  Enable stencil operations
         glEnable(GL_STENCIL_TEST);

         /*
          *  Step 1:  Set stencil buffer to 1 where there are shadows
          */
         //  Existing value of stencil buffer doesn't matter
         glStencilFunc(GL_ALWAYS,1,0xFFFFFFFF);
         //  Set the value to 1 (REF=1 in StencilFunc)
         //  only if Z-buffer would allow write
         glStencilOp(GL_KEEP,GL_KEEP,GL_REPLACE);
         //  Make Z-buffer and color buffer read-only
         glDepthMask(0);
         glColorMask(0,0,0,0);
         //  Draw flattened scene
         glPushMatrix();
         ShadowProjection(Position,E,N);
         scene();
         glPopMatrix();
         //  Make Z-buffer and color buffer read-write
         glDepthMask(1);
         glColorMask(1,1,1,1);

         /*
          *  Step 2:  Draw shadow masked by stencil buffer
          */
         //  Set the stencil test draw where stencil buffer is > 0
         glStencilFunc(GL_LESS,0,0xFFFFFFFF);
         //  Make the stencil buffer read-only
         glStencilOp(GL_KEEP,GL_KEEP,GL_KEEP);
         //  Enable blending
         glEnable(GL_BLEND);
         glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
         glColor4f(0,0,0,0.5);
         //  Draw the shadow over the entire floor
         glBegin(GL_QUADS);
         glVertex3f(-Dfloor,Yfloor,-Dfloor);
         glVertex3f(+Dfloor,Yfloor,-Dfloor);
         glVertex3f(+Dfloor,Yfloor,+Dfloor);
         glVertex3f(-Dfloor,Yfloor,+Dfloor);
         glEnd();
         break;
      default:
         break;
   }    
   //  Undo glEnables
   glPopAttrib();
   
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
   Print("Angle=%d,%d  Dim=%.1f Projection=%s Light Elevation=%.1f",
     th,ph,dim,proj?"Perpective":"Orthogonal",Ylight);
   glWindowPos2i(5,25);
   Print(text[mode]);
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
   zh = fmod(90*t,360.0);
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
   else if (ch == 'x' || ch == 'X')
      axes = 1-axes;
   //  Toggle light movement
   else if (ch == 's' || ch == 'S')
      move = 1-move;
   //  Toggle projection type
   else if (ch == 'p' || ch == 'P')
      proj = 1-proj;
   //  Light elevation
   else if (ch=='-')
      Ylight -= 0.1;
   else if (ch=='+')
      Ylight += 0.1;
   //  Shadow mode
   else if (ch == 'm' && mode<MODE)
      mode++;
   else if (ch == 'M' && mode>0)
      mode--;
   //  Light angle
   else if (ch == '[')
      zh = (zh+1)%360;
   else if (ch == ']')
      zh = (zh-1)%360;
   //  Reproject
   Project(proj?fov:0,asp,dim);
   //  Set idle function
   glutIdleFunc(move?idle:NULL);
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
 *  Start up GLUT and tell it what to do
 */
int main(int argc,char* argv[])
{
   //  Initialize GLUT
   glutInit(&argc,argv);
   //  Request double buffered, true color window with Z buffering at 600x600
   glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL | GLUT_DOUBLE);
   glutInitWindowSize(600,600);
   glutCreateWindow("Simple Shadows");
   //  Set callbacks
   glutDisplayFunc(display);
   glutReshapeFunc(reshape);
   glutSpecialFunc(special);
   glutKeyboardFunc(key);
   glutIdleFunc(idle);
   //  Floor texture
   LoadTexBMP("water.bmp");
   //  Pass control to GLUT so it can interact with the user
   ErrCheck("init");
   glutMainLoop();
   return 0;
}
