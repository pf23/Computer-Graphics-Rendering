INCLUDEDIR=/usr/include/
LIBDIR=/usr/lib

# If you have more source files add them here 
SOURCE= FruitTetris3D.cpp include/InitShader.cpp

# The compiler we are using 
CC= g++

# The flags that will be used to compile the object file.
# If you want to debug your program,
# you can add '-g' on the following line
CFLAGS= -O3 -g -Wall -pedantic -DGL_GLEXT_PROTOTYPES

# The name of the final executable 
EXECUTABLE= FruitTetris3D

# The basic library we are using add the other libraries you want to link
# to your program here 

# Linux (default)
LDFLAGS = -lGL -lglut -lGLEW -lXext -lX11 -lm

# If you have other library files in a different directory add them here 
INCLUDEFLAG= -I. -I$(INCLUDEDIR) -Iinclude/
LIBFLAG= -L$(LIBDIR)

# Don't touch this one if you don't know what you're doing 
OBJECT= $(SOURCE:.cpp=.o)

# Don't touch any of these either if you don't know what you're doing 
all: $(OBJECT) depend
	$(CC) $(CFLAGS) $(INCLUDEFLAG) $(LIBFLAG) $(OBJECT) -o $(EXECUTABLE) $(LDFLAGS)

depend:
	$(CC) -M $(SOURCE) > depend

$(OBJECT):
	$(CC) $(CFLAGS) $(INCLUDEFLAG) -c -o $@ $(@:.o=.cpp)

clean_object:
	rm -f $(OBJECT)

clean:
	rm -f $(OBJECT) depend $(EXECUTABLE)

include depend
