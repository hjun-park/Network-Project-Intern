CC = gcc
CFLAGS = -g -W -Wall -lpthread -lpcap `mysql_config --cflags --libs`
TARGET = ARP_Packet_Capture_Monitoring
OBJS = main.o arp.o buffer.o insert.o

all : $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

clean:
	rm -f *.o
	rm -f $(TARGET)
