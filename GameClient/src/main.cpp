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
#include "graphics/models/RaceTrackRender.h"

#include "Network/Packets.h"

#include "Threads/ThreadSafeQueue.h"

using namespace NetworkClient;
using namespace Engine;
using namespace std::chrono;
using namespace std::this_thread;

constexpr float RENDER_TICK_RATE = 128.0f;
constexpr float PREDICTION_TICK_RATE = 64.0f;
constexpr uint8_t INTERPOLATE_TICKS = 2;
constexpr uint8_t HISTORY_SNAPSHOTS = 100;

static Joystick mainJ(0);
static bool connected = false;
static std::atomic<bool> isProcessingShaders = false;
static std::atomic<bool> IsNetworkThreadRunning = true;
static uint8_t CLIENT_ID = 0; // unique client ID assigned by server

static ThreadSafeQueue<UpdatePacket> updatePacketsQ;
static ThreadSafeQueue<std::unique_ptr<CarPacket>> updateCarStateQ;

class ClientData {
private:
	uint8_t m_id;
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
	bool colliding = false;

	ClientData(uint8_t id) : m_id(id) {
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

	uint8_t GetID() const { return m_id; }
	const std::string& GetUsername() const { return username; }
};

std::map<uint8_t, ClientData*> client_map; // map of client ID to ClientData

// Parsing new clients 
void ParseClientData(ClientDataPacket* data, uint32_t RTT) {
	switch (data->packetHeader.parseType) {
	case 1: // another client joined
	{
		if (client_map.find(data->packetHeader.id) == client_map.end()) {
			std::cout << "id assigned?" << static_cast<int>(data->packetHeader.id) << std::endl;
			client_map[data->packetHeader.id] = new ClientData(data->packetHeader.id);
			client_map[data->packetHeader.id]->SetUsername(data->username);
		}
		break;
	}
	case 2: // new client data
	{
		CLIENT_ID = data->packetHeader.id; // server assigns unique client ID
		client_map[CLIENT_ID] = new ClientData(CLIENT_ID);
		CarState carState = { glm::vec3(0.0f), 0.0f, 0.0f ,0.0f ,0.0f };
		UpdatePacket packet = CarPacketImage{ CAR_PACKET, 0, 2, CLIENT_ID, carState, "assets/car1_2.png" };
		updatePacketsQ.push(packet);
		client_map[CLIENT_ID]->CreateCamera2D();
		// sync new client data with server tick
		int driftTicks = ceil((RTT / 1000.0f) / (1.0f/PREDICTION_TICK_RATE));
		driftTicks += glm::clamp(driftTicks, 2, 8);
		std::cout << RTT << " driftTICKS: " << driftTicks << " "  << std::endl;
		client_map[CLIENT_ID]->currentTick = data->packetHeader.currentTick + driftTicks + RTT;
		connected = true;
		break;
	}
	}
}

// Parsing Players State
void ParseCarData(CarPacketImage* data, double fixedDeltaTime) {
	// update player render state
	switch (data->packetHeader.parseType) {
	case 1: // update car data
	{
		// Reconciliation replay inputs of the server authorative car state
		std::unique_ptr<CarPacket> carState = std::make_unique<CarPacket>(CarPacket{ data->packetHeader, data->carState });
		updateCarStateQ.push(std::move(carState));

		break;
	}
	case 2: // create a car render for other clients
	{ 
		if (data->packetHeader.id != CLIENT_ID) {
			printf("case 2 creating car: %d, null?: %d\n", data->packetHeader.id, client_map[data->packetHeader.id] == nullptr);
			UpdatePacket packet = CarPacketImage{*data};
			updatePacketsQ.push(packet);
		}
		break;
	} 
	}
	
}

void NetWorkDataThread(ENetHost* client, ENetPeer* peer, const char* username) {
	while (IsNetworkThreadRunning) {
		ENetEvent event;
		while (enet_host_service(client, &event, 1) > 0) {
			switch (event.type) {
			case ENET_EVENT_TYPE_RECEIVE:
			{
				PacketHeader* header = reinterpret_cast<PacketHeader*>(event.packet->data);
				uint8_t type = reinterpret_cast<PacketHeader*>(event.packet->data)->packetType;
				switch (type) {
				case PacketType::CAR_PACKET:
				{
					double predictDt = 1.0 / PREDICTION_TICK_RATE;
					ParseCarData(reinterpret_cast<CarPacketImage*>(event.packet->data), predictDt);
					break;
				}
				case PacketType::NEW_CLIENT_PACKET: // new user
				{
					uint32_t RTT = peer->roundTripTime;
					ParseClientData(reinterpret_cast<ClientDataPacket*>(event.packet->data), RTT);
					if (!isProcessingShaders && CLIENT_ID != 0) {
						printf("once\n");
						client_map[CLIENT_ID]->SetUsername(username);

						char str_data[80] = "1|";
						strcat_s(str_data, sizeof(str_data), username);
						SendPacket(peer, ChannelFlag::CHANNEL_RELIABLE, str_data); // send username to server

						char isShaderLoaded[80] = "2|";
						strcat_s(isShaderLoaded, sizeof(isShaderLoaded), "assets/car1_2.png");
						SendPacket(peer, ChannelFlag::CHANNEL_RELIABLE, isShaderLoaded);

						enet_host_flush(client);
						isProcessingShaders = true;
					}
					break;
				}
				case PacketType::WORLD_MAP_PACKET:
				{
					uint8_t* data = (uint8_t*)event.packet->data;
					size_t offset = 0;

					offset += sizeof(PacketHeader);

					uint8_t row = data[offset++];
					uint8_t column = data[offset++];
					size_t mapSize = row * column;

					std::vector<uint8_t> map(mapSize);
					memcpy(map.data(), data + offset, mapSize);
					UpdatePacket packet = WorldMap{ map, row, column };
					updatePacketsQ.push(std::move(packet));
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
		sleep_for(std::chrono::microseconds(200));
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
	client = enet_host_create(NULL, 1, 3, 0, 0); // create a client host

	if (client == NULL) {
		std::cerr << "An error occurred while trying to create an ENet client host." << std::endl;
		return EXIT_FAILURE;
	}

	ENetAddress address;
	ENetEvent event; // data received from server
	ENetPeer* peer; // server peer connection too

	enet_address_set_host(&address, "127.0.0.1"); // set server address
	address.port = 8008; // set server port

	peer = enet_host_connect(client, &address, 3, 0); // connect to server
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

	std::thread networkThread(NetWorkDataThread, client, peer, username);

	// GAME LOOP START
	RaceTrackRender raceTrackRender;
	std::vector<uint8_t> worldMapTiles;
	Screen screen(1270, 720);
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
	Shader objectShader("assets/object.vert", "assets/object.frag");
	Shader textShader("assets/text.vert", "assets/text.frag");
	/*glm::mat4 projection = glm::ortho(-float(Screen::SCR_WIDTH), float(Screen::SCR_WIDTH), -float(Screen::SCR_HEIGHT), float(Screen::SCR_HEIGHT), -1.0f, 1.0f);
	textShader.activate();
	textShader.setMat4("projection", projection);*/
	const double RENDER_FIXED_HERTZ = 1.0 / RENDER_TICK_RATE; // 128 ticks per sec 
	const double PREDICTION_FIXED_HERTZ = 1.0 / PREDICTION_TICK_RATE; // 64 ticks per sec
	const auto TICK_DURATION = std::chrono::duration<double>(PREDICTION_FIXED_HERTZ);

	high_resolution_clock::time_point lastTime = high_resolution_clock::now();
	double renderAtHertz = 0.0;
	double predictionAtHertz = 0.0;
	while (!screen.shouldClose()) {
		high_resolution_clock::time_point currentTime = high_resolution_clock::now();
		double fixedDeltaTime = duration_cast<duration<double>>(currentTime - lastTime).count();
		lastTime = currentTime;
		renderAtHertz += fixedDeltaTime;
		predictionAtHertz += fixedDeltaTime;

		if (connected) {
			while (predictionAtHertz >= PREDICTION_FIXED_HERTZ) {
				ClientData* clientData = client_map[CLIENT_ID];
				clientData->currentTick++;

				//std::cout << "tick: " << clientData->currentTick << std::endl;
				if (Keyboard::key(GLFW_KEY_ESCAPE) || mainJ.buttonState(GLFW_JOYSTICK_BTN_RIGHT)) {
					printf("in escape\n");
					screen.setShouldClose(true);
				}

				if (clientData->GetCar() == nullptr) break;

				std::unique_ptr<CarPacket> data;
				while (updateCarStateQ.try_pop(data)) {
					/*printf("Received packet from server: id=%d vel=%f tick=%llu\n",
						data->packetHeader.id,
						data->carState.velocity,
						data->packetHeader.currentTick);*/
					uint64_t serverTick = data->packetHeader.currentTick;
					const int MIN_LEAD = 5;   // at least 1 tick ahead for prediction
					const int MAX_LEAD = 7;   // don't predict too far

					int delta = clientData->currentTick - serverTick;

					if (delta < MIN_LEAD) {
						clientData->currentTick++;
					}
					else if (delta > MAX_LEAD) {
						clientData->currentTick--;
					}
					std::cout << "tick: " << clientData->currentTick << "server tick" << serverTick << std::endl;


					if (client_map[data->packetHeader.id] == nullptr) continue;
					uint8_t id = data->packetHeader.id;
					ClientData* clientDataRec = client_map[id];
					if (clientDataRec->GetCar() == nullptr) continue;

					if (id == CLIENT_ID) {

						if (serverTick <= clientDataRec->lastServerTick) continue;

						clientDataRec->serverCarState = data->carState;

						CarState predictionCarState = clientDataRec->serverCarState;
						CarRenderer* car1 = clientDataRec->GetCar();
						//clientDataRec->colliding = data->carState.collided;
						//std::cout << clientDataRec->colliding << std::endl;
						//clientDataRec->colliding = false;
						//for (auto& [id2, client2] : client_map) {
						//	if (clientDataRec->GetID() == id2) {
						//		continue;
						//	}
						//	CarRenderer* car2 = client2->GetCar();
						//	if (car1 != nullptr && car2 != nullptr) {
						//		//car1->GetTransform().UpdateWorldVerts(predictionCarState);
						//		clientDataRec->colliding = Collision2D::checkOBBCollisionResolve(car1->GetTransform(), predictionCarState, car2->GetTransform(), client2->serverCarState, predictDt);
						//		std::cout << clientDataRec->colliding << std::endl;
						//	}
						//}
						//if (!clientDataRec->colliding) {
						for (uint64_t i = serverTick + 1; i <= clientDataRec->currentTick; ++i) {
							if (!clientData->colliding) {
								InputState inputState = clientDataRec->GetInputs()->GetInputStateHistory(i);
								predictionCarState = car1->SimulatePhysicsUpdate(predictionCarState, inputState, PREDICTION_FIXED_HERTZ);
								car1->GetTransform().UpdateWorldVerts(predictionCarState);
							}
							bool collided = false;
							glm::vec2 normal{};
							float overlap = 0.0f;
							float slop = 0.01f;
							clientDataRec->colliding = false;
							for (auto& [id2, client2] : client_map) {
								if (clientDataRec->GetID() == id2) {
									continue;
								}
								CarRenderer* car2 = client2->GetCar();
								if (car1 != nullptr && car2 != nullptr) {
									car1->GetTransform().UpdateWorldVerts(predictionCarState);
									car2->GetTransform().UpdateWorldVerts(client2->serverCarState);
									CollisionResult res = Collision2D::checkOBBCollisionResolve(car1->GetTransform(), predictionCarState, car2->GetTransform(), client2->serverCarState, PREDICTION_FIXED_HERTZ);
									if (res.isCollided) {
										float percent = 0.6f;
										float correctionAmount = std::max(res.overlap - slop, 0.0f) * percent;
										predictionCarState.pos -= glm::vec3(res.normal * correctionAmount, 0.0f);
										clientDataRec->colliding = true;
										std::cout << clientDataRec->colliding << std::endl;


									}
									car2->GetTransform().UpdateWorldVerts(client2->serverCarState);
									car1->GetTransform().UpdateWorldVerts(predictionCarState);

								}
							}
						}
						//}
						clientDataRec->predictedCarState = predictionCarState;
					}
					else { // other client snapshots to interpolate 
						clientDataRec->stateSnapShots[serverTick] = data->carState;
						clientDataRec->serverCarState = data->carState;
						clientDataRec->GetCar()->GetTransform().UpdateWorldVerts(data->carState);
						if (clientDataRec->stateSnapShots.size() > HISTORY_SNAPSHOTS) {
							clientDataRec->stateSnapShots.erase(clientDataRec->stateSnapShots.begin());
						}
					}

					clientDataRec->lastServerTick = serverTick;
				}

				// Process input and updates to input history
				InputState& inputs = clientData->GetInputs()->processInputs(clientData->currentTick);

				// Proccess Prediction Update 
				if (!clientData->colliding) {
					clientData->predictedCarState = clientData->GetCar()->SimulatePhysicsUpdate(clientData->predictedCarState, inputs, PREDICTION_FIXED_HERTZ);
					clientData->GetCar()->GetTransform().UpdateWorldVerts(clientData->predictedCarState);
				}
				//std::cout << "inital " << inputs.W << inputs.S << inputs.A << inputs.D << std::endl;
				InputPacket inputPacket = { 
					INPUT_PACKET, clientData->currentTick, 1, CLIENT_ID, inputs
				};
				SendPacket(peer, ChannelFlag::CHANNEL_UNSEQUENCED, inputPacket);

				predictionAtHertz -= PREDICTION_FIXED_HERTZ;
			}

			//printf("connected, id: %d\n", CLIENT_ID);
			while (renderAtHertz >= RENDER_FIXED_HERTZ) {
				ClientData* clientData = client_map[CLIENT_ID];
				double predictDt = 1.0 / RENDER_FIXED_HERTZ;
				UpdatePacket updatePacket;
				if (updatePacketsQ.try_pop(updatePacket)) {
					if (auto* car = std::get_if<CarPacketImage>(&updatePacket)) {
						client_map[car->packetHeader.id]->CreateCar(car->carState.pos, car->assetImage);
					}
					else if (auto* map = std::get_if<WorldMap>(&updatePacket)) {
						raceTrackRender.Init(map->map, map->row, map->column);
					}
					std::cout << "New quad created" << std::endl;
				}

				clientData->GetCamera2D()->followTarget(clientData->predictedCarState.pos);
				clientData->GetCamera2D()->updateCameraDirection(clientData->predictedCarState.currentAngle);

				screen.update();

				// create trasformation for screen
				glm::mat4 view = glm::mat4(1.0f);
				glm::mat4 projection = glm::mat4(1.0f);

				projection = glm::ortho(-float(Screen::SCR_WIDTH), float(Screen::SCR_WIDTH), -float(Screen::SCR_HEIGHT), float(Screen::SCR_HEIGHT), -1.0f, 1.0f);
				view = clientData->GetCamera2D()->getViewMatrix();

				textShader.activate();
				textShader.setMat4("view", view);
				textShader.setMat4("projection", projection);

				objectShader.activate();
				objectShader.setMat4("view", view);
				objectShader.setMat4("projection", projection); 

				raceTrackRender.Render(objectShader);

				for (auto const& [id, client] : client_map) {
					if (client == NULL || client->GetCar() == nullptr) continue;

					if (id == CLIENT_ID) {
						//std::cout << "client x: " << client->predictedCarState.pos.x << " y: " << client->predictedCarState.pos.y << std::endl;
						//client->GetCar()->GetTransform().UpdateWorldVerts(client->predictedCarState);
						client->GetCar()->Render(objectShader, client->predictedCarState.pos, client->predictedCarState.rot - 90.0f);
						continue;
					}

					if (client->stateSnapShots.empty()) {
						continue;
					}

					uint64_t renderTick = (client->lastServerTick > INTERPOLATE_TICKS)
						? client->lastServerTick - INTERPOLATE_TICKS
						: client->lastServerTick;

					std::map<uint64_t, CarState>::iterator itUpper = client->stateSnapShots.lower_bound(renderTick);

					CarState interpolateCar{};
					if (client->stateSnapShots.begin() == itUpper) {
						interpolateCar = itUpper->second;
						std::cout << "state empty1" << std::endl;

					}
					else if (client->stateSnapShots.end() == itUpper) {
						std::map<uint64_t, CarState>::iterator itLast = std::prev(client->stateSnapShots.end());
						interpolateCar = itLast->second;
						std::cout << "state empty2" << std::endl;

					}
					else {
						std::map<uint64_t, CarState>::iterator itLower = std::prev(itUpper);
						CarState state1 = itLower->second;
						CarState state2 = itUpper->second;

						float denom = float(itUpper->first - itLower->first);
						float t = denom > 0.0f ? float(renderTick - itLower->first) / denom : 0.0f;
						//t = glm::clamp(t, 0.0f, 1.0f);
						interpolateCar.pos = glm::mix(state1.pos, state2.pos, t);
						interpolateCar.rot = glm::mix(state1.rot, state2.rot, t);
						/*interpolateCar.velocity = glm::mix(state1.velocity, state2.velocity, t);
						interpolateCar.currentAngle = glm::mix(state1.currentAngle, state2.currentAngle, t);
						interpolateCar.forwardRot = glm::mix(state1.forwardRot, state2.forwardRot, t);*/

					}

					client->serverCarState = interpolateCar;
					client->GetCar()->GetTransform().UpdateWorldVerts(interpolateCar);
					//std::cout << "other client x: " << interpolateCar.pos.x << " y: " << interpolateCar.pos.y << std::endl;
					client->GetCar()->Render(objectShader, interpolateCar.pos, interpolateCar.rot - 90.0f);
				}

				for (auto const& [id, client] : client_map) {
					if (client == NULL || client->GetCar() == nullptr) continue;
						//printf("%s\n", client->GetUsername().c_str());
					text.renderText(textShader, client->GetUsername(), client->predictedCarState.pos.x, client->predictedCarState.pos.y, 1.0f, glm::vec3(0.0f, 0.0f, 0.0f));
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
	IsNetworkThreadRunning = false;
	networkThread.join();
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

