CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra
LDFLAGS = -ldl

EXEC = RobotWarz
OBJ = RobotBase.o

all: $(EXEC) Robot_Maya.so

$(OBJ): RobotBase.cpp RobotBase.h
	$(CXX) $(CXXFLAGS) -fPIC -c RobotBase.cpp -o $(OBJ)

$(EXEC): Arena.cpp $(OBJ)
	$(CXX) $(CXXFLAGS) Arena.cpp $(OBJ) -o $(EXEC) $(LDFLAGS)

Robot_Maya.so: Robot_Maya.cpp $(OBJ)
	$(CXX) $(CXXFLAGS) -shared -fPIC -o Robot_Maya.so Robot_Maya.cpp $(OBJ)

clean:
	rm -f $(EXEC) *.o *.so