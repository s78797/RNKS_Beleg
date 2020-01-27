#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <WinSock2.h>

#include <stdlib.h>
#include <windows.h>
#include "ack.h"
#include "Packet.h"
#include "Text_Processing.h"
#include "Checksum_Processing.h"
#include <ws2ipdef.h>
#include <fcntl.h>
#include <string.h>
#include <WS2tcpip.h>





#define WIN32_LEAN_AND_MEAN




#pragma warning(disable : 4996)
#pragma warning(disable:5000)

int main(int argc, char* argv[]) {
	SOCKET s;
	s = INVALID_SOCKET;
	int rcReceive, rcSend;

	char buf[512];
	char receivedAdr[INET6_ADDRSTRLEN];

	SOCKADDR_IN6 serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	SOCKADDR_IN6 client_info;
	memset(&client_info, 0, sizeof(client_info));
	int client_info_len = sizeof(client_info);

	WSADATA data;
	WORD version = MAKEWORD(2, 2);


	packet recPacket;
	memset(&recPacket, 0, sizeof(recPacket));



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
	serv_addr.sin6_port = htons(50000);// port as arg to add
	serv_addr.sin6_addr = in6addr_any;


	if (bind(s, (struct sockaddr*) & serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {

		printf("Fehler: sendto, fehler code: %d\n", WSAGetLastError());
		WSACleanup();

	}




	long expectedSeqNr = 0;
	ack quittung;
	memset(&quittung, 0, sizeof(quittung));

	while (1) {
		memset(&recPacket, 0, sizeof(recPacket));
		rcReceive = recvfrom(s, (packet*)&recPacket, sizeof(packet), 0, (SOCKADDR*)&client_info, &client_info_len);
		
		//rcReceive = recvfrom(s, (packet*)&recPacket, sizeof(packet), 0, NULL, NULL);

		printf("erhaltener Text: %s \n", recPacket.txtCol);
		printf("erhaltene Sequenznummer: %ld \n", recPacket.seqNr);
			
		printf("received Checksum %ld\n",recPacket.checkSum);
		printf("erhaltene Adresse: %s\n ", inet_ntop(AF_INET6, (SOCKADDR*)&client_info.sin6_addr, receivedAdr, INET6_ADDRSTRLEN));

		int correctSeqNum = (expectedSeqNr == recPacket.seqNr);
		printf("Sequenznummern: erwartet%lu   ==    erhalten%lu ----- true?%d\n", expectedSeqNr, recPacket.seqNr, correctSeqNum);

		long receivedChecksum = recPacket.checkSum;
		recPacket.checkSum = 0;

		long receivedSeqnum = recPacket.seqNr;
		long expected_check = calcChecksum(*(unsigned short*)&recPacket, sizeof(recPacket));
		if (correctSeqNum && (receivedChecksum == expected_check)) {

			
			quittung.seqNum = receivedSeqnum;
			printf("geschickte Seqnum des ack: %ld\n", quittung.seqNum);
			rcSend = sendto(s, (ack*)&quittung, sizeof(quittung), 0, (SOCKADDR*)&client_info, sizeof(SOCKADDR_IN6));
			if (rcSend == SOCKET_ERROR) {
				printf("Fehler beim senden der Quittung mit code %d\n", WSAGetLastError());
				WSACleanup();
				return 1;
			}
			printf("received packet, checksum and seqNr correct\n");
		}
		expectedSeqNr++;
	}


	rcReceive = closesocket(s);
	printf("\n\ncloseddd ");
	//int right=checkChecksum(&recPacket);

//(struct sockaddr *)&client_info, &len)

	return 0;
}