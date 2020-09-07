CC=g++
CPPFLAGS=-std=c++11 -lstdc++ -g -W -Wall

TARGET=dnshijack
OBJS=DNSHijack.o Relay.o Buffer.o MysqlDB.o main.o
SRC=$(OBJS:.o=.cpp)

LIBS=-lpcap `mysql_config --cflags --libs`
PTHREAD=-lpthread

# test=test
# UNITTEST=unittest
# OBJS2 = unittest.o packet_handler.o set_attack_info.o 


${TARGET}: ${OBJS}
	${CC} -o ${TARGET} ${OBJS} ${PTHREAD} ${LIBS} ${CPPFLAGS}

${OBJS} : ${SRC} 
	${CC} -c ${CPPFLAGS} ${LIBS} ${SRC}

#${test} : ${OBJS2}
	#${CC} -o ${UNITTEST} ${OBJS2} ${PTHREAD} ${LIBS} ${CPPFLAGS} 
	#./${UNITTEST}


clean:
	rm -f *.o ${TARGET} #${UNITTEST}
