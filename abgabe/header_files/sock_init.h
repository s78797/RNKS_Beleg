#pragma once
#include <WinSock2.h>
#include <stdio.h>

/*
calls WSAStartup() to initialize winsock
returns 1 - failed
		0 - succeded
*/
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


/*
creates a new SOCKET and returns it if SOCKET is not invalid
*/
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