#ifndef PACKETS_H
#define PACKETS_H

#include <glm/glm.hpp>
#include <iostream>
#include <vector>
#include <map>
#include <variant>
#include <enet/enet.h>
#include "Core/io/InputManager.h"
#include "Core/Physics/CarPhysics.h"

namespace Client {
	namespace Network {
		using namespace Core;

		struct WorldMap {
			std::vector<uint8_t> map;
			uint8_t row;
			uint8_t column;
		};

		struct PacketHeader {
			uint8_t packetType;
			uint64_t currentTick;
			uint8_t parseType;
			uint8_t id;
		};

		struct ClientDataPacket {
			PacketHeader packetHeader;
			char username[28];
		};

		struct InputPacket {
			PacketHeader packetHeader;
			InputState state;
		};

		struct WorldMapPacket {
			PacketHeader packetHeader;
			WorldMap map;
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

		struct DisconnectOtherClient {
			uint8_t id;
		};

		enum PacketType : uint8_t {
			CLIENT_PACKET,
			WORLD_MAP_PACKET,
			INPUT_PACKET,
			CAR_PACKET
		};

		enum ChannelFlag : uint8_t {
			CHANNEL_RELIABLE = 1,
			CHANNEL_UNSEQUENCED
		};

		template <typename Packet>
		static void SendPacket(ENetPeer* peer, ChannelFlag channel, const Packet& data) {
			ENetPacket* packet = enet_packet_create(&data, sizeof(data), static_cast<enet_uint32>(channel));
			enet_peer_send(peer, static_cast<enet_uint8>(channel), packet);
		}

		static void SendPacket(ENetPeer* peer, ChannelFlag channel, const char* data) {
			ENetPacket* packet = enet_packet_create(data, strlen(data) + 1, static_cast<enet_uint32>(channel));
			enet_peer_send(peer, static_cast<enet_uint8>(channel), packet);
		}

		using UpdatePacket = std::variant<CarPacketImage, WorldMap, DisconnectOtherClient>;

		class Packets {
		public:
			Packets() = default;
		private:

		};
	}
}
#endif // !PACKETS_H