#include "headers/ClientClass.hpp"
#include <iostream>


ClientClass::ClientClass(const sf::IpAddress& ip, unsigned short port, sf::UdpSocket& socket) : identity({ip, port}), socket(socket){
    this->loggerManager = std::make_unique<LoggerManager>();
    this->loggerManager->info("ClientClass", "New client instance created: " + ip.toString() + ":" + std::to_string(port));
}

void ClientClass::addPacket(const sf::Packet& packet) {
    this->packetStack.add(packet);
    this->loggerManager->info("ClientClass", "Received packet from " + identity.ip.toString() + ":" + std::to_string(identity.port));
    this->setLastPacketTime();
}

bool ClientClass::getPacket(sf::Packet& packet) {
    return this->packetStack.get(packet); // Récupération d'un paquet de la pile
}

bool ClientClass::isPacketStackEmpty() const {
    return this->packetStack.empty(); // Vérification si la pile est vide
}

size_t ClientClass::getPacketStackSize() const {
    size_t size = this->packetStack.size();
    this->loggerManager->info("ClientClass", "Packet stack size: " + std::to_string(size));
    return this->packetStack.size(); // Récupération de la taille de la pile
}

void ClientClass::setLastPacketTime() {
    this->lastPacketTime = std::chrono::steady_clock::now();
}

void ClientClass::setGameServerSocket(sf::UdpSocket& gameServerSocket) {
    this->gameServerSocket = &gameServerSocket;
}

sf::UdpSocket& ClientClass::getGameServerSocket() const {
    return *this->gameServerSocket;
}

bool ClientClass::isClientInactive() const {
    auto now = std::chrono::steady_clock::now();
    auto inactiveDuration = std::chrono::minutes(1);
    return (now - this->lastPacketTime) > inactiveDuration;
}

bool ClientIdentity::operator==(const ClientIdentity& other) const {
    return ip == other.ip && port == other.port;
}

namespace std {
    size_t hash<ClientIdentity>::operator()(const ClientIdentity& identity) const noexcept {
        return hash<string>()(identity.ip.toString()) ^ hash<unsigned short>()(identity.port);
    }
}