#include <stdio.h>
#include <WinSock2.h>
#include <stdlib.h>
#include <windows.h>
#include <windef.h>

#pragma warning(disable:4996)

int startWinsock(void);

int main()
{
	long rc;
	SOCKET s;
	char buf[1024];
	SOCKADDR_IN addr;
	SOCKADDR_IN remoteAddr;
	int         remoteAddrLen = sizeof(SOCKADDR_IN);

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
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s == INVALID_SOCKET)
	{
		printf("Fehler: Der Socket konnte nicht erstellt werden, fehler code: %d\n", WSAGetLastError());
		return 1;
	}
	else
	{
		printf("UDP Socket erstellt!\n");
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(1234);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	while (1)
	{
		printf("Text eingeben: ");
		fgets(buf,1024,stdin);
		rc = sendto(s, buf, strlen(buf), 0, (SOCKADDR*)&addr, sizeof(SOCKADDR_IN));
		if (rc == SOCKET_ERROR)
		{
			printf("Fehler: sendto, fehler code: %d\n", WSAGetLastError());
			return 1;
		}
		else
		{
			printf("%d Bytes gesendet!\n", rc);
		}

		rc = recvfrom(s, buf, strlen(buf), 0, (SOCKADDR*)&remoteAddr, &remoteAddrLen);
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
		}
	}
	return 0;
}

int startWinsock(void) {
	WSADATA wsa;
	return WSAStartup(MAKEWORD(2, 0), &wsa);
}