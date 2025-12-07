#include <iostream>
#include <string>
#include <cstring>

#include <ctime>
#include <ratio>
#include <chrono>
#include <thread>
#include <string.h>
#include <cstdint>
#include <deque>
#include <limits>

#include "Core/Physics/Collision2D.h"
#include "Core/Physics/UniformGridPartition.h"
#include "Core/Physics/QuadTree.h"
#include "Core/Physics/Quad.h"

#include "Server/Network/Packets.h"
#include "Server/WorldMap.h"

using namespace Server;
using namespace Server::Network;
using namespace Core;
using namespace std::chrono;
using namespace std::this_thread;

constexpr float SERVER_TICK_RATE = 64.0f;
constexpr uint8_t HISTORY_SIZE = 200;

uint64_t currentServerTick = 0;

class ClientData {
private:
	uint8_t m_id;
	std::string username;
	std::unique_ptr<CarPhysics> car;
public:
	bool ready = false;
	std::map<uint64_t, InputState> inputStateHistory;
	std::deque<StateHistoryPacket> statePacketHistory;
	InputState lastKnownInput;

	ClientData(uint8_t id) : m_id(id) {}

	void SetUsername(std::string name) { username = name; }
	void CreateCar() { car = std::make_unique<CarPhysics>(); }

	CarPhysics* GetCar() const { return car.get(); }
	InputState& GetInputState(uint32_t index) { return inputStateHistory[index]; }
	StateHistoryPacket& GetStateHistory(uint32_t index) { return statePacketHistory[index % HISTORY_SIZE]; }
	void SetStateHistory(StateHistoryPacket packet, uint32_t index) { statePacketHistory[index % HISTORY_SIZE] = packet; }

	uint8_t GetID() const { return m_id; }
	const std::string& GetUsername() const { return username; }
};

std::map<uint8_t, ClientData*> client_map; // map of client ID to ClientData

void ParseData(ENetHost* server, ENetPeer* peer, uint8_t id, char* data) {

	int data_type;
	sscanf_s(data, "%d|", &data_type);

	switch (data_type) {
	case 1: // Get username from client
	{
		char username[28] = { '\0' };
		sscanf_s(data, "1|%28[^\n]", &username, (unsigned)28);

		ClientDataPacket data = { CLIENT_PACKET, 0, 1, id, ""};
		strcpy_s(data.username, sizeof(data.username), username);

		std::cout << "Broadcasting: " << data.username << std::endl;

		BroadcastPacket(server, ChannelFlag::CHANNEL_RELIABLE, data);
		client_map[id]->SetUsername(username);
		break;
	}
	case 2: // create car packet for new car and broadcast to other clients
	{
		ClientData* client = client_map[id];

		char img[80] = { '\0' };
		sscanf_s(data, "2|%79[^\n]", img, (unsigned)sizeof(img));

		if (!client->GetCar()) {
			client->CreateCar();
			client->GetCar()->assetImage = img;
		}

		//printf("%s", img);
		CarPacketImage packet = { PacketType::CAR_PACKET, 0, 2, id, glm::vec3(0.0f), 0.0f, 90.0f, 0.0f, 0.0f, ""};
		strcpy_s(packet.assetImage, sizeof(packet.assetImage), img);
		BroadcastPacket(server, ChannelFlag::CHANNEL_RELIABLE, packet);

		for (auto const& [other_id, client] : client_map) {
			CarPhysics* car = client->GetCar();
			if (!car || other_id == id) continue;
			CarPacketImage packet = { PacketType::CAR_PACKET, 0, 2, other_id, car->GetTransform().pos, car->GetTransform().rot, car->currentAngle, car->velocity, car->forwardRot, ""};
			strcpy_s(packet.assetImage, sizeof(packet.assetImage), car->assetImage.c_str());
			SendPacket(peer, ChannelFlag::CHANNEL_RELIABLE, packet);
		}
		
		client_map[id]->ready = true;
		break;
	}
	}
}

void ParseData(ENetHost* server, ENetPeer* peer, uint8_t id, InputPacket* data) {
	switch (data->packetHeader.parseType) {
	case 1:
		InputState input = data->inputs;
		//std::cout << "inital " << input.W << input.S << input.A << input.D << std::endl;
		//std::cout << "inputtick " << input.currentTick << "currentServerTick: " << currentServerTick << std::endl;
		if (input.tick > currentServerTick) {
			client_map[id]->inputStateHistory[input.tick] = input;
		}
		break;
	}
}

void PhysicsUpdate(double fixedDeltaTime) {
	for (auto& [id, client] : client_map) {
		CarPhysics* car = client->GetCar();
		if (car != nullptr) {
			InputState inputState;
			if (client_map[id]->inputStateHistory.count(currentServerTick) < 1) {
				inputState = client_map[id]->inputStateHistory[client->lastKnownInput.tick];
			}
			else {
				inputState = client_map[id]->inputStateHistory[currentServerTick];
			}
			//std::cout << "id " << static_cast<int>(id) << std::endl;
			//std::cout << "inital " << inputState.W << inputState.S << inputState.A << inputState.D << std::endl;
			car->Update(inputState, fixedDeltaTime);
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

	server = enet_host_create(&address, 32, 3, 0, 0); // create a server host
	if (server == NULL) {
		std::cerr << "An error occurred while trying to create an ENet server host." << std::endl;
		return EXIT_FAILURE;
	}
	std::cout << "Server started on port " << address.port << std::endl;

	const double FIXED_HERTZ = 1.0 / SERVER_TICK_RATE; // 64 ticks per sec 
	const auto TICK_DURATION = std::chrono::duration<double>(FIXED_HERTZ);

	high_resolution_clock::time_point lastTime = high_resolution_clock::now();
	double atHertz = 0.0;

	// GAME LOOP START
	WorldMap map;
	UniformPartition uniPartition(64.0f * 11.0f, 64.0f * 11.0f, map.GetMap().size());
	uint8_t new_client_id = 0;
	while (true) {
		high_resolution_clock::time_point currentTime = high_resolution_clock::now();
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

					ClientDataPacket data = { CLIENT_PACKET, 0, 1, pair.first, ""};
					strcpy_s(data.username, sizeof(data.username), pair.second->GetUsername().c_str());
					/*ENetPacket* packet = enet_packet_create(send_data, strlen(send_data) + 1, ENET_PACKET_FLAG_RELIABLE);
					enet_peer_send(event.peer, 0, packet);*/
					SendPacket(event.peer, ChannelFlag::CHANNEL_RELIABLE, data);
				}

				// new client 
				new_client_id++;
				client_map[new_client_id] = new ClientData(new_client_id);
				event.peer->data = client_map[new_client_id]; // store client ID as peer data

				// 2 new id
				ClientDataPacket header = { CLIENT_PACKET, currentServerTick, 2, new_client_id, ""};
				SendPacket(event.peer, ChannelFlag::CHANNEL_RELIABLE, header);

				// =====MAP POSITIONS==============
				size_t mapSize = map.GetRow() * map.GetColumn();
				float size = (64.0f * 11.0f * map.GetRow());
				Quad mapQuad(glm::vec3(size / 2.0f, -size / 2.0f, 0.0f), glm::vec3(size, size, 1.0f), 0.0f, PhysicsType::STATIC);
				std::cout << mapQuad.size.x << " + " << mapQuad.size.y << std::endl;
				mapQuad.CalculateWorldVerts(mapQuad.pos, glm::vec3(1.0f), 0.0f);

				std::vector<uint8_t> buffer;
				buffer.reserve(sizeof(PacketHeader) + 2 + mapSize);

				PacketHeader headerMap{ WORLD_MAP_PACKET, 0, 1, new_client_id };

				uint8_t* headerBytes = reinterpret_cast<uint8_t*>(&headerMap);
				buffer.insert(buffer.end(), headerBytes, headerBytes + sizeof(PacketHeader));

				buffer.push_back(map.GetRow());
				buffer.push_back(map.GetColumn());

				buffer.insert(buffer.end(), map.GetMap().begin(), map.GetMap().end());

				ENetPacket* packet = enet_packet_create(
					buffer.data(),
					buffer.size(),
					ENET_PACKET_FLAG_RELIABLE
				);

				enet_peer_send(event.peer, CHANNEL_RELIABLE, packet);

				enet_host_flush(server);

				break;
			}
			case ENET_EVENT_TYPE_RECEIVE:
			{
				uint8_t header = reinterpret_cast<PacketHeader*>(event.packet->data)->packetType;
				switch (header)
				{
				case PacketType::INPUT_PACKET:
				{
					ParseData(server, event.peer, static_cast<ClientData*>(event.peer->data)->GetID(), reinterpret_cast<InputPacket*>(event.packet->data));
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

				ClientDataPacket header = { CLIENT_PACKET, currentServerTick, 3, static_cast<ClientData*>(event.peer->data)->GetID(), "" };
				BroadcastPacket(server, ChannelFlag::CHANNEL_RELIABLE, header);

				auto it = client_map.find(static_cast<ClientData*>(event.peer->data)->GetID());
				if (it != client_map.end()) {
					ClientData* c = it->second;
					delete c;
					client_map.erase(it);
				}
			}
			}
		}

		// physics update
		while (atHertz >= FIXED_HERTZ) {
			currentServerTick++;
			//std::cout << "tick: " << currentServerTick << std::endl;
			PhysicsUpdate(FIXED_HERTZ);

			for (auto& [id, client] : client_map) {
				CarPhysics* car = client->GetCar();
				if (!car) {
					continue;
				}
				for (uint8_t i = 0; i < car->GetTransformQuery().worldVerts.size(); i++) {
					glm::vec3 vertsQ = car->GetTransformQuery().worldVerts[i];
					uniPartition.Insert(vertsQ.x, vertsQ.y, id, car->GetQueryIndexValue(i));
				}
			}

			for (auto& [id, client] : client_map) {
				CarPhysics* car1 = client->GetCar();
				if (!car1) continue;
				for (uint16_t tileIndex : car1->GetQueryIndex()) {
					if (tileIndex == std::numeric_limits<uint16_t>::max()) continue;
					uint32_t poolStartIndex = uniPartition.GetPoolStart(tileIndex);
					for (uint16_t i = 0; i < uniPartition.tileCount[tileIndex]; i++) {
						uint8_t id2 = uniPartition.pool[poolStartIndex + i];
						if (id == id2) {
							continue;
						}
						CarPhysics* car2 = client_map[id2]->GetCar();
						if (car1 != nullptr && car2 != nullptr) {
							if (!Collision2D::CheckAABBCollsion(car1->GetTransformQuery(), car2->GetTransformQuery())) {
								continue;
							}
							bool collision1 = Collision2D::CheckOBBCollisionResolve(car1->GetTransform(), car2->GetTransform());
							if (collision1) {
								car1->GetTransformQuery().CalculateWorldVerts(glm::vec3(1.0f), 0.0f);
								car1->GetTransform().CalculateWorldVerts(glm::vec3(444.0f, 208.0f, 1.0f), -90.0f);
								car2->GetTransformQuery().CalculateWorldVerts(glm::vec3(1.0f), 0.0f);
								car2->GetTransform().CalculateWorldVerts(glm::vec3(444.0f, 208.0f, 1.0f), -90.0f);
							}
							car1->collided = collision1;
						}
					}
				}
			}

			uniPartition.ClearMap();

			// send updated transform to clients
			for (auto const& [id, client] : client_map) {
				if (!client->ready) continue;

				CarPhysics* car = client->GetCar();
				if (!car) {
					continue;
				}
				//StateHistoryPacket statePacket = client->GetStateHistory(currentServerTick - 1);

				CarPacket packet = { 
					PacketType::CAR_PACKET, currentServerTick, 1, id,
					car->GetTransform().pos, car->GetTransform().rot, 
					car->currentAngle, car->velocity, car->forwardRot, car->collided
				};

				//client->GetCar()->getTransform().calculateWorldVerts(car->getTransform().pos, glm::vec3(444.0f, 208.0f, 1.0f), car->getTransform().rot - 90.0f);

				//strcpy_s(packet.assetImage, sizeof(packet.assetImage), car->assetImage.c_str());
				BroadcastPacket(server, ChannelFlag::CHANNEL_UNSEQUENCED, packet);

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
