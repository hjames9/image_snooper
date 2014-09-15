CXXFLAGS=-Wall -Werror -Weffc++ -g -m64
LDFLAGS=-lpcap -lboost_thread -lboost_system -llog4cxx -lpthread

all:
	$(CXX) $(CXXFLAGS) -o imagespy main.cpp connection.cpp packet.cpp collector.cpp processor.cpp $(LDFLAGS)

clean:
	$(RM) imagespy *.o
