#include "headers/ClientClass.hpp"
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

//---------- TODO :: Make object of ClientClass packet with std::make_unique

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

bool ClientClass::tryLogin(sf::Packet& packet, DatabaseManager* databaseManager) {
    std::string rawData;
    packet >> rawData;
    
    auto data = json::parse(rawData);
    
    std::string pid = data["pid"];
    std::string username = data["username"];
    std::string password = data["password"];
    std::string email = data["email"];

    if(pid == "login") {
        this->loggerManager->info("ClientClass", "Trying to login with username: " + username + " and password: " + password);
        if(databaseManager->accountDB->checkUserCredentials(username, password) == -1) {
            this->loggerManager->error("ClientClass", "Invalid credentials");
            return false;
        }
        else {
            this->loggerManager->info("ClientClass", "Client logged in: " + username);
            return true;
        }
    }
    else if(pid == "register") {
        this->loggerManager->info("ClientClass", "Trying to register with username: " + username + " and password: " + password + " and email: " + email);
        if(databaseManager->accountDB->checkUserExistence(username, email) == -1) {
           this->loggerManager->error("ClientClass", "E-Mail exist");
           return false;
        } else if(databaseManager->accountDB->checkUserExistence(username, email) == -2) {
           this->loggerManager->error("ClientClass", "Username exist");
           return false;
        }
        else {
           this->loggerManager->info("ClientClass", "Adding new user to database");
           int result = databaseManager->accountDB->addNewUser(username, password, email, this->identity.ip.toString());
              if(result == -1) {
                this->loggerManager->error("ClientClass", "Failed to add new user to database");
                return false;
              }
              else {
                this->loggerManager->info("ClientClass", "Client registered: " + username);
                json data = {{"pid", "login_clb"}};
                std::string serializedData = data.dump(); // Serialize the JSON object to a string
                packet << serializedData;
                this->socket.send(packet, this->identity.ip, this->identity.port);
                return true;
              }
        }
    }
    return false;
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