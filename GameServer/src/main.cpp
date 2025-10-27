#include <iostream>
#include <enet\enet.h>
#include <string>
#include <cstring>
#include <map>

class ClientData {
private:
	int m_id;
	std::string username;
public:
	ClientData(int id) : m_id(id) {}

	void SetUsername(const char* name) { username = name; }

	int GetID() const { return m_id; }
	const std::string& GetUsername() const { return username; }
};

std::map<int, ClientData*> client_map; // map of client ID to ClientData

void BroadcastPacket(ENetHost* server, const char* data) {
	ENetPacket* packet = enet_packet_create(data, strlen(data) + 1, ENET_PACKET_FLAG_RELIABLE);
	enet_host_broadcast(server, 0, packet); // broadcast packet on channel 0
	enet_host_flush(server);
}

void SendPacket(ENetPeer* peer, const char* data) {
	ENetPacket* packet = enet_packet_create(data, strlen(data) + 1, ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(peer, 0, packet); // send packet on channel 0
}

void ParseData(ENetHost* server, int id, char* data) {

	int data_type;
	sscanf_s(data, "%d|", &data_type);

	switch (data_type) {
	case 1:
	{
		char msg[80];
		sscanf_s(data, "%*d|%79[^\n]", &msg, (unsigned)80);

		char send_data[1024] = { '\0' };
		sprintf_s(send_data, sizeof(send_data), "1|%d|%s", id, msg);
		BroadcastPacket(server, send_data);
		break;
	}
	case 2:
	{
		char username[80];
		sscanf_s(data, "2|%79[^\n]|", &username, (unsigned)80);

		char send_data[1024] = { '\0' };
		sprintf_s(send_data, sizeof(send_data), "2|%d|%s", id, username);
		std::cout << "Broadcasting: " << send_data << std::endl;

		BroadcastPacket(server, send_data);
		client_map[id]->SetUsername(username);
		break;
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

	server = enet_host_create(&address, 32, 1, 0, 0); // create a server host
	if (server == NULL) {
		std::cerr << "An error occurred while trying to create an ENet server host." << std::endl;
		return EXIT_FAILURE;
	}
	std::cout << "Server started on port " << address.port << std::endl;

	// GAME LOOP START
	int new_client_id = 0;
	while (true) {
		// wait up to 1000 milliseconds for an event
		while (enet_host_service(server, &event, 1000) > 0) {
			switch (event.type) {
			case ENET_EVENT_TYPE_CONNECT:
			{
				std::cout << "A new client connected from "
					<< event.peer->address.host << ":" << event.peer->address.port << "." << std::endl;
				event.peer->data = (void*)"Client information"; // store some data about the peer

				for (auto const& pair : client_map) {
					char send_data[1024] = { '\0' };
					sprintf_s(send_data, sizeof(send_data), "2|%d|%s", pair.first, pair.second->GetUsername().c_str());
					/*ENetPacket* packet = enet_packet_create(send_data, strlen(send_data) + 1, ENET_PACKET_FLAG_RELIABLE);
					enet_peer_send(event.peer, 0, packet);*/
					BroadcastPacket(server, send_data);
				}

				new_client_id++;
				client_map[new_client_id] = new ClientData(new_client_id);
				event.peer->data = client_map[new_client_id]; // store client ID as peer data

				char data_to_send[126] = { '\0' };
				sprintf_s(data_to_send, sizeof(data_to_send), "3|%d", new_client_id);
				SendPacket(event.peer, data_to_send);

				break;
			}
			case ENET_EVENT_TYPE_RECEIVE:
			{
				std::cout << "A packet of length " << event.packet->dataLength << " was received from "
					<< (char*)event.packet->data << " with peer address " << event.peer->address.host << " and port " << event.peer->address.port
					<< " on channel " << (int)event.channelID << "." << std::endl;

				ParseData(server, static_cast<ClientData*>(event.peer->data)->GetID(), (char*)event.packet->data);
				enet_packet_destroy(event.packet); // clean up the packet now that we're done using it
				// echo the packet back to the client
				//enet_peer_send(event.peer, 0, event.packet);
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
	}
	// GAME LOOP END
	enet_host_destroy(server); // destroy the server host

	return EXIT_SUCCESS;
}
