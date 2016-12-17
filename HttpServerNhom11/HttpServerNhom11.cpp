// Bai3HttpServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include<WinSock2.h>
#include<time.h>
#include<string.h>
#define MAX_EVENTS 64


int saveLog(char* ip) {
	time_t rawtime;
	time(&rawtime);
	char *currentTime = ctime(&rawtime);
	char line[64];
	sprintf(line, "%s %s\n", ip, currentTime);
	FILE *listConnectedFile = fopen("ListConnected.txt", "a");
	if (listConnectedFile == NULL) {
		printf("Save log failed !");
		return 0;
	}
	fputs(line, listConnectedFile);
	fclose(listConnectedFile);
	return 1;
}

void executeListConnected() {
	FILE * listConnectedFile = fopen("ListConnected.txt", "rb");
	if (listConnectedFile == NULL) {
		printf("File error, execute failed !\n");
		return;
	}
	fseek(listConnectedFile, 0, SEEK_END);
	int size = ftell(listConnectedFile);
	fseek(listConnectedFile, 0, SEEK_SET);
	char line[64];
	while (ftell(listConnectedFile) < size) {
		fgets(line, 64, listConnectedFile);
		printf("%s", line);
	}
}

DWORD WINAPI executeThread(LPVOID param) {
	char command[64];
	while (1) {
		fgets(command, 64, stdin);
		command[strlen(command) - 1] = 0;
		if (strcmp(command, "list connect") == 0) {
			executeListConnected();
		}
	}
	return 0;
}

int main()
{
	SOCKET socketArray[MAX_EVENTS];
	WSAEVENT eventArray[MAX_EVENTS], newEvent;
	SOCKET sockAccept;
	DWORD eventTotal = 0;
	DWORD index, i;
	// khoi tao winsock
	WSAData wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	// khoi tao socket va dia chi
	SOCKET serverSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	SOCKADDR_IN serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(80);
	serverAddr.sin_addr.s_addr = INADDR_ANY;

	// bind
	int ret = bind(serverSock, (sockaddr *)&serverAddr, sizeof(serverAddr));
	if (ret == SOCKET_ERROR) {
		printf("Loi khi thiet lap IP va PORT");
		WSAGetLastError();
		closesocket(serverSock);
		return 1;
	}
	// khoi tao su kien va gan vao socket server
	newEvent = WSACreateEvent();
	WSAEventSelect(serverSock, newEvent, FD_ACCEPT | FD_CLOSE);
	// listen
	listen(serverSock, 10);

	WSANETWORKEVENTS networkEvent;
	socketArray[eventTotal] = serverSock;
	eventArray[eventTotal] = newEvent;
	
	eventTotal++;

	char headerOk[] = "HTTP/1.1 200 OK\r\n"
		"Server: Http Server\r\n"
		"Date: Sat, 3 November 2016\r\n"
		"Content-Type: text/html\r\n"
		"Accept-Ranges: bytes\r\n"
		"Content-Length: 300\r\n"
		"\r\n";

	char headerRedirect[] = "HTTP/1.1 302 Found\r\n"
		"Server: Http Server\r\n"
		"Date: Sat, 3 November 2016\r\n"
		"Content-Type: text/html\r\n"
		"Accept-Ranges: bytes\r\n"
		"Content-Length: 116\r\n"
		"Location: http://localhost\r\n"
		"\r\n";

	char htmlWellcome[] = "<html>"
		"<head><title>DEMO HTTP SERVER</title></head> \r\n"
		"<body>\r\n"
		"<h1>Hello, Wellcome !</h1>\r\n"
		"<br>\r\n"
		"</body>\r\n"
		"</html>\r\n\n";
	char htmlNoContent[] = "<html>"
		"<head><title>DEMO HTTP SERVER</title></head> \r\n"
		"<body>\r\n"
		"<h1>Not exist !</h1>\r\n"
		"<br>\r\n"
		"</body>\r\n"
		"</html>\r\n\n";

	char htmlForm[] = "<html>"
		"<head><title>DEMO HTTP SERVER</title></head> \r\n"
		"<body>\r\n"
		"<form method=\"POST\">\r\n"
		"<p>User Name : <input type = \"text\" name = \"T1\" size = \"20\"> </p>\r\n"
		"<p>Password : <input type = \"password\" name = \"T2\" size = \"20\"></p>\r\n"
		"<input type = \"submit\" value = \"Login\" name = \"B1\">\n"
		"</form>\r\n"
		"</body>\r\n"
		"</html>\r\n\n";
	char buffRecv[1024] = { 0 };

	CreateThread(0, 0, executeThread, 0, 0, 0);

	while (1) {
		index = WSAWaitForMultipleEvents(eventTotal, eventArray, false, WSA_INFINITE, false);

		index = index - WSA_WAIT_EVENT_0;


		for (i = index; i < eventTotal; i++) {
			index = WSAWaitForMultipleEvents(1, &eventArray[i], true, 1000, false);
			if ((index == WSA_WAIT_FAILED) || (index == WSA_WAIT_TIMEOUT)) {
				continue;
			}
			else {
				index = i;
				// chuyen doi tuong su kien ve trang thai chua bao hieu
				WSAResetEvent(eventArray[index]);
				WSAEnumNetworkEvents(socketArray[index], eventArray[index], &networkEvent);
				//kiem tra xem co user nao ket noi khong
				if (networkEvent.lNetworkEvents&FD_ACCEPT) {
					if (networkEvent.iErrorCode[FD_ACCEPT_BIT] != 0) {
						printf("Loi accept voi ma loi %d\n", networkEvent.iErrorCode[FD_ACCEPT]);
						break;
					}
					//chap nhan ket noi, cho vao danh sach socket va su kien
					SOCKADDR_IN clientAddrTemp;
					int clientAddrLen = sizeof(clientAddrTemp);
					sockAccept = accept(socketArray[index], (sockaddr *)&clientAddrTemp, &clientAddrLen);
					/*for (int j = 0; j < eventTotal; j++) {
						if (sockAccept == socketArray[j]) {
							continue;
						}
					}*/
					if (eventTotal > WSA_MAXIMUM_WAIT_EVENTS) {
						printf("Too many connections");
						break;
					}
					// kiem tra neu ip trung thi khong accept
					
						saveLog(inet_ntoa(clientAddrTemp.sin_addr));
						newEvent = WSACreateEvent();
						WSAEventSelect(sockAccept, newEvent, FD_READ | FD_WRITE | FD_CLOSE);
						eventArray[eventTotal] = newEvent;
						socketArray[eventTotal] = sockAccept;
						eventTotal++;
				}
				// xu ly du lieu tu browser
				if (networkEvent.lNetworkEvents & FD_READ) {
					if (networkEvent.iErrorCode[FD_READ_BIT] != 0) {
						printf("Loi nhan du lieu tu user, ma loi : %d\n", networkEvent.iErrorCode[FD_READ_BIT]);
						break;
					}
					memset(buffRecv, 0, sizeof(buffRecv));
					int ret = recv(sockAccept, buffRecv, sizeof(buffRecv), 0);
					if (ret > 0) {
						printf("%s\n", buffRecv);
						char method[8];
						int length = strchr(buffRecv, ' ') - buffRecv;
						if (length > 0) {
							strncpy(method, buffRecv, length);
							method[length] = 0;
							// xu ly GET
							if (strcmp(method, "GET") == 0) {
								char path[128];

								length = strchr(strchr(buffRecv, ' ') + 1, ' ') - strchr(buffRecv, ' ') - 2;
								if (length > 1) {
									strncpy(path, strchr(buffRecv, ' ') + 2, length);
									path[length] = 0;
									// gui form dang nhap den user
									if (strcmp(path, "dangnhap.html")==0) {

										//sprintf(response, "HTTP/1.1 200 OK\nContent-Type: text/html\nAccept-Ranges: bytes\nContent-Length: 400\n\n");
										send(sockAccept, headerOk, sizeof(headerOk), 0);
										send(sockAccept, htmlForm, sizeof(htmlForm), 0);
										continue;
									}
									FILE *fileRequest = fopen(path, "rb");
									if (fileRequest == NULL) {
										// xu ly khong ton tai file
										//send(socketAccept, headerNoContent, sizeof(headerNoContent), 0);
										send(sockAccept, headerOk, sizeof(headerOk), 0);
										send(sockAccept, htmlNoContent, sizeof(htmlNoContent), 0);
									}
									else {
										//xu ly yeu cau file
										char ext[4];
										strncpy(ext, path + strlen(path) - 3, 3);
										ext[3] = 0;
										// lay dung luong file
										fseek(fileRequest, 0, SEEK_END);
										int size = ftell(fileRequest);
										char response[256];
										if (strcmp(ext, "png") == 0) {
											sprintf(response, "HTTP/1.1 200 OK\nContent-Length: %d\nContent-Type: image/png\nConnection: close\n\n", size);
										}
										else if (strcmp(ext, "mp4") == 0) {
											sprintf(response, "HTTP/1.1 200 OK\nContent-Length: %d\nContent-Type: video/mp4\nConnection: close\n\n", size);
										}
										send(sockAccept, response, strlen(response), 0);

										fseek(fileRequest, 0, SEEK_SET);
										char fbuff[1024];
										while (1) {
											int r = fread(fbuff, 1, 1024, fileRequest);
											if (r < 0) break;
											send(sockAccept, fbuff, r, 0);
											if (r < 1024) break;
										}
										fclose(fileRequest);
									}
									//printf("%s\n", path);
									//closesocket(sockAccept);
								}

								send(sockAccept, headerOk, sizeof(headerOk), 0);
								send(sockAccept, htmlWellcome, sizeof(htmlWellcome), 0);
								continue;
							}
							//xu ly POST
							if (strcmp(method, "POST") == 0) {
								char *content = strrchr(buffRecv, '\n');
								printf("\n\nTa nhan duoc noi dung : %s\n", content);
								printf("Phan tich ra cac cap gia tri :\n");
								char *pair;
								char *T[2];
								int pos = 0;
								pair = strtok(content, "&");
								while (pair != NULL) {
									T[pos] = pair;
									printf("%s\n", pair);
									pair = strtok(NULL, "&");
									pos++;
								}
								send(sockAccept, headerRedirect, sizeof(headerRedirect), 0);
								//send(sockAccept, htmlWellcome, sizeof(htmlWellcome), 0);
							}
						}
					}
				}
			}

		}
	}
	for (i = 0;i < eventTotal;i++) {
		closesocket(socketArray[i]);
	}
	closesocket(serverSock);
	WSACleanup();
	return 0;
}

