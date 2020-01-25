#define _CRT_SECURE_NO_WARNINGS
#include <winsock2.h>
#include <WS2tcpip.h>
#include "Packet.h"
#include "Checksum_Processing.h"
#include "Text_Processing.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#pragma commet(lib, "Ws2_32.lib");

#define WIN32_LEAN_AND_MEAN

SOCKET sock;
struct sockaddr_in6	serv_addr;
int len, rcReceive;
char ip[256];

struct sockaddr_in6 client_info;




#pragma warning(disable : 4996)
#pragma warning(disable:5000)

int main(int argc, char* argv[]) {

	WSADATA data;
	WORD version = MAKEWORD(2, 2);
	char buf[256];
	packet recPaket;



	if ((WSAStartup(version, &data)) != 0) {
		// print error
		WSACleanup();
		// exit
	}
	else printf("Initialisierung erfolgreich\n");

	sock = socket(AF_INET6, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET)
	{
		printf("Fehler: Der Socket konnte nicht erstellt werden, fehler code: %d\n", WSAGetLastError());
		return 1;
	}
	else
	{
		printf("UDP Socket erstellt!\n");
	}



	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin6_family = AF_INET6;
	serv_addr.sin6_port = htons(1234);// port as arg to add
	serv_addr.sin6_addr = in6addr_any;


	if (bind(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {

		printf("Fehler: sendto, fehler code: %d\n", WSAGetLastError());
		WSACleanup();

	}

	len = sizeof(client_info);

	while (1){
		rcReceive = recvfrom(sock, buf, 1024, 0, NULL, NULL);
}
	//(struct sockaddr *)&client_info, &len)

	return 0;
}