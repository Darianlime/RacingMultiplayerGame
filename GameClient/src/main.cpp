#include <iostream>
#include <stdio.h>
#include <enet/enet.h>
#include <string>
#include <cstring>
#include <map>
#include <algorithm>
#include <queue>
#include <atomic>
#include <memory>
#include <cstdint>
#include <chrono>
#include <thread>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "io/Screen.h"
#include "io/Keyboard.h"
#include "io/Joystick.h"
#include "io/Mouse.h"
#include "io/Camera2D.h"
#include "io/InputManager.h"

#include "graphics/models/CarRenderer.h"
#include "graphics/Shader.h"
#include "graphics/models/Quad.hpp"
#include "graphics/Text.h"

#include "Shared/CarState.h"

using namespace NetWork;
using namespace Engine;
using namespace std::chrono;
using namespace std::this_thread;

constexpr float RENDER_TICK_RATE = 128.0f;
constexpr float PREDICTION_TICK_RATE = 64.0f;
constexpr uint8_t INTERPOLATE_TICKS = 1;
constexpr uint8_t HISTORY_SNAPSHOTS = 100;

constexpr uint8_t CHANNEL_UNSEQUENCED = 0;
constexpr uint8_t CHANNEL_RELIABLE = 1;

static Screen screen(1270, 720);
static Joystick mainJ(0);
static std::atomic<bool> connected = false;
static std::atomic<bool> isProcessingShaders = false;
static int CLIENT_ID = -1; // unique client ID assigned by server

struct PacketHeader {
	uint8_t packetType;
	uint64_t currentTick;
	int parseType;
	int id;
};

typedef enum {
	W, S, A, D, SPACE
} KeyInput;

enum PacketType {
	NEW_CLIENT_PACKET,
	INPUT_PACKET,
	CAR_PACKET,
	CAR_PHYSICS_PACKET
};

struct ClientDataPacket {
	PacketHeader packetHeader;
	char username[28];
};

struct InputPacket {
	PacketHeader packetHeader;
	InputState inputs;
};

//struct StateHistoryPacket {
//	glm::vec3 pos;
//	float rot;
//	float currentAngle;
//};

struct CarPacket {
	PacketHeader packetHeader;
	CarState carState;
	char assetImage[80];
};

struct CarPhysicsPacket {
	PacketHeader packetHeader;
	glm::vec3 worldVerts[Quad::noVertices];
};

static std::queue<CarPacket> createNewCars;

class ClientData {
private:
	int m_id;
	std::string username;

	std::unique_ptr<CarRenderer> carRenderer;
	std::unique_ptr<Camera2D> camera2D;
	std::unique_ptr<Text> textRenderer;
	std::unique_ptr<InputManager> inputs;
public:
	uint64_t currentTick = 0;
	uint64_t lastServerTick = 0;
	int renderTick = 0;
	CarState serverCarState;
	CarState predictedCarState;
	std::map<uint64_t, CarState> stateSnapShots;
	bool recivedServerData = false;

	ClientData(int id) : m_id(id) {
		serverCarState = {};
		predictedCarState = {};
		inputs = std::make_unique<InputManager>();
	}

	void SetUsername(std::string name) { username = name; }
	void CreateCar(glm::vec3 pos, const char* assetImage) { carRenderer = std::make_unique<CarRenderer>(pos, assetImage); }
	void CreateCamera2D() { camera2D = std::make_unique<Camera2D>(glm::vec3(0.0f, 0.0f, 0.0f)); }
	void CreateText(const char* fontFile, int fontSize) { textRenderer = std::make_unique<Text>(fontFile, fontSize); }

	CarRenderer* GetCar() const { return carRenderer.get(); }
	Camera2D* GetCamera2D() const { return camera2D.get(); }
	Text* GetText() const { return textRenderer.get(); }
	InputManager* GetInputs() const { return inputs.get(); }

	int GetID() const { return m_id; }
	const std::string& GetUsername() const { return username; }
};

std::map<int, ClientData*> client_map; // map of client ID to ClientData

void SendPacketReliable(ENetPeer* peer, const char* data) {
	ENetPacket* packet = enet_packet_create(data, strlen(data) + 1, ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(peer, CHANNEL_RELIABLE, packet); // send packet on channel 1
}

void SendPacketUnseq(ENetPeer* peer, const char* data) {
	ENetPacket* packet = enet_packet_create(data, strlen(data) + 1, ENET_PACKET_FLAG_UNSEQUENCED);
	enet_peer_send(peer, CHANNEL_UNSEQUENCED, packet); // send packet on channel 1
}

void SendPacketUnseq(ENetPeer* peer, InputPacket data) {
	ENetPacket* packet = enet_packet_create(&data, sizeof(data), ENET_PACKET_FLAG_UNSEQUENCED);
	enet_peer_send(peer, CHANNEL_UNSEQUENCED, packet); // send packet on channel 0
}

void SendPacketUnseq(ENetPeer* peer, CarPacket data) {
	ENetPacket* packet = enet_packet_create(&data, sizeof(data), ENET_PACKET_FLAG_UNSEQUENCED);
	enet_peer_send(peer, CHANNEL_UNSEQUENCED, packet); // send packet on channel 0
}

void SendPacketUnseq(ENetPeer* peer, CarPhysicsPacket data) {
	ENetPacket* packet = enet_packet_create(&data, sizeof(data), ENET_PACKET_FLAG_UNSEQUENCED);
	enet_peer_send(peer, CHANNEL_UNSEQUENCED, packet); // send packet on channel 0
}

// Parsing new clients 
void ParseClientData(ClientDataPacket* data, uint32_t RTT) {
	switch (data->packetHeader.parseType) {
	case 1: // another client joined
	{
		if (client_map.find(data->packetHeader.id) == client_map.end()) {
			std::cout << "id assigned?" << data->packetHeader.id << std::endl;
			client_map[data->packetHeader.id] = new ClientData(data->packetHeader.id);
			client_map[data->packetHeader.id]->SetUsername(data->username);
		}
		break;
	}
	case 2: // new client data
	{
		CLIENT_ID = data->packetHeader.id; // server assigns unique client ID
		client_map[CLIENT_ID] = new ClientData(CLIENT_ID);
		client_map[CLIENT_ID]->CreateCar(glm::vec3(0.0f), "assets/car1_2.png");
		client_map[CLIENT_ID]->CreateCamera2D();
		// sync new client data with server tick
		int driftTicks = ceil((RTT / 1000.0f) / (1.0f/PREDICTION_TICK_RATE));
		driftTicks += glm::clamp(driftTicks, 2, 8);
		std::cout << RTT << " driftTICKS: " << driftTicks << " "  << std::endl;
		client_map[CLIENT_ID]->currentTick = data->packetHeader.currentTick + driftTicks;
		std::cout << "id assigned?" << CLIENT_ID  << std::endl;
		connected = true;
		break;
	}
	}
}

// Parsing Players State
void ParseCarData(CarPacket* data, double fixedDeltaTime) {
	// update player render state
	switch (data->packetHeader.parseType) {
	case 1: // update car data
	{
		// Reconciliation replay inputs of the server authorative car state
		uint64_t serverTick = data->packetHeader.currentTick;

		if (client_map[data->packetHeader.id] == nullptr || !connected) break;

		int id = data->packetHeader.id;
		ClientData* clientData = client_map[id];
		if (clientData->GetCar() == nullptr) break;

		if (id == CLIENT_ID) {

			if (serverTick <= clientData->lastServerTick) break;

			clientData->serverCarState = data->carState;
			
			CarState predictionCarState = clientData->serverCarState;
			CarRenderer* car1 = clientData->GetCar();
			bool collision1 = false;
			for (auto& [id2, client2] : client_map) {
				if (clientData->GetID() == id2) {
					continue;
				}
				CarRenderer* car2 = client2->GetCar();
				if (car1 != nullptr && car2 != nullptr) {
					collision1 = Collision2D::checkOBBCollisionResolve(car1->getTransform(), predictionCarState, car2->getTransform(), client2->serverCarState, fixedDeltaTime);
				}
			}
			if (!collision1) {
				for (uint64_t i = serverTick + 1; i <= clientData->currentTick; ++i) {
					InputState inputState = clientData->GetInputs()->GetInputStateHistory(i);
					predictionCarState = clientData->GetCar()->SimulatePhysicsUpdate(predictionCarState, inputState, fixedDeltaTime);
				}
			}
			clientData->predictedCarState = predictionCarState;
		}
		else { // other client snapshots to interpolate 
			clientData->stateSnapShots[serverTick] = data->carState;

			if (clientData->stateSnapShots.size() > HISTORY_SNAPSHOTS) {
				clientData->stateSnapShots.erase(clientData->stateSnapShots.begin());
			}
		}

		clientData->lastServerTick = serverTick;

		break;
	}
	case 2: // create a car render for other clients
	{ 
		if (data->packetHeader.id != CLIENT_ID) {
			printf("case 2 creating car: %d, null?: %d\n", data->packetHeader.id, client_map[data->packetHeader.id] == nullptr);
			createNewCars.push(*data);
		}
		break;
	} 
	}
	
}

int main(int argc, char **argv) {
	
	printf("Please enter your username: ");
	char username[80] = { '\0' };
	scanf_s("%79s", username, (unsigned)_countof(username));

	if (enet_initialize() != 0) {
		std::cerr << "An error occurred while initializing ENet." << std::endl;
		return EXIT_FAILURE;
	}

	atexit(enet_deinitialize); // deinitialize ENet at exit

	ENetHost* client;
	client = enet_host_create(NULL, 1, 2, 0, 0); // create a client host

	if (client == NULL) {
		std::cerr << "An error occurred while trying to create an ENet client host." << std::endl;
		return EXIT_FAILURE;
	}

	ENetAddress address;
	ENetEvent event; // data received from server
	ENetPeer* peer; // server peer connection too

	enet_address_set_host(&address, "127.0.0.1"); // set server address
	address.port = 8008; // set server port

	peer = enet_host_connect(client, &address, 2, 0); // connect to server
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

	// GAME LOOP START
	if (!screen.init()) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Text text("assets/fonts/OCRAEXT.TTF", 48);
	if (text.init() == -1) {
		std::cout << "Failed to init text" << std::endl;
		glfwTerminate();
		return -1;
	}
	Shader shader("assets/object.vert", "assets/object.frag");
	Shader textShader("assets/text.vert", "assets/text.frag");
	/*glm::mat4 projection = glm::ortho(-float(Screen::SCR_WIDTH), float(Screen::SCR_WIDTH), -float(Screen::SCR_HEIGHT), float(Screen::SCR_HEIGHT), -1.0f, 1.0f);
	textShader.activate();
	textShader.setMat4("projection", projection);*/

	const double RENDER_FIXED_HERTZ = 1.0 / RENDER_TICK_RATE; // 128 ticks per sec 
	const double PREDICTION_FIXED_HERTZ = 1.0 / PREDICTION_TICK_RATE; // 64 ticks per sec
	const auto TICK_DURATION = std::chrono::duration<double>(PREDICTION_FIXED_HERTZ);

	steady_clock::time_point lastTime = high_resolution_clock::now();
	double renderAtHertz = 0.0;
	double predictionAtHertz = 0.0;
	while (!screen.shouldClose()) {
		steady_clock::time_point currentTime = high_resolution_clock::now();
		double fixedDeltaTime = duration_cast<duration<double>>(currentTime - lastTime).count();
		lastTime = currentTime;
		renderAtHertz += fixedDeltaTime;
		predictionAtHertz += fixedDeltaTime;

		ENetEvent event;
		while (enet_host_service(client, &event, 1) > 0) {
			switch (event.type) {
			case ENET_EVENT_TYPE_RECEIVE:
			{
				uint8_t header = reinterpret_cast<PacketHeader*>(event.packet->data)->packetType;
				switch (header) {
				case PacketType::CAR_PACKET:
				{
					double predictDt = 1.0 / PREDICTION_TICK_RATE;
					ParseCarData(reinterpret_cast<CarPacket*>(event.packet->data), predictDt);
					break;
				}
				case PacketType::NEW_CLIENT_PACKET: // new user
				{
					uint32_t RTT = peer->roundTripTime;
					ParseClientData(reinterpret_cast<ClientDataPacket*>(event.packet->data), RTT);
					if (!isProcessingShaders && CLIENT_ID != -1) {
						printf("once\n");
						client_map[CLIENT_ID]->SetUsername(username);

						char str_data[80] = "2|";
						strcat_s(str_data, sizeof(str_data), username);
						SendPacketReliable(peer, str_data); // send username to server

						char isShaderLoaded[80] = "4|";
						strcat_s(isShaderLoaded, sizeof(isShaderLoaded), "assets/car1_2.png");
						SendPacketReliable(peer, isShaderLoaded);

						enet_host_flush(client);
						isProcessingShaders = true;
					}
					break;
				}
				default: 
				{
					fprintf(stderr, "nothing received in client\n");
					break;
				}
				}
				enet_packet_destroy(event.packet); // clean up the packet now that we're done using it
				break;
			}
			}
		}

		if (connected) {
			while (predictionAtHertz >= PREDICTION_FIXED_HERTZ) {
				ClientData* clientData = client_map[CLIENT_ID];

				clientData->currentTick++;
				//std::cout << "tick: " << clientData->currentTick << std::endl;
				if (Keyboard::key(GLFW_KEY_ESCAPE) || mainJ.buttonState(GLFW_JOYSTICK_BTN_RIGHT)) {
					printf("in escape\n");
					screen.setShouldClose(true);
				}

				// Interpolation of other clients


				// Process input and updates to input history
				InputState& inputs = clientData->GetInputs()->processInputs(clientData->currentTick);

				// Proccess Prediction Update 
				//clientData->GetCar()->PredictionPhysicsUpdate(inputs, fixedDeltaTime);
				double predictDt = 1.0 / PREDICTION_TICK_RATE;
				//clientData->predictedCarState = clientData->GetCar()->SimulatePhysicsUpdate(clientData->predictedCarState, inputs, predictDt);
				//const float softDistance = 3.0f;   // adjust depending on your car width
				//const float pushFactor = 5.0f;     // how strongly to apply correction (0.05–0.2 works well)

				//for (auto& [id, other] : client_map)
				//{
				//	if (id == clientData->GetID()) continue; // skip self
				//	if (other == nullptr || other->GetCar() == nullptr) continue;

				//	// remote state is authoritative or interpolated state
				//	CarState remote = other->serverCarState;

				//	glm::vec3 delta = remote.pos - clientData->predictedCarState.pos;
				//	float dist = glm::length(delta);

				//	if (dist < softDistance && dist > 0.001f)
				//	{
				//		glm::vec3 dir = delta / dist; // normalize
				//		float penetration = softDistance - dist;

				//		// push local predicted position away slightly
				//		clientData->predictedCarState.pos -= dir * penetration * pushFactor;
				//	}
				//}
				// Collisions for static objects not other clients
				CarRenderer* car1 = clientData->GetCar();
				for (auto& [id2, client2] : client_map) {
					if (clientData->GetID() == id2) {
						continue;
					}
					CarRenderer* car2 = client2->GetCar();
					if (car1 != nullptr && car2 != nullptr) {
						bool collision1 = Collision2D::checkOBBCollisionResolve(car1->getTransform(), clientData->predictedCarState, car2->getTransform(), client2->serverCarState, predictDt);
					}
				}

				InputPacket inputPacket = { INPUT_PACKET, clientData->currentTick, 1, CLIENT_ID, inputs };
				SendPacketUnseq(peer, inputPacket);

				/*if (client_map[CLIENT_ID]->GetCar() != nullptr) {
					CarPhysicsPacket packet{};
					packet.packetHeader.packetType = PacketType::CAR_PHYSICS_PACKET;
					packet.packetHeader.parseType = 1;
					packet.packetHeader.id = CLIENT_ID;
					std::copy_n(client_map[CLIENT_ID]->GetCar()->getTransform().worldVerts.begin(), Quad::noVertices, packet.worldVerts);
					SendPacketUnseq(peer, packet);
				}*/

				predictionAtHertz -= PREDICTION_FIXED_HERTZ;
			}

			//printf("connected, id: %d\n", CLIENT_ID);
			while (renderAtHertz >= RENDER_FIXED_HERTZ) {
				ClientData* clientData = client_map[CLIENT_ID];

				while (!createNewCars.empty()) {
					CarPacket data = createNewCars.front();
					createNewCars.pop();
					client_map[data.packetHeader.id]->CreateCar(data.carState.pos, data.assetImage);
				}

				clientData->GetCamera2D()->followTarget(clientData->predictedCarState.pos);
				clientData->GetCamera2D()->updateCameraDirection(clientData->predictedCarState.currentAngle - 90.0f);

				screen.update();

				// create trasformation for screen
				glm::mat4 view = glm::mat4(1.0f);
				glm::mat4 projection = glm::mat4(1.0f);

				projection = glm::ortho(-float(Screen::SCR_WIDTH), float(Screen::SCR_WIDTH), -float(Screen::SCR_HEIGHT), float(Screen::SCR_HEIGHT), -1.0f, 1.0f);
				view = clientData->GetCamera2D()->getViewMatrix();

				textShader.activate();
				textShader.setMat4("view", view);
				textShader.setMat4("projection", projection);

				shader.activate();
				shader.setMat4("view", view);
				shader.setMat4("projection", projection);

				for (auto const& [id, client] : client_map) {
					if (client == NULL || client->GetCar() == nullptr) continue;

					if (id == CLIENT_ID) {
						std::cout << "client x: " << client->predictedCarState.pos.x << " y: " << client->predictedCarState.pos.y << std::endl;
						client->GetCar()->render(shader, client->predictedCarState.pos, client->predictedCarState.rot);
					}
					else {
						if (client->stateSnapShots.empty()) {
							continue;
						}

						uint64_t renderTick = client->lastServerTick;
						if (renderTick > INTERPOLATE_TICKS) renderTick -= INTERPOLATE_TICKS;
						else renderTick = client->lastServerTick;

						std::map<uint64_t, CarState>::iterator itUpper = client->stateSnapShots.lower_bound(renderTick);
						CarState interpolateCar{};
						if (client->stateSnapShots.begin() == itUpper) {
							CarState state = itUpper->second;
							interpolateCar = state;
						}
						else if (client->stateSnapShots.end() == itUpper) {
							std::map<uint64_t, CarState>::iterator itLast = std::prev(client->stateSnapShots.end());
							CarState state = itLast->second;
							interpolateCar = state;
						}
						else {
							std::map<uint64_t, CarState>::iterator itLower = std::prev(itUpper);
							CarState state1 = itLower->second;
							CarState state2 = itUpper->second;

							float denom = float(itUpper->first - itLower->first);
							float t = denom > 0.0f ? float(renderTick - itLower->first) / denom : 0.0f;
							interpolateCar.pos = glm::mix(state1.pos, state2.pos, t);
							interpolateCar.rot = glm::mix(state1.rot, state2.rot, t);
						}

						//float dist = glm::distance(interpolateCar.pos, client->stateSnapShots.rbegin()->second.pos);
						////std::cout << dist << std::endl;
						//if (dist > 0.0f) {
						//	interpolateCar = client->stateSnapShots.rbegin()->second;
						//}
						client->serverCarState = interpolateCar;
						std::cout << "other client x: " << interpolateCar.pos.x << " y: " << interpolateCar.pos.y << std::endl;
						client->GetCar()->render(shader, interpolateCar.pos, interpolateCar.rot);
					}
				}
				for (auto const& [id, client] : client_map) {
					if (client != NULL && client->GetCar() != nullptr) {
						//printf("%s\n", client->GetUsername().c_str());
						text.renderText(textShader, client->GetUsername(), client->predictedCarState.pos.x, client->predictedCarState.pos.y, 1.0f, glm::vec3(0.0f, 0.0f, 0.0f));
					}
				}

				//printf("id:%d, x:%.2f, y:%.2f\n", clientData->GetID(), clientData->GetCar()->getTransform().pos.x, clientData->GetCar()->getTransform().pos.y);
				screen.newFrame();

				renderAtHertz -= RENDER_FIXED_HERTZ;
			}
		}

		auto elapsed = high_resolution_clock::now() - currentTime;
		if (elapsed < TICK_DURATION) {
			sleep_for(TICK_DURATION - elapsed);
		}
	}

	glfwTerminate();
	// GAME LOOP END
	//msgThread.join(); // wait for message thread to finish (it won't in this case)

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

