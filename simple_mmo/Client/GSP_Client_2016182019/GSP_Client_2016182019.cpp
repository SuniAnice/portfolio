#define _WINSOCK_DEPRECATED_NO_WARNINGS
// GSP_Client_2016182019.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include <iostream>
#include "GSP_Client_2016182019.h"
#include "resource.h"
#include "../../Protocol/2021_텀프_protocol.h"
#include <WS2tcpip.h>
#include <unordered_map>
#include <fstream>
#include <list>


using namespace std;
#pragma comment(lib, "Ws2_32.lib")

#define MAX_LOADSTRING 100
#define WINDOW_SIZE_X 1000
#define WINDOW_SIZE_Y 1000
#define CHESS_SQUARE_SIZE 40
#define CHESS_SQUARE_OFFSET 70

#define WM_SOCKET (WM_USER + 1)
#define MAX_BUFFER 1024



// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK Dlg_ip(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);


// 서버 변수
SOCKADDR_IN serverAddr;
WSADATA WSAData;
SOCKET serverSocket;

int playerid;
bool chatmode = false;
wchar_t chatbuf[MAX_STR_LEN];
wchar_t messbuf[MAX_STR_LEN];
int chatindex = 0;

list<wchar_t*> messlist;

char Sendbuffer[MAX_BUFFER];

bool obstacles[WORLD_WIDTH][WORLD_HEIGHT];

WSABUF s_wsabuf[1];
DWORD bytes_sent;

char Recvbuffer[MAX_BUFFER];
WSABUF r_wsabuf[1];

DWORD bytes_recv;

HWND hWnd;
char ipstr2[20];

DWORD recv_flag = 0;
typedef struct OBJECT {
    int id;
    int obj_class;
    char messbuf[MAX_STR_LEN];
    char name[MAX_ID_LEN];
    short x, y;
    int	hp, level, exp;
    int potion;
    int effect = 0;
};

unordered_map <int, OBJECT> Objects;



int recvn(SOCKET s)
{
    int received;
    int len = r_wsabuf[0].len;
    int  left = r_wsabuf[0].len;
    while (left > 0)
    {
        received = WSARecv(s, r_wsabuf, 1, &bytes_recv, &recv_flag, NULL, NULL);
        if (received == SOCKET_ERROR)
            return SOCKET_ERROR;
        else if (received == 0)
            break;
        left -= received;
        r_wsabuf[0].buf = (char*)&Recvbuffer + received;
        r_wsabuf[0].len = left;
    }
    return (len - left);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 여기에 코드를 입력합니다.

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_GSPCLIENT2016182019, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GSPCLIENT2016182019));

    MSG msg;

    // 기본 메시지 루프입니다:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}


void Do_recv(char* buffer)
{
    r_wsabuf[0].buf = (char*)&Recvbuffer;
    r_wsabuf[0].len = 1;
    int ret = WSARecv(serverSocket, r_wsabuf, 1, &bytes_recv, &recv_flag, NULL, NULL);
    r_wsabuf[0].buf = (char*)&Recvbuffer + 1;
    r_wsabuf[0].len = Recvbuffer[0] - 1;
    ret = recvn(serverSocket);
    switch (buffer[1])
    {
    case SC_LOGIN_OK:
    {
        auto cast = reinterpret_cast<sc_packet_login_ok*>(buffer);
        playerid = cast->id;;

        Objects[playerid].x = cast->x;
        Objects[playerid].y = cast->y;
        Objects[playerid].exp = cast->EXP;
        Objects[playerid].level = cast->LEVEL;
        Objects[playerid].hp = cast->HP;
        strcpy_s(Objects[playerid].name, ipstr2);
    }
    break;
    case SC_LOGIN_FAIL:
        exit(0);
        break;
    case SC_POSITION:
    {
        auto cast = reinterpret_cast<sc_packet_position*>(buffer);
       
        Objects[cast->id].x = cast->x;
        Objects[cast->id].y = cast->y;
    }
    break;
    case SC_STAT_CHANGE:
    {
        auto cast = reinterpret_cast<sc_packet_stat_change*>(buffer);
        Objects[cast->id].exp = cast->EXP;
        Objects[cast->id].level = cast->LEVEL;
        Objects[cast->id].hp = cast->HP;
    }
        break;
    case SC_ADD_OBJECT:
    {
        auto cast = reinterpret_cast<sc_packet_add_object*>(buffer);
        Objects[cast->id].x = cast->x;
        Objects[cast->id].y = cast->y;
        Objects[cast->id].hp = cast->HP;
        Objects[cast->id].obj_class = cast->obj_class;
        strcpy_s(Objects[cast->id].name, cast->name);
    }
    break;
    case SC_REMOVE_OBJECT:
    {
        auto cast = reinterpret_cast<sc_packet_remove_object*>(buffer);
        int temp = cast->id;
        Objects.erase(temp);
    }
    break;
    case SC_CHAT:
    {
        auto cast = reinterpret_cast<sc_packet_chat*>(buffer);
        wchar_t* buf = new wchar_t[128];
        MultiByteToWideChar(CP_ACP, 0, cast->message, MAX_STR_LEN, messbuf, MAX_STR_LEN);
        if (cast->id != -1) {
            wchar_t temp[20];
            MultiByteToWideChar(CP_ACP, 0, Objects[cast->id].name, 20, temp, 20);
            wsprintf(buf, L"%s : %s", temp, messbuf);
        }
        else
        {
            wsprintf(buf, L"%s", messbuf);
        }
        
        messlist.push_back(buf);
        if (messlist.size() > 4)
        {
            free(messlist.front());
            messlist.pop_front();
        }
    }
    break;
    default:
        break;
    }
}

void display_error(const char* msg, int err_no)
{
    WCHAR* lpMsgBuf;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, err_no, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
    cout << msg;
    wcout << lpMsgBuf << endl;
    LocalFree(lpMsgBuf);
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GSPCLIENT2016182019));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_GSPCLIENT2016182019);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

   hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      0, 0, WINDOW_SIZE_X, WINDOW_SIZE_Y, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);


   return TRUE;
}

void DrawBitmap(HDC hdc, int x, int y, HBITMAP hBit)
{
    HDC MemDC;
    HBITMAP OldBitmap;
    int bx, by;
    BITMAP bit;

    MemDC = CreateCompatibleDC(hdc);
    OldBitmap = (HBITMAP)SelectObject(MemDC, hBit);

    GetObject(hBit, sizeof(BITMAP), &bit);
    bx = bit.bmWidth;
    by = bit.bmHeight;
    GdiTransparentBlt(hdc, x, y, 40, 40, MemDC, 0, 0, bx, by, RGB(255, 255, 255));
    SelectObject(MemDC, OldBitmap);
    DeleteDC(MemDC);

}

INT_PTR CALLBACK Dlg_ip(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    wchar_t buf[30];
    char ipstr[30];

    wchar_t buf2[20];
    
    switch (iMsg)
    {
    case WM_INITDIALOG:
        SetDlgItemText(hDlg, IDC_EDIT1, L"127.0.0.1");
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            GetDlgItemText(hDlg, IDC_EDIT1, buf, 30);

            WideCharToMultiByte(CP_ACP, 0, buf, 30, ipstr, 30, NULL, NULL);

            inet_pton(AF_INET, (PCSTR)ipstr, &serverAddr.sin_addr);

            GetDlgItemText(hDlg, IDC_EDIT2, buf2, 20);

            WideCharToMultiByte(CP_ACP, 0, buf2, 20, ipstr2, 20, NULL, NULL);

            int ret = WSAConnect(serverSocket, reinterpret_cast<sockaddr*>(&serverAddr),
                sizeof(serverAddr), 0, 0, 0, 0);


            s_wsabuf[0].buf = (char*)&Sendbuffer;
            s_wsabuf[0].len = sizeof(cs_packet_login);
            auto cast = reinterpret_cast<cs_packet_login*>(Sendbuffer);
            cast->type = CS_LOGIN;
            cast->size = sizeof(cs_packet_login);
            strcpy_s(cast->player_id, ipstr2);

            ret = WSASend(serverSocket, s_wsabuf, 1, &bytes_sent, 0, 0, 0);



            if (ret == SOCKET_ERROR)
            {
                display_error("connect error : ", WSAGetLastError());
                exit(-1);
            }



            EndDialog(hDlg, 0);
            return (INT_PTR)TRUE;
            break;
        }
        break;

    }

    return (INT_PTR)FALSE;
}



LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HBITMAP KingBitmap;
    static HBITMAP PCBitmap;
    static HBITMAP SlimeBitmap;
    static HBITMAP RedSlimeBitmap;
    static HBITMAP BatBitmap;
    static HBITMAP RedBatBitmap;
    static HBITMAP RockBitmap;
    ifstream is("../../Protocol/obs.txt");


    int ret = 0;

    HANDLE hThread;
    HFONT hFont, OldFont;

    switch (message)
    {
    case WM_CREATE:
    {
        KingBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP1));
        PCBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP2));
        SlimeBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP3));
        RedSlimeBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP4));
        BatBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP5));
        RedBatBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP6));
        RockBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP7));

        for (int i = 0; i < 2000; i++) {
            for (int j = 0; j < 2000; j++) {
                char c;
                is.get(c);
                if (c == '0')
                    obstacles[i][j] = false;
                else
                    obstacles[i][j] = true;

            }
        }

        is.close();

        WSAStartup(MAKEWORD(2, 2), &WSAData);

        serverSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, 0);
        
        memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
        serverAddr.sin_family = AF_INET;

        serverAddr.sin_port = htons(SERVER_PORT);


        DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, Dlg_ip);

       
        WSAAsyncSelect(serverSocket, hWnd, WM_SOCKET, FD_READ | FD_WRITE | FD_CLOSE);

        //Do_recv(Recvbuffer);

        auto hTimer = (HANDLE)SetTimer(hWnd, 1, 16, NULL);
        break;
    }
    case WM_PAINT:
        {
            PAINTSTRUCT ps;

            static HDC hdc, MemDC;
            static HBITMAP BackBit, oldBackBit;
            static RECT bufferRT;
            MemDC = BeginPaint(hWnd, &ps);

            GetClientRect(hWnd, &bufferRT);
            hdc = CreateCompatibleDC(MemDC);
            BackBit = CreateCompatibleBitmap(MemDC, bufferRT.right, bufferRT.bottom);
            oldBackBit = (HBITMAP)SelectObject(hdc, BackBit);
            PatBlt(hdc, 0, 0, bufferRT.right, bufferRT.bottom, WHITENESS);

            // TODO: 여기에 그리기 코드를 추가합니다.
            HBRUSH BlackBrush, WhiteBrush, PawnBrush, oBrush;
            BlackBrush = CreateSolidBrush(RGB(50, 50, 50));
            WhiteBrush = CreateSolidBrush(RGB(255, 255, 255));
            PawnBrush = CreateSolidBrush(RGB(0, 255, 0));
            SetBkMode(hdc, TRANSPARENT);
            hFont = CreateFont(30, 0, 0, 0, 0, 0, 0, 0, HANGEUL_CHARSET, 0, 0, 0,
                VARIABLE_PITCH | FF_ROMAN, TEXT("맑은 고딕"));
            OldFont = (HFONT)SelectObject(hdc, hFont);

            TCHAR buffer[255];
            wsprintf(buffer, TEXT("위치 : (%d, %d)"), Objects[playerid].x, Objects[playerid].y);

            TextOut(hdc, 10, 75, buffer, lstrlen(buffer));

            
            wsprintf(buffer, TEXT("LEVEL : %d"), Objects[playerid].level);
            TextOut(hdc, 10, 0, buffer, lstrlen(buffer));

            wsprintf(buffer, TEXT("HP : %d / %d"), Objects[playerid].hp, (Objects[playerid].level - 1) * 10 + 100);
            TextOut(hdc, 10, 25, buffer, lstrlen(buffer));

            wsprintf(buffer, TEXT("EXP : %d / %d"), Objects[playerid].exp, (int)floor(100 * pow(2, (Objects[playerid].level - 1))));
            TextOut(hdc, 10, 50, buffer, lstrlen(buffer));

            if (chatmode) {
                wsprintf(buffer, TEXT("%s"), chatbuf);
                TextOut(hdc, 10, 910, buffer, lstrlen(buffer));
            }
            

            SetTextAlign(hdc, TA_RIGHT);
            int temp = 0;
            for (auto messbuf : messlist) {
                wsprintf(buffer, TEXT("%s"), messbuf);
                TextOut(hdc, 950, temp * 25, messbuf, lstrlen(buffer));
                temp++;
            }
            

            for (int i = 0; i < 20; i++)
            {
                for (int j = 0; j < 20; j++)
                {
                    if (Objects[playerid].x + i < 10 || Objects[playerid].x + i > WORLD_WIDTH + 9) continue;
                    if (Objects[playerid].y + j < 10 || Objects[playerid].y + j > WORLD_HEIGHT + 9) continue;

                    if ((Objects[playerid].x - 10 + i) % 6 < 3 != (Objects[playerid].y - 10 + j) % 6 < 3)
                    {
                        oBrush = (HBRUSH)::SelectObject(hdc, WhiteBrush);
                    }
                    else
                    {
                        oBrush = (HBRUSH)::SelectObject(hdc, BlackBrush);
                    }

                    Rectangle(hdc, i * CHESS_SQUARE_SIZE + CHESS_SQUARE_OFFSET, j * CHESS_SQUARE_SIZE + CHESS_SQUARE_OFFSET + 50,
                        (i + 1) * CHESS_SQUARE_SIZE + CHESS_SQUARE_OFFSET, (j + 1) * CHESS_SQUARE_SIZE + CHESS_SQUARE_OFFSET + 50);

                    if (obstacles[Objects[playerid].x - 10 + i][Objects[playerid].y - 10 + j])
                    {
                        DrawBitmap(hdc, i* CHESS_SQUARE_SIZE + CHESS_SQUARE_OFFSET, j* CHESS_SQUARE_SIZE + CHESS_SQUARE_OFFSET + 50, RockBitmap);
                    }
                    
                }
            }

            for (auto& player : Objects)
            {
                if (player.first == playerid) continue;
                if (abs(player.second.x - Objects[playerid].x) > VIEW_RADIUS || abs(player.second.y - Objects[playerid].y) > VIEW_RADIUS) continue;
                switch (player.second.obj_class)
                {
                case 0:
                    DrawBitmap(hdc, ((player.second.x - Objects[playerid].x) + 10)* CHESS_SQUARE_SIZE + CHESS_SQUARE_OFFSET, ((player.second.y - Objects[playerid].y) + 10)
                        * CHESS_SQUARE_SIZE + CHESS_SQUARE_OFFSET + 50, PCBitmap);
                    break;
                case 1:
                    DrawBitmap(hdc, ((player.second.x - Objects[playerid].x) + 10) * CHESS_SQUARE_SIZE + CHESS_SQUARE_OFFSET, ((player.second.y - Objects[playerid].y) + 10)
                        * CHESS_SQUARE_SIZE + CHESS_SQUARE_OFFSET + 50, SlimeBitmap);
                    break;
                case 2:
                    DrawBitmap(hdc, ((player.second.x - Objects[playerid].x) + 10) * CHESS_SQUARE_SIZE + CHESS_SQUARE_OFFSET, ((player.second.y - Objects[playerid].y) + 10)
                        * CHESS_SQUARE_SIZE + CHESS_SQUARE_OFFSET + 50, RedSlimeBitmap);
                    break;
                case 3:
                    DrawBitmap(hdc, ((player.second.x - Objects[playerid].x) + 10) * CHESS_SQUARE_SIZE + CHESS_SQUARE_OFFSET, ((player.second.y - Objects[playerid].y) + 10)
                        * CHESS_SQUARE_SIZE + CHESS_SQUARE_OFFSET + 50, BatBitmap);
                    break;
                case 4:
                    DrawBitmap(hdc, ((player.second.x - Objects[playerid].x) + 10) * CHESS_SQUARE_SIZE + CHESS_SQUARE_OFFSET, ((player.second.y - Objects[playerid].y) + 10)
                        * CHESS_SQUARE_SIZE + CHESS_SQUARE_OFFSET + 50, RedBatBitmap);
                    break;
                }
                
            }

            DrawBitmap(hdc, 10 * CHESS_SQUARE_SIZE + CHESS_SQUARE_OFFSET, 10 * CHESS_SQUARE_SIZE + CHESS_SQUARE_OFFSET + 50, KingBitmap);

            

            DeleteObject(WhiteBrush);

            DeleteObject(BlackBrush);

            DeleteObject(PawnBrush);

            SelectObject(hdc, OldFont);
            DeleteObject(hFont);


            GetClientRect(hWnd, &bufferRT);
            BitBlt(MemDC, 0, 0, bufferRT.right, bufferRT.bottom, hdc, 0, 0, SRCCOPY);
            SelectObject(hdc, oldBackBit);
            DeleteObject(BackBit);
            DeleteDC(hdc);
            EndPaint(hWnd, &ps);

        }
        break;
    case WM_CHAR:
    {
        if (!chatmode) {
            switch ((char)wParam)
            {
             case 'a':
             {
                 s_wsabuf[0].buf = (char*)&Sendbuffer;
                 s_wsabuf[0].len = sizeof(cs_packet_attack);
                 auto cast = reinterpret_cast<cs_packet_attack*>(Sendbuffer);
                 cast->type = CS_ATTACK;
                 cast->size = sizeof(cs_packet_attack);
                 int ret = WSASend(serverSocket, s_wsabuf, 1, &bytes_sent, 0, 0, 0);
                 if (ret == SOCKET_ERROR)
                 {
                     exit(0);
                 }
             }
                 break;
            }
        }
        else
        {
            switch (wParam)
            {
            default:
                if (chatindex < MAX_STR_LEN)
                {
                    chatbuf[chatindex] = wParam;
                    chatindex++;
                }
                break;
            }
        }
        
    }
           break;
    case WM_KEYDOWN:
    {
        if (chatmode) {
            switch (wParam)
            {
            case VK_RETURN:
                if (chatindex <= 1) {
                    chatmode = false;
                    break;
                }
                s_wsabuf[0].buf = (char*)&Sendbuffer;
                s_wsabuf[0].len = sizeof(cs_packet_chat);
                auto cast = reinterpret_cast<cs_packet_chat*>(Sendbuffer);
                cast->type = CS_CHAT;
                cast->size = sizeof(cs_packet_chat);

                WideCharToMultiByte(CP_ACP, 0, chatbuf, 50, cast->message, 50, NULL, NULL);

                memset(chatbuf, NULL, sizeof(chatbuf));
                chatindex = 0;

                int ret = WSASend(serverSocket, s_wsabuf, 1, &bytes_sent, 0, 0, 0);
                if (ret == SOCKET_ERROR)
                {
                    exit(0);
                }
                chatmode = false;
                break;
            }
        }
        else
        {
            switch (wParam)
            {
            
            case VK_UP:
            {
                s_wsabuf[0].buf = (char*)&Sendbuffer;
                s_wsabuf[0].len = sizeof(cs_packet_move);
                auto cast = reinterpret_cast<cs_packet_move*>(Sendbuffer);
                cast->type = CS_MOVE;
                cast->size = sizeof(cs_packet_move);
                cast->direction = 0;
                int ret = WSASend(serverSocket, s_wsabuf, 1, &bytes_sent, 0, 0, 0);
                if (ret == SOCKET_ERROR)
                {
                    exit(0);
                }
            }
            break;
            case VK_DOWN:
            {
                s_wsabuf[0].buf = (char*)&Sendbuffer;
                s_wsabuf[0].len = sizeof(cs_packet_move);
                auto cast = reinterpret_cast<cs_packet_move*>(Sendbuffer);
                cast->type = CS_MOVE;
                cast->size = sizeof(cs_packet_move);
                cast->direction = 1;
                int ret = WSASend(serverSocket, s_wsabuf, 1, &bytes_sent, 0, 0, 0);
                if (ret == SOCKET_ERROR)
                {
                    exit(0);
                }
            }
            break;
            case VK_LEFT:
            {
                s_wsabuf[0].buf = (char*)&Sendbuffer;
                s_wsabuf[0].len = sizeof(cs_packet_move);
                auto cast = reinterpret_cast<cs_packet_move*>(Sendbuffer);
                cast->type = CS_MOVE;
                cast->size = sizeof(cs_packet_move);
                cast->direction = 2;
                int ret = WSASend(serverSocket, s_wsabuf, 1, &bytes_sent, 0, 0, 0);
                if (ret == SOCKET_ERROR)
                {
                    exit(0);
                }
            }
            break;
            case VK_RIGHT:
            {
                s_wsabuf[0].buf = (char*)&Sendbuffer;
                s_wsabuf[0].len = sizeof(cs_packet_move);
                auto cast = reinterpret_cast<cs_packet_move*>(Sendbuffer);
                cast->type = CS_MOVE;
                cast->size = sizeof(cs_packet_move);
                cast->direction = 3;
                int ret = WSASend(serverSocket, s_wsabuf, 1, &bytes_sent, 0, 0, 0);
                if (ret == SOCKET_ERROR)
                {
                    exit(0);
                }
            }
            break;
            case VK_RETURN:
                memset(chatbuf, NULL, sizeof(chatbuf));
                chatindex = 0;
                chatmode = true;
                break;
            }
            
        }
        break;
    }
    case WM_TIMER:
        InvalidateRect(hWnd, NULL, FALSE);
        break;
    case WM_DESTROY:
    {
        s_wsabuf[0].buf = (char*)&Sendbuffer;
        s_wsabuf[0].len = sizeof(cs_packet_logout);
        auto cast = reinterpret_cast<cs_packet_logout*>(Sendbuffer);
        cast->type = CS_LOGOUT;
        cast->size = sizeof(cs_packet_logout);
        int ret = WSASend(serverSocket, s_wsabuf, 1, &bytes_sent, 0, 0, 0);

        closesocket(serverSocket);
        WSACleanup();
        PostQuitMessage(0);
    }
        break;
    case WM_SOCKET:
    {
        SOCKET selectedSocket = wParam; //socket번호는 wParam으로 전달
        int event = WSAGETSELECTEVENT(lParam); //이벤트 정보는 lParam에서 추출
        switch (event)
        {
        case FD_READ:
            memset(Recvbuffer, 0, sizeof(Recvbuffer));
            Do_recv(Recvbuffer);
            break;
        case FD_CLOSE:
            exit(2);
            break;
        }
    }
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
