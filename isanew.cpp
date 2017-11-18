#define _XOPEN_SOURCE
#include <sys/types.h>
#include <sys/socket.h>
#include <string>
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <linux/errqueue.h>
#include <time.h>
#include <fcntl.h>
#include <algorithm>
#include <unistd.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#include <signal.h>

struct sigaction act;

void on_alarm(int signum)
{
    printf("TITTENALARM\n");
   
        alarm(5);  // Reschedule alarm
}

uint16_t checksum(void *data, size_t length)
{
	uint32_t sum = 0;
	while(length>1)
	{
		sum += *((uint16_t*)data);
		data += 2;
		length-=2;
		/*if(sum & 0x80000000)
			sum = (sum & 0xFFFF) + (sum >> 16);*/
	}
	if(length)
		sum += *((uint8_t*)data);
	while(sum & 0xFFFF0000)
		sum = (sum & 0xFFFF) + (sum >> 16);
	return (~sum);
}

int main(int argc, char** argv)
{
int datasize = 64;
int sequence=1;
int lenght;
int sockfd;
int okpackets=0;
int allpackets=0;
  socklen_t size;
char buffer[1024];
int s;
int rv;
char IP_string_clike[] = "172.217.23.238";
std::string IP_string (""); //172.217.23.238  2a00:1450:400d:802::1000
std::string str1 (":");
std::string str2 (".");
timespec time1, time2;
int rc;
fd_set mojSet;
char ip6[50];
iphdr *ip;
float rtt_ms;
 unsigned int ttl = 255;
icmphdr *icmp, *icmpRecv;
unsigned short int pid = getpid();
sockaddr_in sendSockAddr, receiveSockAddr;
hostent *host; 
host = gethostbyname(IP_string_clike);

struct sockaddr_in ip4addr;
struct sockaddr_in6 ip6addr;

int ipv_flag=0;  //nastavi sa na 6 a 4 pre respektivne ipv
int on =1;




for (int q=0; q<argc; q++)
{ 
	if (strcmp("-h",argv[q])==0)
	{
		printf("ISA projekt 2017, autor Pavol Debnár\n");
		printf("použite s:\n");
		printf("-h pre pomoc\n");
		printf("-u pre použitie UDP namiesto ICMP, nutne pridať aj -p\n");
		printf("-p <port> špecifikuje číslo portu\n");
		printf("-l <port> špecifikuje číslo poslucháča\n");
		printf("-s <velkosť> veľkosť odosielaných dát");
		printf("-i <interval> interval v ktorom sa budú posielať správy\n");
		printf("-w <sekundy> koľko sekúnd sa čaká po tom, ako sme nič neprijali\n");
		printf("-r <hodnota> hodnota RTT, ktorú keď presiahneme, tak sa reporuje\n");
		printf("-v verbose mód\n");
		printf("-t <sekundy> čas, za ktorý sa vyhodnotí stratovosť\n");
		return 0;
	}

	if (strcmp("-w",argv[q])==0)
	{ }
	

}

struct sigaction act;
act.sa_handler = &on_alarm;
//act.sa_mask = SA_NODEFER;
act.sa_flags = SA_RESTART;  // Restart interrupted system calls
sigaction(SIGALRM, &act, NULL);

alarm(5);



IP_string="172.217.23.238";



  if ((s =
     socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1)
  {
    printf("dojebane\n");;
    return -1;
  }
setsockopt(s, IPPROTO_IP, IP_TTL, (const char *)&ttl, sizeof(ttl));
//setsockopt(sock, SOL_IP, IP_TTL, (const char *)&ttl, sizeof(ttl));
	time_t t;
	srand(time(NULL));
	/*
	 FILE *file = fopen("/dev/urandom", "r");
	 if (file == NULL)
        printf("kokot\n"); //could not open file*/
        printf("size of icmphdr %d\n",sizeof(icmphdr) );

while(1) {
	char icmpbuffer[65000];
	char *p = icmpbuffer;


	sleep(1);
	char str[datasize-41];

	const char alphanum[] =     "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz123456789";
 
    for (int i = 0; i < datasize-41; ++i) {

        str[i] = alphanum[rand() % (sizeof(alphanum) - 1)];

    }
    

   
  


icmp = (icmphdr *) icmpbuffer;
  icmp->type = ICMP_ECHO;
  icmp->code = 0;
  icmp->un.echo.id = pid;
  icmp->checksum=0;
  icmp->un.echo.sequence=sequence;
  p+= sizeof(icmp);

  for (int a =0 ; a!=strlen(str); a++)
  	{

  		*p = str[a];
  		p++;
  		//printf("%s\n", *p);
  	}

  	  printf("%s\n",icmpbuffer );

time1.tv_nsec=0;
  time1.tv_sec=0;


 

sendSockAddr.sin_family = AF_INET;
  sendSockAddr.sin_port = 0;
  memcpy(&(sendSockAddr.sin_addr), host->h_addr, host->h_length);

icmp->checksum = 0;
    icmp->un.echo.sequence = sequence;
    icmp->checksum = checksum((unsigned char *)icmpbuffer, sizeof(icmphdr)+ sizeof(str)-1);
   int check = sendto(s, (char *)icmpbuffer, sizeof(icmphdr)+ sizeof(str)-1, 0, (sockaddr *)&sendSockAddr, sizeof(sockaddr));
//printf("%d\n", check );
   allpackets++;
/*
  clock_gettime(CLOCK_MONOTONIC, &time1);
  int check = send(s , message , strlen(message),0) ;
printf("%d\n", check ); */

  struct timeval timeout;
  timeout.tv_sec = 1;
  timeout.tv_usec = 0;

  FD_ZERO(&mojSet); //vynulujeme štruktúru
  FD_SET(s, &mojSet); //priradíme socket na ktorom chceme čakať

  int timeoutflag=0;

  rc=0;
  rc = select(s+1, &mojSet, NULL, NULL, &timeout);
  printf("%d\n", rc);

  //if (FD_ISSET(s, &mojSet)) {printf("NOICE\n");}
  


  do 
  {
  	lenght = 0;
  	if (rc<=0) {
  		//printf("packet lost\n");
  		break;
  	}

	  if ((lenght = recvfrom(s, buffer, 1024, 0, (sockaddr *)&receiveSockAddr, &size)) == -1)
		{
		  //printf("chyba pri prijimani\n");
		  break;
		}

	ip = (iphdr *) buffer;
	icmpRecv = (icmphdr *) (buffer + ip->ihl * 4);

	if ((icmpRecv->un.echo.id == pid) && (icmpRecv->type == ICMP_ECHOREPLY) && (icmpRecv->un.echo.sequence == sequence))
		{ //printf("sprava prijata\n");
		  okpackets++;
		}

   }
 while (!((icmpRecv->un.echo.id == pid) && (icmpRecv->type == ICMP_ECHOREPLY) && (icmpRecv->un.echo.sequence == sequence)));
 sequence++;
 printf("%d / %d\n",okpackets, allpackets );
}

}


