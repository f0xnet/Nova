#include "headers/NetworkManager.hpp"
#include <iostream>

// Surcharge de l'opérateur '<<' pour ajouter un objet JSON à un sf::Packet
sf::Packet& operator <<(sf::Packet& packet, const json& data) {
    std::string jsonString = data.dump(); // Convertit l'objet JSON en chaîne JSON
    return packet << jsonString;
}

// Constructor
NetworkManager::NetworkManager() {
}

bool NetworkManager::Init(const std::string& ip, unsigned short port) {
    this->ip = ip;
    this->port = port;
    this->localPort = port+2;
    if (socket.bind(0) != sf::Socket::Done) {
        log("ERROR", "Failed to bind UDP socket");
        return false;
    }
    socket.setBlocking(false);
    return true;
}

bool NetworkManager::Connect() {
    if (this->running) {
        this->disconnect();
    }
    running = true;
    packetHandlerThread = std::make_unique<std::thread>(&NetworkManager::handlePackets, this);

    json data = {{"pid", "register"}, {"username", "f0x"}, {"password", "hellsing"}, {"email", "maghielounet@gmail.com"}};
    sf::Packet packet;
    packet << data;
    // En UDP, on utilise sendTo pour envoyer un paquet à une adresse et un port spécifiques
    socket.send(packet, sf::IpAddress(ip), port);

    return true;
}

void NetworkManager::disconnect() {
    if (running) {
        running = false;
        if (packetHandlerThread && packetHandlerThread->joinable()) {
            packetHandlerThread->join();
        }
    }
}

void NetworkManager::handlePackets() {
    while (running) {
        //log("INFO", "Waiting for packets...");
        sf::Packet packet;
        sf::IpAddress senderIp;
        unsigned short senderPort;
        std::string data;
        if (socket.receive(packet, senderIp, senderPort) == sf::Socket::Done) {
            packet >> data;
            json receivedData = json::parse(data);
            std::string pid = receivedData["pid"];
            log("INFO", "Received packet from " + senderIp.toString() + ": " + pid);
        }
    }
}

// Destructor
NetworkManager::~NetworkManager() {
    disconnect();
}

void NetworkManager::log(const std::string& type, const std::string& message) {
    std::lock_guard<std::mutex> guard(this->logMutex);
    std::cout << "[" << type << "] " << message << std::endl;
}
