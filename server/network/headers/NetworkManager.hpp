// NetworkManager.hpp
#ifndef NETWORKMANAGER_HPP
#define NETWORKMANAGER_HPP

#include <SFML/Network.hpp>
#include <unordered_map>
#include <memory>
#include "ClientClass.hpp"
#include "../../system/headers/LoggerManager.hpp"
#include "../../system/headers/InputManager.hpp"
#include "GameServerClass.hpp"
#include "DatabaseManager.hpp"

class NetworkManager {
public:
    explicit NetworkManager(unsigned short port);
    ~NetworkManager();

    bool init();
    void run();
    sf::UdpSocket* assignSocketToNewClient();

private:
    struct SocketInfo {
        sf::UdpSocket socket;
        unsigned short port;
        int clientCount = 0;

        SocketInfo();
    };

    unsigned short port;
    bool alive;
    sf::UdpSocket mainSocket;
    
    std::unique_ptr<LoggerManager> loggerManager;
    std::unique_ptr<InputManager> inputManager;
    std::unique_ptr<DatabaseManager> databaseManager;

    std::vector<std::unique_ptr<GameServerClass>> gameServers;

    std::vector<std::unique_ptr<SocketInfo>> loginSockets;
    std::unordered_map<ClientIdentity, std::unique_ptr<ClientClass>, std::hash<ClientIdentity>, std::equal_to<>, std::allocator<std::pair<const ClientIdentity, std::unique_ptr<ClientClass>>>> clients;

    void startLoginServer();
    bool broadcast(const std::string& data);
    void checkAdminCommands();
    void cleanupClients();
    void removeInactiveClients();
    void startGameServers();
    void stopGameServers();
    bool addClientToGameServer(const std::string& ip, unsigned short port);

    std::vector<std::unique_ptr<SocketInfo>> initializeSockets(int count);
    sf::UdpSocket* getAvailableSocket();
    std::vector<std::thread> threadPool;
    const int CLIENTS_BY_GAMESERVER = 10;
    const int MAX_CLIENTS = 1000;
    unsigned int nextThreadId = 1;
};

#endif // NETWORKMANAGER_HPP