/*Beam Software (GB/GBC) to MIDI converter*/
/*By Will Trowbridge*/
/*Portions based on code by ValleyBell*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>

#define bankSize 16384

FILE* rom, * txt;
long bank;
long patTablePtrLoc;
long patTableOffset;
long seqTablePtrLoc;
long seqTableOffset;
long seqDataOffset;
long seqPTables[4];
int i, j;
char outfile[1000000];
int songNum;
long songPtrs[4];
long songSeqPtrs;
long songSeqData;
int chanMask;
long bankAmt;
int foundTable1 = 0;
int foundTable2 = 0;
int foundTable3 = 0;
long firstPtr = 0;
int curInst = 0;
long tempoTablePtrLoc;
long tempoTableOffset;
int songTempo = 0;

unsigned static char* romData;

const char MagicBytesA[8] = { 0x19, 0x86, 0x18, 0x0D, 0x79, 0xD6, 0x04, 0x07 };
const char MagicBytesB[8] = { 0x5F, 0x16, 0x00, 0x3E, 0x00, 0xE0, 0x25, 0x21 };
const char MagicBytesC[4] = { 0x07, 0x07, 0x07, 0x07 };
const int NoteLenTab[46] = { 0x03, 0x06, 0x09, 0x0C, 0x12, 0x15, 0x18, 0x24, 0x2A, 0x30, 0x48, 0x54, 0x60,
0x90, 0xA8, 0x02, 0x03, 0x04, 0x06, 0x07, 0x08, 0x0C, 0x0E, 0x10, 0x18, 0x1C,
0x20, 0x30, 0x38, 0xFF, 0xFE, 0xFD, 0xFB, 0xF7, 0xEF, 0xDF, 0xBF, 0x7F, 0x01,
0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };

/*Function prototypes*/
unsigned short ReadLE16(unsigned char* Data);
static void Write8B(unsigned char* buffer, unsigned int value);
static void WriteBE32(unsigned char* buffer, unsigned long value);
static void WriteBE24(unsigned char* buffer, unsigned long value);
static void WriteBE16(unsigned char* buffer, unsigned int value);
void song2txt(int songNum, long ptrs[4], long seqPtrs, long seqData);

/*Convert little-endian pointer to big-endian*/
unsigned short ReadLE16(unsigned char* Data)
{
	return (Data[0] << 0) | (Data[1] << 8);
}

static void Write8B(unsigned char* buffer, unsigned int value)
{
	buffer[0x00] = value;
}

static void WriteBE32(unsigned char* buffer, unsigned long value)
{
	buffer[0x00] = (value & 0xFF000000) >> 24;
	buffer[0x01] = (value & 0x00FF0000) >> 16;
	buffer[0x02] = (value & 0x0000FF00) >> 8;
	buffer[0x03] = (value & 0x000000FF) >> 0;

	return;
}

static void WriteBE24(unsigned char* buffer, unsigned long value)
{
	buffer[0x00] = (value & 0xFF0000) >> 16;
	buffer[0x01] = (value & 0x00FF00) >> 8;
	buffer[0x02] = (value & 0x0000FF) >> 0;

	return;
}

static void WriteBE16(unsigned char* buffer, unsigned int value)
{
	buffer[0x00] = (value & 0xFF00) >> 8;
	buffer[0x01] = (value & 0x00FF) >> 0;

	return;
}

int main(int args, char* argv[])
{
	printf("Beam Software (GB/GBC) to TXT converter\n");
	if (args != 3)
	{
		printf("Usage: BEAM2TXT <rom> <bank>\n");
		return -1;
	}
	else
	{
		if ((rom = fopen(argv[1], "rb")) == NULL)
		{
			printf("ERROR: Unable to open file %s!\n", argv[1]);
			exit(1);
		}
		else
		{
			bank = strtol(argv[2], NULL, 16);
			if (bank != 1)
			{
				bankAmt = bankSize;
			}
			else
			{
				bankAmt = 0;
			}
			fseek(rom, ((bank - 1) * bankSize), SEEK_SET);
			romData = (unsigned char*)malloc(bankSize);
			fread(romData, 1, bankSize, rom);
			fclose(rom);

			/*Try to search the bank for song table loader*/
			for (i = 0; i < bankSize; i++)
			{
				if ((!memcmp(&romData[i], MagicBytesA, 8)) && foundTable1 != 1)
				{
					patTablePtrLoc = bankAmt + i - 2;
					printf("Found pointer to song pattern tables at address 0x%04x!\n", patTablePtrLoc);
					patTableOffset = ReadLE16(&romData[patTablePtrLoc - bankAmt]);
					printf("Song pattern tables start at 0x%04x...\n", patTableOffset);
					foundTable1 = 1;
				}
			}

			/*Now search for the sequence data*/
			for (i = 0; i < bankSize; i++)
			{
				if ((!memcmp(&romData[i], MagicBytesB, 8)) && foundTable2 != 1)
				{
					seqTablePtrLoc = bankAmt + i + 8;
					printf("Found pointer to sequence tables at address 0x%04x!\n", seqTablePtrLoc);
					seqTableOffset = ReadLE16(&romData[seqTablePtrLoc - bankAmt + 12]);
					printf("Sequence pointer tables start at 0x%04x...\n", seqTableOffset);
					seqDataOffset = ReadLE16(&romData[seqTablePtrLoc - bankAmt]);
					printf("Sequence data tables start at 0x%04x...\n", seqDataOffset);
					foundTable2 = 1;
				}
			}

			/*Finally, look for the song tempo values*/
			for (i = 0; i < bankSize; i++)
			{
				if ((!memcmp(&romData[i], MagicBytesC, 4)) && foundTable3 != 1 && romData[i - 3] == 0xFA)
				{
					tempoTablePtrLoc = bankAmt + i + 16;
					printf("Found pointer to song tempo table at address 0x%04x!\n", tempoTablePtrLoc);
					tempoTableOffset = ReadLE16(&romData[tempoTablePtrLoc - bankAmt]);
					printf("Tempo table is at 0x%04x...\n", tempoTableOffset);
					foundTable3 = 1;
				}
			}

			if (foundTable1 == 1 && foundTable2 == 1)
			{
				/*Get the locations of the actual channel pointers*/
				seqPTables[0] = ReadLE16(&romData[patTableOffset - bankAmt]);
				seqPTables[1] = ReadLE16(&romData[patTableOffset - bankAmt + 2]);
				seqPTables[2] = ReadLE16(&romData[patTableOffset - bankAmt + 4]);
				seqPTables[3] = ReadLE16(&romData[patTableOffset - bankAmt + 6]);

				/*Now convert the song data*/
				songNum = 1;
				i = seqPTables[0] - bankAmt;

				while ((i + bankAmt) < seqPTables[1])
				{
					songPtrs[0] = ReadLE16(&romData[seqPTables[0] + (2 * (songNum - 1)) - bankAmt]);
					printf("Song %i channel 1: 0x%04X\n", songNum, songPtrs[0]);
					songPtrs[1] = ReadLE16(&romData[seqPTables[1] + (2 * (songNum - 1)) - bankAmt]);
					printf("Song %i channel 2: 0x%04X\n", songNum, songPtrs[1]);
					songPtrs[2] = ReadLE16(&romData[seqPTables[2] + (2 * (songNum - 1)) - bankAmt]);
					printf("Song %i channel 3: 0x%04X\n", songNum, songPtrs[2]);
					songPtrs[3] = ReadLE16(&romData[seqPTables[3] + (2 * (songNum - 1)) - bankAmt]);
					printf("Song %i channel 4: 0x%04X\n", songNum, songPtrs[3]);
					songSeqPtrs = ReadLE16(&romData[seqTableOffset + (2 * (songNum - 1)) - bankAmt]);
					printf("Song %i sequence pointers: 0x%04X\n", songNum, songSeqPtrs);
					songSeqData = ReadLE16(&romData[seqDataOffset + (2 * (songNum - 1)) - bankAmt]);
					printf("Song %i sequence data: 0x%04X\n", songNum, songSeqData);
					songTempo = romData[tempoTableOffset + (songNum - 1) - bankAmt];
					printf("Song %i tempo: %01X\n", songNum, songTempo);
					song2txt(songNum, songPtrs, songSeqPtrs, songSeqData);
					i += 2;
					songNum++;
				}

				printf("The operation was succesfully completed!\n");
				return 0;
			}
			else
			{
				printf("ERROR: Magic bytes not found!\n");
				exit(2);
			}


			
		}
	}
}

void song2txt(int songNum, long ptrs[4], long seqPtrs, long seqData)
{
	unsigned char command[2];
	unsigned char patCom[3];
	long romPos = 0;
	long ptrPos = 0;
	long seqPos = 0;
	int curTrack = 0;
	int curSeq = 0;
	int patTimes = 0;
	int patEnd = 0;
	int seqEnd = 0;
	int curNote = 0;
	int curNoteLen = 0;
	int highestSeq = 0;

	sprintf(outfile, "song%i.txt", songNum);
	if ((txt = fopen(outfile, "wb")) == NULL)
	{
		printf("ERROR: Unable to write to file song%i.txt!\n", songNum);
		exit(2);
	}
	else
	{
		for (curTrack = 0; curTrack < 4; curTrack++)
		{
			fprintf(txt, "Channel %i:\n", (curTrack + 1));

			if (ptrs[curTrack] != 0x0000)
			{
				romPos = ptrs[curTrack];
				patEnd = 0;
			}
			else
			{
				patEnd = 1;
			}
			while (patEnd == 0)
			{
				patCom[0] = romData[romPos - bankAmt];
				patCom[1] = romData[romPos + 1 - bankAmt];
				patCom[2] = romData[romPos + 2 - bankAmt];

				if (patCom[0] == 0x00)
				{
					fprintf(txt, "End song\n\n");
					patEnd = 1;
				}

				else if (patCom[0] == 0x01)
				{
					curSeq = patCom[1];
					if (curSeq > highestSeq)
					{
						highestSeq = curSeq;
					}
					fprintf(txt, "Play pattern: %01X\n", curSeq);
					romPos += 2;
				}

				else if (patCom[0] == 0x02)
				{
					curSeq = patCom[1];
					fprintf(txt, "Go to loop point: %01X\n\n", curSeq);
					patEnd = 1;
				}

				else if (patCom[0] == 0x03)
				{
					curSeq = patCom[1];
					patTimes = patCom[2];
					if (curSeq > highestSeq)
					{
						highestSeq = curSeq;
					}
					fprintf(txt, "Jump to pattern position: %01X, %i times\n", curSeq, patTimes);
					romPos += 3;
				}
			}
		}

		if (songSeqPtrs != 0x0000)
		{
			for (curSeq = 0; curSeq <= highestSeq; curSeq++)
			{
				ptrPos = songSeqPtrs - bankAmt;
				seqPos = ReadLE16(&romData[ptrPos + (curSeq * 2)]) + ReadLE16(&romData[seqDataOffset + ((songNum - 1) * 2) - bankAmt]) - bankAmt;
				fprintf(txt, "Sequence %01X:\n", curSeq);
				seqEnd = 0;

				while (seqEnd == 0)
				{
					command[0] = romData[seqPos];
					command[1] = romData[seqPos + 1];

					if (command[0] < 0x80)
					{
						curNote = command[0];
						curNoteLen = command[1];
						fprintf(txt, "Play note: %01X, length: %01X\n", curNote, curNoteLen);
						seqPos += 2;
					}

					else if (command[0] >= 0x80 && command[0] < 0xFA)
					{
						curInst = command[0];
						fprintf(txt, "Set instrument: %01X\n", curInst);
						seqPos++;
					}

					else if (command[0] == 0xFF)
					{
						fprintf(txt, "End of sequence\n\n");
						seqEnd = 1;
					}

					else
					{
						fprintf(txt, "Unknown command: %01X\n", command[0]);
						seqPos++;
					}
				}

			}
		}

		fclose(txt);
	}
}