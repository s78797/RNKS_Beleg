#pragma once

unsigned short calcChecksum(unsigned short *pack, int size) {



	register long sum = 0;
	int count = size;
	while (count > 1)
	{

		sum += (unsigned short)pack++;

		count -= 2;
	}

	//hinzufügen der übriggebliebenen bytes
	if (count > 0)
	{
		sum += *(unsigned char*)pack;

	}
	//32-bit summe wird zu 16 bit umgeformt
	while (sum >> 16) {
		sum = (sum & 0xffff) + (sum >> 16);

	}

	unsigned short checksum = ~sum;
	printf("\ngebildete checksum: %ld \n", checksum);
	return checksum;
}




int checkChecksum(packet received_packet) {
	unsigned short receivedsum = received_packet.checkSum;
	received_packet.checkSum = 0;

	unsigned short calculatedsum = calcChecksum( *(unsigned short*)&received_packet, sizeof(received_packet));
	if (calculatedsum == receivedsum) {
		
		printf("Checksum stimmt \n");
		return 1;
	}
	else {
		printf("Bitfehler aufgetreten\n Errechnet: %i, Erhalten %i\n",calculatedsum, receivedsum);
		return 0;
	}
}

