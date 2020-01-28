#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#include <stdio.h>
#include <WinSock2.h>
#include <stdlib.h>
#include <windows.h>
#include <ws2ipdef.h>
#include <fcntl.h>
#include <string.h>
#include <WS2tcpip.h>

#include "ack.h"
#include "Packet.h"
#include "Text_Processing.h"
#include "Checksum_Processing.h"

#pragma warning(disable : 4996)
#pragma warning(disable:5000)

int initialze_winsock() {
	WSADATA data;
	WORD version = MAKEWORD(2, 2);
	if ((WSAStartup(version, &data)) != 0) {
		printf("WSAStartup failed with error: %u", WSAGetLastError());
		WSACleanup();
		return 1;
	}
	else {
		printf("Initialization succeded.\n");
	}
	return 0;
}

SOCKET create_new_socket() {
	SOCKET sock = INVALID_SOCKET;
	sock = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
	if (sock == INVALID_SOCKET) {
		printf("socket function failed with error: %u\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}
	else {
		printf("UDP socket was created.\n");
		return sock;
	}
}


int bind_socket_to_port(SOCKET *s, int port) {

	SOCKADDR_IN6 serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));

	serv_addr.sin6_family = AF_INET6;
	serv_addr.sin6_port = htons(port);//TODO port as arg to add
	serv_addr.sin6_addr = in6addr_any;

	if (bind(*s, (SOCKADDR *)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {
		printf("bind of socket failed with error: %u\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}
	else {
		printf("socket bind succeded.\n");
		return 0;
	}

}

int handle_requests(SOCKET *sock) {
	SOCKADDR_IN6 clientAddr;
	packet recPacket;
	memset(&clientAddr, 0, sizeof(clientAddr));
	memset(&recPacket, 0, sizeof(recPacket));

	int client_info_len = sizeof(clientAddr);
	char receivedAddr[INET6_ADDRSTRLEN];

	long expectedSeqNr = 0;
	while (1) {
		printf("waiting...\n");
		int receiveCode = recvfrom(*sock, (packet*)&recPacket, sizeof(packet), 0, (SOCKADDR*)&clientAddr, &client_info_len) == SOCKET_ERROR;
		if (receiveCode == SOCKET_ERROR) {
			printf("receiving data failed with error: %u", WSAGetLastError());
			WSACleanup();
			return 1;
		}
		print_status(&recPacket, expectedSeqNr);
		printf("received address: %s\n ", inet_ntop(AF_INET6, (SOCKADDR*)&clientAddr.sin6_addr, receivedAddr, INET6_ADDRSTRLEN));

		recPacket.checkSum = 0;
		long expectedChecksum = calcChecksum(*(unsigned short*)&recPacket, sizeof(recPacket));
		if ((expectedSeqNr == recPacket.seqNr) && (recPacket.checkSum == expectedChecksum)) {
			send_ackt(&sock, &clientAddr, expectedSeqNr);
		}
		expectedSeqNr++;
	}
}

int print_status(packet *received, int expectedSeqNr) {
	printf("----RECEIVED PACKET\n----");
	printf("Text: %s \n", received->txtCol);
	printf("SeqNr: %ld \n", received->seqNr);
	printf("checksum %ld\n", received->checkSum);
	printf("expected SeqNr: %lu\nreceived SeqNr: %lu\n", expectedSeqNr, received->seqNr);
	printf("----------------\n");
}

int send_ackt(SOCKET *sock, SOCKADDR_IN6 *clientAddr, int seqNum) {
	ack ackt;
	memset(&ackt, 0, sizeof(ackt));
	ackt.seqNum = seqNum;
	printf("SeqNr of acknowledgment: %ld\n", ackt.seqNum);
	int sendCode = sendto(sock, (ack*)&ackt, sizeof(ackt), 0, (SOCKADDR*)&clientAddr, sizeof(SOCKADDR_IN6));
	if (sendCode == SOCKET_ERROR) {
		printf("Fehler beim senden der Quittung mit code %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}
	printf("received packet, checksum and seqNr correct\n");
}


int main(int argc, char* argv[]) {
	initialze_winsock();
	SOCKET sock = create_new_socket();
	bind_socket_to_port(&sock, 50000);

	handle_requests(&sock);

	closesocket(sock);
	printf("\n\ncloseddd ");

	return 0;
}