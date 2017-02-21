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
	if (argc < 3)
	{
		cerr << "Missing arguments!" << endl;

		return 1;
	}

	WSADATA	wsa_data;
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data))
	{
		cerr << "Version is not supported!" << endl;
	}

	SOCKET listen_socket;
	listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons((u_short) SERVER_PORT);

	if (inet_pton(AF_INET, SERVER_ADDRESS, (void*) &(server_addr.sin_addr.s_addr)) != 1)
	{
		cerr << "Can not convert little-endian to big-endian" << endl;

		return 1;
	}

	// Start from first argument
	for (int i = 1; i <= argc && argv[i][0] == '-'; ++i)
	{
		if (argv[i][1] == 'p')
		{
			server_addr.sin_port = htons((u_short) stoi(argv[i + 1]));

			break;
		}
	}

	if (bind(listen_socket, (sockaddr *) &server_addr, sizeof(server_addr)))
	{
		cerr << "Can not bind to this address!" << endl;
		
		return 1;
	}

	if (listen(listen_socket, 10))
	{
		cerr << "Can not not listen!" << endl;

		return 1;
	}

	cout << "Server started!" << endl;

	sockaddr_in client_addr;
	char buffer[BUFFER_SIZE];
	int ret;
	int client_addr_len = sizeof(client_addr);

	for (;;)
	{
		SOCKET connect_socket = accept(listen_socket, (sockaddr *) &client_addr, &client_addr_len);

		ret = recv(connect_socket, buffer, BUFFER_SIZE, 0);

		if (ret == SOCKET_ERROR)
		{
			cerr << "Error: " << WSAGetLastError() << endl;
		}
		else if (ret > 0)
		{
			string result_char;
			string result_int;
			boolean is_error = false;

			for (int i = 0; i < ret; ++i)
			{
				if (buffer[i] >= 48 && buffer[i] <= 57)
				{
					result_int.push_back(buffer[i]);
				}
				else if (
					(buffer[i] >= 65 && buffer[i] <= 90)
					|| (buffer[i] >= 97 && buffer[i] <= 122)
				)
				{
					result_char.push_back(buffer[i]);
				}
				else
				{
					is_error = true;
					break;
				}
			}

			int ret1, ret2;
			if (is_error)
			{
				ret = send(connect_socket, MY_ERROR, strlen(MY_ERROR), 0);
			}
			else
			{
				const char *result_int_char = result_int.c_str();
				const char *result_char_char = result_char.c_str();

				ret1 = send(connect_socket, result_int_char, strlen(result_int_char), 0);
				ret2 = send(connect_socket, result_char_char, strlen(result_char_char), 0);
			}

			if (
				ret == SOCKET_ERROR
				|| ret1 == SOCKET_ERROR
				|| ret2 == SOCKET_ERROR
			)
			{
				cerr << "Error: " << WSAGetLastError() << endl;
			}

			shutdown(connect_socket, SD_SEND);
			closesocket(connect_socket);
		}

		closesocket(listen_socket);

		WSACleanup();

		return 0;
	}
}
