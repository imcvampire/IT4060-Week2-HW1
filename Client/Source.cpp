#include<WinSock2.h>
#include<WS2tcpip.h>
#include<iostream>
#include<string>
#include<cstdbool>

using namespace std;

const char SERVER_ADDRESS[] = "127.0.0.1";
const int SERVER_PORT = 5000;
const int BUFFER_SIZE = 2048;
const char MY_ERROR[] = "Error!";

int main(int argc, char* argv[])
{
	if (argc < 5)
	{
		cerr << "Missing arguments!" << endl;

		return 1;
	}

	WSADATA wsa_data;
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data))
	{
		cerr << "Version is not supported!" << endl;
	}

	SOCKET client;
	client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	int timeout = 10000;
	setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (const char *) &timeout, sizeof(int));

	sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons((u_short) SERVER_PORT);
	if (inet_pton(AF_INET, SERVER_ADDRESS, (void*) &(server_addr.sin_addr.s_addr)) != 1)
	{
		cerr << "Can not convert little-endian to big-endian" << endl;

		return 1;
	}

	for (int i = 1; i < argc; ++i)
	{
		if (argv[i][0] != '-') continue;
		
		switch(argv[i][1]) 
		{
			case 'a':
				if (inet_pton(AF_INET, argv[i + 1], (void*) &(server_addr.sin_addr.s_addr)) != 1)
				{
					cerr << "Can not convert little-endian to big-endian" << endl;

					return 1;
				}

				break;

			case 'p':
				server_addr.sin_port = htons((u_short) stoi(argv[i + 1]));
				break;
		}
	}

	if (connect(client, (sockaddr *) &server_addr, sizeof(server_addr)))
	{
		cerr << "Error! Can not connect to server! Error: " << WSAGetLastError() << endl;

		return 1;
	}

	cout << "Connected!" << endl;

	char buffer[BUFFER_SIZE];
	int ret;

	cout << "Send to server: " << endl;
	cin >> buffer;

	ret = send(client, buffer, strlen(buffer), 0);
	if (ret == SOCKET_ERROR)
	{
		cerr << "Error! Can not send message." << endl;

		return 1;
	}

	int ret1, ret2;
	char buffer1[BUFFER_SIZE], buffer2[BUFFER_SIZE];

	ret1 = recv(client, buffer1, BUFFER_SIZE, 0);
	ret2 = recv(client, buffer2, BUFFER_SIZE, 0);
	if (ret1 == SOCKET_ERROR || ret2 == SOCKET_ERROR)
	{
		if (WSAGetLastError() == WSAETIMEDOUT)
		{
			cerr << "Timeout!" << endl;
		}
		else
		{
			cerr << "Error: " << WSAGetLastError() << endl;
		}
	}
	else if (ret1 > 0 || ret2 > 0)
	{
		if (strcmp(buffer1, MY_ERROR) == 0)
		{
			cerr << "Error: Your request contains none alphabetic characters or numbers!" << endl;

			return 1;
		}

		buffer1[ret1] = 0;
		buffer2[ret2] = 0;

		cout
			<< "Numbers: " << buffer1 
			<< "\n"
			<< "Characters: " << buffer2
			<< endl;
	}

	shutdown(client, SD_SEND);
	closesocket(client);

	WSACleanup();

	return 0;
}