# Dwarf compiler
CC = g++
# Torrie OSX
#CC = clang++

CXX = $(CC)

# Dwarf include paths
IDIR = ../include
CFLAGS = -I$(IDIR)  # opencv includes are in /usr/include
# Torrie OSX
#CFLAGS = -I/opt/local/include -I../include

CXXFLAGS = $(CFLAGS)

# Dwarf Library paths
# LDFLAGS = -L/usr/lib64 # opencv libraries are here
# Jack Library paths
LDFLAGS = `pkg-config opencv --cflags --libs`
# Torrie OSX
#LDFLAGS = -L/opt/local/lib


# Dwarf libraries
#LDLIBS = -lopencv_core -lopencv_highgui -lopencv_video -lopencv_imgproc 
# Torrie OSX
LDLIBS = -lopencv_core -lopencv_highgui -lopencv_video -lopencv_videoio -lopencv_imgproc -lopencv_imgcodecs

BINDIR = ../bin

_DEPS = feature_code.h featureDB_ops.h image_control.h segment.h pipeline_control.h
DEPS = $(patsubst %, $(IDIR)/%,$(_DEPS))

# Temporarily removed 2D_imgRecognition.o to avoid "multiple declaration of main" error
OBJ = feature_code.o featureDB_ops.o image_control.o segment.o pipeline_control.o

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

# See comment about OBJ
#2D_imgRecognition: $(OBJ)
#	$(CC) $^ -o $(BINDIR)/$@ $(LDFLAGS) $(LDLIBS)

pipeline_control: $(OBJ)
	$(CC) $^ -o $(BINDIR)/$@ $(LDFLAGS) $(LDLIBS)


clean:
	rm -f *.o *~ 
