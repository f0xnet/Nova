#include "headers/NetworkManager.hpp"
#include <iostream>

NetworkManager::NetworkManager(unsigned short port) : port(port), alive(false) {
    this->loggerManager = std::make_unique<LoggerManager>();
    this->inputManager = std::make_unique<InputManager>();
    this->databaseManager = std::make_unique<DatabaseManager>();

    this->loggerManager->info("NetworkManager", "NetworkManager constructor called");

    if (this->mainSocket.bind(this->port) != sf::Socket::Done) {
        throw std::runtime_error("Failed to bind main UDP socket to port " + std::to_string(this->port));
    }
    this->mainSocket.setBlocking(false);
    this->loginSockets = this->initializeSockets(1000);
}

bool NetworkManager::init() {
    this->loggerManager->info("NetworkManager", "Initializing NetworkManager");
    this->alive = true;
    return true;
}

void NetworkManager::run() {
    this->loggerManager->info("NetworkManager", "Starting game servers");
    this->startGameServers();
    this->loggerManager->info("NetworkManager", "Starting NetworkManager run loop");
    this->databaseManager->init();
    this->inputManager->startListening();

    while (this->alive) {
        this->startLoginServer();
        this->removeInactiveClients();
        this->checkAdminCommands();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    this->loggerManager->info("NetworkManager", "NetworkManager run loop stopped");
    this->shutdown();
}

void NetworkManager::startGameServers() {
    int gameServerCount = MAX_CLIENTS / CLIENTS_BY_GAMESERVER;
    for (int i = 0; i < gameServerCount; ++i) {
        auto gameServer = std::make_unique<GameServerClass>();
        if (gameServer->init(CLIENTS_BY_GAMESERVER)) {
            this->gameServers.push_back(std::move(gameServer));
            this->loggerManager->info("NetworkManager", "Initialized GameServer with id: " + std::to_string(i));
        } else {
            this->loggerManager->error("NetworkManager", "Failed to initialize GameServer with id: " + std::to_string(i));
        }
    }
    this->loggerManager->info("NetworkManager", "GameServers started successfully: " + std::to_string(gameServerCount));
}

void NetworkManager::startLoginServer() {
    sf::Packet packet;
    sf::IpAddress senderIP;
    unsigned short senderPort;

    while (mainSocket.receive(packet, senderIP, senderPort) == sf::Socket::Done && this->alive) {
        ClientIdentity identity = {senderIP, senderPort};

        auto clientIter = clients.find(identity);
        if (clientIter == clients.end()) {
            sf::UdpSocket* clientSocket = this->assignSocketToNewClient();
            auto insertResult = clients.emplace(identity, std::make_unique<ClientClass>(senderIP, senderPort, *clientSocket));
            if (insertResult.second) {
                this->loggerManager->info("NetworkManager", "New client connected: " + senderIP.toString() + ":" + std::to_string(senderPort));
                insertResult.first->second->addPacket(packet);
            } else {
                this->loggerManager->error("NetworkManager", "Failed to add new client: " + senderIP.toString() + ":" + std::to_string(senderPort));
            }
        } else {
            if (clientIter->second->tryLogin(packet, this->databaseManager.get())) {
                this->addClientToGameServer(clientIter->second.get());
            }
        }
    }
}

bool NetworkManager::addClientToGameServer(ClientClass* client) {
    for (auto& gameServer : this->gameServers) {
        if (gameServer->getCurrentClientCount() < CLIENTS_BY_GAMESERVER) {
            gameServer->addLoggedClient(client);
            return true;
        }
    }
    return false;
}

std::vector<std::unique_ptr<NetworkManager::SocketInfo>> NetworkManager::initializeSockets(int count) {
    std::vector<std::unique_ptr<SocketInfo>> initializedSockets;
    for (int i = 0; i < count; ++i) {
        initializedSockets.emplace_back(std::make_unique<SocketInfo>());
    }
    return initializedSockets;
}

sf::UdpSocket* NetworkManager::assignSocketToNewClient() {
    return this->getAvailableSocket();
}

sf::UdpSocket* NetworkManager::getAvailableSocket() {
    for (auto& socketInfo : this->loginSockets) {
        if (socketInfo->clientCount < 10) {
            socketInfo->clientCount++;
            this->loggerManager->info("NetworkManager", "Assigned socket to client: Port " + std::to_string(socketInfo->port) + ", Client count: " + std::to_string(socketInfo->clientCount));
            return &socketInfo->socket;
        }
    }
    return nullptr;
}

NetworkManager::SocketInfo::SocketInfo() {
    if (this->socket.bind(0) != sf::Socket::Done) {
        throw std::runtime_error("Failed to bind socket to an available port");
    }
    this->port = this->socket.getLocalPort();
}

bool NetworkManager::broadcast(const std::string& data) {
    sf::Packet packet;
    packet << data;
    for (auto& client : clients) {
        if (mainSocket.send(packet, client.second->identity.ip, client.second->identity.port) != sf::Socket::Done) {
            this->loggerManager->error("NetworkManager", "Failed to broadcast data to client: " + client.second->identity.ip.toString());
        }
    }
    return true;
}

void NetworkManager::checkAdminCommands() {
    if (this->inputManager->key == "q") {
        this->alive = false;
    }
}

void NetworkManager::removeInactiveClients() {
    auto it = clients.begin();
    while (it != clients.end()) {
        if (it->second->isClientInactive()) {
            loggerManager->info("NetworkManager", "Client is inactive: " + it->first.ip.toString() + ":" + std::to_string(it->first.port));
            it->second->socket.unbind();
            it = clients.erase(it);
        } else {
            ++it;
        }
    }
}

void NetworkManager::stopGameServers() {
    for (auto& gameServer : this->gameServers) {
        gameServer->stop();
        this->loggerManager->info("NetworkManager", "GameServer stopped");
    }
    for (auto& thread : this->threadPool) {
        if (thread.joinable()) {
            thread.join();
            this->loggerManager->info("NetworkManager", "Thread joined");
        }
    }
}

void NetworkManager::cleanupClients() {
    this->loggerManager->info("NetworkManager", "Cleaning up clients");
    this->clients.clear();
}

void NetworkManager::shutdown() {
    this->loggerManager->info("NetworkManager", "Shutting down NetworkManager");
    this->alive = false; // Indicate to the main loop to stop

    // Wait for the main loop to stop
    if (mainThread.joinable()) {
        mainThread.join();
        this->loggerManager->info("NetworkManager", "Main thread joined");
    }

    this->stopGameServers();
    this->cleanupClients();
    this->inputManager->stopListening();
}

NetworkManager::~NetworkManager() {
    if (this->alive) {
        this->shutdown();
    }
    this->loggerManager->info("NetworkManager", "NetworkManager destructor called");
}