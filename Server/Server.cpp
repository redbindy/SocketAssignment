#include <iostream>
#include <fstream>
#include <stdio.h>

// 파일 전송
// 클라이언트 -> 서버
// 서버는 파일 받아서 저장

enum
{
	SERVER_PORT = 10005,

	BUFFER_SIZE = 4096
};

#if defined(_WIN32) || defined(_WIN64)
#include <WinSock2.h>
#pragma comment (lib, "ws2_32.lib")

#include <WS2tcpip.h>
#include <Windows.h>

bool socketWindows()
{
	WSADATA wsaData;

	std::cout << "WSAStartup..." << std::endl;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (result != 0)
	{
		std::cerr << "WSAStartup Failed: Code - " << WSAGetLastError() << std::endl;

		return false;
	}

	std::cout << "socket..." << std::endl;
	SOCKET listeningSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (listeningSock == INVALID_SOCKET)
	{
		std::cerr << "Invalid Socket: Code - " << WSAGetLastError() << std::endl;

		WSACleanup();

		return false;
	}

	sockaddr_in hint;
	ZeroMemory(&hint, sizeof(hint));

	hint.sin_family = AF_INET;
	hint.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	hint.sin_port = htons(SERVER_PORT);

	std::cout << "bind..." << std::endl;
	result = bind(listeningSock, reinterpret_cast<sockaddr*>(&hint), sizeof(hint));

	if (result == SOCKET_ERROR)
	{
		std::cerr << "Bind Failed: Code - " << WSAGetLastError() << std::endl;

		closesocket(listeningSock);
		WSACleanup();

		return false;
	}

	std::cout << "listen..." << std::endl;
	result = listen(listeningSock, SOMAXCONN);

	if (result == SOCKET_ERROR)
	{
		std::cerr << "listen Failed: Code - " << WSAGetLastError() << std::endl;

		closesocket(listeningSock);
		WSACleanup();

		return false;
	}

	sockaddr_in clientSocketInfo;
	int clientSize = sizeof(clientSocketInfo);

	std::cout << "accept..." << std::endl;
	SOCKET clientSocket = accept(listeningSock, reinterpret_cast<sockaddr*>(&clientSocketInfo), &clientSize);

	if (clientSocket == INVALID_SOCKET)
	{
		std::cerr << "accept Failed: Code - " << WSAGetLastError() << std::endl;

		closesocket(listeningSock);
		WSACleanup();

		return false;
	}

	// 1:1이라 미리 닫기
	closesocket(listeningSock);

	// 널 문자 자리 확보
	char buffer[BUFFER_SIZE + 1] = { 0, };

	std::ofstream fout;
	fout.open("file.txt", std::ios_base::out | std::ios_base::binary);
	{
		std::cout << "server ready..." << std::endl;
		while (true)
		{
			int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);

			if (bytesReceived == SOCKET_ERROR)
			{
				std::cerr << "recv Failed: Code - " << WSAGetLastError() << std::endl;
				break;
			}

			if (bytesReceived == 0)
			{
				std::cout << "Client disconnected" << std::endl;
				break;
			}

			size_t length = 0;
			for (char c : buffer)
			{
				if (c == '\0')
				{
					break;
				}

				++length;
			}

			if (length == BUFFER_SIZE)
			{
				fout << buffer;
			}
			else
			{
				fout << buffer << std::endl;
			}

			std::cout << buffer << std::endl;
			send(clientSocket, buffer, bytesReceived + 1, 0);
		}
	}
	fout.close();

	closesocket(clientSocket);
	WSACleanup();

	return true;
}

#endif

#ifdef __linux__
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

bool socketLinux()
{
	std::cout << "socket..." << std::endl;
	int listeningSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (listeningSock < 0)
	{
		std::cerr << "Invalid Socket: Code - " << errno << std::endl;

		return false;
	}

	sockaddr_in hint;
	memset(&hint, 0, sizeof(hint));

	hint.sin_family = AF_INET;
	hint.sin_addr.s_addr = htonl(INADDR_ANY);
	hint.sin_port = htons(SERVER_PORT);

	std::cout << "bind..." << std::endl;
	int result = bind(listeningSock, reinterpret_cast<sockaddr*>(&hint), sizeof(hint));

	if (result < 0)
	{
		std::cerr << "Bind Failed: Code - " << errno << std::endl;

		close(listeningSock);

		return false;
	}

	std::cout << "listen..." << std::endl;
	result = listen(listeningSock, SOMAXCONN);

	if (result < 0)
	{
		std::cerr << "listen Failed: Code - " << errno << std::endl;

		close(listeningSock);

		return false;
	}

	sockaddr_in clientSocketInfo;
	socklen_t clientSize = sizeof(clientSocketInfo);

	std::cout << "accept..." << std::endl;
	int clientSocket = accept(listeningSock, reinterpret_cast<sockaddr*>(&clientSocketInfo), &clientSize);

	if (clientSocket < 0)
	{
		std::cerr << "accept Failed: Code - " << errno << std::endl;

		close(listeningSock);

		return false;
	}

	// 1:1이라 미리 닫기
	close(listeningSock);

	// 널 문자 자리 확보
	char buffer[BUFFER_SIZE + 1] = { 0, };

	std::ofstream fout;
	fout.open("file.txt", std::ios_base::out | std::ios_base::binary);
	{
		std::cout << "server ready..." << std::endl;
		while (true)
		{
			int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);

			if (bytesReceived < 0)
			{
				std::cerr << "recv Failed: Code - " << errno << std::endl;
				break;
			}

			if (bytesReceived == 0)
			{
				std::cout << "Client disconnected" << std::endl;
				break;
			}

			size_t length = 0;
			for (char c : buffer)
			{
				if (c == '\0')
				{
					break;
				}

				++length;
			}

			if (length == BUFFER_SIZE)
			{
				fout << buffer;
			}
			else
			{
				fout << buffer << std::endl;
			}

			std::cout << buffer << std::endl;
			send(clientSocket, buffer, bytesReceived + 1, 0);
		}
	}
	fout.close();

	close(clientSocket);

	return true;
}

#endif

int main()
{
	while (true)
	{
		bool bSuccess;

#if defined(_WIN32) || defined(_WIN64)
		bSuccess = socketWindows();
#endif

#ifdef __linux__
		bSuccess = socketLinux();
#endif
		if (bSuccess)
		{
			std::cout << std::endl << "File Receive End" << std::endl;
			break;
		}
	}

	std::cout << std::endl << "Press any key to exit" << std::endl;
	getchar();

	return 0;
}
