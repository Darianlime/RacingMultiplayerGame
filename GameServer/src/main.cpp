#include <iostream>
#include <enet\enet.h>
#include <string>
#include <cstring>
#include <map>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <ctime>
#include <ratio>
#include <chrono>
#include <thread>

using namespace std::chrono;
using namespace std::this_thread;

struct Inputs {
	bool W = false;
	bool S = false;
	bool A = false;
	bool D = false;
	bool Space = false;
};

struct CarPacket {
	int parseType;
	int id;
	glm::vec3 pos;
	float rot;
	char assetImage[80];
};

struct Car {
	glm::vec3 pos;
	glm::vec3 size;
	float rot;

	float velocity;
	float accel;
	float minSpeed;
	float maxSpeed;
	float currentAngle;
	float forwardRot;

	std::string assetImage;

	Car() : 
		pos(0.0f), size(1.0f), rot(0.0f), velocity(0.0f), accel(0.0f), minSpeed(-800.0f), maxSpeed(2700.0f), currentAngle(0.0f), forwardRot(0.0f), assetImage("") { }

	glm::vec3 getForwardDirection() {
		float angleRad = glm::radians(currentAngle - 90.0f);
		return glm::normalize(glm::vec3(glm::sin(angleRad), glm::cos(angleRad), 0.0f));
	}
};

class ClientData {
private:
	int m_id;
	std::string username;

	std::unique_ptr<Car> car;
	Inputs inputState;
public:

	ClientData(int id) : m_id(id) {}

	void SetUsername(std::string name) { username = name; }
	void CreateCar() { car = std::make_unique<Car>(); }

	Car* GetCar() const { return car.get(); }
	Inputs& GetInputState() { return inputState; }

	int GetID() const { return m_id; }
	const std::string& GetUsername() const { return username; }
};

std::map<int, ClientData*> client_map; // map of client ID to ClientData

void BroadcastPacket(ENetHost* server, const char* data) {
	ENetPacket* packet = enet_packet_create(data, strlen(data) + 1, ENET_PACKET_FLAG_RELIABLE);
	enet_host_broadcast(server, 0, packet); // broadcast packet on channel 0
	enet_host_flush(server);
}

void BroadcastPacket(ENetHost* server, CarPacket data) {
	ENetPacket* packet = enet_packet_create(&data, sizeof(data), ENET_PACKET_FLAG_RELIABLE);
	enet_host_broadcast(server, 0, packet); // broadcast packet on channel 0
	enet_host_flush(server);
}

void SendPacket(ENetPeer* peer, const char* data) {
	ENetPacket* packet = enet_packet_create(data, strlen(data) + 1, ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(peer, 0, packet); // send packet on channel 0
}

void SendPacket(ENetPeer* peer, CarPacket data) {
	ENetPacket* packet = enet_packet_create(&data, sizeof(data), ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(peer, 0, packet); // send packet on channel 0
}

void ParseData(ENetHost* server, ENetPeer* peer, int id, char* data) {

	int data_type;
	sscanf_s(data, "%d|", &data_type);

	switch (data_type) {
	case 2: // Get username from client
	{
		char username[80] = { '\0' };
		sscanf_s(data, "2|%79[^\n]", &username, (unsigned)80);

		char send_data[1024] = { '\0' };
		sprintf_s(send_data, sizeof(send_data), "2|%d|%s", id, username);
		std::cout << "Broadcasting: " << send_data << std::endl;

		BroadcastPacket(server, send_data);
		client_map[id]->SetUsername(username);
		break;
	}
	case 3: // process input update physics
	{
		unsigned int input = 0;
		sscanf_s(data, "3|%u", &input);
		//std::cout << "Input Sent by " << input << peer->address.host << peer->address.port << std::endl;
		Inputs& inputState = client_map[id]->GetInputState();
		inputState.W = (input & (1 << 0));
		inputState.S = (input & (1 << 1)) >> 1;
		inputState.A = (input & (1 << 2)) >> 2;
		inputState.D = (input & (1 << 3)) >> 3;
		inputState.Space = (input & (1 << 4)) >> 4;
		//std::cout << inputState.W << inputState.S << inputState.A << inputState.D << std::endl;
		break;
	} 
	case 4: // create car packet for new car and broadcast to other clients
	{
		char img[80] = { '\0' };
		sscanf_s(data, "4|%79[^\n]", img, (unsigned)sizeof(img));
		printf("%s", img);
		CarPacket packet = { 3, id, glm::vec3(0.0f), 0, "" };
		strcpy_s(packet.assetImage, sizeof(packet.assetImage), img);
		BroadcastPacket(server, packet);
		client_map[id]->GetCar()->assetImage = img;

		for (auto const& [id, client] : client_map) {
			Car* car = client->GetCar();
			CarPacket packet = { 2, id, car->pos, car->rot, ""};
			strcpy_s(packet.assetImage, sizeof(packet.assetImage), car->assetImage.c_str());
			SendPacket(peer, packet);
		}
		break;
	}
	}
}

void ParseData(ENetHost* server, ENetPeer* peer, int id, CarPacket* data) {
	printf("ParseType: %d\n", data->parseType);
	switch (data->parseType) {
	case 1:
		//printf("parsing car packet id: %d\n", id);
		CarPacket packet = { 2, id, data->pos, data->rot, "" };
		strcpy_s(packet.assetImage, sizeof(packet.assetImage), data->assetImage);
		BroadcastPacket(server, packet);
		break;
	}
}

void PhysicsUpdate(double fixedDeltaTime) {
	for (auto& [id, client] : client_map) {
		Car* car = client->GetCar();
		Inputs& inputState = client_map[id]->GetInputState();
		//std::cout << "inital " << inputState.W << inputState.S << inputState.A << inputState.D << std::endl;
		if (inputState.W) {
			car->accel = car->maxSpeed;
		}
		else if (inputState.S) {
			car->accel = car->minSpeed;
		}
		else {
			car->accel = 0.0;
		}
		car->velocity += car->accel * fixedDeltaTime;
		float drag = 500.0f + 0.3f * glm::abs(car->velocity);
		if (car->velocity > 0.0f)
			car->velocity -= drag * fixedDeltaTime;
		else if (car->velocity < 0.0f)
			car->velocity += drag * fixedDeltaTime;
		car->velocity = glm::clamp(car->velocity, car->minSpeed, car->maxSpeed);
		//printf("Velocity: %.2f\n", client_map[id]->GetCar()->velocity);
		glm::vec3 forward = car->getForwardDirection();
		forward = glm::normalize(forward);
		car->pos += forward * (car->velocity * (float)fixedDeltaTime);
		//printf("x: %.2f, y: %.2f\n", client_map[id]->GetCar()->pos.x, client_map[id]->GetCar()->pos.y);
		car->currentAngle = -car->forwardRot;

		//zero out inputs
		memset(&inputState, 0, sizeof(struct Inputs));
		//std::cout << "end " << inputState.W << inputState.S << inputState.A << inputState.D << std::endl;
	}
}

int main(int argc, char** argv) {
	if (enet_initialize() != 0) {
		std::cerr << "An error occurred while initializing ENet." << std::endl;
		return EXIT_FAILURE;
	}
	atexit(enet_deinitialize); // deinitialize ENet at exit

	ENetAddress address;
	ENetHost* server;
	ENetEvent event; // data received from clients

	address.host = ENET_HOST_ANY; // bind the server to any address
	address.port = 8008; // bind the server to port 8008

	server = enet_host_create(&address, 32, 1, 0, 0); // create a server host
	if (server == NULL) {
		std::cerr << "An error occurred while trying to create an ENet server host." << std::endl;
		return EXIT_FAILURE;
	}
	std::cout << "Server started on port " << address.port << std::endl;

	const double fixedHertz = 1.0 / 60.0; // 60 ticks per sec 
	steady_clock::time_point lastTime = high_resolution_clock::now();
	double atHertz = 0.0;

	// GAME LOOP START
	int new_client_id = 0;
	while (true) {
		steady_clock::time_point currentTime = high_resolution_clock::now();
		double deltaTime = duration_cast<duration<double>>(currentTime - lastTime).count();
		lastTime = high_resolution_clock::now();
		atHertz += deltaTime;
		// wait up to 1000 milliseconds for an event
		while (enet_host_service(server, &event, 0) > 0) {
			switch (event.type) {
			case ENET_EVENT_TYPE_CONNECT:
			{
				//std::cout << "A new client connected from "
				//	<< event.peer->address.host << ":" << event.peer->address.port << "." << std::endl;
				//event.peer->data = (void*)"Client information"; // store some data about the peer
				for (auto const& pair : client_map) {
					char send_data[1024] = { '\0' };
					sprintf_s(send_data, sizeof(send_data), "2|%d|%s", pair.first, pair.second->GetUsername().c_str());
					/*ENetPacket* packet = enet_packet_create(send_data, strlen(send_data) + 1, ENET_PACKET_FLAG_RELIABLE);
					enet_peer_send(event.peer, 0, packet);*/
					BroadcastPacket(server, send_data);
				}

				// new client 
				new_client_id++;
				client_map[new_client_id] = new ClientData(new_client_id);
				client_map[new_client_id]->CreateCar();
				event.peer->data = client_map[new_client_id]; // store client ID as peer data

				// 3 new id
				char data_to_send[126] = { '\0' };
				sprintf_s(data_to_send, sizeof(data_to_send), "3|%d", new_client_id);
				SendPacket(event.peer, data_to_send);

				break;
			}
			case ENET_EVENT_TYPE_RECEIVE:
			{
				//std::cout << "A packet of length " << event.packet->dataLength << " was received from "
				//	<< (char*)event.packet->data << " with peer address " << event.peer->address.host << " and port " << event.peer->address.port
				//	<< " on channel " << (int)event.channelID << "." << std::endl;
				//std::cout << "receiving data " << std::endl;
				if (event.packet->dataLength == sizeof(CarPacket)) {
					//std::cout << "parsing Car packet " << std::endl;
					ParseData(server, event.peer, static_cast<ClientData*>(event.peer->data)->GetID(), reinterpret_cast<CarPacket*>(event.packet->data));
				}
				else {
					//std::cout << "parsing event packet " << std::endl;
					ParseData(server, event.peer, static_cast<ClientData*>(event.peer->data)->GetID(), (char*)event.packet->data);
				}
				enet_packet_destroy(event.packet); // clean up the packet now that we're done using it
		
				break;
			}
			case ENET_EVENT_TYPE_DISCONNECT:
			{
				std::cout << (char*)event.peer->data << event.peer->address.host << event.peer->address.port << " disconnected." << std::endl;

				char disconnected_data[126] = { '\0' };
				sprintf_s(disconnected_data, sizeof(disconnected_data), "4|%d", static_cast<ClientData*>(event.peer->data)->GetID());
				BroadcastPacket(server, disconnected_data);

				event.peer->data = NULL;
				break;
			}
			}
		}

		// physics update
		while (atHertz >= fixedHertz) {
			PhysicsUpdate(fixedHertz);
			atHertz -= fixedHertz;
		}

		// send updated transform to clients
		for (auto const& [id, client] : client_map) {
			Car* car = client->GetCar();
			CarPacket packet = { 1, id, car->pos, car->rot, "" };
			strcpy_s(packet.assetImage, sizeof(packet.assetImage), car->assetImage.c_str());
			BroadcastPacket(server, packet);
		}

		sleep_for(std::chrono::milliseconds(1));
	}
	// GAME LOOP END
	enet_host_destroy(server); // destroy the server host

	return EXIT_SUCCESS;
}
