# Compiler flags
CXX         =g++
CPPFLAGS    +=-Wall
CPPFLAGS    +=-std=c++1y
CPPFLAGS    +=-pedantic

# For Debug
#CPPFLAGS   +=-Wextra
CPPFLAGS    +=-g

# For Optimization
CPPFLAGS    +=-O3

ifeq ($(OS),Windows_NT)
	# For GLFW
	CPPFLAGS    +=-Id:/SDK/glfw-3.1/include
	LIBS        +=-Ld:/SDK/glfw-3.1/src
	LDFLAGS     +=-lglfw3 -lgdi32

	# For OpenGL
	LDFLAGS     +=-lopengl32 -lglu32

	# For GLEW
	CPPFLAGS    +=-Id:/SDK/glew-1.12.0/include
	LIBS        +=-Ld:/SDK/glew-1.12.0/lib
	LDFLAGS     +=-lglew32
else
	# Some may need this - depends on package manager
	CPPFLAGS    +=-I/opt/local/include/
	CPPFLAGS    +=-I/opt/local
	LDFLAGS     +=-L/opt/local/lib

	CPPFLAGS    +=-I/usr/local/include/libpng16
	LDFLAGS     +=-L/usr/local/lib -lpng16

	# Add location for X11 library
	LDFLAGS     +=-L/opt/X11/lib

	# Should be added when using GLFW (http://www.glfw.org/docs/latest/build.html#build_link_xcode)
	LDFLAGS     +=-framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo

	# For GLFW
	LDFLAGS     +=-lglfw3 -lXinerama -lX11 -lXrandr -lXi -lXxf86vm -lXcursor -lpthread

	# For OpenGL
	LDFLAGS     +=-lGL -lGLU -lGLEW
endif

# File objects
SOURCES     =$(wildcard *.cpp) $(wildcard */*.cpp)
OBJECTS     =$(SOURCES:.cpp=.o)
WINOBJECTS  =$(subst /,\,$(OBJECTS))
ifeq ($(OS),Windows_NT)
	EXECUTABLE = test.exe
else
	EXECUTABLE = test
endif

.PHONY: default clean cleanexe

default: cleanexe $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
		$(CXX) $(CPPFLAGS) -o  $(EXECUTABLE) $(OBJECTS) $(LIBS) $(LDFLAGS)

clean: cleanexe
ifeq ($(OS),Windows_NT)
	del $(WINOBJECTS)
else
	rm -rf $(OBJECTS)
endif

cleanexe:
ifeq ($(OS),Windows_NT)
	del $(EXECUTABLE)
else
	rm -rf $(EXECUTABLE)
endif
