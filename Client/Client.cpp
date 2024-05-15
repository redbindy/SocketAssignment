#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <string>
#include <fstream>
#include <assert.h>
#include <stdio.h>

// 파일 전송
// 클라이언트 -> 서버
// 입력: 상대방의 IP(v4) 주소, 포트번호, 파일의 이름

enum
{
	BUFFER_SIZE = 4096
};

typedef int error_code_t;

#ifdef _WIN32 || _WIN64
#include <WinSock2.h>
#pragma comment (lib, "ws2_32.lib")

#include <WS2tcpip.h>
#include <Windows.h>

bool socketWindows()
{
	std::cout << "Input Format: IP Port FileName" << std::endl
		<< "> ";

	std::string ip;
	std::cin >> ip;

	// ip 형식 검사
	{
		if (ip.size() > 15)
		{
			goto INVALID_IP_INPUT;
		}

		unsigned int numbers[4];
		char dots[3];

		if (sscanf(ip.c_str(), "%d %c %d %c %d %c %d", numbers, dots, numbers + 1, dots + 1, numbers + 2, dots + 2, numbers + 3) != 7)
		{
			goto INVALID_IP_INPUT;
		}

		int i;
		for (i = 0; i < 2; ++i)
		{
			if (numbers[i] > 255 || dots[i] != '.')
			{
				goto INVALID_IP_INPUT;
			}
		}

		if (numbers[i] > 255)
		{
			goto INVALID_IP_INPUT;
		}

		goto GETTING_PORT;

	INVALID_IP_INPUT:
		std::cerr << "IP Format Wrong" << std::endl;

		std::cin.ignore(LLONG_MAX, '\n');
		std::cin.clear();

		return false;
	}

GETTING_PORT:
	unsigned short port;
	std::cin >> port;

	std::string fileName;
	std::cin >> fileName;

	std::cin.ignore(LLONG_MAX, '\n');
	std::cin.clear();

	std::ifstream fin;

	fin.open(fileName, std::ios_base::in);
	{
		if (!fin.is_open())
		{
			std::cerr << "File Open Failed" << std::endl;

			return false;
		}
	}
	fin.close();

	WSADATA wsaData;

	std::cout << "WSAStartup..." << std::endl;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (result != 0)
	{
		std::cerr << "WSAStartup Failed: Code - " << WSAGetLastError() << std::endl;

		return false;
	}

	std::cout << "socket..." << std::endl;
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);

	if (sock == INVALID_SOCKET)
	{
		std::cerr << "Invalid Socket: Code - " << WSAGetLastError() << std::endl;

		WSACleanup();

		return false;
	}

	sockaddr_in hint;
	ZeroMemory(&hint, sizeof(hint));

	hint.sin_family = AF_INET;
	hint.sin_port = htons(port);

	std::cout << "convert ip..." << std::endl;
	result = inet_pton(AF_INET, ip.c_str(), &hint.sin_addr);

	if (result != 1)
	{
		std::cerr << "convert ip address Failed: Code - " << WSAGetLastError() << std::endl;

		WSACleanup();

		return false;
	}

	std::cout << "connect..." << std::endl;
	result = connect(sock, reinterpret_cast<sockaddr*>(&hint), sizeof(hint));

	if (result == SOCKET_ERROR)
	{
		std::cerr << "connect Failed: Code - " << WSAGetLastError() << std::endl;

		closesocket(sock);
		WSACleanup();

		return false;
	}

	// 널 문자 자리 확보
	char buffer[BUFFER_SIZE + 1];
	std::string line;

	fin.open(fileName, std::ios_base::in);
	{
		std::cout << "client ready" << std::endl;

		while (!fin.eof())
		{
			std::getline(fin, line);

			const char* sendingStr = line.c_str();
			size_t sendingLength = line.size() + 1;

			while (sendingLength > BUFFER_SIZE)
			{
				result = send(sock, sendingStr, BUFFER_SIZE, 0);
				if (result != SOCKET_ERROR)
				{
					ZeroMemory(buffer, BUFFER_SIZE + 1);

					int bytesReceived = recv(sock, buffer, BUFFER_SIZE, 0);
					if (bytesReceived > 0)
					{
						std::cout << "server> " << buffer << std::endl;
					}
				}

				sendingStr += BUFFER_SIZE;
				sendingLength -= BUFFER_SIZE;
			}

			result = send(sock, sendingStr, sendingLength, 0);
			if (result != SOCKET_ERROR)
			{
				ZeroMemory(buffer, BUFFER_SIZE + 1);

				int bytesReceived = recv(sock, buffer, BUFFER_SIZE, 0);
				if (bytesReceived > 0)
				{
					std::cout << "server> " << buffer << std::endl;
				}
			}
		}
	}
	fin.close();

	closesocket(sock);
	WSACleanup();

	return true;
}

#endif

#ifdef __linux__

void socketLinux()
{

}

#endif

int main()
{
	while (true)
	{
		bool bSuccess;

#ifdef _WIN32 || _WIN64
		bSuccess = socketWindows();
#endif

#ifdef __linux__
		bSuccess = socketLinux();
#endif
		if (bSuccess)
		{
			std::cout << "File Transfer End" << std::endl;
			break;
		}
	}
}
