#ifndef CSCIx229
#define CSCIx229

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#ifdef USEGLEW
#include <GL/glew.h>
#endif
#define GL_GLEXT_PROTOTYPES
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#define Cos(th) cos(3.1415926/180*(th))
#define Sin(th) sin(3.1415926/180*(th))
#define Acos(x) (acos((x)) * 180 / 3.1415927)
#define Asin(x) (asin((x)) * 180 / 3.1415927)

#ifdef __cplusplus
extern "C" {
#endif


void Fatal(const char* format , ...);
unsigned int LoadTexBMP(const char* file);
void ErrCheck(const char* where);

#ifdef __cplusplus
}
#endif

#endif
