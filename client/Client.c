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
#include "\Users\phili\git\uni\RNKS_Beleg\header\sock_init.h"
#include "\Users\phili\git\uni\RNKS_Beleg\header\Packet.h"
#include "\Users\phili\git\uni\RNKS_Beleg\header\Checksum_Processing.h"
#include "\Users\phili\git\uni\RNKS_Beleg\header\Text_Processing.h"

#pragma warning(disable : 4996)
#pragma warning(disable:5000)



/*
creates a SOCKADDR_IN6 with the specified port and ipv6
@param port - port to set
@param ipv6 - IN6_ADDR to set
@return the creates SOCKADDR_IN6
*/
SOCKADDR_IN6 create_sockAddr(int port, char *ipv6) {
	SOCKADDR_IN6 serverAddr;
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN6));
	serverAddr.sin6_family = AF_INET6;
	serverAddr.sin6_port = htons(port);
	inet_pton(AF_INET6, ipv6, &(serverAddr.sin6_addr));

	return serverAddr;
}

/*
sets the I/O-mode of the socket to the given value
@param socket to set io mode
@param u_long iMode - value of io mode
@retruns 1 - successful
		-1 - if SOCKET_ERROR occurs
*/
int set_io_mode(SOCKET *s, u_long iMode) {
	// sets IO mode of socket to non-blocking
	int rc = ioctlsocket(*s, FIONBIO, &iMode);
	if (rc == SOCKET_ERROR) {
		printf("setting io mode failed with error: %u\n", WSAGetLastError());
		return -1;
	}
	else {
		return 1;
	}
}

/*
creates a packet with the given buffer and seqNo
*/
packet create_packet(char *buf, long seqNo) {
	packet pack;
	memset(&pack, 0, sizeof(packet));

	strncpy(pack.txtCol, buf, sizeof(pack.txtCol));
	pack.checkSum = calcChecksum(*(unsigned short *)&pack, sizeof(pack));
	pack.seqNo = seqNo;
	return pack;
}

/*
sends given paket to sererAddr over given socket
*/
void send_packet_to(packet pack, SOCKET *s, SOCKADDR_IN6 *serverAddr) {
	int rc = sendto(*s, (packet*)&pack, sizeof(pack), 0, (SOCKADDR*)serverAddr, sizeof(SOCKADDR_IN6));
	if (rc == SOCKET_ERROR) {
		printf("sendto failed with error code: %u\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}
	print_status(&pack);
}

/*
prints the status of a sent packed on the console
*/
int print_status(packet *sent) {
	printf("\n\n-------SENT PACKET--------\n");
	printf("Text: %s", sent->txtCol);
	printf("SeqNo: %ld \n", sent->seqNo);
	printf("checksum %ld\n", sent->checkSum);
	printf("--------------------------\n");
	return 0;
}

/*
receives an ACK over given SOCKET and returns its SeqNo
*/
int receive_ack(SOCKET *sock) {
	ack recAck;
	memset(&recAck, 0, sizeof(ack));
	int rcReceive = recvfrom(*sock, (ack*)&recAck, sizeof(recAck), 0, NULL, NULL);
	if (rcReceive == SOCKET_ERROR) {
		printf("failed receiving acknowledgement with error code %u\n", WSAGetLastError());
		WSACleanup();
		return -1;
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

/*
Function that handles saw protocol on client side. 
It calls the get_next_frame method to receive the next buffer and sends his buffer to the given SOCKADDR_IN6
by using the given SOCKET.
*/
int saw_send(SOCKET *sock, SOCKADDR_IN6 *serverAddr, FILE *filePointer) {
	TIMEVAL timeout;
	FD_SET fdSet;
	init_set_SET(&sock, &fdSet);
	set_io_mode(sock, 1);
	
	char *buffer = malloc(sizeof(char)*256);
	long seqNo = 0;

	while (get_next_frame(&buffer, filePointer) != -1) {
		packet pack = create_packet(buffer, seqNo);
		send_packet_to(pack, sock, serverAddr);
		set_timeout(&timeout, 5);
		int timer;
		int numberOfTimeouts = 0;
		int successful = 0;
		while ((!successful) && (numberOfTimeouts <= 3)) {
			timer = select(0, &fdSet, NULL, NULL, &timeout);	// starts a timer and returns 0 on timeout, 1 if socket is ready to be read
			if (numberOfTimeouts == 2) {
				printf("Coud not receive acknowledgement after 3 tries\n");
				return -1;
			}
			else if (timer == 0) {
				printf("TIMEOUT...send packet again.\n");
				send_packet_to(pack, sock, serverAddr);
				init_set_SET(&sock, &fdSet);
				set_timeout(&timeout, 5);
				numberOfTimeouts++;
			}
			else if (timer > 0) {
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
		}
		seqNo++;
	}
	return 1;
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
	SOCKET sock = create_new_socket();
	SOCKADDR_IN6 serverAddr = create_sockAddr(port, ipv6);
	FILE *filePointer = get_file_pointer(filepath);

	if ( (saw_send(&sock, &serverAddr, filePointer) == -1)) {
		printf("Lost Connection to Receiver\n");
	}

	getch(); // to see log on console
	WSACleanup();

	return 0;
}