  
//socket
#include <sys/types.h>  
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <netinet/in.h>

//
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int SERVEPORT= 30000;
void client(char* t)
{
	printf("start client\n");    
	//nh_.getParam("SERVEPORT", SERVEPORT);
	//nh_.getParam("SERVER_IP", SERVER_IP);
	
	int sockfd, recvbytes;
	struct hostent *host;
	struct sockaddr_in serv_addr;

	if (( sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
	{
		perror("socket error!");
		exit(1);
	}
		    
	bzero(&serv_addr,sizeof(serv_addr));
	serv_addr.sin_family    = AF_INET;
	serv_addr.sin_port      = htons(SERVEPORT);
	serv_addr.sin_addr.s_addr= inet_addr("127.0.0.1");
	//serv_addr.sin_addr = inet_addr(SERVER_IP);
		    
	if (connect(sockfd, (struct sockaddr *)&serv_addr,sizeof(struct sockaddr)) == -1) 
	{
		perror("connect error!");
		exit(1);
	}
	
	printf("strlen(t) = %d\n",strlen(t));
	send(sockfd, t, 100, 0);
	sleep(2);

	//strcpy(test.name, "orange");
	//send(sockfd, (char*)&test, sizeof(test), 0);
	//sleep(1);
	
	printf("end client");
	close(sockfd);
	return;
}

int main(int argc, char** argv)
{
	char message[100];
	strcpy(message, "hello server\n");
	printf(message);
	client(message);
	sleep(1);
	return 0;
}
