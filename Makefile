SRCS = $(addprefix ./SOURCE/, Semantics.cpp KirbyAgent.cpp MAILMAgent.cpp KnowledgeBase.cpp Rule.cpp Element.cpp Dictionary.cpp XMLreader.cpp XMLreader2.cpp IndexFactory.cpp Prefices.cpp LogBox.cpp MT19937.cpp Semantics.cpp MAILMParameters.cpp Parameters.cpp)
DEPS = $(patsubst %.cpp,%.d,$(SRCS))
OBJS = $(patsubst %.cpp,%.o,$(SRCS))
LIBS = -lboost_serialization -lboost_system -lboost_program_options -lboost_filesystem

CXXFLAGS = -std=c++17 -MMD

all: ma

%.o: %.cpp
	${CXX} ${CXXFLAGS} -o $@ -c $<

.PHONY: ma test clean

ma: $(OBJS)
	${CXX} ${CXXFLAGS} ./SOURCE/MAILM_main.cpp ${OBJS} ${LIBS} -o ./SOURCE/mailm.exe

boost:
	${CXX} ./SOURCE/boost_version.cpp -o b_ver.exe

clean:
	rm -f ./SOURCE/*.o ./SOURCE/*.dump ./SOURCE/*.exe ./SOURCE/*.log ./SOURCE/*.rst

-include $(DEPS)
