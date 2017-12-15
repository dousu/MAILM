OUTPUT = a.out
LD = -L/usr/local/lib
LIBS = -lboost_serialization -lboost_thread -lboost_system -lboost_program_options -lboost_filesystem
OBJ = KirbyAgent.o KnowledgeBase.o Rule.o Element.o Dictionary.o IndexFactory.o Prefices.o LogBox.o Parameters.o MT19937.o
OBJ3 = MSILMAgent.o KirbyAgent.o KnowledgeBase.o Rule.o Element.o Dictionary.o IndexFactory.o Prefices.o LogBox.o Parameters.o MT19937.o MSILMParameters.o
OBJ41 = KnowledgeBase.o Rule.o Element.o Dictionary.o XMLreader.o IndexFactory.o Prefices.o LogBox.o MT19937.o Semantics.o
OBJ42 = KnowledgeBase.o Rule.o Element.o Dictionary.o XMLreader2.o IndexFactory.o Prefices.o LogBox.o MT19937.o Semantics.o
OBJ43 = KnowledgeBase.o Rule.o Element.o Dictionary.o XMLreader3.o IndexFactory.o Prefices.o LogBox.o MT19937.o Semantics.o
#OBJ5 = Rule.o Element.o Dictionary.o XMLreader.o IndexFactory.o Prefices.o LogBox.o MT19937.o Semantics.o
OBJ5 = Rule.o Element.o Dictionary.o XMLreader2.o IndexFactory.o Prefices.o LogBox.o MT19937.o Semantics.o
HD = Distance.hpp
OPT = --std=c++14 -g -O2
#OPT = -g -O0

ma: ${OBJ41}
	${CXX} ${OPT} MAILM_main.cpp ${OBJ41} ${LD} ${LIBS} -o mailm.exe

ma2: ${OBJ42}
	${CXX} ${OPT} MAILM_main.cpp ${OBJ42} ${LD} ${LIBS} -o mailm2.exe

ma3: ${OBJ43}
	${CXX} ${OPT} MAILM_main.cpp ${OBJ43} ${LD} ${LIBS} -o mailm3.exe

kt: ${OBJ5}
	${CXX} ${OPT} KnowledgeBase.cpp ${OBJ5} ${LD} ${LIBS} -o ktest.exe

.cpp.o:
	${CXX} ${OPT} -c $< ${ID} ${LIBS2}

Kirby_main.o:Parameters.o Kirby_main.h
KirbyAgent.o: KnowledgeBase.o LogBox.o KirbyAgent.h
KnowledgeBase.o: ${HD} Rule.o IndexFactory.o Prefices.o LogBox.o MT19937.o Semantics.o KnowledgeBase.h
Semantics.o: Element.o Semantics.h
Rule.o: Element.o Dictionary.o IndexFactory.o Prefices.o Rule.h
Element.o:Dictionary.o XMLreader.o XMLreader2.o XMLreader3.o IndexFactory.o Prefices.o Element.h
Dictionary.o:Dictionary.h
XMLreader.o:XMLreader.h
XMLreader2.o:XMLreader.h
XMLreader3.o:XMLreader.h
IndexFactory.o:IndexFactory.h
Prefices.o:Prefices.h
LogBox.o:LogBox.h
MT19937.o:MT19937.h
Parameters.o:Parameters.h
MSILMParameters.o:Parameters.o MSILMParameters.h
MSILMAgent.o:KirbyAgent.o MSILMAgent.h
MSILM_main.cpp: MSILMAgent.o Rule.o Element.o MT19937.o Dictionary.o LogBox.o MSILMParameters.o MSILM_main.h
MAILM_main.cpp: Rule.o Element.o MT19937.o Dictionary.o XMLreader.o XMLreader2.o XMLreader3.o LogBox.o KnowledgeBase.o

clean:
	rm -f *.o *.dump *.exe
