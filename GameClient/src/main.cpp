#include <iostream>
#include <stdio.h>
#include <tchar.h>
#include <enet/enet.h>
#include <string>
#include <cstring>
#include <thread>
#include <map>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "io/Screen.h"
//#include "chat_screen.hpp"

//static ChatScreen chatScreen;
static Screen screen(1270, 720);
static int CLIENT_ID = -1; // unique client ID assigned by server

class ClientData {
private:
	int m_id;
	std::string username;
public:
	ClientData(int id) : m_id(id) {}

	void SetUsername(std::string name) { username = name; }

	int GetID() const { return m_id; }
	const std::string& GetUsername() const { return username; }
};

std::map<int, ClientData*> client_map; // map of client ID to ClientData

void SendPacket(ENetPeer* peer, const char* data) {
	ENetPacket* packet = enet_packet_create(data, strlen(data) + 1, ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(peer, 0, packet); // send packet on channel 0

}

void ParseData(char* data) {
	int data_type;
	int id;
	sscanf_s(data, "%d|%d", &data_type, &id);

	switch (data_type) {
	case 1: // chat message
		if (id != CLIENT_ID) {
			char msg[80];
			sscanf_s(data, "%*d|%*d|%79[^|]", &msg, (unsigned)80);
			//if (client_map.find(id) != client_map.end()) {
			//chatScreen.PostMessageW(client_map[id]->GetUsername().c_str(), msg);
			//}
		}
		break;
	case 2: // user joined
		if (id != CLIENT_ID) {
			char username[80];
			sscanf_s(data, "%*d|%*d|%79[^|]", &username, (unsigned)80);
			client_map[id] = new ClientData(id);
			client_map[id]->SetUsername(username);
		}
		break;
	case 3:
		CLIENT_ID = id; // server assigns unique client ID
		break;
	}
}

void* MsgLoop(ENetHost* client) {
	while (true) {
		ENetEvent event;
		while (enet_host_service(client, &event, 1000) > 0) {
			switch (event.type) {
			case ENET_EVENT_TYPE_RECEIVE:
				//printf("Message from server: %s\n", event.packet->data);

				ParseData((char*)event.packet->data);
				enet_packet_destroy(event.packet); // clean up the packet now that we're done using it
				break;
			}
		}
	}
}

int main(int argc, char **argv) {
	
	printf("Please enter your username: ");
	char username[80];
	scanf_s("%79s", username, (unsigned)_countof(username));

	if (enet_initialize() != 0) {
		std::cerr << "An error occurred while initializing ENet." << std::endl;
		return EXIT_FAILURE;
	}

	atexit(enet_deinitialize); // deinitialize ENet at exit

	ENetHost* client;
	client = enet_host_create(NULL, 1, 1, 0, 0); // create a client host

	if (client == NULL) {
		std::cerr << "An error occurred while trying to create an ENet client host." << std::endl;
		return EXIT_FAILURE;
	}

	ENetAddress address;
	ENetEvent event; // data received from server
	ENetPeer* peer; // server peer connection too

	enet_address_set_host(&address, "127.0.0.1"); // set server address
	address.port = 8008; // set server port

	peer = enet_host_connect(client, &address, 1, 0); // connect to server
	if (peer == NULL) {
		std::cerr << "No available peers for initiating an ENet connection." << std::endl;
		enet_host_destroy(client);
		return EXIT_FAILURE;
	}

	// wait up to 5 seconds for the connection attempt to succeed
	if (enet_host_service(client, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
		puts("Connection to 127.0.0.1:8008 succeeded.");
	}
	else {
		enet_peer_reset(peer);
		puts("Connection to server failed.");
		return EXIT_SUCCESS; // exit without error, server might be offline
	}

	// Send username to server
	char str_data[80] = "2|";
	strcat_s(str_data, sizeof(str_data), username);
	SendPacket(peer, str_data); // send username to server

	// GAME LOOP START
	if (!screen.init()) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	//chatScreen.Init();

	// Create a thread to handle incoming messages
	std::thread msgThread(MsgLoop, client);

	while (!screen.shouldClose()) {
		screen.update();
		/*std::string msg = chatScreen.CheckBoxInput();
		if (msg == "/quit") break;
		chatScreen.PostMessageW(username, msg.c_str());

		char msg_data[80] = "1|";
		strcat_s(msg_data, sizeof(msg_data), msg.c_str());
		SendPacket(peer, msg_data);*/
		screen.newFrame();
	}

	// GAME LOOP END
	msgThread.join(); // wait for message thread to finish (it won't in this case)

	enet_peer_disconnect(peer, 0); // disconnect from server

	while(enet_host_service(client, &event, 3000) > 0) {
		switch (event.type) {
		case ENET_EVENT_TYPE_RECEIVE:
			enet_packet_destroy(event.packet); // clean up the packet now that we're done using it
			break;
		case ENET_EVENT_TYPE_DISCONNECT:
			puts("Disconnection succeeded.");
			break;
		}
	}

	return EXIT_SUCCESS;
}