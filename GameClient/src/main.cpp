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

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "io/Screen.h"
#include "io/Keyboard.h"
#include "io/Joystick.h"
#include "io/Mouse.h"

#include "graphics/models/CarRenderer.h"
#include "graphics/Shader.h"

static Screen screen(1270, 720);
static Joystick mainJ(0);
static int CLIENT_ID = -1; // unique client ID assigned by server

class ClientData {
private:
	int m_id;
	std::string username;

	std::unique_ptr<CarRenderer> carRenderer;
	
public:
	ClientData(int id) : m_id(id) {}

	void SetUsername(std::string name) { username = name; }
	void CreateCar(glm::vec3 pos, const char* assetImage) { carRenderer = std::make_unique<CarRenderer>(pos, assetImage); }

	CarRenderer* GetCar() const { return carRenderer.get(); }

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
	{
		if (id != CLIENT_ID) {
			char msg[80];
			sscanf_s(data, "%*d|%*d|%79[^|]", &msg, (unsigned)80);
			//if (client_map.find(id) != client_map.end()) {
			//chatScreen.PostMessageW(client_map[id]->GetUsername().c_str(), msg);
			//}
		}
		break;
	}
	case 2: // user joined
	{
		if (id != CLIENT_ID) {
			std::cout << "id assigned?" << id << std::endl;
			char username[80];
			sscanf_s(data, "%*d|%*d|%79[^|]", &username, (unsigned)80);
			client_map[id] = new ClientData(id);
			client_map[id]->SetUsername(username);
		}
		break;
	}
	case 3: // new client data
	{
		CLIENT_ID = id; // server assigns unique client ID
		std::cout << "id assigned?" << CLIENT_ID << std::endl;
		client_map[id] = new ClientData(id);
		break;
	}
	case 4: // update player render
	{
		break;
	}
	}
}

void* MsgLoop(ENetHost* client) {
	while (true) {
		ENetEvent event;
		while (enet_host_service(client, &event, 10) > 0) {
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

void processInput(float dt)
{
	if (Keyboard::key(GLFW_KEY_ESCAPE) || mainJ.buttonState(GLFW_JOYSTICK_BTN_RIGHT)) {
		printf("in escape\n");
		//screen.setShouldClose(true);
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

	// Create a thread to handle incoming messages
	std::thread msgThread(MsgLoop, client);

	// GAME LOOP START
	if (!screen.init()) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Shader shader("assets/object.vert", "assets/object.frag");
	//client_map[CLIENT_ID]->CreateCar(glm::vec3(0.0f), "assets/car1_2.png");

	client_map[CLIENT_ID]->SetUsername(username);
	client_map[CLIENT_ID]->CreateCar(glm::vec3(0.0f), "assets/car1_2.png");

	float deltaTime = 0.0f;
	float lastFrame = 0.0f;
	while (!screen.shouldClose()) {
		float currentTime = glfwGetTime();
		deltaTime = currentTime - lastFrame;
		lastFrame = currentTime;
		if (deltaTime > 0.033f)
			deltaTime = 0.033f;

		processInput(deltaTime);
		//send inputs to client to parse
		char msg_data[32] = { '\0' };
		unsigned int input = 0;
		if (Keyboard::key(GLFW_KEY_W)) {
			printf("W");
			input |= 1 << 0;
		}
		if (Keyboard::key(GLFW_KEY_S)) {
			input |= 1 << 1;
			printf("S");
		}
		if (Keyboard::key(GLFW_KEY_A)) {
			printf("A");
			input |= 1 << 2;
		}
		if (Keyboard::key(GLFW_KEY_D)) {
			printf("D");
			input |= 1 << 3;
		}
		if (Keyboard::key(GLFW_KEY_SPACE)) {
			printf("SPACE");
			input |= 1 << 4;
		}
		if (input) {
			sprintf_s(msg_data, sizeof(msg_data), "3|%d", input);
			SendPacket(peer, msg_data);
		}

		screen.update();

		shader.activate();

		// create trasformation for screen
		glm::mat4 view = glm::mat4(1.0f);
		glm::mat4 projection = glm::mat4(1.0f);

		projection = glm::ortho(-float(Screen::SCR_WIDTH), float(Screen::SCR_WIDTH), -float(Screen::SCR_HEIGHT), float(Screen::SCR_HEIGHT), -1.0f, 1.0f);
		view = glm::mat4(1.0f);

		shader.setMat4("view", view);
		shader.setMat4("projection", projection);

		client_map[CLIENT_ID]->GetCar()->render(shader);
		screen.newFrame();
		/*std::string msg = chatScreen.CheckBoxInput();
		if (msg == "/quit") break;
		chatScreen.PostMessageW(username, msg.c_str());

		char msg_data[80] = "1|";
		strcat_s(msg_data, sizeof(msg_data), msg.c_str());
		SendPacket(peer, msg_data);*/
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

