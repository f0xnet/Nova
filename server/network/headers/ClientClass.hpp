#ifndef CLIENTCLASS_HPP
#define CLIENTCLASS_HPP
#include <SFML/Network.hpp>
#include <functional>
#include <chrono>
#include "../../system/headers/LoggerManager.hpp"
#include "../../database/headers/DatabaseManager.hpp"
#include "PacketClass.hpp"

struct ClientIdentity {
    sf::IpAddress ip;
    unsigned short port;

    bool operator==(const ClientIdentity& other) const;
};

namespace std {
    template<>
    struct hash<ClientIdentity> {
        size_t operator()(const ClientIdentity& identity) const noexcept;
    };
}

class ClientClass {
public:
    ClientIdentity identity;

    ClientClass(const sf::IpAddress& ip, unsigned short port, sf::UdpSocket& socketRef);
    void addPacket(const sf::Packet& packet);
    bool getPacket(sf::Packet& packet);
    bool isPacketStackEmpty() const;
    bool isClientInactive() const;
    size_t getPacketStackSize() const;
    void setLastPacketTime();
    void setGameServerSocket(sf::UdpSocket& gameServerSocket);
    bool tryLogin(sf::Packet& packet, DatabaseManager* databaseManager);
    sf::UdpSocket& getGameServerSocket() const;
    sf::UdpSocket& socket;

private:
    std::unique_ptr<LoggerManager> loggerManager;
    PacketClass packetStack;
    std::chrono::steady_clock::time_point lastPacketTime;
    sf::UdpSocket* gameServerSocket;
};
#endif // CLIENTCLASS_HPP