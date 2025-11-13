#include "headers/GameServerClass.hpp"
#include "../system/headers/LoggerManager.hpp" // Assurez-vous que ce chemin d'accès correspond à votre structure de projet
#include <iostream>

GameServerClass::GameServerClass() : maxClientsPerThread(10), alive(false) {
    loggerManager = std::make_unique<LoggerManager>();
}

GameServerClass::~GameServerClass() {
    stop();
    for (auto& state : threadStates) {
        if (state->thread.joinable()) {
            state->thread.join();
        }
    }
}

bool GameServerClass::init(int maxClients) {
    this->alive = true;
    return this->initRecvSockets();
}

bool GameServerClass::initRecvSockets() {
    for (int i = 0; i < maxClientsPerThread; ++i) {
        auto state = std::make_unique<ThreadState>();
        state->socket = std::make_unique<sf::UdpSocket>();
        if (state->socket->bind(sf::Socket::AnyPort) != sf::Socket::Done) {
            loggerManager->error("GameServerClass", "Failed to bind socket to any port");
            return false;
        }
        state->thread = std::thread(&GameServerClass::receiveData, this, state.get());
        threadStates.push_back(std::move(state));
    }
    return true;
}

void GameServerClass::run() {
    loggerManager->info("GameServerClass", "GameServerClass started");
}

void GameServerClass::stop() {
    {
        std::lock_guard<std::mutex> lock(mutex);
        alive = false;
    }
    condition.notify_all();

    for (auto& state : threadStates) {
        state->socket->unbind();
    }
    for (auto& state : threadStates) {
        if (state->thread.joinable()) {
            state->thread.join();
        }
    }
}

void GameServerClass::receiveData(ThreadState* state) {
    sf::Packet packet;
    sf::IpAddress sender;
    unsigned short port;

    while (true) {
        {
            std::unique_lock<std::mutex> lock(mutex);
            if (!alive) break;
        }

        sf::Socket::Status status = state->socket->receive(packet, sender, port);

        if (status == sf::Socket::Done) {
            loggerManager->info("GameServerClass", "Packet received from " + sender.toString());
            packet.clear();
        } else if (status == sf::Socket::NotReady) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        } else if (status == sf::Socket::Disconnected || status == sf::Socket::Error) {
            if (state->socket->getLocalPort() == 0) {
                break;
            }
            loggerManager->error("GameServerClass", "Error receiving packet from " + sender.toString());
        }
    }
}

void GameServerClass::addLoggedClient(ClientClass* client) {
    ThreadState* availableState = findAvailableThreadState();
    if (availableState != nullptr && availableState->clientCount.load() < 10) {
        client->setGameServerSocket(*availableState->socket);
        availableState->clients.push_back(client);
        availableState->clientCount++;
        this->currentClientCount++;

        sf::Packet packet;
        unsigned short gameServerPort = availableState->socket->getLocalPort();
        std::string jsonStr = "{\"pid\": \"swgm\", \"gmp\": " + std::to_string(gameServerPort) + "}";
        packet << jsonStr;

        if (client->getGameServerSocket().send(packet, client->identity.ip, client->identity.port) != sf::Socket::Done) {
            loggerManager->error("GameServerClass", "Failed to send game server port to client");
        } else {
            loggerManager->info("GameServerClass", "Game server port sent to client: " + std::to_string(gameServerPort));
        }
    } else {
        std::cout << "All threads are at full capacity." << std::endl;
    }
}

int GameServerClass::getCurrentClientCount() const {
    return this->currentClientCount;
}

ThreadState* GameServerClass::findAvailableThreadState() {
    for (auto& state : threadStates) {
        if (state->clientCount < 10) {
            return state.get();
        }
    }
    return nullptr;
}