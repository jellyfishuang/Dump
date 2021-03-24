#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define BYTES_PER_LINE	16
#define LINES_PER_PAGE	23
#define COMMANDCHARS_CAPACITY 50

FILE *fp = NULL;
long g_lFileSize;
long g_lCurrOffset = 0;
char g_cCommand[COMMANDCHARS_CAPACITY] = { NULL };
char g_cCapitalCommand = NULL;

void InputCommand();
void InputCommandLine();
void ProcessCommandLine(char* command);
void Print();
void Print(long offset);
void FillBinaryInData(char* offset, char* data);
void Search(unsigned char* data);
void SearchString(char* data);
void SearchBinary(char* data);

void InputCommand()
{
	memset(g_cCommand, '\0', COMMANDCHARS_CAPACITY);
	fgets(g_cCommand, sizeof(g_cCommand) / sizeof(g_cCommand[0]), stdin);
	g_cCapitalCommand = g_cCommand[0];
}

void InputCommandLine()
{
	InputCommand();
	ProcessCommandLine(g_cCommand);
}

void ProcessCommandLine(char* command)
{
	int iLengthofCommand = strlen(command);
	char cCapitalCommand = command[0];
	g_cCapitalCommand = cCapitalCommand;

	if (' ' != command[1] && '\n' != command[1])
	{
		printf("Invalid Input!\n");
		InputCommandLine();
	}

	for (int i = 0; i < iLengthofCommand - 1; i++)
	{	//delete the cCapitalCommand and space
		command[i] = command[i + 2];
	}
	command[iLengthofCommand - 1] = '\0';
	command[iLengthofCommand - 2] = '\0';
	iLengthofCommand = strlen(command);

	if ('s' == cCapitalCommand)
	{
		fseek(fp, 0, SEEK_SET);
		if (0 != iLengthofCommand)	//s data
		{
			if (39 == command[0])	//string
			{
				for (int j = 0; j < iLengthofCommand - 1; j++)
				{
					command[j] = command[j + 1];
				}
				command[strlen(command) - 3] = '\0';	//delete the ' and \n
				SearchString(command);
			}
			else //binary
			{
				SearchBinary(command);
			}
		}
		else
		{
			printf("Invalid input because of no using search data!\n");
		}
	} 
	else if ('d' == cCapitalCommand)
	{
		for (int i = 0; i < iLengthofCommand - 1; i++)
		{
			if (command[i] < 48 || command[i] > 102)	//要能key 0-f 進去
			{
				printf("Invalid Input!\n");
				InputCommandLine();
			}
		}
		if (0 != strlen(command))	//d offset
		{
			char *cTmp;
			long lOffset = strtol(command, &cTmp, 16);
			if (lOffset > g_lFileSize)
			{
				printf("Dump size is overflow!\n");
				printf("The Max File address is %08X\n", g_lFileSize);
				InputCommandLine();
			}
			else
			{
				Print(lOffset);
			}
		}
		else //d
		{
			Print();
		}
	}
	else if ('f' == cCapitalCommand)
	{
		char* cOffset = NULL;
		char* cData = NULL;
		cOffset = strtok_s(command, " ", &cData);

		for (unsigned int i = 0; i < strlen(cOffset); i++)
		{
			if (cOffset[i] < 48 || cOffset[i] > 102)
			{
				printf("Invalid Input!\n");
				InputCommandLine();
			}
		}
		FillBinaryInData(cOffset, cData);
	}
	else
	{
		printf("Invalid Input!\n");
		InputCommandLine();
	}
}

void Print(long loffset)
{
	unsigned char cPrintChar[BYTES_PER_LINE + 1] = { NULL };
	long lQuotient = loffset / BYTES_PER_LINE;
	long lRemainder = loffset % BYTES_PER_LINE;
	
	//process the remainder offset ,ex: d 3f8
	fseek(fp, lQuotient*BYTES_PER_LINE, SEEK_SET);
	printf("%08X | ", ftell(fp));
	fread(&cPrintChar, sizeof(unsigned char), BYTES_PER_LINE, fp);
	for (int k = 0; k < BYTES_PER_LINE; k++)
	{
		if ((BYTES_PER_LINE / 2) == k)
		{
			printf("- ");
		}
		if (k < lRemainder)
		{
			printf("   ");
		}
		else
		{
			printf("%02X ", cPrintChar[k]);
		}
		if (false == isgraph(cPrintChar[k]))
		{
			cPrintChar[k] = 46;
		}
	}
	printf("| ");
	for (int u = 0; u < lRemainder; u++)
	{
		printf(" ");
	}
	printf("%s\n", &cPrintChar[lRemainder]);

	//normal print
	fseek(fp, (lQuotient + 1)*BYTES_PER_LINE, SEEK_SET);
	for (int i = 1; i < LINES_PER_PAGE + 1; i++)
	{
		printf("%08X | ", ftell(fp));
		//print the address
		fread(&cPrintChar, sizeof(unsigned char), BYTES_PER_LINE, fp);
		for (int j = 0; j < BYTES_PER_LINE; j++)
		{
			if ((BYTES_PER_LINE / 2) == j)
			{
				printf("- ");
			}
			printf("%02X ", cPrintChar[j]);
			//print byte data
			if (false == isgraph(cPrintChar[j]))
			{	//ignore the trash code
				cPrintChar[j] = 46;
			}
		}
		printf("| ");
		cPrintChar[BYTES_PER_LINE] = '\0';	//delete the trash code
		printf("%s\n", cPrintChar);
	}
	InputCommandLine();
}

void Print()
{
	g_lCurrOffset = ftell(fp);
	Print(g_lCurrOffset);
}

void PrintArgumentError()
{
	printf("Arguments aren't enough ! \n");
	printf("usage: fdump <filename> \n");
	printf("command:\n");
	printf("    d [offset]	dump from new offset (eg. d 3f8)\n");
	printf("    d				continue dump from current offset\n");
	printf("    f <offset> <data>	fill <data> from <offset>\n");
	printf("    s <data>		search <data> from current offset\n");
	printf("    s				continue search \n");
	system("pause");
}

void FillBinaryInData(char* offset ,char* data)
{
	char* cEnd;
	long lOffset = strtol(offset, &cEnd, 16);
	//transfer the  char array to the hex int
	if (lOffset > g_lFileSize)
	{
		printf("Filldata size is overflow!\n");
		printf("The Max File address is %08X\n", g_lFileSize);
		InputCommandLine();
	}

	fseek(fp, lOffset, SEEK_SET);
	if (39 == data[0])	//write the string ,and 39 is the (')	
	{
		//先切掉' '再寫入
		int iLength = strlen(data);
		for (int i = 0; i < iLength - 1; i++)
		{
			data[i] = data[i + 1];
		}
		data[iLength - 3] = '\0';
		fwrite(data, 1, strlen(data), fp);
	}
	else //	write the binary data
	{		
		char* cBufferBinaryData[COMMANDCHARS_CAPACITY] = { NULL };
		char* cTmp = NULL;
		int iNumbersOfAddress = 0;
		unsigned char iAddressOffset[COMMANDCHARS_CAPACITY] = {};

		cBufferBinaryData[0] = strtok_s(data, " ", &cTmp);
		while (1)
		{	
			iNumbersOfAddress++;
			cBufferBinaryData[iNumbersOfAddress] = strtok_s(NULL, " ", &cTmp);
			if (cBufferBinaryData[iNumbersOfAddress] == NULL)
			{
				break;
			}
		}
		for (int i = 0; i < iNumbersOfAddress; i++)
		{	//check the invalid input binary data
			if (2 != strlen(cBufferBinaryData[i]) && 3 != strlen(cBufferBinaryData[i]))
			{
				printf("Invalid Input Data!\n");
				InputCommandLine();
			}
		}
		for (int i = 0; i < iNumbersOfAddress; i++)
		{
			iAddressOffset[i] = strtol(cBufferBinaryData[i], &cEnd, 16);
			//transfer to the binary in file
		}
		fwrite(iAddressOffset, sizeof(char)*iNumbersOfAddress, 1, fp);
	}
	InputCommandLine();
}

void SearchString(char* data)
{
	char cCopyData[COMMANDCHARS_CAPACITY] = { NULL };
	int iLengthString = strlen(data);
	strcpy_s(cCopyData, iLengthString+1, data);

	Search((unsigned char*)cCopyData);
	InputCommand();

	if ('s' == g_cCapitalCommand && 2 == strlen(g_cCommand))
	{
		SearchString(cCopyData);
	}
	else
	{
		ProcessCommandLine(g_cCommand);
	}
}

void SearchBinary(char* data)
{
	char* cBufferBinaryData[COMMANDCHARS_CAPACITY] = { NULL };
	char cCopyData[COMMANDCHARS_CAPACITY] = { NULL };
	char cTmpData[COMMANDCHARS_CAPACITY] = { NULL };
	char* cTmp = NULL;
	char* cEnd = NULL;
	int iLengthData = strlen(data);
	int iNumbersOfData = 0;
	unsigned char cSearchBinary[COMMANDCHARS_CAPACITY] = {};
	strcpy_s(cTmpData, iLengthData + 1, data);
	strcpy_s(cCopyData, iLengthData + 1, data);

	cBufferBinaryData[0] = strtok_s(cTmpData, " ", &cTmp);
	cSearchBinary[0] = strtol(cBufferBinaryData[0], &cEnd, 16);

	while (1)
	{	
		iNumbersOfData++;
		cBufferBinaryData[iNumbersOfData] = strtok_s(NULL, " ", &cTmp);
		if (cBufferBinaryData[iNumbersOfData] == NULL)
		{
			break;
		}
		cSearchBinary[iNumbersOfData] = strtol(cBufferBinaryData[iNumbersOfData], &cEnd, 16);
	}

	Search(cSearchBinary);
	InputCommand();

	if ('s' == g_cCapitalCommand && 2 == strlen(g_cCommand))
	{
		SearchBinary(cCopyData);
	}
	else
	{
		ProcessCommandLine(g_cCommand);
	}
}

void Search(unsigned char* data)
{
	int iState = 0;
	int iResult = 0;
	int iLengthOfData = strlen((const char*)data);
	unsigned char cCompareChar;
	while (1)
	{
		if (feof(fp))
		{
			break;
		}
		if (iState == iLengthOfData)
			//find the same Data
		{
			g_lCurrOffset = ftell(fp);
			iResult = 1;
			printf("Found the offset %08X.\n", (g_lCurrOffset - iLengthOfData));
			break;
		}
		cCompareChar = getc(fp);
		if (cCompareChar == data[iState])
		{
			iState++;
		}
		else if (cCompareChar != data[iState])
		{
			iState = 0;
		}
	}
	if (0 == iResult)
	{
		printf("Not Found.\n");
	}
}

int main(int argc, char *argv[])
{
	errno_t iRet;
	if (argc < 2)
		// if not input the filename or any error
	{
		PrintArgumentError();
		return 0;
	}
	iRet = fopen_s(&fp, argv[1], "rb+");
		//rb+ : binary reading and writing
		//open the file
	if (NULL == fp)
	{
		//open file fail 
		printf("File %s is not found!\n", argv[1]);
		system("pause");
		return 0;
	}
	fseek(fp, 0, SEEK_END);
	g_lFileSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	printf("The File name is %s and the File Size is %d.\n", argv[1], g_lFileSize);

	InputCommandLine();
	
	fclose(fp);
	system("pause");
	return 0;
}