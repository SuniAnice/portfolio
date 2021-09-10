#include <iostream>
#include <unordered_map>
#include <thread>
#include <vector>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <windows.h>  
#include <sqlext.h>  
#include <mutex>
#include <array>
#include <fstream>
#include <unordered_set>
#include "../../Protocol/2021_텀프_protocol.h"
#include <queue>
#include <algorithm>
#include "server.h"
extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "MSWSock.lib")
#pragma comment (lib, "lua54.lib")
#define MAX_BUFFER 1024

using namespace std;
enum OBJ_STATE {
	STATE_READY, STATE_CONNECTED, STATE_INGAME
};

enum OP_TYPE{
	OP_RECV, OP_SEND, OP_ACCEPT, OP_RANDOM_MOVE, OP_PLAYER_ENCOUNTER, OP_GET_PLAYER_INFO, OP_SET_PLAYER_INFO,
	OP_REGEN_HP, OP_SAVE, OP_AI_MOVE, OP_RESPAWN
};

struct OVERLAPPED_EXTENDED
{
	WSAOVERLAPPED overlapped;
	WSABUF wsabuf[1];
	unsigned char packetbuf[MAX_BUFFER];
	OP_TYPE op_type;
	SOCKET socket;
};
struct TIMER_EVENT {
	OP_TYPE ev_type;
	chrono::system_clock::time_point event_time;
	int object_id;
	int target_id;

	constexpr bool operator< (const TIMER_EVENT& l) const {
		return (event_time > l.event_time);
	}
};
struct DB_EVENT {
	OP_TYPE ev_type;
	chrono::system_clock::time_point event_time;
	int object_index;
	char object_id[MAX_ID_LEN];
	int object_exp, object_hp, object_level;
	short object_x, object_y;
};


struct OBJECT
{
	SOCKET socket;
	mutex state_lock;
	atomic <OBJ_STATE> object_state;
	OVERLAPPED_EXTENDED m_recv_over;
	int prev_packet_size;
	int id;
	atomic_int sec_x;
	atomic_int sec_y;
	int obj_class;

	char m_name[MAX_ID_LEN];
	short x, y;
	short init_x, init_y;
	int hp;
	int level;
	int exp;
	int move_time;

	chrono::system_clock::time_point last_move_time;
	chrono::system_clock::time_point last_attack_time;

	unordered_set <int> m_viewlist;
	mutex vl_lock;

	atomic_bool is_active;
	atomic_bool is_tracking;

	lua_State* L;
	mutex	lua_lock;
};

bool do_npc_move(OBJECT& npc, char dir);
void search_sector(int x, int y, unordered_set <int>& new_vl, int p_id);

bool obstacles[WORLD_WIDTH][WORLD_HEIGHT];

priority_queue <TIMER_EVENT> timer_queue;
mutex timer_lock;

queue <DB_EVENT> db_queue;
mutex db_lock;

array <OBJECT, MAX_USER + 1> objects;

mutex m_seclock[WORLD_WIDTH / SECTOR_X][WORLD_HEIGHT / SECTOR_Y];
unordered_set <int> sectors[WORLD_WIDTH / SECTOR_X][WORLD_HEIGHT / SECTOR_Y];

HANDLE iocp_handle;

void add_event(int obj, OP_TYPE ev_t, int delay_ms) {
	using namespace chrono;
	TIMER_EVENT ev;
	ev.ev_type = ev_t;
	ev.object_id = obj;
	ev.event_time = system_clock::now() + milliseconds(delay_ms);
	timer_lock.lock();
	timer_queue.push(ev);
	timer_lock.unlock();

}

void add_event(int obj, OP_TYPE ev_t, int delay_ms, int target) {
	using namespace chrono;
	TIMER_EVENT ev;
	ev.ev_type = ev_t;
	ev.object_id = obj;
	ev.event_time = system_clock::now() + milliseconds(delay_ms);
	ev.target_id = target;
	timer_lock.lock();
	timer_queue.push(ev);
	timer_lock.unlock();

}

void add_db_access(DB_EVENT ev) {
	db_lock.lock();
	db_queue.push(ev);
	db_lock.unlock();
}


bool is_npc(int id) {
	return id > NPC_ID_START;
}

void wake_up_npc(int npc_id) {
	if (objects[npc_id].is_active == false) {
		bool old_state = false;
		if (true == atomic_compare_exchange_strong(&objects[npc_id].is_active, &old_state, true))
		{
			if (objects[npc_id].obj_class >= 3)
				add_event(npc_id, OP_RANDOM_MOVE, 1000);
		}
	}
}

void display_error(const char* msg, int err_no)
{
	WCHAR* lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	//cout << msg;
	//wcout << lpMsgBuf << endl;
	LocalFree(lpMsgBuf);

}

bool can_see(int id_a, int id_b) {
	return VIEW_RADIUS >= abs(objects[id_a].x - objects[id_b].x) &&
		VIEW_RADIUS >= abs(objects[id_a].y - objects[id_b].y);
}

int get_dist(int x, int y, int tx, int ty)
{
	return abs(x - tx) + abs(y - ty);
}


struct node {
	int x, y;
	int f;
	int g;
	int dir = -1;
	node *parent = nullptr;

	constexpr bool operator< (const node& l) const {
		return (f > l.f);
	}

};

bool comp(const node* lhs, const node* rhs)
{
	return lhs->f < rhs->f;   //오름차순 정렬
};


int a_star_search(int p_id, int o_id) {
	int x = objects[p_id].x;
	int y = objects[p_id].y;
	int tx = objects[o_id].x;
	int ty = objects[o_id].y;

	int dist;

	int g = 0;
	int h = get_dist(x, y, tx, ty);
	int f = g + h;

	list<node*> open;
	list<node*> closed;
	node* parent;
	node* cur;
	node* child;
	parent = new node();
	parent->x = x;
	parent->y = y;
	parent->g = 0;
	parent->f = f;
	open.push_back(parent);

	while (!open.empty())
	{
		open.sort(comp);
		cur = open.front();
		
		if (cur->g > 10)
		{
			int dir = cur->dir;
			for (auto& c : open) {
				delete c;
			}
			for (auto& c : closed) {
				delete c;
			}
			return dir;
		}

		if (get_dist(cur->x, cur->y, tx, ty) <= 1)
		{
			node* ptr = cur;
			if (ptr->parent == nullptr)
				return -2;
			while (ptr->parent->parent != nullptr)
			{
				ptr = ptr->parent;
			}
			int dir = ptr->dir;
			for (auto& c : open) {
				delete c;
			}
			for (auto& c : closed) {
				delete c;
			}
			return dir;
		}
		else {
			if (cur->y > 0)
			{
				if (!obstacles[cur->x][cur->y - 1]) {
					bool found = false;
					child = new node();
					child->dir = 0;
					child->x = cur->x;
					child->y = cur->y - 1;
					child->g = cur->g + 1;
					child->parent = cur;
					child->f = get_dist(child->x, child->y, tx, ty) + child->g;
					for (auto& c : open) {
						if (c->x == child->x && c->y == child->y)
							found = true;
					}
					for (auto& c : closed) {
						if (c->x == child->x && c->y == child->y)
							found = true;
					}
					if (!found)
						open.push_back(child);
					else
						delete child;
				}
			}
			if (cur->y < WORLD_HEIGHT - 1)
			{
				if (!obstacles[cur->x][cur->y + 1]) {
					bool found = false;
					child = new node();
					child->dir = 1;
					child->x = cur->x;
					child->y = cur->y + 1;
					child->g = cur->g + 1;
					child->parent = cur;
					child->f = get_dist(child->x, child->y, tx, ty) + child->g;
					for (auto& c : open) {
						if (c->x == child->x && c->y == child->y)
							found = true;
					}
					for (auto& c : closed) {
						if (c->x == child->x && c->y == child->y)
							found = true;
					}
					if (!found)
						open.push_back(child);
					else
						delete child;
				}
			}
			if (cur->x > 0)
			{
				if (!obstacles[cur->x - 1][cur->y]) {
					bool found = false;
					child = new node();
					child->dir = 2;
					child->x = cur->x - 1;
					child->y = cur->y;
					child->g = cur->g + 1;
					child->parent = cur;
					child->f = get_dist(child->x, child->y, tx, ty) + child->g;
					for (auto& c : open) {
						if (c->x == child->x && c->y == child->y)
							found = true;
					}
					for (auto& c : closed) {
						if (c->x == child->x && c->y == child->y)
							found = true;
					}
					if (!found)
						open.push_back(child);
					else
						delete child;
				}
			}
			if (cur->x < WORLD_WIDTH - 1)
			{
				if (!obstacles[cur->x + 1][cur->y]) {
					bool found = false;
					child = new node();
					child->dir = 3;
					child->x = cur->x + 1;
					child->y = cur->y;
					child->g = cur->g + 1;
					child->parent = cur;
					child->f = get_dist(child->x, child->y, tx, ty) + child->g;
					for (auto& c : open) {
						if (c->x == child->x && c->y == child->y)
							found = true;
					}
					for (auto& c : closed) {
						if (c->x == child->x && c->y == child->y)
							found = true;
					}
					if (!found)
						open.push_back(child);
					else
						delete child;
				}
			}
			
			closed.push_back(cur);
		}

		open.pop_front();

	}
	for (auto& c : open) {
		delete c;
	}
	for (auto& c : closed) {
		delete c;
	}
	return -2;
}


void send_packet(int p_id, void *p)
{
	int p_size = reinterpret_cast<unsigned char*>(p)[0];
	OVERLAPPED_EXTENDED *overlapped = new OVERLAPPED_EXTENDED;
	overlapped->op_type = OP_SEND;
	memset(&overlapped->overlapped, 0, sizeof(overlapped->overlapped));
	memcpy(&overlapped->packetbuf, p, p_size);
	overlapped->wsabuf[0].buf = reinterpret_cast<char*>(overlapped->packetbuf);
	overlapped->wsabuf[0].len = p_size;

	int ret = WSASend(objects[p_id].socket, (overlapped->wsabuf), 1, NULL, 0,
		&overlapped->overlapped, NULL);

	if (0 != ret) {
		int err_no = WSAGetLastError();
		if (WSA_IO_PENDING != err_no)
			display_error("Error in SendPacket:", err_no);
	}
}


void do_recv(int id)
{
	objects[id].m_recv_over.wsabuf[0].buf = reinterpret_cast<char*>(objects[id].m_recv_over.packetbuf) + objects[id].prev_packet_size;
	objects[id].m_recv_over.wsabuf[0].len = MAX_BUFFER - objects[id].prev_packet_size;

	memset(&objects[id].m_recv_over.overlapped, 0, sizeof(objects[id].m_recv_over.overlapped));
	DWORD r_flag = 0;
	int ret = WSARecv(objects[id].socket, objects[id].m_recv_over.wsabuf, 1, NULL,
		&r_flag, &objects[id].m_recv_over.overlapped, NULL);
	if (0 != ret) {
		int err_no = WSAGetLastError();
		if (WSA_IO_PENDING != err_no)
			display_error("Error in RecvPacket:", err_no);
	}
}

int get_new_player_id(SOCKET p_socket)
{
	for (int i = 1; i < MAX_USER; ++i)
	{
		lock_guard<mutex> lg{ objects[i].state_lock };
		if (objects[i].object_state == STATE_READY) { 
			objects[i].object_state = STATE_CONNECTED;
			objects[i].socket = p_socket;
			return i;
		}
	}
	return -1;
}

bool search_player_id(char* id) {
	for (int i = 1; i < NPC_ID_START; ++i)
	{
		lock_guard<mutex> lg{ objects[i].state_lock };
		if (objects[i].object_state == STATE_INGAME && strcmp(objects[i].m_name,id) == 0) {
			return true;
		}
	}
	return false;
}



void send_login_fail_packet(SOCKET p_socket) {
	sc_packet_login_fail p;
	p.size = sizeof(p);
	p.type = SC_LOGIN_FAIL;

	int p_size = reinterpret_cast<unsigned char*>(&p)[0];
	OVERLAPPED_EXTENDED* overlapped = new OVERLAPPED_EXTENDED;
	overlapped->op_type = OP_SEND;
	memset(&overlapped->overlapped, 0, sizeof(overlapped->overlapped));
	memcpy(&overlapped->packetbuf, &p, p_size);
	overlapped->wsabuf[0].buf = reinterpret_cast<char*>(overlapped->packetbuf);
	overlapped->wsabuf[0].len = p_size;

	int ret = WSASend(p_socket, (overlapped->wsabuf), 1, NULL, 0,
		&overlapped->overlapped, NULL);

}

void send_chat_packet(int c_id, int p_id, const char* mess)
{
	sc_packet_chat p;
	p.id = p_id;
	p.size = sizeof(p);
	p.type = SC_CHAT;
	strcpy_s(p.message, mess);
	send_packet(c_id, &p);
}

void send_position_packet(int c_id, int p_id) {
	sc_packet_position p;
	p.id = p_id;
	p.size = sizeof(p);
	p.type = SC_POSITION;
	p.x = objects[p_id].x;
	p.y = objects[p_id].y;
	p.move_time = objects[p_id].move_time;

	send_packet(c_id, &p);
}

void send_add_packet(int c_id, int p_id) {
	sc_packet_add_object p;
	p.id = p_id;
	p.size = sizeof(p);
	p.type = SC_ADD_OBJECT;
	p.x = objects[p_id].x;
	p.y = objects[p_id].y;
	strcpy_s(p.name, objects[p_id].m_name);
	p.obj_class = objects[p_id].obj_class;
	p.HP = objects[p_id].hp;
	p.LEVEL = objects[p_id].level;
	p.EXP = objects[p_id].exp;


	send_packet(c_id, &p);
}

void send_login_fail_packet(int p_id) {
	sc_packet_login_fail p;
	p.size = sizeof(p);
	p.type = SC_LOGIN_FAIL;

	send_packet(p_id, &p);
}

void send_login_ok_packet(int p_id, DB_EVENT ev) {
	sc_packet_login_ok p;
	objects[p_id].x = ev.object_x;
	objects[p_id].y = ev.object_y;
	objects[p_id].level = ev.object_level;
	objects[p_id].hp = ev.object_hp;
	objects[p_id].exp = ev.object_exp;
	strcpy_s(objects[p_id].m_name, ev.object_id);
	objects[p_id].last_move_time = chrono::system_clock::now();
	objects[p_id].last_attack_time = chrono::system_clock::now();


	objects[p_id].sec_x = floor(objects[p_id].x / SECTOR_X);
	objects[p_id].sec_y = floor(objects[p_id].y / SECTOR_Y);


	m_seclock[objects[p_id].sec_x][objects[p_id].sec_y].lock();
	sectors[objects[p_id].sec_x][objects[p_id].sec_y].insert(p_id);
	m_seclock[objects[p_id].sec_x][objects[p_id].sec_y].unlock();

	p.id = p_id;
	p.size = sizeof(p);
	p.type = SC_LOGIN_OK;
	p.x = objects[p_id].x;
	p.y = objects[p_id].y;
	p.LEVEL = objects[p_id].level;
	p.HP = objects[p_id].hp;
	p.EXP = objects[p_id].exp;


	send_packet(p_id, &p);

	objects[p_id].object_state = STATE_INGAME;

	unordered_set <int> login_vl;

	search_sector(objects[p_id].sec_x, objects[p_id].sec_y, login_vl, p_id);

	int flagx = 0;
	int flagy = 0;

	if (objects[p_id].sec_x > 0) {
		if (objects[p_id].sec_x != (int)((objects[p_id].x - VIEW_RADIUS) / SECTOR_X))
		{
			flagx = -1;
		}
	}
	if (objects[p_id].sec_x < (int)(WORLD_WIDTH / SECTOR_X - 1)) {
		if (objects[p_id].sec_x != (int)((objects[p_id].x + VIEW_RADIUS) / SECTOR_X))
		{
			flagx = 1;
		}
	}
	if (objects[p_id].sec_y > 0) {
		if (objects[p_id].sec_y != (int)((objects[p_id].y - VIEW_RADIUS) / SECTOR_Y))
		{
			flagy = -1;
		}
	}
	if (objects[p_id].sec_y < (int)(WORLD_HEIGHT / SECTOR_Y - 1)) {
		if (objects[p_id].sec_y != (int)((objects[p_id].y + VIEW_RADIUS) / SECTOR_Y))
		{
			flagy = 1;
		}
	}

	if (flagx == -1) {
		switch (flagy) {
		case -1:
			search_sector(objects[p_id].sec_x - 1, objects[p_id].sec_y, login_vl, p_id);
			search_sector(objects[p_id].sec_x, objects[p_id].sec_y - 1, login_vl, p_id);
			search_sector(objects[p_id].sec_x - 1, objects[p_id].sec_y - 1, login_vl, p_id);
			break;
		case 0:
			search_sector(objects[p_id].sec_x - 1, objects[p_id].sec_y, login_vl, p_id);

			break;
		case 1:
			search_sector(objects[p_id].sec_x - 1, objects[p_id].sec_y, login_vl, p_id);
			search_sector(objects[p_id].sec_x, objects[p_id].sec_y + 1, login_vl, p_id);
			search_sector(objects[p_id].sec_x - 1, objects[p_id].sec_y + 1, login_vl, p_id);
			break;
		}
	}
	else if (flagx == 1) {
		switch (flagy) {
		case -1:
			search_sector(objects[p_id].sec_x + 1, objects[p_id].sec_y, login_vl, p_id);
			search_sector(objects[p_id].sec_x, objects[p_id].sec_y - 1, login_vl, p_id);
			search_sector(objects[p_id].sec_x + 1, objects[p_id].sec_y - 1, login_vl, p_id);
			break;
		case 0:
			search_sector(objects[p_id].sec_x + 1, objects[p_id].sec_y, login_vl, p_id);

			break;
		case 1:
			search_sector(objects[p_id].sec_x + 1, objects[p_id].sec_y, login_vl, p_id);
			search_sector(objects[p_id].sec_x, objects[p_id].sec_y + 1, login_vl, p_id);
			search_sector(objects[p_id].sec_x + 1, objects[p_id].sec_y + 1, login_vl, p_id);
			break;
		}
	}
	else {
		switch (flagy) {
		case -1:
			search_sector(objects[p_id].sec_x, objects[p_id].sec_y - 1, login_vl, p_id);
			break;
		case 1:
			search_sector(objects[p_id].sec_x, objects[p_id].sec_y + 1, login_vl, p_id);
			break;
		}
	}

	for (auto& pl : login_vl) {
		if (p_id != pl) {
			lock_guard <mutex> gl{ objects[pl].state_lock };
			if (STATE_INGAME == objects[pl].object_state) {
				if (can_see(p_id, pl)) {
					objects[p_id].vl_lock.lock();
					objects[p_id].m_viewlist.insert(pl);
					objects[p_id].vl_lock.unlock();
					send_add_packet(p_id, pl);

					if (false == is_npc(pl)) {
						objects[pl].vl_lock.lock();
						objects[pl].m_viewlist.insert(p_id);
						objects[pl].vl_lock.unlock();
						send_add_packet(pl, p_id);
					}
					else {
						wake_up_npc(pl);
					}
				}
			}
		}
	}
}

void send_remove_packet(int c_id, int p_id) {
	sc_packet_remove_object p;
	p.id = p_id;
	p.size = sizeof(p);
	p.type = SC_REMOVE_OBJECT;

	send_packet(c_id, &p);
}

void send_stat_change_packet(int c_id, int p_id) {
	sc_packet_stat_change p;
	p.size = sizeof(p);
	p.type = SC_STAT_CHANGE;
	p.LEVEL = objects[p_id].level;
	p.HP = objects[p_id].hp;
	p.EXP = objects[p_id].exp;
	p.id = p_id;


	send_packet(c_id, &p);
}

void search_sector(int x, int y, unordered_set <int>& new_vl, int p_id) {
	if (x < 0 || y < 0 || x > 80 || y > 80)
		return;
	if (is_npc(p_id) == true) {
		for (auto& pl : sectors[x][y]) {
			if (pl == p_id) continue;
			if (is_npc(pl) == true) continue;
			if (objects[pl].object_state == STATE_INGAME && can_see(p_id, pl))
				new_vl.insert(pl);
		}
	}
	else {
		for (auto& pl : sectors[x][y]) {
			if (pl == p_id) continue;
			if (objects[pl].object_state == STATE_INGAME && can_see(p_id, pl))
				new_vl.insert(pl);
		}
	}
}

void do_move(int p_id, char dir) {
	auto &x = objects[p_id].x;
	auto &y = objects[p_id].y;

	if ((chrono::system_clock::now() - objects[p_id].last_move_time) < chrono::milliseconds(move_cooldown) && dir != 4)
		return;

	switch (dir)
	{
	case 0:
		if (y > 0 && !obstacles[x][y-1]) {
			y--;
			if (objects[p_id].sec_y != (int)(objects[p_id].y / SECTOR_Y)) {
				m_seclock[objects[p_id].sec_x][objects[p_id].sec_y].lock();
				sectors[objects[p_id].sec_x][objects[p_id].sec_y].erase(p_id);
				m_seclock[objects[p_id].sec_x][objects[p_id].sec_y].unlock();

				objects[p_id].sec_y = (int)(objects[p_id].y / SECTOR_Y);

				m_seclock[objects[p_id].sec_x][objects[p_id].sec_y].lock();
				sectors[objects[p_id].sec_x][objects[p_id].sec_y].insert(p_id);
				m_seclock[objects[p_id].sec_x][objects[p_id].sec_y].unlock();
			}
		}
		else
			return;
		break;
	case 1:
		if (y < (WORLD_HEIGHT - 1) && !obstacles[x][y + 1]) {
			y++;
			if (objects[p_id].sec_y != (int)(objects[p_id].y / SECTOR_Y)) {
				m_seclock[objects[p_id].sec_x][objects[p_id].sec_y].lock();
				sectors[objects[p_id].sec_x][objects[p_id].sec_y].erase(p_id);
				m_seclock[objects[p_id].sec_x][objects[p_id].sec_y].unlock();

				objects[p_id].sec_y = (int)(objects[p_id].y / SECTOR_Y);

				m_seclock[objects[p_id].sec_x][objects[p_id].sec_y].lock();
				sectors[objects[p_id].sec_x][objects[p_id].sec_y].insert(p_id);
				m_seclock[objects[p_id].sec_x][objects[p_id].sec_y].unlock();
			}
		}
		else
			return;
		break;
	case 2:
		if (x > 0 && !obstacles[x - 1][y]) {
			x--;
			if (objects[p_id].sec_x != (int)(objects[p_id].x / SECTOR_X)) {
				m_seclock[objects[p_id].sec_x][objects[p_id].sec_y].lock();
				sectors[objects[p_id].sec_x][objects[p_id].sec_y].erase(p_id);
				m_seclock[objects[p_id].sec_x][objects[p_id].sec_y].unlock();

				objects[p_id].sec_x = (int)(objects[p_id].x / SECTOR_X);

				m_seclock[objects[p_id].sec_x][objects[p_id].sec_y].lock();
				sectors[objects[p_id].sec_x][objects[p_id].sec_y].insert(p_id);
				m_seclock[objects[p_id].sec_x][objects[p_id].sec_y].unlock();
			}
		}
		else
			return;
		break;
	case 3:
		if (x < (WORLD_WIDTH - 1) && !obstacles[x + 1][y]) {
			x++;
			if (objects[p_id].sec_x != (int)(objects[p_id].x / SECTOR_X)) {
				m_seclock[objects[p_id].sec_x][objects[p_id].sec_y].lock();
				sectors[objects[p_id].sec_x][objects[p_id].sec_y].erase(p_id);
				m_seclock[objects[p_id].sec_x][objects[p_id].sec_y].unlock();

				objects[p_id].sec_x = (int)(objects[p_id].x / SECTOR_X);

				m_seclock[objects[p_id].sec_x][objects[p_id].sec_y].lock();
				sectors[objects[p_id].sec_x][objects[p_id].sec_y].insert(p_id);
				m_seclock[objects[p_id].sec_x][objects[p_id].sec_y].unlock();
			}
		}
		else
			return;
		break;
	case 4:
		break;
	}
	objects[p_id].last_move_time = chrono::system_clock::now();
	unordered_set <int> old_vl;
	objects[p_id].vl_lock.lock();
	old_vl = objects[p_id].m_viewlist;
	objects[p_id].vl_lock.unlock();
	unordered_set <int> new_vl;

	search_sector(objects[p_id].sec_x, objects[p_id].sec_y, new_vl, p_id);

	int flagx = 0;
	int flagy = 0;
	
	if (objects[p_id].sec_x != 0) {
		if (objects[p_id].sec_x != (int)((objects[p_id].x - VIEW_RADIUS) / SECTOR_X))
		{
			flagx = -1;
		}
	}
	if (objects[p_id].sec_x != (int)(WORLD_WIDTH / SECTOR_X - 1)) {
		if (objects[p_id].sec_x != (int)((objects[p_id].x + VIEW_RADIUS) / SECTOR_X))
		{
			flagx = 1;
		}
	}
	if (objects[p_id].sec_y != 0) {
		if (objects[p_id].sec_y != (int)((objects[p_id].y - VIEW_RADIUS) / SECTOR_Y))
		{
			flagy = -1;
		}
	}
	if (objects[p_id].sec_y != (int)(WORLD_HEIGHT / SECTOR_Y - 1)) {
		if (objects[p_id].sec_y != (int)((objects[p_id].y + VIEW_RADIUS) / SECTOR_Y))
		{
			flagy = 1;
		}
	}

	if (flagx == -1) {
		switch (flagy) {
		case -1:
			search_sector(objects[p_id].sec_x - 1, objects[p_id].sec_y, new_vl, p_id);
			search_sector(objects[p_id].sec_x, objects[p_id].sec_y - 1, new_vl, p_id);
			search_sector(objects[p_id].sec_x - 1, objects[p_id].sec_y - 1, new_vl, p_id);
			break;
		case 0:
			search_sector(objects[p_id].sec_x - 1, objects[p_id].sec_y, new_vl, p_id);

			break;
		case 1:
			search_sector(objects[p_id].sec_x - 1, objects[p_id].sec_y, new_vl, p_id);
			search_sector(objects[p_id].sec_x, objects[p_id].sec_y + 1, new_vl, p_id);
			search_sector(objects[p_id].sec_x - 1, objects[p_id].sec_y + 1, new_vl, p_id);
			break;
		}
	}
	else if (flagx == 1) {
		switch (flagy) {
		case -1:
			search_sector(objects[p_id].sec_x + 1, objects[p_id].sec_y, new_vl, p_id);
			search_sector(objects[p_id].sec_x, objects[p_id].sec_y - 1, new_vl, p_id);
			search_sector(objects[p_id].sec_x + 1, objects[p_id].sec_y - 1, new_vl, p_id);
			break;
		case 0:
			search_sector(objects[p_id].sec_x + 1, objects[p_id].sec_y, new_vl, p_id);

			break;
		case 1:
			search_sector(objects[p_id].sec_x + 1, objects[p_id].sec_y, new_vl, p_id);
			search_sector(objects[p_id].sec_x, objects[p_id].sec_y + 1, new_vl, p_id);
			search_sector(objects[p_id].sec_x + 1, objects[p_id].sec_y + 1, new_vl, p_id);
			break;
		}
	}
	else {
		switch (flagy) {
		case -1:
			search_sector(objects[p_id].sec_x, objects[p_id].sec_y - 1, new_vl, p_id);
			break;
		case 1:
			search_sector(objects[p_id].sec_x, objects[p_id].sec_y + 1, new_vl, p_id);
			break;
		}
	}

	for (auto& pl : new_vl) {
		if (pl == p_id) continue;
		if (true == is_npc(pl)) {
			if (objects[pl].obj_class % 2 == 0)
			{
				OVERLAPPED_EXTENDED* ex_over = new OVERLAPPED_EXTENDED;
				ex_over->op_type = OP_PLAYER_ENCOUNTER;
				*reinterpret_cast<int*>(ex_over->packetbuf) = p_id;
				PostQueuedCompletionStatus(iocp_handle, 1, pl, &ex_over->overlapped);
			}
		}

	}

	send_position_packet(p_id, p_id);
	for (auto pl : new_vl) {
		if (0 == old_vl.count(pl)) {
			objects[p_id].vl_lock.lock();
			objects[p_id].m_viewlist.insert(pl);
			objects[p_id].vl_lock.unlock();
			send_add_packet(p_id, pl);
			if (is_npc(pl) == false) {
				objects[pl].vl_lock.lock();
				if (0 == objects[pl].m_viewlist.count(p_id)) {
					objects[pl].m_viewlist.insert(p_id);
					objects[pl].vl_lock.unlock();
					send_add_packet(pl, p_id);
				}
				else
				{
					objects[pl].vl_lock.unlock();
					send_position_packet(pl, p_id);
				}
			}
			else {
				wake_up_npc(pl);
			}
			
		}
		else {
			if (is_npc(pl) == false) {
				objects[pl].vl_lock.lock();
				if (0 == objects[pl].m_viewlist.count(p_id)) {
					objects[pl].m_viewlist.insert(p_id);
					objects[pl].vl_lock.unlock();
					send_add_packet(pl, p_id);
				}
				else
				{
					objects[pl].vl_lock.unlock();
					send_position_packet(pl, p_id);
				}
			}
			
		}
	}

	for (auto pl : old_vl) {
		if (0 == new_vl.count(pl)) {
			objects[p_id].vl_lock.lock();
			objects[p_id].m_viewlist.erase(pl);
			objects[p_id].vl_lock.unlock();
			send_remove_packet(p_id, pl);

			if (is_npc(pl) == false) {
				objects[pl].vl_lock.lock();
				if (0 != objects[pl].m_viewlist.count(p_id)) {
					objects[pl].m_viewlist.erase(p_id);
					objects[pl].vl_lock.unlock();
					send_remove_packet(pl, p_id);
				}
				else {
					objects[pl].vl_lock.unlock();
				}
			}
			
		}
	}
}

void do_chat(int p_id, const char* mess) {
	send_chat_packet(p_id, p_id, mess);
	for (auto pl : objects[p_id].m_viewlist)
	{
		if (!is_npc(pl))
			send_chat_packet(pl, p_id, mess);
	}
}

void do_attack_npc(int p_id, int o_id) {
	int dmg = objects[p_id].level * 3 + 3;
	objects[o_id].hp -= dmg;
	char buf[MAX_STR_LEN];
	sprintf_s(buf, "%s의 공격으로 %d의 피해를 입었습니다.", objects[p_id].m_name, dmg);
	send_chat_packet(o_id, -1, buf);
	if (objects[o_id].hp <= 0)
	{
		objects[p_id].is_tracking = false;

		objects[o_id].hp = (int)((objects[o_id].level * 10 + 90) / 2);
		objects[o_id].exp = (int)(objects[o_id].exp / 2);

		objects[o_id].vl_lock.lock();
		objects[o_id].m_viewlist.clear();
		objects[o_id].vl_lock.unlock();


		m_seclock[objects[o_id].sec_x][objects[o_id].sec_y].lock();
		sectors[objects[o_id].sec_x][objects[o_id].sec_y].erase(p_id);
		m_seclock[objects[o_id].sec_x][objects[o_id].sec_y].unlock();

		objects[o_id].x = 1000;
		objects[o_id].y = 1000;

		objects[o_id].sec_x = (int)(objects[o_id].x / SECTOR_X);
		objects[o_id].sec_y = (int)(objects[o_id].y / SECTOR_Y);

		m_seclock[objects[o_id].sec_x][objects[o_id].sec_y].lock();
		sectors[objects[o_id].sec_x][objects[o_id].sec_y].insert(p_id);
		m_seclock[objects[o_id].sec_x][objects[o_id].sec_y].unlock();

		do_move(o_id, 4);

		sprintf_s(buf, "%s의 공격으로 죽었습니다.", objects[p_id].m_name);
		send_chat_packet(o_id, -1, buf);


		DB_EVENT ev;
		ev.ev_type = OP_SET_PLAYER_INFO;
		ev.object_index = o_id;
		ev.object_exp = objects[o_id].exp;
		ev.object_hp = objects[o_id].hp;
		ev.object_level = objects[o_id].level;
		ev.object_x = objects[o_id].x;
		ev.object_y = objects[o_id].y;
		strcpy_s(ev.object_id, objects[o_id].m_name);

		add_db_access(ev);
	}
	send_stat_change_packet(o_id, o_id);
}

void do_attack_player(int p_id, int o_id) {
	int dmg = objects[p_id].level * 10 + 20;
	objects[o_id].hp -= dmg;

	char buf[MAX_STR_LEN];
	sprintf_s(buf, "%s에게 %d의 피해를 입혔습니다.", objects[o_id].m_name, dmg);
	send_chat_packet(p_id, -1, buf);

	if (!objects[o_id].is_tracking)
	{
		objects[o_id].is_tracking = true;
		add_event(o_id, OP_AI_MOVE, 1000, p_id);
	}
	if (objects[o_id].hp < 0)
	{
		objects[o_id].state_lock.lock();
		objects[o_id].object_state = STATE_CONNECTED;
		objects[o_id].is_active = false;
		objects[o_id].is_tracking = false;
		objects[o_id].state_lock.unlock();

		do_npc_move(objects[o_id], -1);

		int exp = objects[o_id].level * objects[o_id].level * 2;
		if (objects[o_id].obj_class % 2 == 0)
			exp *= 2;
		if (objects[o_id].obj_class >= 3)
			exp *= 2;

		objects[p_id].exp += exp;
		sprintf_s(buf, "%s에게서 %d의 경험치를 얻었습니다.", objects[o_id].m_name, exp);
		send_chat_packet(p_id, -1, buf);


		add_event(o_id, OP_RESPAWN, 30000);

		if (objects[p_id].exp > 100 * pow(2, (objects[p_id].level - 1)))
		{
			objects[p_id].level++;
			objects[p_id].exp = 0;

			sprintf_s(buf, "레벨이 올랐습니다.");
			send_chat_packet(p_id, -1, buf);

			DB_EVENT ev;
			ev.ev_type = OP_SET_PLAYER_INFO;
			ev.object_index = p_id;
			ev.object_exp = objects[p_id].exp;
			ev.object_hp = objects[p_id].hp;
			ev.object_level = objects[p_id].level;
			ev.object_x = objects[p_id].x;
			ev.object_y = objects[p_id].y;
			strcpy_s(ev.object_id, objects[p_id].m_name);

			add_db_access(ev);
		}


		send_stat_change_packet(p_id, p_id);
	}
}

void process_packet(int p_id, unsigned char* p_buf)
{
	switch (p_buf[1])
	{
	case CS_LOGIN: {
		cs_packet_login* packet = reinterpret_cast<cs_packet_login*>(p_buf);
		if (!search_player_id(packet->player_id))
		{
			DB_EVENT ev;
			ev.ev_type = OP_GET_PLAYER_INFO;
			ev.object_index = p_id;
			strcpy_s(ev.object_id, packet->player_id);
			add_db_access(ev);
			add_event(p_id, OP_REGEN_HP, 5000);
		}
		else {
			send_login_fail_packet(p_id);
		}
		
	}
		break;
	case CS_CHAT:
	{
		cs_packet_chat* packet = reinterpret_cast<cs_packet_chat*>(p_buf);
		do_chat(p_id, packet->message);
		
	}
		break;
	case CS_LOGOUT:
	{
		cs_packet_logout* packet = reinterpret_cast<cs_packet_logout*>(p_buf);

		DB_EVENT ev;
		ev.ev_type = OP_SET_PLAYER_INFO;
		ev.object_index = p_id;
		ev.object_exp = objects[p_id].exp;
		ev.object_hp = objects[p_id].hp;
		ev.object_level = objects[p_id].level;
		ev.object_x = objects[p_id].x;
		ev.object_y = objects[p_id].y;
		strcpy_s(ev.object_id, objects[p_id].m_name);

		add_db_access(ev);
	}
		break;
	case CS_ATTACK:
	{
		if ((chrono::system_clock::now() - objects[p_id].last_attack_time) < chrono::milliseconds(attck_cooldown))
			break;
		
		for (auto pl : objects[p_id].m_viewlist)
		{
			if (is_npc(pl))
			{
				if (abs(objects[pl].x - objects[p_id].x) + abs(objects[pl].y - objects[p_id].y) == 1)
				{
					do_attack_player(p_id, pl);
				}
			}
		}
		objects[p_id].last_attack_time = chrono::system_clock::now();
	}
		break;
	case CS_TELEPORT:
	{
		cs_packet_teleport* packet = reinterpret_cast<cs_packet_teleport*>(p_buf);
		m_seclock[objects[p_id].sec_x][objects[p_id].sec_y].lock();
		sectors[objects[p_id].sec_x][objects[p_id].sec_y].erase(p_id);
		m_seclock[objects[p_id].sec_x][objects[p_id].sec_y].unlock();

		objects[p_id].x = rand() % WORLD_WIDTH;
		objects[p_id].y = rand() % WORLD_HEIGHT;

		objects[p_id].sec_x = (int)(objects[p_id].x / SECTOR_X);
		objects[p_id].sec_y = (int)(objects[p_id].y / SECTOR_Y);

		m_seclock[objects[p_id].sec_x][objects[p_id].sec_y].lock();
		sectors[objects[p_id].sec_x][objects[p_id].sec_y].insert(p_id);
		m_seclock[objects[p_id].sec_x][objects[p_id].sec_y].unlock();

		do_move(p_id, 4);

	}
		break;

	case CS_MOVE:
	{
		cs_packet_move* packet = reinterpret_cast<cs_packet_move*>(p_buf);
		objects[p_id].move_time = packet->move_time;
		do_move(p_id, packet->direction);
	}
		break;
	default:
		//cout << "Unknown Packet Type from Client [" << p_id << "] Packet Type [" << p_buf[1] << "]" << endl;
		//exit(-1);
		break;
	}
}


void disconnect(int p_id) {
	{
		if (STATE_READY == objects[p_id].object_state) return;
		closesocket(objects[p_id].socket);
		objects[p_id].object_state = STATE_READY;
		sectors[objects[p_id].sec_x][objects[p_id].sec_y].erase(p_id);
	}
	for (auto& pl : objects) {
		if (false == is_npc(pl.id)) {
			if (STATE_INGAME == pl.object_state)
				send_remove_packet(pl.id, p_id);
		}
	}
}

void worker(HANDLE h_iocp, SOCKET l_socket) {
	while (true) {
		DWORD bytes_recved;
		ULONG_PTR ikey;
		WSAOVERLAPPED* over;

		BOOL ret = GetQueuedCompletionStatus(h_iocp, &bytes_recved, &ikey, &over, INFINITE);
		int key = static_cast<int>(ikey);

		if (FALSE == ret) {
			if (0 == key) {
				display_error("GQCS : ", WSAGetLastError());
				exit(-1);
			}
			else {
				display_error("GQCS : ", WSAGetLastError());
				disconnect(key);
			}
		}

		if ((key != 0) && (0 == bytes_recved)) {
			disconnect(key);
			continue;
		}


		OVERLAPPED_EXTENDED* over_ex = reinterpret_cast<OVERLAPPED_EXTENDED*>(over);

		switch (over_ex->op_type)
		{
		case OP_RECV: {
			unsigned char* packet_ptr = over_ex->packetbuf;
			int data_bytes = bytes_recved + objects[key].prev_packet_size;
			int packet_size = packet_ptr[0];

			while (data_bytes >= packet_size)
			{
				process_packet(key, packet_ptr);
				data_bytes -= packet_size;
				packet_ptr += packet_size;
				if (0 >= data_bytes)	break;
				packet_size = packet_ptr[0];
			}
			objects[key].prev_packet_size = data_bytes;
			if (data_bytes > 0)	
				memcpy(over_ex->packetbuf, packet_ptr, data_bytes);

			do_recv(key);
		}
					break;
		case OP_SEND:
			delete over_ex;
			break;
		case OP_ACCEPT:
		{
			int c_id = get_new_player_id(over_ex->socket);
			if (-1 == c_id) {
				send_login_fail_packet(over_ex->socket);
				closesocket(over_ex->socket);
				break;
			}

			
			
			objects[c_id].m_recv_over.op_type = OP_RECV;
			objects[c_id].prev_packet_size = 0;

			CreateIoCompletionPort(reinterpret_cast<HANDLE>(over_ex->socket), h_iocp, c_id, 0);

			do_recv(c_id);


			memset(&over_ex->overlapped, 0, sizeof(over_ex->overlapped));
			SOCKET c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
			over_ex->socket = c_socket;
			AcceptEx(l_socket, c_socket, over_ex->packetbuf, 0, 32, 32, NULL, &over_ex->overlapped);
			break;
		}
		case OP_RANDOM_MOVE:
		{
			if (!objects[key].is_tracking && objects[key].is_active)
			{
				bool ret = do_npc_move(objects[key], rand() % 4);
				if (ret == true) {
					add_event(key, OP_RANDOM_MOVE, 1000);
				}
				else {
					objects[key].is_active = false;
				}
			}
			delete over_ex;
			break;
		}
		case OP_PLAYER_ENCOUNTER:
		{
			objects[key].lua_lock.lock();
			int move_player = *reinterpret_cast<int*>(over_ex->packetbuf);
			lua_State* L = objects[key].L;
			lua_getglobal(L, "player_is_near");
			lua_pushnumber(L, move_player);
			lua_pcall(L, 1, 0, 0);
			objects[key].lua_lock.unlock();
			
			delete over_ex;
			break;
		}
		case OP_GET_PLAYER_INFO:
			send_login_ok_packet(key, *reinterpret_cast<DB_EVENT*>(over_ex->packetbuf));
			delete over_ex;

			break;
		break;
		case OP_REGEN_HP:
			if (objects[key].object_state == STATE_INGAME && objects[key].hp < objects[key].level * 10 + 90)
			{
				objects[key].hp = min(objects[key].level * 10 + 90, objects[key].hp + (objects[key].level * 10 + 90) / 10);
				send_stat_change_packet(key, key);
			}
			add_event(key, OP_REGEN_HP, 5000);
			break;
		case OP_AI_MOVE:
		{
			char dir = -2;
			int target = reinterpret_cast<TIMER_EVENT*>(over_ex->packetbuf)->target_id;
			if (!objects[key].is_active) {
				delete over_ex;
				break;
			}
			if (abs(objects[target].x - objects[key].x) > 8 || abs(objects[target].y - objects[key].y) > 8)
			{
				objects[key].is_tracking = false;
				delete over_ex;
				break;
			}
			if (abs(objects[target].x - objects[key].x) + abs(objects[target].y - objects[key].y) <= 1)
			{
				dir = -2;
				do_attack_npc(key, target);
			}
			int astar = a_star_search(key, target);

			bool ret = do_npc_move(objects[key], astar);


			if (ret == true) {
				add_event(key, OP_AI_MOVE, 1000, target);
			}
			else {
				objects[key].is_active = false;
				objects[key].is_tracking = false;
			}
			
			delete over_ex;
		}
			break;
		case OP_SAVE:
		{
			add_event(1, OP_SAVE, 300000);
			DB_EVENT ev;
			ev.ev_type = OP_SAVE;
			add_db_access(ev);
		}
			break;
		case OP_RESPAWN:
		{
			objects[key].is_active = STATE_INGAME;
			objects[key].x = objects[key].init_x;
			objects[key].y = objects[key].init_y;
			objects[key].sec_x = (int)(objects[key].x / SECTOR_X);
			objects[key].sec_y = (int)(objects[key].y / SECTOR_Y);
			objects[key].hp = objects[key].level * 10 + 90;
			sectors[objects[key].sec_x][objects[key].sec_y].insert(objects[key].id);
			do_npc_move(objects[key], 4);
		}
			break;
		}
		

	}
}

void timer_func() {
	using namespace chrono;
	add_event(1, OP_SAVE, 300000);

	while (true) {
		timer_lock.lock();
		if (!timer_queue.empty() &&
			timer_queue.top().event_time <= system_clock::now()) {
			TIMER_EVENT ev = timer_queue.top();
			timer_queue.pop();

			OVERLAPPED_EXTENDED* overlapped = new OVERLAPPED_EXTENDED;
			overlapped->op_type = ev.ev_type;
			memcpy(&overlapped->packetbuf[0], &ev, sizeof(TIMER_EVENT));

			PostQueuedCompletionStatus(iocp_handle, 1, ev.object_id, &overlapped->overlapped);
			timer_lock.unlock();
		}
		else {
			timer_lock.unlock();
			this_thread::sleep_for(10ms);
		}
	}
}

void HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode)
{
	SQLSMALLINT iRec = 0;
	SQLINTEGER iError;
	WCHAR wszMessage[1000];
	WCHAR wszState[SQL_SQLSTATE_SIZE + 1];
	if (RetCode == SQL_INVALID_HANDLE) {
		fwprintf(stderr, L"Invalid handle!\n");
		return;
	}
	while (SQLGetDiagRec(hType, hHandle, ++iRec, wszState, &iError, wszMessage,
		(SQLSMALLINT)(sizeof(wszMessage) / sizeof(WCHAR)), (SQLSMALLINT*)NULL) == SQL_SUCCESS) {
		// Hide data truncated..
		if (wcsncmp(wszState, L"01004", 5)) {
			fwprintf(stderr, L"[%5.5s] %s (%d)\n", wszState, wszMessage, iError);
		}
	}
}


void db_func() {
	using namespace chrono;
	SQLHENV henv;
	SQLHDBC hdbc;
	SQLHSTMT hstmt = 0;
	SQLRETURN retcode;
	SQLWCHAR szName[MAX_ID_LEN];
	SQLINTEGER char_level, char_hp, char_exp, char_x, char_y;
	SQLLEN cbCharhp = 0, cbCharexp = 0, cbCharLevel = 0, cbCharX = 0, cbCharY = 0;

	// Allocate environment handle  
	retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);

	// Set the ODBC version environment attribute  
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);

		// Allocate connection handle  
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);

			// Set login timeout to 5 seconds  
			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);

				// Connect to data source  
				retcode = SQLConnect(hdbc, (SQLWCHAR*)L"my_server", SQL_NTS, (SQLWCHAR*)NULL, 0, NULL, 0);
				while (true) {
					db_lock.lock();
					if (!db_queue.empty() &&
						db_queue.front().event_time <= system_clock::now()) {
						DB_EVENT ev = db_queue.front();
						db_queue.pop();

						switch (ev.ev_type) {
						case OP_GET_PLAYER_INFO:
						{
							retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
							SQLWCHAR buf[255];
							wchar_t oid[20];
							MultiByteToWideChar(CP_ACP, 0, ev.object_id, 20, oid, 20);
							wsprintf(buf, L"EXEC READ_PLAYERINFO %s", oid);
							retcode = SQLExecDirect(hstmt, (SQLWCHAR*)buf, SQL_NTS);

							OVERLAPPED_EXTENDED* overlapped = new OVERLAPPED_EXTENDED;
							overlapped->op_type = ev.ev_type;
							HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);

							if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {

								// Bind columns 1, 2, and 3  
								retcode = SQLBindCol(hstmt, 2, SQL_C_ULONG, &char_level, 10, &cbCharLevel);
								retcode = SQLBindCol(hstmt, 3, SQL_C_ULONG, &char_hp, 10, &cbCharhp);
								retcode = SQLBindCol(hstmt, 4, SQL_C_ULONG, &char_exp, 10, &cbCharexp);
								retcode = SQLBindCol(hstmt, 5, SQL_C_SHORT, &char_x, 10, &cbCharX);
								retcode = SQLBindCol(hstmt, 6, SQL_C_SHORT, &char_y, 10, &cbCharY);


								// Fetch and print each row of data. On an error, display a message and exit.  
								for (int i = 0; ; i++) {
									retcode = SQLFetch(hstmt);
									if (retcode == SQL_ERROR || retcode == SQL_SUCCESS_WITH_INFO) {
										HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
									}
									if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
									{
										ev.object_exp = char_exp;
										ev.object_hp = char_hp;
										ev.object_level = char_level;
										ev.object_x = char_x;
										ev.object_y = char_y;

									}
									else
									{
										if (i != 0)
											break; // end of data
										else
										{
											ev.object_exp = 0;
											ev.object_hp = 100;
											ev.object_level = 1;
											ev.object_x = 1000;
											ev.object_y = 1000;
											break;
										}
									}
								}
							}
							// Process data  
							
							SQLCancel(hstmt);
							SQLFreeHandle(SQL_HANDLE_STMT, hstmt);


							memcpy(&overlapped->packetbuf[0], &ev, sizeof(DB_EVENT));



							PostQueuedCompletionStatus(iocp_handle, 1, ev.object_index, &overlapped->overlapped);
						}
						break;
						case OP_SET_PLAYER_INFO:
						{
							retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
							SQLWCHAR buf[255];
							wchar_t oid[20];
							MultiByteToWideChar(CP_ACP, 0, ev.object_id, 20, oid, 20);
							wsprintf(buf, L"EXEC SAVE_PLAYERINFO %s, %d, %d, %d, %d, %d",oid, ev.object_level, ev.object_hp, ev.object_exp, ev.object_x, ev.object_y);
							retcode = SQLExecDirect(hstmt, (SQLWCHAR*)buf, SQL_NTS);
							HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
							// Process data  
							if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
								SQLCancel(hstmt);
								SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
							}
						}
						break;
						case OP_SAVE:
						{
							for (int i = 1; i < NPC_ID_START; i++) {
								if (objects[i].object_state == STATE_INGAME) {
									retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
									SQLWCHAR buf[255];
									wchar_t oid[20];
									MultiByteToWideChar(CP_ACP, 0, objects[i].m_name, 20, oid, 20);
									wsprintf(buf, L"EXEC SAVE_PLAYERINFO %s, %d, %d, %d, %d, %d", oid, objects[i].level, objects[i].hp, objects[i].exp, objects[i].x, objects[i].y);
									retcode = SQLExecDirect(hstmt, (SQLWCHAR*)buf, SQL_NTS);
									HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
									// Process data  
									if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
										SQLCancel(hstmt);
										SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
									}
								}
							}
						}
							break;


						}
						db_lock.unlock();
						
					}
					else {
						db_lock.unlock();
						this_thread::sleep_for(10ms);
					}
				}

				SQLDisconnect(hdbc);
				SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
				SQLFreeHandle(SQL_HANDLE_ENV, henv);
			}
		}
	}
	

	


	

}


bool do_npc_move(OBJECT& npc, char dir) {
	int x = npc.x;
	int y = npc.y;
	unordered_set<int> old_vl;


	int flagx = 0;
	int flagy = 0;

	search_sector(npc.sec_x, npc.sec_y, old_vl, npc.id);

	if (npc.sec_x != 0) {
		if (npc.sec_x != (int)((npc.x - VIEW_RADIUS) / SECTOR_X))
		{
			flagx = -1;
		}
	}
	if (npc.sec_x != (int)(WORLD_WIDTH / SECTOR_X - 1)) {
		if (npc.sec_x != (int)((npc.x + VIEW_RADIUS) / SECTOR_X))
		{
			flagx = 1;
		}
	}
	if (npc.sec_y != 0) {
		if (npc.sec_y != (int)((npc.y - VIEW_RADIUS) / SECTOR_Y))
		{
			flagy = -1;
		}
	}
	if (npc.sec_y != (int)(WORLD_HEIGHT / SECTOR_Y - 1)) {
		if (npc.sec_y != (int)((npc.y + VIEW_RADIUS) / SECTOR_Y))
		{
			flagy = 1;
		}
	}

	if (flagx == -1) {
		switch (flagy) {
		case -1:
			search_sector(npc.sec_x - 1, npc.sec_y, old_vl, npc.id);
			search_sector(npc.sec_x, npc.sec_y - 1, old_vl, npc.id);
			search_sector(npc.sec_x - 1, npc.sec_y - 1, old_vl, npc.id);
			break;
		case 0:
			search_sector(npc.sec_x - 1, npc.sec_y, old_vl, npc.id);

			break;
		case 1:
			search_sector(npc.sec_x - 1, npc.sec_y, old_vl, npc.id);
			search_sector(npc.sec_x, npc.sec_y + 1, old_vl, npc.id);
			search_sector(npc.sec_x - 1, npc.sec_y + 1, old_vl, npc.id);
			break;
		}
	}
	else if (flagx == 1) {
		switch (flagy) {
		case -1:
			search_sector(npc.sec_x + 1, npc.sec_y, old_vl, npc.id);
			search_sector(npc.sec_x, npc.sec_y - 1, old_vl, npc.id);
			search_sector(npc.sec_x + 1, npc.sec_y - 1, old_vl, npc.id);
			break;
		case 0:
			search_sector(npc.sec_x + 1, npc.sec_y, old_vl, npc.id);

			break;
		case 1:
			search_sector(npc.sec_x + 1, npc.sec_y, old_vl, npc.id);
			search_sector(npc.sec_x, npc.sec_y + 1, old_vl, npc.id);
			search_sector(npc.sec_x + 1, npc.sec_y + 1, old_vl, npc.id);
			break;
		}
	}
	else {
		switch (flagy) {
		case -1:
			search_sector(npc.sec_x, npc.sec_y - 1, old_vl, npc.id);
			break;
		case 1:
			search_sector(npc.sec_x, npc.sec_y + 1, old_vl, npc.id);
			break;
		}
	}



	switch (dir) {
	case -1:
		//제거
		for (auto pl : old_vl) {
			objects[pl].vl_lock.lock();
			if (objects[pl].m_viewlist.count(npc.id) != 0) {
				objects[pl].m_viewlist.erase(npc.id);
				objects[pl].vl_lock.unlock();

				send_remove_packet(pl, npc.id);
			}
			else {
				objects[pl].vl_lock.unlock();
			}
		}
		return false;
	case 0:
		if (y > 0 && y > npc.init_y - 20) {
			if (!obstacles[x][y - 1])
			{
				y--;
				if (npc.sec_y != (int)(npc.y / SECTOR_Y)) {
					m_seclock[npc.sec_x][npc.sec_y].lock();
					sectors[npc.sec_x][npc.sec_y].erase(npc.id);
					m_seclock[npc.sec_x][npc.sec_y].unlock();

					npc.sec_y = (int)(npc.y / SECTOR_Y);

					m_seclock[npc.sec_x][npc.sec_y].lock();
					sectors[npc.sec_x][npc.sec_y].insert(npc.id);
					m_seclock[npc.sec_x][npc.sec_y].unlock();
				}
			}
			
		}
		else
			return true;
		break;
	case 1:
		if (y < (WORLD_HEIGHT - 1) && y < npc.init_y + 20) {
			if (!obstacles[x][y + 1])
			{
				y++;
				if (npc.sec_y != (int)(npc.y / SECTOR_Y)) {
					m_seclock[npc.sec_x][npc.sec_y].lock();
					sectors[npc.sec_x][npc.sec_y].erase(npc.id);
					m_seclock[npc.sec_x][npc.sec_y].unlock();

					npc.sec_y = (int)(npc.y / SECTOR_Y);

					m_seclock[npc.sec_x][npc.sec_y].lock();
					sectors[npc.sec_x][npc.sec_y].insert(npc.id);
					m_seclock[npc.sec_x][npc.sec_y].unlock();
				}
			}
			
		}
		else
			return true;
		break;
	case 2:
		if (x > 0 && x > npc.init_x - 20) {
			if (!obstacles[x - 1][y])
			{
				x--;
				if (npc.sec_x != (int)(npc.x / SECTOR_X)) {
					m_seclock[npc.sec_x][npc.sec_y].lock();
					sectors[npc.sec_x][npc.sec_y].erase(npc.id);
					m_seclock[npc.sec_x][npc.sec_y].unlock();

					npc.sec_x = (int)(npc.x / SECTOR_X);

					m_seclock[npc.sec_x][npc.sec_y].lock();
					sectors[npc.sec_x][npc.sec_y].insert(npc.id);
					m_seclock[npc.sec_x][npc.sec_y].unlock();
				}
			}
		}
		else
			return true;
		break;
	case 3:
		if (x < (WORLD_WIDTH - 1) && x < npc.init_x + 20) {
			if (!obstacles[x + 1][y])
			{
				x++;
				if (npc.sec_x != (int)(npc.x / SECTOR_X)) {
					m_seclock[npc.sec_x][npc.sec_y].lock();
					sectors[npc.sec_x][npc.sec_y].erase(npc.id);
					m_seclock[npc.sec_x][npc.sec_y].unlock();

					npc.sec_x = (int)(npc.x / SECTOR_X);

					m_seclock[npc.sec_x][npc.sec_y].lock();
					sectors[npc.sec_x][npc.sec_y].insert(npc.id);
					m_seclock[npc.sec_x][npc.sec_y].unlock();
				}
			}
		}
		else
			return true;
		break;
	case 4:
		//생성
		for (auto pl : old_vl) {

			objects[pl].vl_lock.lock();
			objects[pl].m_viewlist.insert(npc.id);
			objects[pl].vl_lock.unlock();

			send_add_packet(pl, npc.id);

		}
		return true;
		break;
	default:
		break;
	}
	npc.x = x;
	npc.y = y;
	flagx = 0;
	flagy = 0;

	unordered_set<int> new_vl;

	search_sector(npc.sec_x, npc.sec_y, new_vl, npc.id);

	if (npc.sec_x != 0) {
		if (npc.sec_x != (int)((npc.x - VIEW_RADIUS) / SECTOR_X))
		{
			flagx = -1;
		}
	}
	if (npc.sec_x != (int)(WORLD_WIDTH / SECTOR_X - 1)) {
		if (npc.sec_x != (int)((npc.x + VIEW_RADIUS) / SECTOR_X))
		{
			flagx = 1;
		}
	}
	if (npc.sec_y != 0) {
		if (npc.sec_y != (int)((npc.y - VIEW_RADIUS) / SECTOR_Y))
		{
			flagy = -1;
		}
	}
	if (npc.sec_y != (int)(WORLD_HEIGHT / SECTOR_Y - 1)) {
		if (npc.sec_y != (int)((npc.y + VIEW_RADIUS) / SECTOR_Y))
		{
			flagy = 1;
		}
	}

	if (flagx == -1) {
		switch (flagy) {
		case -1:
			search_sector(npc.sec_x - 1, npc.sec_y, new_vl, npc.id);
			search_sector(npc.sec_x, npc.sec_y - 1, new_vl, npc.id);
			search_sector(npc.sec_x - 1, npc.sec_y - 1, new_vl, npc.id);
			break;
		case 0:
			search_sector(npc.sec_x - 1, npc.sec_y, new_vl, npc.id);

			break;
		case 1:
			search_sector(npc.sec_x - 1, npc.sec_y, new_vl, npc.id);
			search_sector(npc.sec_x, npc.sec_y + 1, new_vl, npc.id);
			search_sector(npc.sec_x - 1, npc.sec_y + 1, new_vl, npc.id);
			break;
		}
	}
	else if (flagx == 1) {
		switch (flagy) {
		case -1:
			search_sector(npc.sec_x + 1, npc.sec_y, new_vl, npc.id);
			search_sector(npc.sec_x, npc.sec_y - 1, new_vl, npc.id);
			search_sector(npc.sec_x + 1, npc.sec_y - 1, new_vl, npc.id);
			break;
		case 0:
			search_sector(npc.sec_x + 1, npc.sec_y, new_vl, npc.id);

			break;
		case 1:
			search_sector(npc.sec_x + 1, npc.sec_y, new_vl, npc.id);
			search_sector(npc.sec_x, npc.sec_y + 1, new_vl, npc.id);
			search_sector(npc.sec_x + 1, npc.sec_y + 1, new_vl, npc.id);
			break;
		}
	}
	else {
		switch (flagy) {
		case -1:
			search_sector(npc.sec_x, npc.sec_y - 1, new_vl, npc.id);
			break;
		case 1:
			search_sector(npc.sec_x, npc.sec_y + 1, new_vl, npc.id);
			break;
		}
	}


	for (auto pl : new_vl) {
		if (0 == old_vl.count(pl)) {
			objects[pl].vl_lock.lock();
			objects[pl].m_viewlist.insert(npc.id);
			objects[pl].vl_lock.unlock();

			send_add_packet(pl, npc.id);
		}
		else {
			send_position_packet(pl, npc.id);
		}
	}
	for (auto pl : old_vl) {
		if (0 == new_vl.count(pl)) {
			objects[pl].vl_lock.lock();
			if (objects[pl].m_viewlist.count(npc.id) != 0) {
				objects[pl].m_viewlist.erase(npc.id);
				objects[pl].vl_lock.unlock();

				send_remove_packet(pl, npc.id);
			}
			else {
				objects[pl].vl_lock.unlock();
			}
		}
	}

	if (new_vl.empty())
		return false;
	else
		return true;
}

int API_get_x(lua_State* L) {
	int obj_id = lua_tonumber(L, -1);
	lua_pop(L, 2);
	int x = objects[obj_id].x;
	lua_pushnumber(L, x);
	return 1;
}
int API_get_y(lua_State* L) {
	int obj_id = lua_tonumber(L, -1);
	lua_pop(L, 2);
	int y = objects[obj_id].y;
	lua_pushnumber(L, y);
	return 1;
}

int API_ATTACK_PLAYER(lua_State* L) {
	int o_id = lua_tonumber(L, -2);
	int p_id = lua_tonumber(L, -1);
	lua_pop(L, 3);
	if (objects[p_id].is_tracking == false)
	{
		add_event(p_id, OP_AI_MOVE, 1000, o_id);
		objects[p_id].is_tracking = true;
	}
	return 0;
}



int main()
{
	ifstream is("../../Protocol/obs.txt");
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

	cout << "npc 생성중" << endl;
	for (int i = 0; i < MAX_USER + 1; i++) {
		auto& pl = objects[i];
		pl.id = i;
		pl.object_state = STATE_READY;
		if (true == is_npc(i)) {
			pl.object_state = STATE_INGAME;
			pl.x = rand() % WORLD_WIDTH;
			pl.y = rand() % WORLD_HEIGHT;
			pl.init_x = pl.x;
			pl.init_y = pl.y;
			pl.sec_x = (int)(pl.x / SECTOR_X);
			pl.sec_y = (int)(pl.y / SECTOR_Y);
			pl.obj_class = rand() % 4 + 1;
			pl.level = rand() % 10;
			pl.hp = 90 + pl.level * 10;
			switch (pl.obj_class) {
			case 1:
				sprintf_s(pl.m_name, "슬라임lv%d", pl.level);
				break;
			case 2:
				sprintf_s(pl.m_name, "레드 슬라임lv%d", pl.level);
				break;
			case 3:
				sprintf_s(pl.m_name, "박쥐lv%d", pl.level);
				break;
			case 4:
				sprintf_s(pl.m_name, "붉은 박쥐lv%d", pl.level);
				break;
			}
			sectors[pl.sec_x][pl.sec_y].insert(pl.id);
			
			
			lua_State* L_state = pl.L = luaL_newstate();

			luaL_openlibs(L_state);
			luaL_loadfile(L_state, "npc.lua");
			int ret = lua_pcall(L_state, 0, 0, 0);

			lua_getglobal(L_state, "set_object_info");
			lua_pushnumber(L_state, i);
			lua_pcall(L_state, 1, 0, 0);

			lua_register(L_state, "API_get_x", API_get_x);
			lua_register(L_state, "API_get_y", API_get_y);
			lua_register(L_state, "API_ATTACK_PLAYER", API_ATTACK_PLAYER);

		}
	}
	cout << "npc 생성완료" << endl;

	
	setlocale(LC_ALL, "korean");
	wcout.imbue(locale("korean"));
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
	iocp_handle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);

	SOCKET l_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	CreateIoCompletionPort(reinterpret_cast<HANDLE>(l_socket), iocp_handle, 0, 0);

	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	::bind(l_socket, (struct sockaddr*)&serverAddr, sizeof(SOCKADDR_IN));
	listen(l_socket, SOMAXCONN);

	OVERLAPPED_EXTENDED accept_over;
	accept_over.op_type = OP_ACCEPT;
	memset(&accept_over.overlapped, 0, sizeof(accept_over.overlapped));
	SOCKET c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	accept_over.socket = c_socket;
	BOOL ret = AcceptEx(l_socket, c_socket, accept_over.packetbuf, 0, 32, 32, NULL, &accept_over.overlapped);
	if (FALSE == ret) {
		int err_num = WSAGetLastError();
		if (err_num != WSA_IO_PENDING)
			display_error("AcceptEX : ", err_num);
	}
	vector <thread> worker_threads;
	thread db_thread{ db_func };
	thread timer_thread{ timer_func };
	for (int i = 0; i < 4; ++i)
	{
		worker_threads.emplace_back(worker, iocp_handle, l_socket);
	}
	timer_thread.join();
	db_thread.join();
	for (auto& th : worker_threads)
	{
		th.join();
	}
	

	closesocket(l_socket);

	WSACleanup();

}