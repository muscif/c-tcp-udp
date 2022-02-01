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

#include <stdio.h>
#include <stdlib.h>
#define BUFFERSIZE 512
#define PORT 60000

void errorhandler(char *errorMessage) {
	printf("%s", errorMessage);
}

void clearwinsock() {
#if defined WIN32
	WSACleanup();
#endif
}

int main(void) {
#if defined WIN32
	WSADATA wsa_data;
	int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	if (result != NO_ERROR) {
		printf("Error at WSAStartup()\n");
		return 0;
	}
#endif
	int c_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (c_socket < 0) {
		errorhandler("Socket creation failed.\n");
		closesocket(c_socket);
		clearwinsock();
		return -1;
	}

	struct sockaddr_in sad;
	memset(&sad, 0, sizeof(sad));

	sad.sin_family = AF_INET;
	sad.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	sad.sin_port = htons(PORT);

	if (connect(c_socket, (struct sockaddr*) &sad, sizeof(sad)) < 0) {
		errorhandler("Failed to connect.\n");
		closesocket(c_socket);
		clearwinsock();
		return -1;
	}

	char input_string[BUFFERSIZE];
	char buf[BUFFERSIZE];
	memset(buf, 0, sizeof(buf));
	memset(input_string, 0, sizeof(input_string));

	while(input_string[0] != '='){
		printf("Input operation in the form [operation] [operand] [operand]: ");
		scanf("%[^\n]s", input_string);

		int string_len = strlen(input_string);

		if(send(c_socket, input_string, string_len, 0) != string_len){
			errorhandler("send() sent a different number of bytes than expected.\n");
			closesocket(c_socket);
			clearwinsock();
			return -1;
		}

		if(recv(c_socket, buf, BUFFERSIZE - 1, 0) <= 0){
			errorhandler("recv() failed or connection closed prematurely.\n");
			closesocket(c_socket);
			clearwinsock();
			return -1;
		} else {
			printf("Result: %s\n", buf);
		}
	}


	closesocket(c_socket);
	clearwinsock();
	printf("\n");

	system("pause");
	return 0;
}
