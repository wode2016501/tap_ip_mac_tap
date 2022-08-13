#include <fcntl.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
char *str2mac(char *str, char *mac)
{
	int p = (strlen(str));
	if (p != 17)
	{
		printf("mac错误: %s\n", str);
		return -1;
	}
	sscanf(str, "%2x:%2x:%2x:%2x:%2x:%2x", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);

	printf("设置mac: %02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4],
			mac[5]);

	return 0;
}



int main(int argc, char **argv)
{
	int f = open("/dev/iptables", O_RDWR);
	if (f < 0)
	{
		printf("没有启动服务");
		exit(0);
	}
	struct in_addr addr1;
	char mac[6];
	memset(mac, 0, 6);
	char buf[10];
	int ret = 0;
	printf("\n");
	printf("列表:\n");
	ret=0;
	while (read(f, buf, 10) == 10)
	{
		addr1.s_addr = *(int *) buf;
		printf("%s", inet_ntoa(addr1));
		printf("\t->\t%02x:%02x:%02x:%02x:%02x:%02x\n", buf[4], buf[5], buf[6], buf[7], buf[8],
				buf[9]);
	}
	printf("\n");
	if (argc > 1)
	{
		ret = str2mac(argv[1], mac);
		if (ret == -1)
			goto ext;
	}
	for (int i = 2; i < argc; i++)
	{
		addr1.s_addr = inet_addr(argv[i]);
		if (addr1.s_addr == INADDR_NONE)
		{
			printf("err: %s\n", argv[i]);
			continue;
		}
		if(addr1.s_addr==0)
		{
			printf("err: %s\n", argv[i]);
			continue;
		}
		ret=0;
		lseek(f,0,SEEK_SET);
		while (read(f, buf, 10) == 10)
		{
			if(*(int *)buf==addr1.s_addr ){
				ret=1;
				break;
			}
		}
		if(ret==1)
			continue;
		lseek(f,0,SEEK_END);
		printf("set: %s", inet_ntoa(addr1));
		printf("->%02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4],
				mac[5]);
		if( write(f,&(addr1.s_addr),4)!=4){
			perror("err: 写入ip错误\n");
			goto ext;
		}
		if( write(f,mac,6)!=6){
			perror("err: 写入mac错误\n");
			goto ext;
		}
	}
ext:
	close(f);
	return 0;
}
