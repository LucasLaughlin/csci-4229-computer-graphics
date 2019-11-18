#  Hello World Examples
EXE=ex1 ex2 ex3 ex4 ex5
all: $(EXE)

# Main target
all: $(EXE)

#  MinGW
ifeq "$(OS)" "Windows_NT"
CFLG=-O3 -Wall
LIBS=-lglut32cu -lglu32 -lopengl32
CLEAN=del *.exe *.o *.a
else
#  OSX
ifeq "$(shell uname)" "Darwin"
CFLG=-O3 -Wall -Wno-deprecated-declarations
LIBS=-framework GLUT -framework OpenGL
CLEAN=rm -f $(EXE) *.o *.a
#  Linux/Unix/Solaris
else
CFLG=-O3 -Wall
LIBS=-lglut -lGLU -lGL -lm
CLEAN=rm -f $(EXE) *.o *.a
endif
endif

#  Generic compile and link
%:%.c
	gcc $(CFLG) -o $@ $^ $(LIBS)

#  Clean
clean:
	$(CLEAN)
