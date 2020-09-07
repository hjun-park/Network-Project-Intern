# ARP-Packet-Capture-Monitoring
ARP Packet Capture Monitoring service captures ARP packets and stores them in the database through a buffer(Circular Buffer).
#### blog link ( https://blog.naver.com/tkdldjs35/222011901658 )
### ARP source file 
> arp.c : arp packet capture

> insert.c : Remove from Queue and save to DB

> main.c : Initialize DB, Queue and make 2 threads.

> buffer.c : Circular Queue



## Installation

 1. Linux : CentOS 7
 2. mysql 5.6.48
 3. You have to prepare library (  <pcap.h>, <mysql.h> )

```bash
# yum -y install mysql-devel or yum -y install mysql-community-devel
# yum -y install libpcap-devel
```

## Usage


```bash
1. git clone https://github.com/merassom/ARP-Packet-Capture-Monitoring.git
2. cd ARP-Packet-Capture-Monitoring; cd src
3. Use "sql_db_query.txt" and Create Databses and Table
4. make clean && make
5. ./ARP_Packet_Capture_Monitoring [interface]
```





### DataFlow
<img width="600" alt="image01" src="https://user-images.githubusercontent.com/26227820/85484185-465ddb00-b601-11ea-80f7-9c115965fda3.PNG">



### FlowChart
<img width="600" alt="image02" src="https://user-images.githubusercontent.com/26227820/85484202-4f4eac80-b601-11ea-86ad-c1a646ae71f9.PNG">




### DB Table 
<img width="600" alt="image03" src="https://user-images.githubusercontent.com/26227820/85484097-18789680-b601-11ea-98a6-f41e0ad9e929.png">



### Result
<img width="600" alt="image04" src="https://user-images.githubusercontent.com/26227820/85652437-b3e33780-b6e5-11ea-9eae-b2f91b600062.jpg">




