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
#include <pthread.h>
#include <unistd.h>     /* Symbolic Constants */
#include <sys/types.h>  /* Primitive System Data Types */ 
#include <errno.h>      /* Errors */
#include <stdio.h>      /* Input/Output */
#include <stdlib.h>     /* General Utilities */
#include <pthread.h>    /* POSIX Threads */
#include <string.h>     /* String handling */
#include <sys/time.h>
#include <time.h>
#include <math.h>

#define RESTPACKETSIZE 28 //velkost IP a ICPMHDR



int okpackets=0;
int allpackets=0;
struct sigaction act;
int datasize = 57 -sizeof(timespec) ;
hostent *host;
int s;
unsigned int ttl = 255;
float rtt_ms;
float rtt_new;
float rtt_avg;
float smean_rtt;
float rtt_max;
float rtt_min;
int sendtime=100; 
int verbose = 0;
float rtime;
int rflag=0;
int rpackets;
int ttime=300;
float wtime=2000;




typedef struct str_thdata
{
    int kpackets;
    char message[100];
} thdata;



void on_alarm(int signum)
{
    time_t rawtime;
  struct tm * timeinfo;
  char buffer [80];

  time (&rawtime);
  timeinfo = localtime (&rawtime);

  struct timeval tv;
  int millisec;

  gettimeofday(&tv, NULL);
  millisec = lrint(tv.tv_usec/10000.0);

  strftime (buffer,80,"%F %T",timeinfo);
  float rtt_avg_vypis;
  float smean_rtt_vypis;

   

   double loss= (double )(allpackets - okpackets)/allpackets;
  	loss=loss*100;
  	if (okpackets==0)
  	{
  		printf("%s.%d %s status down\n",buffer,millisec,host->h_name);
  		
  	} 
  	else 
  	{
  		rtt_avg_vypis = (float) (rtt_avg /okpackets);
  		smean_rtt_vypis =  (float) (smean_rtt /okpackets);

  	printf("%s.%d %s %.3lf%% packet loss, rtt min/avg/max/mdev %.3lf/%.3lf/%.3lf/%.3lf\n",buffer,millisec,host->h_name,loss,rtt_min,rtt_avg_vypis,rtt_max,float (sqrt(smean_rtt_vypis - rtt_avg_vypis*rtt_avg_vypis )));
     
  	}
  	   alarm(3600);  // Reschedule alarm
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



 bool isValidIpAddress(char *ipAddress)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
    return result != 0;
}

bool isValidIp6Address(char *ipAddress)
{
    struct sockaddr_in6 sa;
    int result = inet_pton(AF_INET6, ipAddress, &(sa.sin6_addr));
    return result != 0;
}
    
 
void * sender_func (void *ptr  )
{
  icmphdr *icmp;
  unsigned short int pid = getpid();
  srand(time(NULL));
  sockaddr_in sendSockAddr;
  timespec time1;
   
  while(1) {
	
	char icmpbuffer[65000];
	char *p = icmpbuffer;

	time1.tv_nsec=0;
	time1.tv_sec=0;
    clock_gettime(CLOCK_MONOTONIC, &time1);
    
    char b[sizeof(timespec)];
    memcpy(b,&time1,sizeof(timespec));
   // printf("%d\n", time1.tv_sec);

	
	char str[datasize];

	const char alphanum[] =     "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz123456789";
 
    for (int i = 0; i < datasize; ++i) {

        str[i] = alphanum[rand() % (sizeof(alphanum) - 1)];

    }
    
   icmp = (icmphdr *) icmpbuffer;
   icmp->type = ICMP_ECHO;
   icmp->code = 0;
   icmp->un.echo.id = pid;
   icmp->checksum=0;
   icmp->un.echo.sequence=1;
   p+= sizeof(icmp);


   for (int a =0 ; a!=sizeof(b); a++)
  	{

  		*p = b[a];
  		p++;
  		//printf("%s\n", *p);
  	}
/*
  	p=p-sizeof(b);
  	char c [sizeof(timespec)];
  	memcpy(c,p,sizeof(timespec));
  	timespec* kunt;
  	kunt = (timespec*) c;
  	p=p+strlen(b)+1;
  	printf("\n %d  %d \n",time1.tv_sec, time1.tv_nsec );
  	printf("\n %d  %d \n",kunt->tv_sec, kunt->tv_nsec );
  	*/
  for (int a =0 ; a!=strlen(str); a++)
  	{

  		*p = str[a];
  		p++;
  		//printf("%s\n", *p);
  	}

  	 // printf("%s\n",icmpbuffer );

 

sendSockAddr.sin_family = AF_INET;
  sendSockAddr.sin_port = 0;
  memcpy(&(sendSockAddr.sin_addr), host->h_addr, host->h_length);

icmp->checksum = 0;
    icmp->un.echo.sequence = 1;
    icmp->checksum = checksum((unsigned char *)icmpbuffer, sizeof(icmphdr)+ sizeof(str)-1+ sizeof(b));
   int check = sendto(s, (char *)icmpbuffer, sizeof(icmphdr)+ sizeof(str)-1 + sizeof(b), 0, (sockaddr *)&sendSockAddr, sizeof(sockaddr));



	//printf("%d check\n", check );
   allpackets++;
   usleep(sendtime*1000);
}
}











void * receiver_func (void *ptr  )
{
	int lenght;
	char buffer[65000];
	sockaddr_in receiveSockAddr;
 	socklen_t size;
 	iphdr *ip;
 	icmphdr *icmpRecv;
 	unsigned short int pid = getpid();
 	char timestr[sizeof (timespec)];

  		timespec* recv_time;
		timespec actual_time;
 	//timespec recv_time;


	while(1) {
	  	lenght = 0;
	  	 if ((lenght = recvfrom(s, buffer, 65000, 0, (sockaddr *)&receiveSockAddr, &size)) == -1)
			{
			  printf("chyba pri prijimani\n");
			  break;
			}

		ip = (iphdr *) buffer;
		icmpRecv = (icmphdr *) (buffer + ip->ihl * 4);
		


		if ((icmpRecv->un.echo.id == pid) && (icmpRecv->type == ICMP_ECHOREPLY)  && (lenght==datasize-1 + RESTPACKETSIZE + sizeof(timespec)))
			{ //printf("sprava prijata\n");
				memcpy(timestr,&buffer[28], sizeof(timespec));
				recv_time = (timespec*) timestr;
				clock_gettime(CLOCK_MONOTONIC, &actual_time);


				struct timeval tv;
				int millisec;

				gettimeofday(&tv, NULL);
				millisec = lrint(tv.tv_usec/10000.0);
				struct tm * timeinfo;
				time_t rawtime;
				time (&rawtime);
				timeinfo = localtime (&rawtime);

				strftime (buffer,80,"%F %T",timeinfo);


				if (okpackets == 0)
				{

					rtt_ms = (actual_time.tv_nsec-recv_time->tv_nsec)/(double)1000000;
					rtt_ms += (actual_time.tv_sec - recv_time->tv_sec)*(double)1000;
					if (rtt_ms <wtime)
					{
						rtt_avg=rtt_ms;
						smean_rtt= (float) rtt_ms*rtt_ms;
						rtt_max=rtt_ms;
						rtt_min=rtt_ms;
						okpackets++;
						if (verbose==1)
						{
							printf("%s.%d %d bytes from %s time=%.3lf\n",buffer,millisec,datasize -1 + sizeof(timespec) + sizeof(icmphdr) ,host->h_name,rtt_ms);
						}
					}
				}
				else
				{

					rtt_new = (actual_time.tv_nsec-recv_time->tv_nsec)/(double)1000000;
					rtt_new += (actual_time.tv_sec - recv_time->tv_sec)*(double)1000;

					if (rtt_new < 2*rtt_ms)
					{
						rtt_avg = (float) rtt_new + rtt_avg;	
						if (rtt_new > rtt_max)
							{rtt_max = rtt_new;}
						if (rtt_new < rtt_min)
							{rtt_min = rtt_new;}
						smean_rtt = (float) ((rtt_new * rtt_new) + smean_rtt) ;
						rtt_ms=rtt_new;
						okpackets++;

						if (verbose==1)
						{
							printf("%s.%d %d bytes from %s time=%.3lf\n",buffer,millisec,datasize -1 + sizeof(timespec) + sizeof(icmphdr) ,host->h_name,rtt_new);
						}

						if (rflag==1)
						{
							if (rtt_new>rtime)
							{
								rpackets++;
							}
						}

					}
				

				}


				//printf("%f ms RTT\n",rtt_ms);
			  

			}

	   
	 

	 	 //printf("%d / %d\n",okpackets, allpackets );
		

		}

}


























int main(int argc, char** argv)
{
pthread_t thread1, sender, receiver;
  unsigned short int pid = getpid();


int sockfd;




int rv;
//char IP_string_clike[] = "172.217.23.238";
std::string IP_string (""); //172.217.23.238  2a00:1450:400d:802::1000
std::string str1 (":");
std::string str2 (".");
timespec time2;
int rc;
fd_set mojSet;
char ip6[50];

float rtt_ms;





thdata data1;
 



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

	if (strcmp("-s",argv[q])==0)
	{ datasize = strtol(argv[q+1], NULL, 10);
	  datasize++;}

	 if (strcmp("-i",argv[q])==0)
	{ sendtime=strtol(argv[q+1], NULL, 10);}

	if (strcmp("-r",argv[q])==0)
	{ rtime=strtof(argv[q+1], NULL);
		rflag=1;}



	 if (strcmp("-v",argv[q])==0)
	{ verbose=1;}


	if (strcmp("-w",argv[q])==0)
	{ wtime=strtof(argv[q+1], NULL);
	  wtime=wtime*1000;} 

	 if (strcmp("-t",argv[q])==0)
	{ ttime=strtol(argv[q+1], NULL, 10);}
	
	if ((isValidIpAddress(argv[q]) || (gethostbyname(argv[q]) != NULL && (strchr(argv[q], '.') != NULL))) && q+1==argc)
	{
		host = gethostbyname(argv[q]);

		
	}
	else 
	 {
	 	if (isValidIpAddress(argv[q])|| (gethostbyname(argv[q]) != NULL && (strchr(argv[q], '.') != NULL)) )
	 	{
	 		if(fork() == 0)
			{
				host = gethostbyname(argv[q]);
				
				break;
			}
			else continue;
	 	}

	 	
	 }
}
//printf("%d\n", datasize );

//printf("%s\n", host->h_name);


//pthread_create (&thread1, NULL, print_message_function,(void *) &data1);


struct sigaction act;
act.sa_handler = &on_alarm;
//act.sa_mask = SA_NODEFER;
act.sa_flags = SA_RESTART;  // Restart interrupted system calls
sigaction(SIGALRM, &act, NULL);

alarm(3600);



//IP_string="172.217.23.238";



  if ((s =
     socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1)
  {
    printf("dojebane\n");;
    return -1;
  }
setsockopt(s, IPPROTO_IP, IP_TTL, (const char *)&ttl, sizeof(ttl));
//setsockopt(sock, SOL_IP, IP_TTL, (const char *)&ttl, sizeof(ttl));
	


pthread_create (&sender, NULL, sender_func,(void *) &s);
pthread_create (&receiver, NULL, receiver_func,(void *) &s);
while(1) {
 
  sleep(ttime);
  time_t rawtime;
  struct tm * timeinfo;
  char buffer [800];
  char buffer2 [800];
  struct timeval tv;
  int millisec;

  gettimeofday(&tv, NULL);
  millisec = lrint(tv.tv_usec/10000.0);

  time (&rawtime);
  timeinfo = localtime (&rawtime);

  strftime (buffer,80,"%F %T",timeinfo);
  double loss= (double )(allpackets - okpackets)/allpackets;
  	loss=loss*100;
  //okpackets = 1;
  if (okpackets!=allpackets)
  {
	//printf("%d  %d\n" ,allpackets, okpackets);
  	
  	if (okpackets==0)
  	{
  		printf("%s.%d %s status down\n",buffer,millisec,host->h_name);
  		continue;
  	}
  	
  	
  }

  if (rflag==1)
  	{
  		if (rpackets!=0)
  		{
  			float pomer=0;
  			pomer= (float)rpackets / (float) okpackets;
  			pomer=pomer*100;
			//printf("%f\n", pomer);
  			printf("%s.%d %s %.3lf%% (%d) packets exceeded RTT threshold %dms\n",buffer,millisec,host->h_name,pomer, rpackets,(int)rtime );
  			
  			continue;
  		}
  		

  	}
  
if (okpackets!=allpackets)
  {
  printf("%s.%d %s %.3lf%% packet loss, %d packet lost\n",buffer,millisec,host->h_name,loss,allpackets-okpackets );
  }		 
}



  

}



//podpora hostname - skontrolovať done
//verbose done
//ip/name namiesto uzol done
//-r