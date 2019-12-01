EXE=model_tester

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
#  Linux/Unix/Solaris
else
CFLG=-O3 -Wall
LIBS=-lglut -lGLU -lGL -lm
endif
#  OSX/Linux/Unix/Solaris
CLEAN=rm -f $(EXE) *.o *.a
endif

# Dependencies
model_tester.o: model_tester.c CSCI_helpers/CSCIx229.h
fatal.o: CSCI_helpers/fatal.c CSCI_helpers/CSCIx229.h
loadtexbmp.o: CSCI_helpers/loadtexbmp.c CSCI_helpers/CSCIx229.h
print.o: CSCI_helpers/print.c CSCI_helpers/CSCIx229.h
project.o: CSCI_helpers/project.c CSCI_helpers/CSCIx229.h
errcheck.o: CSCI_helpers/errcheck.c CSCI_helpers/CSCIx229.h
object.o: CSCI_helpers/object.c CSCI_helpers/CSCIx229.h

#  Create archive
CSCIx229.a:fatal.o loadtexbmp.o print.o project.o errcheck.o object.o
	ar -rcs $@ $^

# Compile rules
.c.o:
	gcc -c $(CFLG) $<
.cpp.o:
	g++ -c $(CFLG) $<

#  Link
model_tester:model_tester.o CSCIx229.a
	gcc -O3 -o $@ $^   $(LIBS)

#  Clean
clean:
	$(CLEAN)