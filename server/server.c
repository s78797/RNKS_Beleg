#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <WinSock2.h>

#include <stdlib.h>
#include <windows.h>
#include "Packet.h"
#include "Text_Processing.h"
#include "Checksum_Processing.h"
#include <ws2ipdef.h>
#include <fcntl.h>
#include <string.h>
#include <WS2tcpip.h>



#pragma commet(lib, "Ws2_32.lib");

#define WIN32_LEAN_AND_MEAN




#pragma warning(disable : 4996)
#pragma warning(disable:5000)

int main(int argc, char* argv[]) {
	SOCKET s;
	s = INVALID_SOCKET;
	int len, rcReceive;
	char ip[256];
	char buf[512];

	SOCKADDR_IN6 serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	SOCKADDR_IN6 client_info;
	memset(&client_info, 0, sizeof(client_info));
	WSADATA data;
	WORD version = MAKEWORD(2, 2);
	
	packet recPacket;
	memset(&recPacket, 0, sizeof(recPacket));;



	if ((WSAStartup(version, &data)) != 0) {
		// print error
		WSACleanup();
		// exit
	}
	else printf("Initialisierung erfolgreich\n");

	s = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
	if (s == INVALID_SOCKET)
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
	serv_addr.sin6_port = htons(5000);// port as arg to add
	serv_addr.sin6_addr = in6addr_any;

	
	if (bind(s, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {

		printf("Fehler: sendto, fehler code: %d\n", WSAGetLastError());
		WSACleanup();

	}

	len = sizeof(client_info);
	int checkSeq=1;

	while (rcReceive = recvfrom(s, (packet*)&recPacket, sizeof(packet), 0, NULL, NULL)) {
		//rcReceive = recvfrom(s, (packet*)&recPacket, sizeof(packet), 0, NULL, NULL);
		printf("%s/n", recPacket.txtCol);
		printf("erhaltene Sequenznummer: %d\n", recPacket.seqNr);
		
	}
		//int right=checkChecksum(&recPacket);
	
	//(struct sockaddr *)&client_info, &len)

	return 0;
}