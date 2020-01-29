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
#include <stdbool.h>

#include "Packet.h"
#include "Text_Processing.h"
#include "Checksum_Processing.h"
#include "ack.h"



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

void set_addr_and_port(SOCKET *s, int port, SOCKADDR_IN6 *serverAddr) {

	memset(serverAddr, 0, sizeof(SOCKADDR_IN6));
	serverAddr->sin6_family = AF_INET6;
	serverAddr->sin6_port = htons(port); //TODO port as arg to add
	inet_pton(AF_INET6, "fe80::e9e0:ca4b:d325:19cd", &(serverAddr->sin6_addr));

	return 0;
}

bool set_io_mode(SOCKET *s, u_long iMode) {
	// sets IO mode of socket to non-blocking
	int rc = ioctlsocket(*s, FIONBIO, &iMode);
	if (rc == SOCKET_ERROR) {
		printf("setting io mode failed with error: %u\n", WSAGetLastError());
	}
	else {
		return true;
	}
}

FILE* open_text_file(char *path) {
	FILE *fp = fopen(path, "r");
	if (fp == NULL) {
		printf("Reading file from path '%s' failed with error: %d \n", errno);
		return 1;
	}
	return fp;
}

packet create_packet(char *buf, long seqNum) {
	packet pack;
	memset(&pack, 0, sizeof(packet));

	strncpy(pack.txtCol, buf, sizeof(pack.txtCol));
	pack.checkSum = calcChecksum(*(unsigned short *)&pack, sizeof(pack));
	pack.seqNum = seqNum;
	return pack;
}

void send_packet_to(packet pack, SOCKET *s, SOCKADDR_IN6 *serverAddr) {
	int rc = sendto(*s, (packet*)&pack, sizeof(pack), 0, (SOCKADDR*)serverAddr, sizeof(SOCKADDR_IN6));
	if (rc == SOCKET_ERROR) {
		printf("sendto failed with error code: %u\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}
	printf("sent %d bytes!\n", rc);
}

int receive_ack(SOCKET *sock) {
	ack recAck;
	memset(&recAck, 0, sizeof(ack));

	int rcReceive = recvfrom(*sock, (ack*)&recAck, sizeof(recAck), 0, NULL, NULL);
	if (rcReceive == SOCKET_ERROR) {
		printf("failed receiving acknowledgement with error code %u\n", WSAGetLastError());
		WSACleanup();
	}
	else {
		printf("received SeqNum of acknowledgement: %lu\n", recAck.seqNum);
	}
	return recAck.seqNum;
}

int check_for_timeout(SOCKET *sock, int s) {
	FD_SET fdSet;
	//memset(&fdSet, 0, sizeof(FD_SET));
	FD_ZERO(&fdSet);
	FD_SET(*sock, &fdSet);

	TIMEVAL timeoutVal;
	timeoutVal.tv_sec = s;
	timeoutVal.tv_usec = 0;

	return select(0, &fdSet, NULL, NULL, &timeoutVal);
}

void saw_send(SOCKET *sock, FILE *filePointer, SOCKADDR_IN6 *serverAddr) {

	char buf[512];

	long seqNum = 0;
	while (fgets(buf, BUFFER_LEN, filePointer) != NULL) {
		packet pack = create_packet(buf, seqNum);
		for (int timeoutsLeft = 3; timeoutsLeft > 0; timeoutsLeft--) {
			send_packet_to(pack, sock, serverAddr);
			int timer = check_for_timeout(sock, 5);
			if (timer > 0 && (receive_ack(sock) == pack.seqNum)) {
				break;
			}
			else if (timeoutsLeft == 1) {
				printf("Coud not receive acknowledgement after 3 tries\n", timeoutsLeft);
			}
			else if (timer == 0) {
				printf("TIMEOUT...send packet again.\n");
			}
			else if (timer == SOCKET_ERROR) {
				printf("select function failed with error: %u\n", WSAGetLastError());
			}
		}
		seqNum++;
	}
}


int main(int argc, char *argv[]) {

	initialze_winsock();

	SOCKADDR_IN6 serverAddr;
	SOCKET sock = create_new_socket();
	FILE *filePointer = open_text_file("D:\\Dokumente\\input.txt");

	set_addr_and_port(&sock, 50000, &serverAddr);
	set_io_mode(&sock, (u_long)1);

	saw_send(&sock, filePointer, &serverAddr);

	getch();
	WSACleanup();

	return 0;
}

