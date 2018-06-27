#include "comm.h"

int main(int argc, char *argv[])
{
	if(argc != 3){
		printf("Usage ./client [ip] [port]\n");
		return 1;
	}

	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0){
		perror("socket");
		return 2;
	}

	struct sockaddr_in server_socket;
	struct sockaddr_in client_socket;

	server_socket.sin_family = AF_INET;
	server_socket.sin_addr.s_addr = inet_addr(argv[1]);
	server_socket.sin_port = htons(atoi(argv[2]));

	int ret = connect(sock, (struct sockaddr*)&server_socket, sizeof(server_socket));
	if(ret < 0){
		perror("connect");
		return 3;
	}

	printf("connect success...\n");
	char buf[1024];

	if(fcntl(0, F_SETFL, FNDELAY) < 0){
		perror("fcntl");
		return 4;
	}

	response* res = malloc(sizeof(response));
	for(;;)
	{
		//printf("Please Enter: ");
		int s = read(0, buf, sizeof(buf)-1);
		if(s > 0){
			buf[s-1] = 0;
			send(sock, buf, strlen(buf), MSG_DONTWAIT);
		}
		int _s = recv(sock, res, sizeof(response), MSG_DONTWAIT);
		if(_s > 0){
			buf[_s] = 0;
			fflush(stdout);
			printf("\t\t[%s] [%d] say: %s\n", res->ip, res->port, res->msg);
		} 
	}
	return 0;
}
