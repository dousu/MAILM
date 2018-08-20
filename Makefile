SRCS = $(addprefix ./Sources/, Semantics.cpp Agent.cpp Knowledge.cpp Rule.cpp Element.cpp Dictionary.cpp XMLreader.cpp IndexFactory.cpp LogBox.cpp MT19937.cpp Semantics.cpp MAILMParameters.cpp)
DEPS = $(patsubst %.cpp,%.d,$(SRCS))
OBJS = $(patsubst %.cpp,%.o,$(SRCS))
LIBS = -lstdc++fs

#auto configuration for the relation of dependencies (-MMD option)
CXXFLAGS = -std=c++17 -MMD

all: ma

%.o: %.cpp
	${CXX} ${CXXFLAGS} -o $@ -c $<

.PHONY: ma test clean

ma: $(OBJS)
	${CXX} ${CXXFLAGS} ./Sources/MAILM_main.cpp ${OBJS} ${LIBS} -o ./exe/mailm.exe
	${CXX} ${CXXFLAGS} ./Sources/KnwTest.cpp ${OBJS} ${LIBS} -o ./exe/knwtest.exe

boost:
	${CXX} ./Sources/boost_version.cpp -o b_ver.exe

test: ma

clean:
	rm -f ./Sources/*.o ./Sources/*.dump ./Sources/*.exe ./Sources/*.log ./Sources/*.rst ./Sources/*.d ./exe/*.exe ./exe/*.d

-include $(DEPS)
