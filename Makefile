EXTRA_INCLUDE := $(shell echo $CPLUS_INCLUDE_PATH | sed 's/:/ -I/g')
EXTRA_LIBS    := $(shell echo $LD_LIBRARY_PATH | sed 's/:/ -L/g')

ROOTCFLAGS	:= $(shell root-config --cflags)
ROOTLIBS	:= $(shell root-config --libs)
CXXFLAGS	:= -Iinclude $(ROOTCFLAGS) $(EXTRA_INCLUDE)
SRCS	:= src/main.cxx src/QA_tools.cxx src/QAServer.cxx
OBJS	:= $(SRCS:.cxx=.o)
EXE	:= run_analysis


all: $(EXE)

$(EXE): $(OBJS)
	$(CXX) -o $@ $^ $(ROOTLIBS) $(EXTRA_LIBS)

%.o: %.cxx
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(EXE)

