CXX=g++

CXXFLAGS = -Wall -g  
LDFLAGS =  

OBJS = main.o

all: a1

a1: $(OBJS)
	$(CXX) $(CXXFLAGS) -o a1 main.cpp $(LDFLAGS) -lreadline -lhistory -ltermcap

clean: 
	rm -rf $(OBJS) a1 
