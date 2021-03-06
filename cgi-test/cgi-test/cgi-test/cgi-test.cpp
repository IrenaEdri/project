// cgi-test.cpp : Defines the entry point for the console application.
//
//#define WIN32_LEAN_AND_MEAN
#include "stdafx.h"
#include <string.h>

#include <windows.h>
//#include <winsock2.h>
//#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

// Helper macro to convert two-character hex strings to character value
#define ToHex(Y) (Y>='0'&&Y<='9'?Y-'0':Y-'A'+10)

char InputData[4096];

extern int TakeScreenShot(unsigned int id);

/**
 * Convert human readable IPv4 address to UINT32
 * @param pDottedQuad   Input C string e.g. "192.168.0.1"
 * @param pIpAddr       Output IP address as UINT32
 * return 1 on success, else 0
 */
int ipStringToNumber(const char*       pDottedQuad,
	unsigned int *    pIpAddr)
{
	unsigned int            byte3;
	unsigned int            byte2;
	unsigned int            byte1;
	unsigned int            byte0;
	char              dummyString[2];

	/* The dummy string with specifier %1s searches for a non-whitespace char
	 * after the last number. If it is found, the result of sscanf will be 5
	 * instead of 4, indicating an erroneous format of the ip-address.
	 */
	if (sscanf(pDottedQuad, "%u.%u.%u.%u%1s",
		&byte3, &byte2, &byte1, &byte0, dummyString) == 4)
	{
		if ((byte3 < 256)
			&& (byte2 < 256)
			&& (byte1 < 256)
			&& (byte0 < 256)
			)
		{
			*pIpAddr = (byte0 << 24)
				+ (byte1 << 16)
				+ (byte2 << 8)
				+ byte3;

			return 1;
		}
	}

	return 0;
}

void getAllParams() {
	// Determing if it is a POST or GET method
	if (getenv("REQUEST_METHOD") == 0) {
		printf("No REQUEST_METHOD, must be running in DOS mode");
		return;
	}
	else if (strcmp(getenv("REQUEST_METHOD"), "POST") == 0) {
		// If POST 
		char *endptr;	// quite useless, but required 
		char *len1 = getenv("CONTENT_LENGTH");
		int contentlength = strtol(len1, &endptr, 10);
		fread(InputData, contentlength, 1, stdin);
	}
	else {
		// If GET 
		strcpy(InputData, getenv("QUERY_STRING"));
	}

}

void getParam(const char *Name, char *Value) {
	char *pos1 = strstr(InputData, Name);

	if (pos1) {
		pos1 += strlen(Name);

		if (*pos1 == '=') { // Make sure there is an '=' where we expect it 
			pos1++;

			while (*pos1 && *pos1 != '&') {
				if (*pos1 == '%') { // Convert it to a single ASCII character and store at our Valueination 
					*Value++ = (char)ToHex(pos1[1]) * 16 + ToHex(pos1[2]);
					pos1 += 3;
				}
				else if (*pos1 == '+') { // If it's a '+', store a space at our Valueination 
					*Value++ = ' ';
					pos1++;
				}
				else {
					*Value++ = *pos1++; // Otherwise, just store the character at our Valueination 
				}
			}

			*Value++ = '\0';
			return;
		}

	}

	//strcpy(Value, "undefine");	// If param not found, then use default parameter
	Value[0] = 0;
	return;
}



char buff[10000] = {0};
char source_site[1000] = {0};
char source_ip[20] = {0};
char protocol[20] = { 0 };
char destination_ip[20] = {0};
char filename[260] = {0};
char username[100] = { 0 };
char password[100] = { 0 };

char Ws_Buff[64 * 1024 * 1024];
int main(int argc, char *argv[])
{
	
	int first_time = 1;
	FILE *fp;
	

	fp = fopen("C:/Work/Ws/first_time", "rb");
	if (fp)
	{
		first_time = 0;
		fclose(fp);
	}
	
	getAllParams();

	getParam("username", username);
	getParam("password", password);
	getParam("destination_ip", destination_ip);
	getParam("protocol", protocol);
	getParam("source_site", source_site);
	getParam("source_ip", source_ip);
	getParam("destination_ip", destination_ip);
	getParam("protocol", protocol);

	if (username[0] && password[0])
	{

		

		int found = 0;
		char temp_username[100] = { 0 }, temp_password[100] = { 0 };
		fp = fopen("C:/Work/Ws/users_database.txt", "rt");
		if (fp)
		{
			while (!feof(fp))
			{
				memset(temp_username, 0, sizeof(temp_username));
				memset(temp_username, 0, sizeof(temp_password));
				fscanf(fp, "%s", temp_username);
				fscanf(fp, "%s", temp_password);
				if (!strcmp(temp_username, username)
					&&
					!strcmp(temp_password, password))
				{
					// found user
					found = 1;
					//printf("Content-Type: text/html\n\n");
					//printf("<br>Found<br>");
					break;
				}
			}
			if (!found)
			{
				
				printf("Content-Type: text/html\n\n");
				printf("<br>Login Failed<br>");
				printf("<br>");
				printf("username: %s", username);
				printf("<br>");
				printf("password: %s", password);
				printf("<br>");
				printf("temp_username: %s", temp_username);
				printf("<br>");
				printf("temp_password: %s", temp_password);
				printf("<br>");
				username[0] = password[0] = 0;
				_unlink("C:/Work/Ws/first_time");
				
			}
		}
	}

	if (
		(!username[0] || 
		!password[0]) &&
		!source_site[0] &&
		!source_ip[0] &&
		!destination_ip[0] &&
		!protocol[0])
	{
		_unlink("C:/Work/Ws/first_time");
		
		fp = fopen("C:/Work/Ws/jsource/login.html", "rb");
		fread(buff, 1, sizeof(buff), fp);
		fclose(fp);

		printf("Content-Type: text/html\n\n");

		printf("%s", buff);
	}
		
	if (first_time &&
		!source_site[0] &&
		!source_ip[0] &&
		!destination_ip[0] &&
		!protocol[0] &&
		username[0] &&
		password[0])
	{


		fp = fopen("C:/Work/Ws/first_time", "wb");
		fwrite(username, 1, 1, fp);
		fclose(fp);


		const char* exec_ws_command = "C:/Work/Ws/jsource/WsExec.bat";

		WinExec(exec_ws_command, SW_FORCEMINIMIZE);
	}

	// Initial screen
	if(
		!source_site[0] &&
		!source_ip[0] && 
		!destination_ip[0] && 
		!protocol[0] && 
		username[0] && 
		password[0])
	{
	

		memset(buff, 0, sizeof(buff));

		fp = fopen("C:/Work/Ws/jsource/initial_snap.html", "rb");
		fread(buff, 1, sizeof(buff), fp);
		fclose(fp);

		printf("Content-Type: text/html\n\n");
		
		printf("%s", buff);
	
		getAllParams();

		getParam("username", username);
		getParam("password", password);
		getParam("destination_ip", destination_ip);
		getParam("protocol", protocol);
		getParam("source_site", source_site);
		getParam("source_ip", source_ip);
		getParam("destination_ip", destination_ip);
		getParam("protocol", protocol);



		
	}


	if (source_site[0] 
		|| source_ip[0] 
		|| destination_ip[0] 
		|| protocol[0]
		&&
		username[0] &&
		password[0])
	{
		
		if (destination_ip[0])
		{
			unsigned int destination_ip_uint32=0;
			unsigned int source_ip_uint32=0;
			printf("Content-Type: text/html\n\n");
			printf("dest_ip=%s<br>", destination_ip);
			ipStringToNumber(destination_ip, &destination_ip_uint32);
			ipStringToNumber(source_ip, &source_ip_uint32);
			if (destination_ip_uint32
				||
				source_ip_uint32)
			{
			//	printf("%08X\n", destination_ip_uint32);
			//	char destination_ip_hex[16];
			//	int i;
			//	_itoa(destination_ip_uint32, destination_ip_hex, 16);
			//	for (i = 0; i < sizeof(destination_ip_hex); i++)
			//		destination_ip_hex[i] = toupper(destination_ip_hex[i]);

			//	printf("destination_ip_hex = %s\n", destination_ip_hex);

				fp = fopen("C:/Work/Ws/wireshark.out", "rb");
				
				//printf("fp=%d\n", fp);
				if (fp)
				{
					memset(Ws_Buff, 0, sizeof(Ws_Buff));
					int bytes_read = fread(Ws_Buff, 1, sizeof(Ws_Buff), fp);
					fclose(fp);
					printf("Wireshark capture size=%d bytes<br>", bytes_read);
					char *ptrToWireSharkBuff = Ws_Buff;
					int destination_ip_count = 0;
					int source_ip_count = 0;
					while (ptrToWireSharkBuff < Ws_Buff + bytes_read)
					{
						if(destination_ip_uint32 == *(unsigned int*)ptrToWireSharkBuff)
							destination_ip_count++;
						if (source_ip_uint32 == *(unsigned int*)ptrToWireSharkBuff)
							source_ip_count++;
						ptrToWireSharkBuff++;
					}
					printf("Found %d source IP occurances<br>", source_ip_count);
					printf("Found %d destination IP occurances<br>", destination_ip_count);

				}
			}
		}
	}
	
	//printf("QueryString: %s", InputData);

	//printf("Content-Type: text/html\n\n");
	printf("<br>");
	printf("username: %s", username);
	printf("<br>");
	printf("password: %s", password);
	printf("<br>");
	printf("source_site: %s", source_site);
	printf("<br>");
	printf("source_ip: %s", source_ip);
	printf("<br>");
	printf("destination_ip: %s", destination_ip);
	printf("<br>");
	printf("protocol: %s", protocol);
	printf("<br>");
	printf("<br>");
	
	
	return 0;
	
	
}