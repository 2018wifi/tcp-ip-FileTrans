#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 6000
#define LISTENQ 20
#define BUFFSIZE 4096
#define FILE_NAME_MAX_SIZE 512

int main()
{
	double interval;	//The time using for transfering file
	clock_t start_time, finished_time;

	//Input the file name
	char filename[FILE_NAME_MAX_SIZE];
	bzero(filename, FILE_NAME_MAX_SIZE);
	printf("Please input the file name you want to send:");
	scanf("%s",filename);
	getchar();

	//Create socket
	int sockfd, connfd;
	struct sockaddr_in svraddr, clientaddr;
	bzero(&svraddr, sizeof(svraddr));

	svraddr.sin_family=AF_INET;
	svraddr.sin_addr.s_addr=htonl(INADDR_ANY);
	svraddr.sin_port=htons(PORT);

	sockfd=socket(AF_INET,SOCK_STREAM,0);

	if(sockfd<0)
	{
		perror("socket");
		exit(1);
	}

	//bind
	if(bind(sockfd,(struct sockaddr*)&svraddr,sizeof(svraddr))<0)
	{
		perror("bind");
		exit(1);
	}

	//listen
	if(listen(sockfd,LISTENQ)<0)
	{
		perror("listen");
		exit(1);
	}

	while(1)
	{
		socklen_t length=sizeof(clientaddr);
		printf("Waiting for connect...\n");
		connfd=accept(sockfd,(struct sockaddr*)&clientaddr,&length);

		if(connfd<0)
		{
			perror("connect");
			exit(1);
		}

		//send file imformation
		char buff[BUFFSIZE];
		int count;
		bzero(buff,BUFFSIZE);
		strncpy(buff,filename,strlen(filename)>FILE_NAME_MAX_SIZE?FILE_NAME_MAX_SIZE:strlen(filename));
		count=send(connfd,buff,BUFFSIZE,0);
		if(count<0)
		{
			perror("Send file imformation");
			exit(1);
		}

		//read file
		FILE *fd=fopen(filename,"rb");
		if(fd==NULL)
		{
			printf("File:%s not found!\n",filename);
		}
		else
		{
			bzero(buff,BUFFSIZE);
			int file_block_length=0;
			start_time = clock();
			while((file_block_length=fread(buff,sizeof(char),BUFFSIZE,fd))>0)
			{
				printf("file_block_length:%d\n",file_block_length);
				if(send(connfd,buff,file_block_length,0)<0)
				{
					perror("Send");
					exit(1);
				}
				bzero(buff,BUFFSIZE);
			}
			finished_time = clock();
			fclose(fd);
			send(connfd,buff,file_block_length,0);
			interval = (double)(finished_time - start_time) / CLOCKS_PER_SEC;
			printf("Transfer file finished using %f seconds\n", interval);
		}
		close(connfd);
	}
	close(sockfd);
	return 0;
}

