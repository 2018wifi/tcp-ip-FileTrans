#include <WinSock2.h>
#include <stdio.h>
#include <WS2tcpip.h>

#pragma  comment(lib,"ws2_32.lib")

#define PORT 6000
#define LISTENQ 20
#define BUFFSIZE 4096
#define FILE_NAME_MAX_SIZE 512
#define IP_SIZE 64

int main()
{
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	char svr_ip[IP_SIZE];
	memset(svr_ip, '\0', IP_SIZE);
	printf("Please input the server ip:");
	scanf_s("%s", svr_ip, IP_SIZE);
	getchar();

	if (WSAStartup(sockVersion, &wsaData) != 0)
	{
		printf("WSAStartup");
		return 0;
	}
	SOCKET sclient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sclient == INVALID_SOCKET)
	{
		printf("invalid socket");
		return 0;
	}
	SOCKADDR_IN serAddr;
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(PORT);
	inet_pton(AF_INET, svr_ip, &serAddr.sin_addr);
	if (connect(sclient, (SOCKADDR *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
	{
		printf("connect error");
		closesocket(sclient);
		return 0;
	}

	//recv file imformation
	char buff[BUFFSIZE];
	char filename[FILE_NAME_MAX_SIZE];
	int count;
	memset(buff, 0, BUFFSIZE);

	count = recv(sclient, buff, BUFFSIZE, 0);
	if (count < 0)
	{
		perror("recv");
		exit(1);
	}
	strncpy_s(filename, FILE_NAME_MAX_SIZE, buff, BUFFSIZE);

	printf("Preparing recv file : %s from %s \n", filename, svr_ip);

	//recv file
	FILE *fd = NULL;
	fopen_s(&fd, filename, "wb+");
	if (NULL == fd)
	{
		perror("open");
		closesocket(sclient);
		exit(1);
	}
	memset(buff, 0, BUFFSIZE);

	int length = 0;
	while (length = recv(sclient, buff, BUFFSIZE, 0))
	{
		if (length < 0)
		{
			perror("recv");
			exit(1);
		}

		int writelen = fwrite(buff, sizeof(char), length, fd);
		if (writelen < length)
		{
			perror("write");
			exit(1);
		}
		printf("receieved file_block_length:%d\n", length);
		memset(buff, 0, BUFFSIZE);
	}
	printf("Receieved file:%s from %s finished!\n", filename, svr_ip);
	fclose(fd);
	closesocket(sclient);
	return 0;
}
