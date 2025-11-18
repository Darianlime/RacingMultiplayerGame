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

#include "Physics/Collision2D.h"
#include "Models/Quad.hpp"
#include <string.h>
#include <cstdint>
#include <deque>

using namespace std::chrono;
using namespace std::this_thread;

constexpr float SERVER_TICK_RATE = 64.0f;
constexpr uint8_t CHANNEL_UNSEQUENCED = 0;
constexpr uint8_t CHANNEL_RELIABLE = 1;
constexpr uint8_t HISTORY_SIZE = 200;

uint64_t currentServerTick = 0;

typedef enum {
	W, S, A, D, SPACE
} KeyInput;

enum PacketType {
	NEW_CLIENT_PACKET,
	INPUT_PACKET,
	CAR_PACKET,
	CAR_PHYSICS_PACKET
};

struct InputState {
	bool W = false;
	bool S = false;
	bool A = false;
	bool D = false;
	bool Space = false;
	uint64_t currentTick;
};

struct PacketHeader {
	uint8_t packetType;
	uint64_t currentTick;
	int parseType;
	int id;
};

struct StateHistoryPacket {
	uint64_t serverTick;
	InputState inputState;
	glm::vec3 pos;
	float rot;
	float currentAngle;
};

struct ClientDataPacket {
	PacketHeader packetHeader;
	char username[28];
};

struct InputPacket {
	PacketHeader packetHeader;
	InputState inputs;
};

struct CarState {
	glm::vec3 pos;
	float rot;
	float currentAngle;
	float velocity;
	float forwardRot;
};

struct CarPacket {
	PacketHeader packetHeader;
	CarState carState;
};

struct CarPacketImage {
	PacketHeader packetHeader;
	CarState carState;
	char assetImage[80];
};

struct CarPhysicsPacket {
	PacketHeader packetHeader;
	glm::vec3 worldVerts[Quad::noVertices];
};

struct Car {
	Quad transform;

	float velocity;
	float accel;
	float minSpeed;
	float maxSpeed;
	float currentAngle;
	float forwardRot;

	std::string assetImage;

	Car() : 
		transform(DYNAMIC), velocity(0.0f), accel(0.0f), minSpeed(-800.0f), maxSpeed(2700.0f), currentAngle(0.0f), forwardRot(0.0f), assetImage("") { }

	glm::vec3 getForwardDirection() {
		float angleRad = glm::radians(currentAngle - 90.0f);
		return glm::normalize(glm::vec3(glm::sin(angleRad), glm::cos(angleRad), 0.0f));
	}

	Quad& getTransform() { return transform; }
};

class ClientData {
private:
	int m_id;
	std::string username;
	std::unique_ptr<Car> car;
public:
	bool ready = false;
	std::map<uint64_t, InputState> inputStateHistory;
	std::deque<StateHistoryPacket> statePacketHistory;
	InputState lastKnownInput;

	ClientData(int id) : m_id(id) {}

	void SetUsername(std::string name) { username = name; }
	void CreateCar() { car = std::make_unique<Car>(); }

	Car* GetCar() const { return car.get(); }
	InputState& GetInputState(int index) { return inputStateHistory[index]; }
	StateHistoryPacket& GetStateHistory(int index) { return statePacketHistory[index % HISTORY_SIZE]; }
	void SetStateHistory(StateHistoryPacket packet, int index) { statePacketHistory[index % HISTORY_SIZE] = packet; }

	int GetID() const { return m_id; }
	const std::string& GetUsername() const { return username; }
};

std::map<int, ClientData*> client_map; // map of client ID to ClientData

void BroadcastPacketReliable(ENetHost* server, const char* data) {
	ENetPacket* packet = enet_packet_create(data, strlen(data) + 1, ENET_PACKET_FLAG_RELIABLE);
	enet_host_broadcast(server, CHANNEL_RELIABLE, packet); // broadcast packet on channel 0
}

void BroadcastPacketReliable(ENetHost* server, CarPacket data) {
	ENetPacket* packet = enet_packet_create(&data, sizeof(data), ENET_PACKET_FLAG_RELIABLE);
	enet_host_broadcast(server, CHANNEL_RELIABLE, packet); // broadcast packet on channel 0
}

void BroadcastPacketReliable(ENetHost* server, CarPacketImage data) {
	ENetPacket* packet = enet_packet_create(&data, sizeof(data), ENET_PACKET_FLAG_RELIABLE);
	enet_host_broadcast(server, CHANNEL_RELIABLE, packet); // broadcast packet on channel 0
}

void BroadcastPacketReliable(ENetHost* server, ClientDataPacket data) {
	ENetPacket* packet = enet_packet_create(&data, sizeof(data), ENET_PACKET_FLAG_RELIABLE);
	enet_host_broadcast(server, CHANNEL_RELIABLE, packet); // broadcast packet on channel 0
}

void BroadcastPacketUnseq(ENetHost* server, CarPacket data) {
	ENetPacket* packet = enet_packet_create(&data, sizeof(data), ENET_PACKET_FLAG_UNSEQUENCED);
	enet_host_broadcast(server, CHANNEL_UNSEQUENCED, packet); // broadcast packet on channel 0
}

void SendPacketReliable(ENetPeer* peer, const char* data) {
	ENetPacket* packet = enet_packet_create(data, strlen(data) + 1, ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(peer, CHANNEL_RELIABLE, packet); // send packet on channel 0
}

void SendPacketReliable(ENetPeer* peer, CarPacketImage data) {
	ENetPacket* packet = enet_packet_create(&data, sizeof(data), ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(peer, CHANNEL_RELIABLE, packet); // send packet on channel 0
}

void SendPacketReliable(ENetPeer* peer, ClientDataPacket data) {
	ENetPacket* packet = enet_packet_create(&data, sizeof(data), ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(peer, CHANNEL_RELIABLE, packet); // send packet on channel 0
}

void SendPacketReliable(ENetPeer* peer, CarPacket data) {
	ENetPacket* packet = enet_packet_create(&data, sizeof(data), ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(peer, CHANNEL_RELIABLE, packet); // send packet on channel 0
}

void SendPacketUnseq(ENetPeer* peer, CarPacket data) {
	ENetPacket* packet = enet_packet_create(&data, sizeof(data), ENET_PACKET_FLAG_UNSEQUENCED);
	enet_peer_send(peer, CHANNEL_UNSEQUENCED, packet); // send packet on channel 0
}

void ParseData(ENetHost* server, ENetPeer* peer, int id, char* data) {

	int data_type;
	sscanf_s(data, "%d|", &data_type);

	switch (data_type) {
	case 2: // Get username from client
	{
		char username[28] = { '\0' };
		sscanf_s(data, "2|%28[^\n]", &username, (unsigned)28);

		ClientDataPacket data = {NEW_CLIENT_PACKET, 0, 1, id, ""};
		strcpy_s(data.username, sizeof(data.username), username);

		std::cout << "Broadcasting: " << data.username << std::endl;

		BroadcastPacketReliable(server, data);
		client_map[id]->SetUsername(username);
		break;
	}
	case 4: // create car packet for new car and broadcast to other clients
	{
		ClientData* client = client_map[id];

		char img[80] = { '\0' };
		sscanf_s(data, "4|%79[^\n]", img, (unsigned)sizeof(img));

		if (!client->GetCar()) {
			client->CreateCar();
			client->GetCar()->assetImage = img;
		}

		printf("%s", img);
		CarPacketImage packet = { PacketType::CAR_PACKET, 0, 2, id, glm::vec3(0.0f), 0.0f, 0.0f, 0.0f, 0.0f, ""};
		strcpy_s(packet.assetImage, sizeof(packet.assetImage), img);
		BroadcastPacketReliable(server, packet);

		for (auto const& [other_id, client] : client_map) {
			Car* car = client->GetCar();
			if (!car || other_id == id) continue;
			CarPacketImage packet = { PacketType::CAR_PACKET, 0, 2, other_id, car->getTransform().pos, car->getTransform().rot, car->currentAngle, car->velocity, car->forwardRot, ""};
			strcpy_s(packet.assetImage, sizeof(packet.assetImage), car->assetImage.c_str());
			SendPacketReliable(peer, packet);
		}
		
		client_map[id]->ready = true;
		break;
	}
	}
}

void ParseData(ENetHost* server, ENetPeer* peer, int id, CarPacket* data) {
	//printf("ParseType: %d\n", data->parseType);
	//switch (data->parseType) {
	//case 1:
	//	//printf("parsing car packet id: %d\n", id);
	//	CarPacket packet = { 2, id, data->pos, data->rot, "" };
	//	strcpy_s(packet.assetImage, sizeof(packet.assetImage), data->assetImage);
	//	BroadcastPacket(server, packet);
	//	break;
	//}
}

void ParseData(ENetHost* server, ENetPeer* peer, int id, InputPacket* data) {
	switch (data->packetHeader.parseType) {
	case 1:
		//printf("parsing car packet id: %d\n", id);
		//client_map[id]->currentTick = data->packetHeader.currentTick;

		InputState input = data->inputs;
		if (input.currentTick > currentServerTick) {
			client_map[id]->inputStateHistory[input.currentTick] = input;
		}

		//std::cout << "Input Sent by " << input << peer->address.host << peer->address.port << std::endl;
	/*	InputState& inputState = client_map[id]->GetInputState(indexInput);
		inputState.W = input.W;
		inputState.S = input.S;
		inputState.A = input.A;
		inputState.D = input.D;
		inputState.Space = input.Space;*/
		break;
	}
}

void ParseData(ENetHost* server, ENetPeer* peer, int id, CarPhysicsPacket* data) {
	switch (data->packetHeader.parseType) {
	case 1:
		//printf("parsing car packet id: %d\n", id);
		client_map[id]->GetCar()->getTransform().worldVerts.assign(data->worldVerts, data->worldVerts + Quad::noVertices);
		break;
	}
}

void PhysicsUpdate(double fixedDeltaTime) {
	for (auto& [id, client] : client_map) {
		Car* car = client->GetCar();
		if (car != nullptr) {
			InputState inputState;
			if (client_map[id]->inputStateHistory.count(currentServerTick) < 1) {
				inputState = client_map[id]->inputStateHistory[client->lastKnownInput.currentTick];
			}
			else {
				inputState = client_map[id]->inputStateHistory[currentServerTick];
			}
			//std::cout << "inital " << inputState.W << inputState.S << inputState.A << inputState.D << std::endl;
			if (inputState.W) {
				car->accel = car->maxSpeed;
			}
			else if (inputState.S) {
				car->accel = -2700.0f;
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
			car->velocity = glm::clamp(car->velocity, -2700.0f, car->maxSpeed);

			float turnRate = 100.0f;
			float calcRot = glm::sign(car->velocity) * turnRate * (float)fixedDeltaTime;

			if (inputState.D && glm::abs(car->velocity) > 100.0f) {
				car->getTransform().rot -= calcRot;
				car->forwardRot -= calcRot;
			}
			if (inputState.A && glm::abs(car->velocity) > 100.0f) {
				car->getTransform().rot += calcRot;
				car->forwardRot += calcRot;
			}
			if (inputState.Space && (!((inputState.Space && inputState.D)) || !((inputState.Space && inputState.A)))) {
				car->velocity -= 100.0f * float(fixedDeltaTime);
				if (car->velocity < 0.0f) car->velocity = 0.0f;
			}

			glm::vec3 forward = car->getForwardDirection();
			forward = glm::normalize(forward);
			car->getTransform().pos += forward * car->velocity * (float)fixedDeltaTime;
			car->currentAngle = -car->forwardRot;
		}
	}
	for (auto& [id1, client1] : client_map) {
		Car* car1 = client1->GetCar();
		for (auto& [id2, client2] : client_map) {
			if (id1 == id2) {
				continue;
			}
			Car* car2 = client2->GetCar();
			if (car1 != nullptr && car2 != nullptr) {
				bool collision1 = Collision2D::checkOBBCollisionResolve(car1->getTransform(), car2->getTransform());
			}
		}
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

	server = enet_host_create(&address, 32, 2, 0, 0); // create a server host
	if (server == NULL) {
		std::cerr << "An error occurred while trying to create an ENet server host." << std::endl;
		return EXIT_FAILURE;
	}
	std::cout << "Server started on port " << address.port << std::endl;

	const double FIXED_HERTZ = 1.0 / SERVER_TICK_RATE; // 64 ticks per sec 
	const auto TICK_DURATION = std::chrono::duration<double>(FIXED_HERTZ);

	steady_clock::time_point lastTime = high_resolution_clock::now();
	double atHertz = 0.0;

	// GAME LOOP START
	int new_client_id = 0;
	while (true) {
		steady_clock::time_point currentTime = high_resolution_clock::now();
		double deltaTime = duration_cast<duration<double>>(currentTime - lastTime).count();
		lastTime = currentTime;
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
					if (pair.second->GetCar() == nullptr) continue;

					ClientDataPacket data = { NEW_CLIENT_PACKET, 0, 1, pair.first, ""};
					strcpy_s(data.username, sizeof(data.username), pair.second->GetUsername().c_str());
					/*ENetPacket* packet = enet_packet_create(send_data, strlen(send_data) + 1, ENET_PACKET_FLAG_RELIABLE);
					enet_peer_send(event.peer, 0, packet);*/
					SendPacketReliable(event.peer, data);
				}

				// new client 
				new_client_id++;
				client_map[new_client_id] = new ClientData(new_client_id);
				event.peer->data = client_map[new_client_id]; // store client ID as peer data

				// 3 new id
				ClientDataPacket header = { NEW_CLIENT_PACKET, currentServerTick, 2, new_client_id, ""};
				SendPacketReliable(event.peer, header);
				enet_host_flush(server);
				break;
			}
			case ENET_EVENT_TYPE_RECEIVE:
			{
				//std::cout << "A packet of length " << event.packet->dataLength << " was received from "
				//	<< (char*)event.packet->data << " with peer address " << event.peer->address.host << " and port " << event.peer->address.port
				//	<< " on channel " << (int)event.channelID << "." << std::endl;
				//std::cout << "receiving data " << std::endl;
				uint8_t header = reinterpret_cast<PacketHeader*>(event.packet->data)->packetType;
				switch (header)
				{
				case PacketType::INPUT_PACKET:
				{
					ParseData(server, event.peer, static_cast<ClientData*>(event.peer->data)->GetID(), reinterpret_cast<InputPacket*>(event.packet->data));
					break;
				}
				case PacketType::CAR_PHYSICS_PACKET:
				{
					ParseData(server, event.peer, static_cast<ClientData*>(event.peer->data)->GetID(), reinterpret_cast<CarPhysicsPacket*>(event.packet->data));
					break;
				}
				case PacketType::CAR_PACKET:
				{
					ParseData(server, event.peer, static_cast<ClientData*>(event.peer->data)->GetID(), reinterpret_cast<CarPacket*>(event.packet->data));
					break;
				}
				default:
					ParseData(server, event.peer, static_cast<ClientData*>(event.peer->data)->GetID(), (char*)event.packet->data);
					break;
				}
				enet_packet_destroy(event.packet); // clean up the packet now that we're done using it
		
				break;
			}
			case ENET_EVENT_TYPE_DISCONNECT:
			{
				std::cout << (char*)event.peer->data << event.peer->address.host << event.peer->address.port << " disconnected." << std::endl;

				char disconnected_data[126] = { '\0' };
				sprintf_s(disconnected_data, sizeof(disconnected_data), "4|%d", static_cast<ClientData*>(event.peer->data)->GetID());
				BroadcastPacketReliable(server, disconnected_data);

				event.peer->data = NULL;
				break;
			}
			}
		}

		// physics update
		while (atHertz >= FIXED_HERTZ) {
			currentServerTick++;
			//std::cout << "tick: " << currentServerTick << std::endl;
			PhysicsUpdate(FIXED_HERTZ);

			// send updated transform to clients
			for (auto const& [id, client] : client_map) {
				if (!client->ready) continue;

				Car* car = client->GetCar();
				if (!car) {
					continue;
				}
				//StateHistoryPacket statePacket = client->GetStateHistory(currentServerTick - 1);

				CarPacket packet = { 
					PacketType::CAR_PACKET, currentServerTick, 1, id,
					car->getTransform().pos, car->getTransform().rot, car->currentAngle, car->velocity, car->forwardRot
				};

				//strcpy_s(packet.assetImage, sizeof(packet.assetImage), car->assetImage.c_str());
				BroadcastPacketUnseq(server, packet);

				client->lastKnownInput = client->GetInputState(currentServerTick);
				client->inputStateHistory.erase(currentServerTick);
			}

			atHertz -= FIXED_HERTZ;
		}

		enet_host_flush(server);

		auto elapsed = high_resolution_clock::now() - currentTime;
		if (elapsed < TICK_DURATION) {
			sleep_for(TICK_DURATION - elapsed);
		}

	}
	// GAME LOOP END
	enet_host_destroy(server); // destroy the server host

	return EXIT_SUCCESS;
}
