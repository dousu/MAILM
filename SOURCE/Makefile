LD = -L/usr/local/lib
ID = -I/usr/local/include
SOURCEDIR = .
LIBS = -lboost_serialization -lboost_system -lboost_program_options
OBJ1 = MAILMAgent.o KnowledgeBase.o Rule.o Element.o Dictionary.o XMLreader.o IndexFactory.o Prefices.o LogBox.o MT19937.o Semantics.o MAILMParameters.o
OBJ2 = MAILMAgent.o KnowledgeBase.o Rule.o Element.o Dictionary.o XMLreader2.o IndexFactory.o Prefices.o LogBox.o MT19937.o Semantics.o MAILMParameters.o
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

.cpp.o:
	${CXX} ${OPT} -c $< ${ID} ${LIBS2}

KirbyAgent.o: KnowledgeBase.o LogBox.o KirbyAgent.h
MAILMAgent.o: KirbyAgent.o MAILMAgent.h
KnowledgeBase.o: ${HDS} Rule.o IndexFactory.o Prefices.o LogBox.o MT19937.o Semantics.o KnowledgeBase.h
Semantics.o: Element.o Semantics.h
Rule.o: Element.o Dictionary.o IndexFactory.o Prefices.o Rule.h
Element.o: Dictionary.o XMLreader.o XMLreader2.o IndexFactory.o Prefices.o Element.h
Dictionary.o: Dictionary.h
XMLreader.o: XMLreader.h
XMLreader2.o: XMLreader.h
IndexFactory.o: IndexFactory.h
Prefices.o: Prefices.h
LogBox.o: LogBox.h
MT19937.o: MT19937.h
Parameters.o: Parameters.h
MAILMParameters.o: Parameters.o MSILMParameters.h
MAILM_main.cpp: Rule.o Element.o MT19937.o Dictionary.o XMLreader.o XMLreader2.o LogBox.o KnowledgeBase.o MAILMAgent.o MAILMParameters.o

clean:
	rm -f ${SOURCEDIR}/*.o ${SOURCEDIR}/*.dump ${SOURCEDIR}/*.exe ${SOURCEDIR}/*.log ${SOURCEDIR}/*.rst

all: ma1 ma2
