SRCS = $(addprefix ./SOURCE/, Semantics.cpp Agent.cpp Knowledge.cpp Rule.cpp Element.cpp Dictionary.cpp XMLreader.cpp IndexFactory.cpp LogBox.cpp MT19937.cpp Semantics.cpp MAILMParameters.cpp)
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
	${CXX} ${CXXFLAGS} ./SOURCE/MAILM_main.cpp ${OBJS} ${LIBS} -o ./SOURCE/mailm.exe
	${CXX} ${CXXFLAGS} ./SOURCE/KnwTest.cpp ${OBJS} ${LIBS} -o ./SOURCE/knwtest.exe

boost:
	${CXX} ./SOURCE/boost_version.cpp -o b_ver.exe

test: ma

clean:
	rm -f ./SOURCE/*.o ./SOURCE/*.dump ./SOURCE/*.exe ./SOURCE/*.log ./SOURCE/*.rst

allclean:
	rm -f ./SOURCE/*.o ./SOURCE/*.dump ./SOURCE/*.exe ./SOURCE/*.log ./SOURCE/*.rst ./SOURCE/*.d

-include $(DEPS)
