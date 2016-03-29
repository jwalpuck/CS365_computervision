# Dwarf compiler
CC = g++
# Torrie OSX
#CC = clang++

CXX = $(CC)

# Dwarf include paths
IDIR = ../../include
CFLAGS = -I$(IDIR) -O2 # opencv includes are in /usr/include
# Torrie OSX
#CFLAGS = -I/opt/local/include -I../../include

CXXFLAGS = $(CFLAGS)

# Dwarf Library paths
# LDFLAGS = -L/usr/lib64 # opencv libraries are here
# Jack Library paths
LDFLAGS = `pkg-config opencv --cflags --libs`
# Torrie OSX
#LDFLAGS = -L/opt/local/lib


# Dwarf libraries
LDLIBS = -lopencv_core -lopencv_highgui -lopencv_video -lopencv_imgproc 
# Torrie OSX
#LDLIBS = -lopencv_core -lopencv_highgui -lopencv_video -lopencv_videoio -lopencv_imgproc -lopencv_imgcodecs

BINDIR = ../../bin

_DEPS = master_pipeline.h
DEPS = $(patsubst %, $(IDIR)/%,$(_DEPS))

OBJ = master_pipeline.o

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) -g

master_pipeline: $(OBJ)
	$(CC) $^ -o $(BINDIR)/$@ $(LDFLAGS) $(LDLIBS) -g


clean:
	rm -f *.o *~ 
