PROGRAM = gsetest
FRAMEWORKS = -framework Carbon -framework IOKit
CXX = g++ -g -arch i386 -arch x86_64
OBJECTS = \
	gsetest.o \
	okFrontPanelDLL.o


.SUFFIXES: .o .cpp

all: $(PROGRAM)

$(OBJECTS): %.o:%.cpp
	$(CXX) -DMACOSX -I../.. -c $<

$(PROGRAM): $(OBJECTS)
	$(CXX) -o $(PROGRAM) $(OBJECTS) $(FRAMEWORKS) 

clean:
	rm -f *.o $(PROGRAM)

