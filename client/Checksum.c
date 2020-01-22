#include <stdio.h>
#include <WinSock2.h>
#include <stdlib.h>
#include <windows.h>

#include <windef.h>
#include <ws2ipdef.h>

#include <fcntl.h>
#include <string.h>

#include <limits.h>
#include <float.h>
#include "Header.h"
#include "Windows.h"

#pragma warning(disable : 4996)
//#pragma warning(disable:50000)





int main()
{
	printf("Start");
	packet p1;
	p1.checkSum = 0;
	p1.seqNr = 1;
	strcpy(p1.txtCol, "test");



}