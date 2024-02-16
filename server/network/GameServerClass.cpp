#include "headers/GameServerClass.hpp"
#include "../system/headers/LoggerManager.hpp" // Assurez-vous que ce chemin d'accès correspond à votre structure de projet
#include <iostream>

GameServerClass::GameServerClass() : maxClientsPerThread(10), alive(false) {
    loggerManager = std::make_unique<LoggerManager>();
}

GameServerClass::~GameServerClass() {
    stop(); // S'assure que `alive` est mis à `false` et que les threads sont arrêtés proprement
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
    alive = false;
}

void GameServerClass::addLoggedClient(ClientClass* client) {
    ThreadState* availableState = findAvailableThreadState();
    if (availableState != nullptr && availableState->clientCount.load() < 10) {
        client->setGameServerSocket(*availableState->socket);
        availableState->clients.push_back(client);
        availableState->clientCount++;
        std::cout << "Client added to ThreadState with less than 10 clients. Total now: " << availableState->clientCount.load() << std::endl;
        this->currentClientCount++;
    } else {
        std::cout << "All threads are at full capacity." << std::endl;
    }
}

void GameServerClass::receiveData(ThreadState* state) {
    while (alive) {
        // Logique de réception des données
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Pour éviter une utilisation excessive du CPU
    }
}

int GameServerClass::getCurrentClientCount() const {
    return this->currentClientCount; // Supposons que currentClientCount est un membre qui suit le nombre de clients
}

ThreadState* GameServerClass::findAvailableThreadState() {
    for (auto& state : threadStates) {
        if (state->clientCount < 10) {
            return state.get(); // Retourne un pointeur vers le ThreadState disponible
        }
    }
    return nullptr; // Aucun ThreadState disponible trouvé
}