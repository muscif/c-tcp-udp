#include <stdio.h>
#include <math.h>

#if defined WIN32
#include <winsock.h>
#else
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#define closesocket close
#endif

#define PORT 60000
#define QLEN 6
#define BUFFERSIZE 256

int add(int a, int b){
	return a + b;
}

int sub(int a, int b){
	return a - b;
}

int mult(int a, int b){
	return a * b;
}

float division(int a, int b){
	return (float)a / b;
}

void errorhandler(char *errorMessage) {
	printf("%s", errorMessage);
}

void clearwinsock() {
#if defined WIN32
	WSACleanup();
#endif
}

int main(int argc, char *argv[]) {
	int port;

	if (argc > 1) {
		port = atoi(argv[1]);
	} else {
		port = PORT;
		if (port < 0) {
			printf("Bad port number %s\n", argv[1]);
			return 0;
		}
	}

#if defined WIN32
	WSADATA wsa_data;
	int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	if (result != NO_ERROR) {
		printf("Error at WSAStartup()\n");
		return 0;
	}
#endif

	int s_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s_socket < 0) {
		errorhandler("Socket creation failed.\n");
		return -1;
	}

	int qlen = 5;
	int client_socket = 0;
	int client_len;
	char buf[BUFFERSIZE];
	memset(buf, 0, sizeof(buf));

	struct sockaddr_in sad;
	memset(&sad, 0, sizeof(sad));

	struct sockaddr_in cad;
	memset(&cad, 0, sizeof(cad));

	sad.sin_family = AF_INET;
	sad.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	sad.sin_port = htons(PORT);

	cad.sin_family = AF_INET;
	cad.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	cad.sin_port = htons(PORT);

	if (bind(s_socket, (struct sockaddr*) &sad, sizeof(sad)) < 0) {
		errorhandler("bind() failed.\n");
		clearwinsock();
		return -1;
	} else if (listen(s_socket, qlen) < 0) {
		errorhandler("listen() failed.\n");
		clearwinsock();
		return -1;
	} else {
		while (1) {
			client_len = sizeof(cad);
			printf("Waiting for client...\n");

			if ((client_socket = accept(s_socket, (struct sockaddr*) &cad, &client_len)) < 0) {
				errorhandler("accept() failed.\n");
				closesocket(s_socket);
				clearwinsock();
				return -1;
			} else {
				printf("Connection established with %s:%d\n", inet_ntoa(cad.sin_addr), cad.sin_port);

				if(recv(client_socket, buf, BUFFERSIZE - 1, 0) <= 0){
					errorhandler("recv() failed or connection closed prematurely.\n");
					closesocket(client_socket);
					clearwinsock();
					return -1;
				} else {
					printf("Received: %s\n", buf);
					char result[BUFFERSIZE] = {""};
					char tmp[BUFFERSIZE][BUFFERSIZE];
					memset(tmp, 0, sizeof(tmp));

					char op = 0;
					int op1 = 0;
					int op2 = 0;
					int i = 0;
					char* pch;

					pch = strtok(buf, " ");

					while(pch != NULL){
						strncpy(tmp[i], pch, strlen(pch));
						pch = "";
						pch = strtok(NULL, " ");
						i++;
					}

					op = tmp[0][0];
					op1 = atoi(tmp[1]);
					op2 = atoi(tmp[2]);

					printf("op: %c\n", op);
					printf("op1: %d\n", op1);
					printf("op2: %d\n", op2);

					switch(op){
					case '+':
						sprintf(result, "%d", add(op1, op2));
						break;

					case '-':
						sprintf(result, "%d", sub(op1, op2));
						break;

					case '*':
						printf("res: %d\n", mult(op1, op2));
						sprintf(result, "%d", mult(op1, op2));
						break;

					case '/':
						printf("res: %f\n", division(op1, op2));
						sprintf(result, "%f", division(op1, op2));
						break;

					case '=':
						closesocket(client_socket);
						clearwinsock();
						break;
					}

					int string_len = strlen(result);

					if(send(client_socket, result, string_len, 0) != string_len){
						errorhandler("send() sent a different number of bytes than expected.\n");
						closesocket(client_socket);
						clearwinsock();
						return -1;
					}
				}
			}
		}
	}

	closesocket(s_socket);
	clearwinsock();
	return 0;
}
