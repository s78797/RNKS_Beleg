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

#include "\Users\phili\git\uni\RNKS_Beleg\header\ack.h"
#include "\Users\phili\git\uni\RNKS_Beleg\header\Packet.h"
#include "\Users\phili\git\uni\RNKS_Beleg\header\Checksum_Processing.h"
#include "\Users\phili\git\uni\RNKS_Beleg\header\Text_Processing.h"

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

void set_addr_and_port(int port, char *ipv6, SOCKADDR_IN6 *serverAddr) {

	memset(serverAddr, 0, sizeof(SOCKADDR_IN6));
	serverAddr->sin6_family = AF_INET6;
	serverAddr->sin6_port = htons(port); //TODO port as arg to add
	inet_pton(AF_INET6, ipv6, &(serverAddr->sin6_addr));

	return 0;
}

int set_io_mode(SOCKET *s, u_long iMode) {
	// sets IO mode of socket to non-blocking
	int rc = ioctlsocket(*s, FIONBIO, &iMode);
	if (rc == SOCKET_ERROR) {
		printf("setting io mode failed with error: %u\n", WSAGetLastError());
	}
	else {
		return 1;
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

packet create_packet(char *buf, long seqNo) {
	packet pack;
	memset(&pack, 0, sizeof(packet));

	strncpy(pack.txtCol, buf, sizeof(pack.txtCol));
	pack.checkSum = calcChecksum(*(unsigned short *)&pack, sizeof(pack));
	pack.seqNo = seqNo;
	return pack;
}


void send_packet_to(packet pack, SOCKET *s, SOCKADDR_IN6 *serverAddr) {
	int rc = sendto(*s, (packet*)&pack, sizeof(pack), 0, (SOCKADDR*)serverAddr, sizeof(SOCKADDR_IN6));
	if (rc == SOCKET_ERROR) {
		printf("sendto failed with error code: %u\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}
	print_status(&pack);
}

int print_status(packet *sent) {
	printf("\n\n----SENT PACKET----\n");
	printf("Text: %s", sent->txtCol);
	printf("SeqNo: %ld \n", sent->seqNo);
	printf("checksum %ld\n", sent->checkSum);
	printf("---------------------------\n");
	return 0;
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
		printf("received ack with SeqNo: %ld\n", recAck.seqNo);
	}
	return recAck.seqNo;
}

void set_timeout(TIMEVAL *t, int s) {
	t->tv_sec = s;
	t->tv_usec = 0;
}

void init_set_SET(SOCKET **sock, FD_SET *set) {
	FD_ZERO(set);
	FD_SET(**sock, set);
}


int saw_send(SOCKET *sock, FILE *filePointer, SOCKADDR_IN6 *serverAddr, int failmode) {
	
	TIMEVAL timeout;
	FD_SET fdSet;
	init_set_SET(&sock, &fdSet);
	char buf[BUFFER_LEN];
	set_io_mode(sock, 1);

	long seqNo = 0;
	while (fgets(buf, BUFFER_LEN, filePointer) != NULL) {
		packet pack = create_packet(buf, seqNo);
		
		send_packet_to(pack, sock, serverAddr);
		set_timeout(&timeout, 5);
		int timer;
		int numberOfTimeouts = 0;
		int successful = 0;
		while ((!successful) && (numberOfTimeouts <= 3)) {
			timer = select(0, &fdSet, NULL, NULL, &timeout);
			if (timer == 0) {
				printf("TIMEOUT...send packet again.\n");
				send_packet_to(pack, sock, serverAddr);
				init_set_SET(&sock, &fdSet);
				set_timeout(&timeout, 5);
				numberOfTimeouts++;
			}
			else if (timer > 0){
				int recAck = receive_ack(sock);
				if (recAck == pack.seqNo) {
					successful = 1;
				}
				else {
					init_set_SET(&sock, &fdSet);
				}
			}
			else if (timer == SOCKET_ERROR) {
				printf("select function failed with error: %u\n", WSAGetLastError());
				return -1;
			}
			if (numberOfTimeouts == 3) {
				printf("Coud not receive acknowledgement after 3 tries\n");
				return -1;
			}
		}
		seqNo++;
	}
}


int main(int argc, char *argv[]) {

	if (argc != 4) {
		printf("usage: %s [ipv6 port filepath]");
		exit(-1);
	}
	char *ipv6 = argv[1];
	int port = atoi(argv[2]);
	char *filepath = argv[3];

	initialze_winsock();

	SOCKADDR_IN6 serverAddr;
	SOCKET sock = create_new_socket();
	FILE *filePointer = open_text_file(filepath);
	set_addr_and_port(port, ipv6, &serverAddr);

	if (saw_send(&sock, filePointer, &serverAddr, 1) == -1) {
		printf("Lost Connection to Receiver\n");
	}

	getch();
	WSACleanup();

	return 0;
}