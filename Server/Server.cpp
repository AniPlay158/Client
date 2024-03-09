#define WIN32_LEAN_AND_MEAN
#include <iostream>
#include <windows.h>
#include <ws2tcpip.h>
using namespace std;

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

SOCKET ClientSocket = INVALID_SOCKET;

int smileyX = 5;
int smileyY = 5;

DWORD WINAPI Sender(void* param) {
    while (true) {
        char answer[200];
        cin.getline(answer, 199);
        int iSendResult = send(ClientSocket, answer, strlen(answer), 0);

        if (iSendResult == SOCKET_ERROR) {
            cout << "send завершился с ошибкой: " << WSAGetLastError() << "\n";
            cout << "упс, отправка (send) ответного сообщения не состоялась ((\n";
            closesocket(ClientSocket);
            WSACleanup();
            return 7;
        }
    }


    return 0;
}

DWORD WINAPI Receiver(void* param) {
    while (true) {
        char message[DEFAULT_BUFLEN];
        int iResult = recv(ClientSocket, message, DEFAULT_BUFLEN, 0);
        message[iResult] = '\0';

        if (iResult > 0) {
            char direction = message[0];

            switch (direction) {
            case 'a': // Влево
                if (smileyX > 0)
                    --smileyX;
                break;
            case 'd': // Вправо
                ++smileyX;
                break;
            case 'w': // Вверх
                if (smileyY > 0)
                    --smileyY;
                break;
            case 's': // Вниз
                ++smileyY;
                break;
            }

            system("cls");
            for (int i = 0; i < smileyY; ++i)
                cout << "\n";

            for (int i = 0; i < smileyX; ++i)
                cout << " ";

            cout << ":-)";
            cout.flush();
            Sleep(10);
        }
    }
    return 0;
}

int main() {
    setlocale(0, "");
    system("title СЕРВЕР");

    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        cout << "WSAStartup завершился с ошибкой: " << iResult << "\n";
        cout << "подключение Winsock.dll прошло с ошибкой!\n";
        return 1;
    }

    struct addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo* result = NULL;
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        cout << "getaddrinfo завершился с ошибкой: " << iResult << "\n";
        cout << "получение адреса и порта сервера прошло c ошибкой!\n";
        WSACleanup();
        return 2;
    }

    SOCKET ListenSocket = INVALID_SOCKET;
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        cout << "socket завершился с ошибкой: " << WSAGetLastError() << "\n";
        cout << "создание сокета прошло c ошибкой!\n";
        freeaddrinfo(result);
        WSACleanup();
        return 3;
    }

    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        cout << "bind завершился с ошибкой: " << WSAGetLastError() << "\n";
        cout << "внедрение сокета по IP-адресу прошло с ошибкой!\n";
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 4;
    }

    freeaddrinfo(result);

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        cout << "listen завершился с ошибкой: " << WSAGetLastError() << "\n";
        cout << "прослушивание информации от клиента не началось. что-то пошло не так!\n";
        closesocket(ListenSocket);
        WSACleanup();
        return 5;
    }
    else {
        cout << "Пожалуйста, запустите client.exe\n";
    }

    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
        cout << "accept завершился с ошибкой: " << WSAGetLastError() << "\n";
        cout << "соединение с клиентским приложением не установлено! печаль!\n";
        closesocket(ListenSocket);
        WSACleanup();
        return 6;
    }

    CreateThread(0, 0, Receiver, 0, 0, 0);
    CreateThread(0, 0, Sender, 0, 0, 0);

    Sleep(INFINITE);

    return 0;
}