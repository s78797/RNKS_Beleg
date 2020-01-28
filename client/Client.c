#define _CRT_SECURE_NO_WARNINGS
#include <WinSock2.h>
#include <stdio.h>

#include <stdlib.h>
#include <windows.h>
#include <ws2ipdef.h>
#include <fcntl.h>
#include <string.h>
#include <WS2tcpip.h>
#include <errno.h>

#include "Packet.h"
#include "Text_Processing.h"
#include "Checksum_Processing.h"
#include "ack.h"



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

	int rc;
	SOCKET s = INVALID_SOCKET;
	SOCKADDR_IN6 addr;
	FD_SET fdSet;
	FD_ZERO(&fdSet);

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
	if (s == INVALID_SOCKET){
		printf("Fehler: Der Socket konnte nicht erstellt werden, fehler code: %d\n", WSAGetLastError());
		return 1;
	}
	else{
		printf("UDP Socket erstellt!\n");
	}

	memset(&addr, 0, sizeof(addr));

	inet_pton(AF_INET6, "fe80::a861:ec31:c15e:89a7", &(addr.sin6_addr));
	addr.sin6_family = AF_INET6;
	addr.sin6_port = htons(50000);

	//int packetsize = sizeof(p1);
	FILE *fp = fopen("D:\\Dokumente\\input.txt", "r");
	if (fp == NULL) {
		printf("Error %d \n", errno);
		printf("Fehler beim lesen der Datei");
		return 2;
	}

	ioctlsocket(s, FIONBIO, 0);



	SOCKADDR_IN6 serv_addr;

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin6_family = AF_INET6;
	serv_addr.sin6_port = htons(50000);// port as arg to add
	serv_addr.sin6_addr = in6addr_any;


	if (bind(s, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {

		printf("Fehler: bind, fehler code: %d\n", WSAGetLastError());
		WSACleanup();

	}



	long iteratedSeqNum = 0;

	while (fgets(buf, BUFFER_LEN, fp) != NULL){
		int i = 1;
		while (i <= 3){
		
		memset(&p1, 0, sizeof(packet));
		strncpy(p1.txtCol, buf, sizeof(p1.txtCol));
		printf("Text: %s", p1.txtCol);
		//unsigned short placeholder = calcChecksum(*(unsigned short *)&p1, sizeof(p1));
		p1.checkSum = calcChecksum(*(unsigned short *)&p1, sizeof(p1));
		p1.seqNr = iteratedSeqNum;


		rc = sendto(s, (packet*)&p1, sizeof(p1), 0, (SOCKADDR*)&addr, sizeof(SOCKADDR_IN6));
		if (rc == SOCKET_ERROR)
		{
			printf("Fehler: sendto, fehler code: %d\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}
		
		printf("%d Bytes gesendet!\n", rc);


		FD_SET(s, &fdSet);
		TIMEVAL totimer;
		totimer.tv_sec = 5;
		totimer.tv_usec = 0;

		int timer = select(0, &fdSet, NULL, NULL, &totimer);
		if (timer > 0){

			int rcReceive = recvfrom(s, (ack*)&recAck, sizeof(recAck), 0, NULL, NULL);
			if (rcReceive == SOCKET_ERROR) {
					printf("Fehler beim erhalten der Quittung mit code %d\n", WSAGetLastError());
					WSACleanup();
					return 1;

			}
			else {
				printf("erhaltene SeqNum der Quittung: %lu\n", recAck.seqNum);
				if (recAck.seqNum == p1.seqNr)break;;
			}
		}
			else if (timer == 0){
				printf("TIMEOUT SENDE PAKET NOCHMAL:\n");
				i++;
			}
		
			if (i == 3){ printf("Verbindung zum Host verloren"); return 8; }
		}
				iteratedSeqNum++;
	}
	












	WSACleanup();
	s = INVALID_SOCKET;
	return 0;
}

