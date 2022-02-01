/*
 * client_main.c
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
#define INPUTMAX 263
#define DEFAULT_PORT 48000
#define DEFAULT_HOST_ADDR "127.0.0.1"
#define DEFAULT_HOST_NAME "localhost"
#define HOST_ADDR_LEN 16
#define HOST_NAME_LEN 256

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
	struct sockaddr_in fromAddr;
	int fromSize;
	char inputString[INPUTMAX];
	char result[INPUTMAX];
	int inputStringLen;
	int respStringLen;

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
	servAddr.sin_family = PF_INET;
	servAddr.sin_port = htons(port);
	servAddr.sin_addr.s_addr = inet_addr(hostAddr);

	while(inputString[0] != '='){
		printf("Input operation in the form [operation] [operand] [operand]: ");
		scanf("%[^\n]s", inputString);

		if ((inputStringLen = strlen(inputString)) > INPUTMAX) {
			errorHandler("Input too long");
		} else if (inputString[0] != '='){
			// Sending string to the server
			if (sendto(sock, inputString, inputStringLen, 0, (struct sockaddr*) &servAddr, sizeof(servAddr)) != inputStringLen) {
				errorHandler("sendto() sent different number of bytes than expected.\n");
			}

			// Server response
			fromSize = sizeof(fromAddr);
			respStringLen = recvfrom(sock, result, INPUTMAX, 0, (struct sockaddr*) &fromAddr, &fromSize);

			if (servAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr) {
				fprintf(stderr, "Error: received a packet from unknown source.\n");
				exit(EXIT_FAILURE);
			}

			// String management before output
			struct hostent *servHost = gethostbyaddr((char*) &fromAddr.sin_addr, 4, AF_INET);

			result[respStringLen] = '\0'; // inutile con memset
			printf("Result: %s\n", result);
			printf("Received from server %s, ip %s\n\n", servHost->h_name, inet_ntoa(fromAddr.sin_addr));
		}

		while(getchar() != '\n'); // Removes from the buffer excess characters
	}

	closesocket(sock);
	clearWinSock();
	return EXIT_SUCCESS;
}
