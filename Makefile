LD = -L/usr/local/lib
ID = -I/usr/local/include
SOURCEDIR = ./SOURCE
LIBS = -lboost_serialization -lboost_system -lboost_program_options -lboost_filesystem
OBJ = Semantics.o KirbyAgent.o MAILMAgent.o KnowledgeBase.o Rule.o Element.o Dictionary.o XMLreader.o XMLreader2.o IndexFactory.o Prefices.o LogBox.o MT19937.o Semantics.o MAILMParameters.o Parameters.o
OBJ1 = Semantics.o KirbyAgent.o MAILMAgent.o KnowledgeBase.o Rule.o Element.o Dictionary.o XMLreader.o IndexFactory.o Prefices.o LogBox.o MT19937.o Semantics.o MAILMParameters.o Parameters.o
OBJ2 = Semantics.o KirbyAgent.o MAILMAgent.o KnowledgeBase.o Rule.o Element.o Dictionary.o XMLreader2.o IndexFactory.o Prefices.o LogBox.o MT19937.o Semantics.o MAILMParameters.o Parameters.o
OBJS = $(addprefix ${SOURCEDIR}/, $(OBJ))
OBJS1 = $(addprefix ${SOURCEDIR}/, $(OBJ1))
OBJS2 = $(addprefix ${SOURCEDIR}/, $(OBJ2))
HD = Distance.hpp
HDS = $(addprefix ${SOURCEDIR}/, $(HD))
OPT = --std=c++14 -g -O2
CXX = g++

ma1: ${OBJS1}
	${CXX} ${OPT} ${ID} ${SOURCEDIR}/MAILM_main.cpp ${OBJS1} ${LD} ${LIBS} -o ${SOURCEDIR}/mailm.exe

ma2: ${OBJS2}
	${CXX} ${OPT} ${ID} ${SOURCEDIR}/MAILM_main.cpp ${OBJS2} ${LD} ${LIBS} -o ${SOURCEDIR}/mailm2.exe

$(SOURCEDIR)/%.o: $(SOURCEDIR)/%.cpp
	@[ -d $(SOURCEDIR) ]
	${CXX} ${OPT} ${ID} ${LD} ${LIBS} -o $@ -c $<

boost:
	${CXX} ${ID} ${SOURCEDIR}/boost_version.cpp -o b_ver.exe

$(SOURCEDIR)/KirbyAgent.o: $(SOURCEDIR)/KnowledgeBase.o $(SOURCEDIR)/LogBox.o $(SOURCEDIR)/KirbyAgent.h
$(SOURCEDIR)/MAILMAgent.o: $(SOURCEDIR)/KirbyAgent.o $(SOURCEDIR)/KnowledgeBase.o $(SOURCEDIR)/XMLreader.o $(SOURCEDIR)/XMLreader2.o $(SOURCEDIR)/MAILMAgent.h
$(SOURCEDIR)/KnowledgeBase.o: ${HDS} $(SOURCEDIR)/Rule.o $(SOURCEDIR)/IndexFactory.o $(SOURCEDIR)/Prefices.o $(SOURCEDIR)/LogBox.o $(SOURCEDIR)/MT19937.o $(SOURCEDIR)/Semantics.o $(SOURCEDIR)/KnowledgeBase.h
$(SOURCEDIR)/Semantics.o: $(SOURCEDIR)/Element.o $(SOURCEDIR)/Semantics.h
$(SOURCEDIR)/Rule.o: $(SOURCEDIR)/Element.o $(SOURCEDIR)/Dictionary.o $(SOURCEDIR)/IndexFactory.o $(SOURCEDIR)/Prefices.o $(SOURCEDIR)/Rule.h
$(SOURCEDIR)/Element.o: $(SOURCEDIR)/Dictionary.o $(SOURCEDIR)/XMLreader.o $(SOURCEDIR)/XMLreader2.o $(SOURCEDIR)/IndexFactory.o $(SOURCEDIR)/Prefices.o $(SOURCEDIR)/Element.h
$(SOURCEDIR)/Dictionary.o: $(SOURCEDIR)/Dictionary.h
$(SOURCEDIR)/XMLreader.o: $(SOURCEDIR)/XMLreader.h
$(SOURCEDIR)/XMLreader2.o: $(SOURCEDIR)/XMLreader.h
$(SOURCEDIR)/IndexFactory.o: $(SOURCEDIR)/IndexFactory.h
$(SOURCEDIR)/Prefices.o: $(SOURCEDIR)/Prefices.h
$(SOURCEDIR)/LogBox.o: $(SOURCEDIR)/LogBox.h
$(SOURCEDIR)/MT19937.o: $(SOURCEDIR)/MT19937.h
$(SOURCEDIR)/Parameters.o: $(SOURCEDIR)/Parameters.h
$(SOURCEDIR)/MAILMParameters.o: $(SOURCEDIR)/Parameters.o $(SOURCEDIR)/MAILMParameters.h
$(SOURCEDIR)/MAILM_main.cpp: ${OBJS} $(SOURCEDIR)/MAILM_main.h

clean:
	rm -f ${SOURCEDIR}/*.o ${SOURCEDIR}/*.dump ${SOURCEDIR}/*.exe ${SOURCEDIR}/*.log ${SOURCEDIR}/*.rst

all: ma1 ma2