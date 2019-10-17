#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 6000
#define LISTENQ 20
#define BUFFSIZE 4096
#define FILE_NAME_MAX_SIZE 512
#define IP_SIZE 64

int main()
{
	int clientSocket;
	char svr_ip[IP_SIZE];

	bzero(svr_ip, IP_SIZE);
	printf("Please input the server ip:");
	scanf("%s",svr_ip);
	getchar();

	struct sockaddr_in clientaddr;
	bzero(&clientaddr, sizeof(clientaddr));

	clientaddr.sin_family = AF_INET;
	clientaddr.sin_addr.s_addr = htons(INADDR_ANY);
	clientaddr.sin_port = htons(0);

	clientSocket = socket(AF_INET, SOCK_STREAM, 0);

	if(clientSocket<0)
	{
		perror("socket");
		exit(1);
	}

	if(bind(clientSocket, (struct sockaddr*)&clientaddr, sizeof(clientaddr))<0)
	{
		perror("connect client");
		exit(1);
	}

	struct sockaddr_in svraddr;
	bzero(&svraddr, sizeof(svraddr));
	if(inet_aton(svr_ip, &svraddr.sin_addr)==0)
	{
		perror("inet_aton");
		exit(1);
	}

	svraddr.sin_family = AF_INET;
	svraddr.sin_port = htons(PORT);

	if(connect(clientSocket, (struct sockaddr*)&svraddr, sizeof(svraddr))<0)
	{
		perror("connect server");
		exit(1);
	}

	//recv file imformation
	char buff[BUFFSIZE];
	char filename[FILE_NAME_MAX_SIZE];
	int count;
	bzero(buff, BUFFSIZE);

	count = recv(clientSocket, buff, BUFFSIZE, 0);
	if(count<0)
	{
		perror("recv");
		exit(1);
	}
	strncpy(filename, buff, strlen(buff)>FILE_NAME_MAX_SIZE?FILE_NAME_MAX_SIZE:strlen(buff));

	printf("Preparing recv file : %s from %s \n", filename, svr_ip);

	//recv file
	FILE *fd = fopen(filename, "wb+");
	if(NULL==fd)
	{
		perror("open");
		exit(1);
	}
	bzero(buff, BUFFSIZE);

	int length = 0;
	while(length = recv(clientSocket, buff, BUFFSIZE, 0))
	{
		if(length<0)
		{
			perror("recv");
			exit(1);
		}

		int writelen = fwrite(buff, sizeof(char), length, fd);
		if(writelen<length)
		{
			perror("write");
			exit(1);
		}
		printf("receieved file_block_length:%d\n",length);
		bzero(buff, BUFFSIZE);
	}
	printf("Receieved file:%s from %s finished!\n", filename, svr_ip);
	fclose(fd);
	close(clientSocket);
	return 0;
}
