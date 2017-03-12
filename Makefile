##############
# Make the project
# By qinjianbin
##############


PROD	:= DEBUG
OPT     := -O0
VERSION := \"2.0_${PROD}\"
TARGETS := txt2bin vanidxgen search querygen mgdidxgen dumpgrp
DEFINES := -DMEM_ACCESS
SRCS    := binio.cpp index.cpp txt2bin.cpp mgdidx.cpp dict.cpp vanidxgen.cpp vanidx.cpp boolean_query_processing.cpp search.cpp boolean_queue.cpp querygen.cpp bq_heap_queue.cpp merge_boolean_queue.cpp mgdidxgen.cpp greedymerge.cpp usage.cpp dumpgrp.cpp
OBJS    := ${SRCS:.cpp=.o} 

CCFLAGS = ${OPT} -Wall -Wno-deprecated -ggdb -D${PROD} ${DEFINES} -DVERSION=${VERSION} 
LDFLAGS = ${OPT} -ggdb
LIBS    = -lcrypto
CC	= g++


.PHONY: all clean distclean 
all:: ${TARGETS} 

mgdidxgen: binio.o index.o mgdidx.o dict.o greedymerge.o mgdidxgen.o bq_heap_queue.o merge_boolean_queue.o
	${CC} ${LDFLAGS} -o $@ $^ ${LIBS} 

querygen: index.o binio.o querygen.o
	${CC} ${LDFLAGS} -o $@ $^ ${LIBS} 

vanidxgen: binio.o  dict.o vanidxgen.o vanidx.o boolean_queue.o bq_heap_queue.o
	${CC} ${LDFLAGS} -o $@ $^ ${LIBS} 

search: bq_heap_queue.o search.o boolean_queue.o boolean_query_processing.o vanidx.o mgdidx.o dict.o binio.o bq_heap_queue.o merge_boolean_queue.o usage.o
	${CC} ${LDFLAGS} -o $@ $^ ${LIBS} 

dumpgrp: dumpgrp.o mgdidx.o dict.o binio.o merge_boolean_queue.o
	${CC} ${LDFLAGS} -o $@ $^ ${LIBS} 

${OBJS}: %.o: %.cpp
	${CC} ${CCFLAGS} -o $@ -c $< 

clean:: 
	-rm -f *~ *.o ${TARGETS}

install::
	mkdir -p ../bin
	cp ${TARGETS} ../bin

installall::
	cp ${TARGETS} /import/adams/1/jqin/snare06/sim/index_merge/bin/
	cp ${TARGETS} /import/adams/1/jqin/snare03/sim/index_merge/bin/
	cp ${TARGETS} /import/adams/1/jqin/snare04/sim/index_merge/bin/ 
	cp ${TARGETS} /import/adams/1/jqin/snare05/sim/index_merge/bin/ 

distclean:: clean
