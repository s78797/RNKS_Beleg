#define _CRT_SECURE_NO_WARNINGS
#include <WinSock2.h>
#include <stdio.h>

#include <stdlib.h>
#include <windows.h>
#include "Packet.h"
#include "Text_Processing.h"
#include "Checksum_Processing.h"
#include "ack.h"
#include <ws2ipdef.h>
#include <fcntl.h>
#include <string.h>
#include <WS2tcpip.h>



#pragma warning(disable : 4996)
#pragma warning(disable:5000)


int startWinsock(void) {
	WSADATA wsa;
	return WSAStartup(MAKEWORD(2, 2), &wsa);
}



int main(int argc, char *argv[]) {
	
	packet p1;
	memset(&p1, 0, sizeof(packet));
	ack recAck;
	memset(&recAck, 0, sizeof(ack));
	
	long rc;
	SOCKET s = INVALID_SOCKET;
	SOCKADDR_IN6 addr;
	FD_SET fdSet;
	SOCKET servers[1];



	FILE *fp;
	char buf[512];



	//init Winsock
	rc = startWinsock();
	if (rc != 0)
	{
		printf("Fehler: startWinsock, fehler code: %d\n", rc);
		return 1;
	}
	else
	{
		printf("Winsock gestartet!\n");
	}

	//UDP Socket erstellen
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

	memset(&addr, 0, sizeof(addr));

	inet_pton(AF_INET6, "2003:c2:7727:6800:e9e0:ca4b:d325:19cd", &(addr.sin6_addr));
	addr.sin6_family = AF_INET6;
	addr.sin6_port = htons(5000);

	//int packetsize = sizeof(p1);
	fp = fopen(argv[1], "r");
	if (fp == NULL) {
		printf("Fehler beim lesen der Datei");
		return 2;
	}
	
	int iteratedSeqNum = 0;
	

	while (fgets(buf, BUFFER_LEN, fp) != NULL)
	{
		
	
		memset(&p1, 0, sizeof(packet));
		strncpy(p1.txtCol, buf, sizeof(p1.txtCol));
		p1.checkSum = NULL;
		p1.checkSum = calcChecksum(*(unsigned short *)&p1, sizeof(p1));
		p1.seqNr = iteratedSeqNum;
		iteratedSeqNum++;

		rc = sendto(s, (packet*)&p1, sizeof(p1), 0, (SOCKADDR*)&addr, sizeof(SOCKADDR_IN6));
			if (rc == SOCKET_ERROR)
			{
				printf("Fehler: sendto, fehler code: %d\n", WSAGetLastError());
				WSACleanup();
				return 1;
			}
		else {
			printf("%d Bytes gesendet!\n", rc);
			TIMEVAL totimer;
			totimer.tv_sec = 10;
			FD_ZERO(&fdSet);
			FD_SET(s, &fdSet);
			
			while (select(0, &fdSet, NULL, NULL, &totimer) > 0) {
				


			}
		
	
			
			}

		}
	





		/*rc = recvfrom(s, buf, strlen(buf), 0, (SOCKADDR*)&remoteAddr, &remoteAddrLen);
		if (rc == SOCKET_ERROR)
		{
			printf("Fehler: recvfrom, fehler code: %d\n", WSAGetLastError());
			return 1;
		}
		else
		{
			printf("%d Bytes empfangen!\n", rc);
			buf[rc] = '\0';
			printf("Empfangene Daten: %s\n", buf);
		}*/
	
	
	



	
	WSACleanup();
	s = INVALID_SOCKET;
	return 0;
}

