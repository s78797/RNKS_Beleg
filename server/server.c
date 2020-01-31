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
#include <math.h>

#include "sock_init.h"
#include "ack.h"
#include "Packet.h"
#include "Checksum_Processing.h"
#include "Text_Processing.h"

#pragma warning(disable : 4996)
#pragma warning(disable:5000)


/*
binds given socket to specified port
*/
int bind_socket_to_port(SOCKET *s, int port) {
	SOCKADDR_IN6 serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));

	serv_addr.sin6_family = AF_INET6;
	serv_addr.sin6_port = htons(port);//TODO port as arg to add
	serv_addr.sin6_addr = in6addr_any;

	if (bind(*s, (SOCKADDR *)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {
		printf("bind of socket failed with error: %u\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}
	else {
		printf("socket bind succeded.\n");
		return 1;
	}

}

/*
creates a packet with some randomly changed bits
*/
void create_malformed_packet(packet *pack) {
	char *buf = pack->txtCol;

	int rand1 = rand() % 20;
	int rand2 = rand() % 256;
	buf[rand1] = buf[rand1] & buf[rand2];
	buf[rand2] = buf[rand1] ^ buf[rand2];
	
	return 0;
}

/*
failmode: 0 - no fail
failmode: 1 - no ack was sent / ack got lost
failmode: 2 - wrong ack was sent
failmode: 3 - wrong ack was sent, but correct followed
failmode: 4 - send delayed ack after 6 seconds
failmode: 5 - simulate bit errors in packet data
failmode: 6 - manipulate checksum of ack
*/
int saw_receive(SOCKET *sock, char *filePath, int failmode) {
	SOCKADDR_IN6 clientAddr;
	packet recPacket;
	memset(&clientAddr, 0, sizeof(clientAddr));
	memset(&recPacket, 0, sizeof(recPacket));

	int client_info_len = sizeof(clientAddr);
	char receivedAddr[INET6_ADDRSTRLEN];

	long expectedSeqNr = 0;
	while (1) {
		printf("\nwaiting...\n");
		int receiveCode = recvfrom(*sock, (packet*)&recPacket, sizeof(packet), 0, (SOCKADDR*)&clientAddr, &client_info_len);
		if (receiveCode == SOCKET_ERROR) {
			printf("receiving data failed with error: %u", WSAGetLastError());
			WSACleanup();
			return 1;
		}
		// simulates that some bits changed over transmission
		if (failmode == 5) {
			create_malformed_packet(&recPacket);
			failmode = 0;
		}
		//prints status of receives packet to console
		print_status(&recPacket, expectedSeqNr);

		long receivedChecksum = recPacket.checkSum;
		recPacket.checkSum = 0;

		long expectedChecksum = calcChecksum(*(unsigned short*)&recPacket, sizeof(recPacket));
		printf("checksum server expected: %lu\n", expectedChecksum);
		printf("received address: %s\n", inet_ntop(AF_INET6, (SOCKADDR*)&clientAddr.sin6_addr, receivedAddr, INET6_ADDRSTRLEN));
		
		int correctChecksum = receivedChecksum == expectedChecksum;
		if ((expectedSeqNr > recPacket.seqNo) && correctChecksum) {
			send_ackt(sock, &clientAddr, recPacket.seqNo);
			printf("received duplicated pacekt again...ignoring...\n");
		}
		else if ((expectedSeqNr == recPacket.seqNo) && correctChecksum) {
			printf("received correct packet\n");
			write_to_file(filePath, recPacket.txtCol);
			if (failmode == 1) {
				failmode = 0;
			}
			else if (failmode == 2) {
				send_ackt(sock, &clientAddr, -99999,0);
				failmode = 0;
			}
			else if (failmode == 3) {
				send_ackt(sock, &clientAddr, -99999,0);
				Sleep(1000);
				send_ackt(sock, &clientAddr, recPacket.seqNo,0);
				failmode = 0;
			}
			else if (failmode == 4) {
				Sleep(6000);
				send_ackt(sock, &clientAddr, expectedSeqNr,0);
				failmode = 0;
			}
			else if (failmode == 6){
				send_ackt(sock, &clientAddr, expectedSeqNr,1);
				failmode = 0;

			}
			else {
				send_ackt(sock, &clientAddr, expectedSeqNr,0);
			}
			expectedSeqNr++;

		}
	}
}
/*
prints data of received packet
*/
int print_status(packet *received, int expectedSeqNr) {
	printf("\n\n----RECEIVED PACKET----\n");
	printf("Text: %s", received->txtCol);
	printf("SeqNo: %ld \n", received->seqNo);
	printf("checksum %ld\n", received->checkSum);
	printf("expected SeqNo: %ld\nreceived SeqNo: %ld\n", expectedSeqNr, received->seqNo);
	printf("---------------------------\n");
	return 0;
}

/*
this function sends Ack to given @param{socket} and @param{addr} with given @param{seqNo}
*/
int send_ackt(SOCKET *sock, SOCKADDR_IN6 *clientAddr, int seqNo, int rightChecksum) {

	ack ackt;
	memset(&ackt, 0, sizeof(ackt));
	ackt.seqNo = seqNo;
	ackt.checkSum = calcChecksum(*(unsigned short*)&ackt, sizeof(ackt));
	if (rightChecksum==1){
		printf("original checksum %ld\n", ackt.checkSum);
		ackt.checkSum++;
	}

	int sendCode = sendto(*sock, (ack*)&ackt, sizeof(ackt), 0, (SOCKADDR*)clientAddr, sizeof(SOCKADDR_IN6));
	if (sendCode == SOCKET_ERROR) {
		printf("sending acknowledgement failed with error: %d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}
	printf("sent ack with seqNr %ld\n", ackt.seqNo);
	printf("sent ack with Checksum %ld\n", ackt.checkSum);
	return 0;
}


int main(int argc, char* argv[]) {

	if (argc != 4) {
		printf("usage: %s [port filepath errorcode]", argv[0]);
		return -1;
	}
	int port = atoi(argv[1]);
	char* fileadress= argv[2];
	int failmode=atoi(argv[3]);
	initialze_winsock();
	SOCKET sock = create_new_socket();
	bind_socket_to_port(&sock, port);


	if (saw_receive(&sock, fileadress, failmode) == 1) {
		printf("Connection was closed.\n");
	}

	closesocket(sock);

	return 0;
}
