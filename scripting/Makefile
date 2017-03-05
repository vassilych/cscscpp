CXX       = g++
CXXFLAGS  = -g -std=c++0x -std=c++11
          # -pedantic -Wall -Wc++98-compat
SRC_FILES = main.cpp Constants.cpp Parser.cpp Translation.cpp Variable.cpp \
            Functions.cpp ParserFunction.cpp Utils.cpp UtilsOS.cpp \
            Interpreter.cpp ParsingScript.cpp	 
OBJS      = $(SRC_FILES:%.cpp=%.o)

APP       = cscs
EXEC      = cscs

all: $(EXEC)

$(EXEC): $(OBJS)

cscs: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(APP)

clean:
	rm -f *.o
clean2:
	rm -f *.o $(APP)
