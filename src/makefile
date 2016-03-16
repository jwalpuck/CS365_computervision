# Dwarf compiler
CC = g++

CXX = $(CC)

# Dwarf include paths
IDIR = ../include
CFLAGS = -I$(IDIR)  # opencv includes are in /usr/include
CXXFLAGS = $(CFLAGS)

# Dwarf Library paths
# LDFLAGS = -L/usr/lib64 # opencv libraries are here
# Jack Library paths
LDFLAGS = `pkg-config opencv --cflags --libs`

# Dwarf libraries
LDLIBS = -lopencv_core -lopencv_highgui -lopencv_video -lopencv_imgproc 

BINDIR = ../bin

_DEPS = feature_code.h featureDB_ops.h image_control.h segment.h
DEPS = $(patsubst %, $(IDIR)/%,$(_DEPS))

OBJ = feature_code.o featureDB_ops.o image_control.o 2D_imgRecognition.o segment.o

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

2D_imgRecognition: $(OBJ)
	$(CC) $^ -o $(BINDIR)/$@ $(LDFLAGS) $(LDLIBS)


clean:
	rm -f *.o *~ 
