/*
 * server_main.c
 *
 * Author: Francesco Musci
 */

#if defined WIN32
#include <winsock.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif
#include <stdio.h>
#include <string.h>
#define INPUTMAX 255
#define DEFAULT_PORT 48000
#define DEFAULT_HOST_ADDR "127.0.0.1"
#define DEFAULT_HOST_NAME "localhost"
#define HOST_ADDR_LEN 16
#define HOST_NAME_LEN 256
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

void errorHandler(char *errorMessage) {
	printf(errorMessage);
}

void clearWinSock() {
#if defined WIN32
	WSACleanup();
#endif
}

int main(int argc, char **argv) {
#if defined WIN32
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("Error at WSASturtup.\n");
		return EXIT_FAILURE;
	}
#endif
	int sock;
	struct sockaddr_in servAddr;
	struct sockaddr_in clntAddr;
	int clntAddrLen;
	char buf[INPUTMAX];
	memset(buf, 0, sizeof(buf));

	char hostAddr[HOST_ADDR_LEN] = DEFAULT_HOST_ADDR;
	char hostName[HOST_NAME_LEN] = DEFAULT_HOST_NAME;
	int port = DEFAULT_PORT;

	// Host name resolution and port assignment
	if (argc > 1) {
		strncpy(hostName, strtok(argv[1], ":"), HOST_NAME_LEN);
		struct in_addr* ina = (struct in_addr*) gethostbyname(hostName)->h_addr_list[0];
		strcpy(hostAddr, inet_ntoa(*ina));
		port = atoi(strtok(NULL, ":"));
	}

	// Socket creation
	if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		errorHandler("socket() failed.\n");
	}

	// Server address construction
	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(port);
	servAddr.sin_addr.s_addr = inet_addr(hostAddr);

	// Socket bind
	if ((bind(sock, (struct sockaddr*) &servAddr, sizeof(servAddr))) < 0) {
		errorHandler("bind() failed.\n");
	} else {
		clntAddrLen = sizeof(clntAddr);

		char op = 0;
		int op1 = 0;
		int op2 = 0;
		int i = 0;
		char* pch;
		char toks[BUFFERSIZE][BUFFERSIZE];
		char result[BUFFERSIZE] = {""};

		while (1) {
			// Receive string from client
			recvfrom(sock, buf, INPUTMAX, 0, (struct sockaddr*) &clntAddr, &clntAddrLen);

			struct hostent *clntHost = gethostbyaddr((char*) &clntAddr.sin_addr, 4, AF_INET);

			printf("Operation: \"%s\"\n", buf);
			printf("Received from client %s, ip %s\n", clntHost->h_name, inet_ntoa(clntAddr.sin_addr));
			memset(toks, 0, sizeof(toks));

			op = 0;
			op1 = 0;
			op2 = 0;
			i = 0;

			// Input string tokenization
			pch = strtok(buf, " ");

			while(pch != NULL){
				strncpy(toks[i], pch, strlen(pch));
				pch = "";
				pch = strtok(NULL, " ");
				i++;
			}

			op = toks[0][0];
			op1 = atoi(toks[1]);
			op2 = atoi(toks[2]);

			// Prepare string for output
			memset(buf, 0, sizeof(buf));
			strcat(buf, toks[1]);
			strcat(buf, " ");
			strcat(buf, toks[0]);
			strcat(buf, " ");
			strcat(buf, toks[2]);
			strcat(buf, " = ");

			switch(op){
			case '+':
				sprintf(result, "%d", add(op1, op2));
				break;

			case '-':
				sprintf(result, "%d", sub(op1, op2));
				break;

			case '*':
				sprintf(result, "%d", mult(op1, op2));
				break;

			case '/':
				sprintf(result, "%f", division(op1, op2));
				break;

			default:
				memset(buf, 0, sizeof(buf));
				memset(result, 0, sizeof(result));
				break;
			}

			strcat(buf, result);
			buf[BUFFERSIZE] = '\0';

			if (sendto(sock, buf, BUFFERSIZE, 0, (struct sockaddr*) &clntAddr, sizeof(clntAddr)) != BUFFERSIZE) {
				errorHandler("sendto() sent different number of bytes than expected.\n");
			}

			memset(buf, 0, sizeof(buf));
			memset(result, 0, sizeof(result));
			printf("\n");
		}
	}
}
