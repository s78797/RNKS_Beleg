#pragma once
typedef struct Packet {
	char txtCol[256];
	int seqNr;
	unsigned short int checkSum;
	

}packet;