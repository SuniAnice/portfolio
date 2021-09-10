#pragma once

#include "stdafx.h"
#include "../../CommonLibrary/GameObject.h"

// #define SERVERIP   "127.0.0.1"
#define SERVERPORT 9000
#define BUFSIZE    200

// ���� �Լ� ���� ��� �� ����
void err_quit(const char* msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    MessageBox(NULL, (LPCTSTR)lpMsgBuf, (LPCWSTR)msg, MB_ICONERROR);
    LocalFree(lpMsgBuf);
    exit(1);
}

// ���� �Լ� ���� ���
void err_display(const char* msg)
{
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    printf("[%s] %s", msg, (char*)lpMsgBuf);
    LocalFree(lpMsgBuf);
}

// ����� ���� ������ ���� �Լ�
int recvn(SOCKET s, char* buf, int len, int flags)
{
    int received;
    char* ptr = buf;
    int left = len;

    while (left > 0) {
        received = recv(s, ptr, left, flags);	// ������� ���� ������ ����Ʈ
        if (received == SOCKET_ERROR)
            return SOCKET_ERROR;
        else if (received == 0)
            break;
        left -= received;	// left�� �� �޾ƾ� �� ������ ũ��
        ptr += received;	// ������ ���� ����, ���� ��ŭ �̵�.
    }
    return (len - left);
}

void SendData(SOCKET sock, int retval, const char* test[])
{
    char buf[BUFSIZE];
    int len;

    // ������ ������ ���
    for (int i = 0; i < 4; i++) {
        // ������ �Է�(�ùķ��̼�)
        len = strlen((const char*)test[i]);
        strncpy(buf, (const char*)test[i], len);

        // ������ ������(���� ����)
        retval = send(sock, (char*)&len, sizeof(int), 0);
        if (retval == SOCKET_ERROR) {
            err_display("send()");
            break;
        }

        // ������ ������(���� ����)
        retval = send(sock, buf, len, 0);
        if (retval == SOCKET_ERROR) {
            err_display("send()");
            break;
        }
        printf("[TCP Ŭ���̾�Ʈ] %d+%d����Ʈ�� "
            "���½��ϴ�.\n", sizeof(int), retval);
    }
}