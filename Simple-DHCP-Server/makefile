CC = g++
CFLAGS = -std=c++11 -g -W -Wall -pthread -lpcap `mysql_config --cflags --libs`
TARGET = main_exe
OBJS = main.o rapid.o buffer.o dhcp_server.o interface.o  linked_list.o
CFILE = main.cpp rapid.cpp buffer.cpp dhcp_server.cpp interface.cpp linked_list.cpp

all : $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

main.o : main.cpp
	$(CC) $(CFLAGS) -c -o main.o main.cpp

rapid.o : rapid.cpp
	$(CC) $(CFLAGS) -c -o rapid.o rapid.cpp

buffer.o : buffer.cpp
	$(CC) $(CFLAGS) -c -o buffer.o buffer.cpp

dhcp_server.o : dhcp_server.cpp
	$(CC) $(CFLAGS) -c -o dhcp_server.o dhcp_server.cpp -lpcap

interface.o : interface.cpp
	$(CC) $(CFLAGS) -c -o interface.o interface.cpp

linked_list.o : linked_list.cpp
	$(CC) $(CFLAGS) -c -o linked_list.o linked_list.cpp



clean:
	rm -f *.o
	rm -f $(TARGET)

