#include <signal.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>

volatile sig_atomic_t wasSigHup = 0; // сигнал SIGHUP
void sigHupHandler(int r) 
{
	wasSigHup = 1;
}

int main()
{	
	int clientSock_fd;
	int server_fd;

	struct sockaddr_in addr;
	int addrLen = sizeof(addr);
	
	// Создание TCP-сокета
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1) {
    	printf("Socket creation error");
    	exit(EXIT_FAILURE);
	}
	// Настраивание адреса (привязка к адресу и порту)
	addr.sin_family = AF_INET; 
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(8000);

	// Привязывание сокета к адресу
	int addrBind = bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));
	if(addrBind == -1) {
    	printf("Bind error");
    	exit(EXIT_FAILURE);
	}
	// Сокет в состоянии ожидания входящих соеденений
	int listn = listen(server_fd, 1);
	if (listn == -1) {
		printf("Listen error");
		exit(EXIT_FAILURE);
	}
	//Регистрация обработчика сигнала
	struct sigaction sa;
	sigaction(SIGHUP, NULL, &sa); 
	sa.sa_handler = sigHupHandler; 
	sa.sa_flags |= SA_RESTART;
	sigaction(SIGHUP, &sa, NULL);

	//Блокировка сигнала SIGHUP
	sigset_t blockedMask, origMask;
	sigemptyset(&blockedMask);
	sigemptyset(&origMask);
	sigaddset(&blockedMask, SIGHUP);
	sigprocmask(SIG_BLOCK, &blockedMask, &origMask);

	// Основной цикл
	clientSock_fd = 0;
	int maxFd;
	fd_set fds;
	while (true) {
    	FD_ZERO(&fds);
    	FD_SET(server_fd, &fds);

		if (clientSock_fd > 0) {
			FD_SET(clientSock_fd, &fds); 
		}

		if (clientSock_fd > server_fd) {
			maxFd = clientSock_fd;
		}
		else {
			maxFd = server_fd;
		}
		// Проход по множеству дескрипторов
	    if (pselect(maxFd + 1, &fds, NULL, NULL, NULL, &origMask) == -1 && errno != EINTR) {
			printf("Pselect error");
			exit(EXIT_FAILURE);
	    }
		// Проверка, что сигнал прошел
		if (wasSigHup == 1) {
			wasSigHup = 0;
			continue;
		}

		if (FD_ISSET(server_fd, &fds)) {
			struct sockaddr_in clientAddr;
			int clientAddrLen = sizeof(clientAddr);
			clientSock_fd = accept(server_fd, (struct sockaddr*)&clientAddr, (socklen_t*)&clientAddrLen);
			if (clientSock_fd < 0) {
				printf("Accept error");
				exit(EXIT_FAILURE);
			}
			printf("Client connected\n");
		}

	    if (clientSock_fd > 0 && FD_ISSET(clientSock_fd, &fds)) { 
	        char buffer[1024] = {0};
	        int bytes = read(clientSock_fd, buffer, 1024);
			if (bytes > 0) {
				printf("Received data: %d bytes\n", bytes);
			}
			else if (bytes == 0) {
				printf("Client disconnected\n");
				close(clientSock_fd);
				clientSock_fd = 0;
			}
	    }
	}
	close(server_fd);
}
