#include <stdio.h>
#include <WinSock2.h>
#include <stdlib.h>
#include <windows.h>

#include <windef.h>
#include <ws2ipdef.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "Header.h"
#include "Windows.h"

#pragma warning(disable:50000)

int startWinsock(void);


void calcChecksum( packet *pack) {
	
		byte buffer = (byte)pack;
		
		register unsigned int sum = 0;
		int count = sizeof(pack);
		while (count > 1)
		{
			sum += (unsigned short int) buffer++;
			count -= 2;
		}
		
		//hinzufügen der übriggebliebenen bytes
		if (count>0)
		{
			sum += *(byte*)buffer;
		}
		//32-bit summe wird zu 16 bit umgeformt
		while (sum >> 16) {
			sum = (sum & 0xfffff) + (sum >> 16);

		}
		pack->checkSum = htons(sum);
	return;
}




int main()
{

	long rc;
	SOCKET s;
	char buf[1024];
	IN6_ADDR myaddr= IN6ADDR_ANY_INIT;
	SOCKADDR_IN6 addr;
	SOCKADDR_IN6 remoteAddr;
	int         remoteAddrLen = sizeof(SOCKADDR_IN6);

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
	s = socket(AF_INET6, SOCK_DGRAM, 0);
	if (s == INVALID_SOCKET)
	{
		printf("Fehler: Der Socket konnte nicht erstellt werden, fehler code: %d\n", WSAGetLastError());
		return 1;
	}
	else
	{
		printf("UDP Socket erstellt!\n");
	}

	addr.sin6_family = AF_INET6;
	addr.sin6_port = htons(50000);
	addr.sin6_addr = myaddr;// PLACEHOLDER





	while (1)
	{
		printf("Text eingeben: ");
		fgets(buf, 1024, stdin);
		rc = sendto(s,buf, strlen(buf), 0, (SOCKADDR_IN6*)&addr, sizeof(SOCKADDR_IN6));
		if (rc == SOCKET_ERROR)
		{
			
			printf("Fehler: sendto, fehler code: %d\n", WSAGetLastError());
			WSACleanup();
			
			return 1;
		}
		else
		{
			printf("%d Bytes gesendet!\n", rc);
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
	}

	int fd;
	char buffer[256];
	//fd = open("a.txt,", O_RDONLY, S_IRUSR);
	//read(fd, buffer, sizeof(buffer));



	WSACleanup();
	return 0;
}

int startWinsock(void) {
	WSADATA wsa;
	return WSAStartup(MAKEWORD(2, 0), &wsa);
}