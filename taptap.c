#include<pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <linux/if_tun.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h> 
#include <sys/select.h>
#include <sys/time.h>
#include <errno.h>
#include <stdarg.h>
fd_set rd_set,wd_set;
int nwrite =0;
int nread =0;
int iptables=0; 
//#define BUF_SIZE 1472
#define BUF_SIZE 2000
void error_handling(char *message);
int maxfd=0;
int ret=0;
int tun_alloc(char *dev0) {
	if(dev0==0)
		dev0="tmp";
	char dev[20];
	sprintf(dev,"%s",dev0);
	int flags = IFF_TAP;
	flags=flags | IFF_NO_PI;
	struct ifreq ifr;
	int fd, err;
	char *clonedev = "/dev/net/tun";
	int k=0;
	sprintf(dev,"%s%d",dev0,k);
tap: 	
	if( (fd = open("/dev/tun" , O_RDWR)) < 0)
		if( (fd = open(clonedev , O_RDWR)) < 0 ) {
			perror("Opening /dev/net/tun");
			exit(1);
		}

	memset(&ifr, 0, sizeof(ifr));

	ifr.ifr_flags = flags;

	if (*dev) {
		strncpy(ifr.ifr_name, dev, strlen(dev));
	}
	if( (err = ioctl(fd, TUNSETIFF, (void *)&ifr)) < 0 ) {
		// 		perror("ioctl(TUNSETIFF)");
		close(fd);
		sprintf(dev,"%s%d",dev0,k); 
		k++; 
		goto tap; 
	}
	strcpy(dev, ifr.ifr_name);
	printf("%s\n",dev); 
	char cmd[60];
	sprintf(cmd,"ifconfig %s up",dev);
	system(cmd);
	//sprintf(cmd,"ifconfig %s mtu %d ",dev,BUF_SIZE-14);
	system(cmd);
	return fd;
}


int  tmp=0;
int  ip(char *message){
	//	 printf("tmp!=%d,%d,%d",tmp,*(int *)(message+30),message);

	char  buf[10];
	if(message[12]==8&& message[13]==0){
		ret=0;
		if(message[23]==0x6){
			printf("tcp: ");
			ret=1;
		}
		if(message[23]==0x11){
			printf("udp: ");
			ret=1;
		}
		if(ret==1){
			//printf("tmp!=%d,%d ",tmp,*(int *)(message+26));
			//u= ( struct in_addr  *) message+30;
			//printf("目标地址: %s ",inet_ntoa(*u));
			if(tmp!=*(int *)(message+30)){
				printf("目标地址: ");
				for(int i=30;i<34;i++)
					printf("%02x",message[i]&0xff);

				printf(" 目标mac: ");
				for(int i=0;i<6;i++)
					printf("%02x",message[i]&0xff);
			}
			//tmp=*(int *)(message+30);
			memcpy(&tmp,message+30,sizeof(int));
			lseek(iptables,0,SEEK_SET);
			while(read(iptables,buf,10)==10){
				if(*(int *)buf==*(int *)(message+30)){
					memcpy(message,buf+4,6);
					printf(" 改mac了 "); 
				}
			}
			printf("\n");
		}
	}
	//	printf("\n"); 
	return 0;
}


int main(){
	iptables=open("/dev/iptables",O_RDWR|O_CREAT);
	if(iptables<0)
		exit(0); 
	char *p=0;
	close(0);
	int fd=tun_alloc(p); 
	int sock=tun_alloc(p);
	printf("fd=%d ,sock=%d\n", fd,sock );	
	if(fd>sock)
		maxfd=fd;
	else
		maxfd=sock;
	maxfd+=1;
	char message[BUF_SIZE  ];
	while (1)
	{
		FD_ZERO(&rd_set);
		FD_SET(fd, &rd_set);   
		FD_SET(sock, &rd_set);
		ret = select(maxfd, &rd_set, 0, NULL,  0);
		if (ret < 1) {
			perror("select()");
			exit(1);
		}
		if(FD_ISSET(fd, &rd_set)){
			nread = read(fd , message , BUF_SIZE    );
			if( nread >0){
				//printf("读取a: %d ",nread ); 
				//ip(message);
				nwrite=write(sock, message, nread);
			}

		}
		if(FD_ISSET(sock, &rd_set)){
			nread= read(sock,message, BUF_SIZE);
			if (nread > 0) {
				//printf("读取b: %d ",nread);
				ip(message);
				nwrite = write(fd ,message, nread );


			}
		}
	}
	return 0;
}



void error_handling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

