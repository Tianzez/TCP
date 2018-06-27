#include "comm.h"

typedef struct sockfd_list{
	int fd;
	response res;
	struct sockfd_list* _prev;
	struct sockfd_list* _next;
}sockfd_list;

// 管理连接的客户端的信息
sockfd_list* head = NULL;

// 线程执行函数
void* threadwork(void* arg)
{
	sockfd_list* Node  = (sockfd_list*)arg;

	char buf[1024];
	while(1){
		// 读取客户端发来的消息
		int s = recv(Node->fd, buf, sizeof(buf), 0);
		if(s == 0){
			// 客户端断开连接，把该客户端的套接字信息从链表中删除
			printf("client disconnect...\n");
			Node->_prev->_next = Node->_next;
			Node->_next->_prev = Node->_prev;
			free(Node);
			break;
		}
		else if(s < 0){
			break;
		}

		buf[s] = '\0';
		printf("client [%s] [%d] : #%s\n", Node->res.ip, Node->res.port, buf);
		strcpy(Node->res.msg, buf);

		sockfd_list* cur = head->_next;
		while(cur != head){
			if(cur != Node){
				send(cur->fd, &(Node->res), sizeof(response), 0);
			}
			cur = cur->_next;
		}
	}
}


sockfd_list* BuyNode(int fd, char* ip, int port)
{
	sockfd_list* tmp = malloc(sizeof(sockfd_list));
	memset(tmp, 0, sizeof(sockfd_list));
	if(tmp == NULL){
		perror("malloc");
		exit(1);
	}
	tmp->fd = fd;
	strcpy(tmp->res.ip, ip);
	tmp->res.port = port;
	tmp->_prev = NULL;
	tmp->_next = NULL;
	return tmp;
}


int main(int argc, char *argv[])
{
	if(argc != 3){
		printf("Usage ./server [ip] [port]\n");
		return 1;
	}

	head = BuyNode(0, "head", 0);
	// 参数1:IPV4协议  参数2:面向字节流
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0){
		perror("socket");
		return 2;
	}
	// 服务器
	struct sockaddr_in server_socket;

	// 把服务器的IP地址和端口号填充到对应套接字结构体中
	server_socket.sin_family = AF_INET;
	server_socket.sin_addr.s_addr = inet_addr(argv[1]);
	server_socket.sin_port = htons(atoi(argv[2]));

	int ret = bind(sock, (struct sockaddr*)&server_socket, sizeof(server_socket));
	// 如果端口号已经被其他进程占有了，就会绑定失败
	if(ret < 0){
		perror("bind");
		close(sock);
		return 3;
	}

	if(listen(sock, 5) < 0){
		perror("listen");
		close(sock);
		return 4;
	}

	printf("bind and listen success, wait accept...\n");

	for(;;)
	{
		// 连接的客户端
		struct sockaddr_in client_socket;
		socklen_t len = sizeof(client_socket);

		// 链接客服端
		int client_sock = accept(sock, (struct sockaddr*)&client_socket, &len);
		if(client_sock < 0){
			printf("accept error\n");
			continue;
		}
		char* ip = inet_ntoa(client_socket.sin_addr);
		int port = ntohs(client_socket.sin_port);
		printf("get accept, ip is : %s, port is : %d\n", ip, port);

		sockfd_list* NewNode = BuyNode(client_sock, ip, port);
		if(head->_next == NULL){
			head->_next = NewNode;
			head->_prev = NewNode;
			NewNode->_prev = head;
			NewNode->_next = head;
		} else {
			NewNode->_next = head;
			NewNode->_prev = head->_prev->_next; 
			head->_prev->_next = NewNode;
			head->_prev = NewNode;
		}

		pthread_t tid = 0;
		pthread_create(&tid, NULL, threadwork, (void*)NewNode);
		
		//这里不能用pthread_join()来回收线程，
		//因为这个函数是阻塞式的，如果主线程没有回收到指定线程，那他就会一直阻塞，
		//这样就没法处理其他客户端请求。
		pthread_detach(tid);
	}
	close(sock);
	return 0;
}
