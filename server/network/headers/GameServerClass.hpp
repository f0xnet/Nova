#ifndef GAMESERVERCLASS_HPP
#define GAMESERVERCLASS_HPP

#include <SFML/Network.hpp>
#include <memory>
#include <vector>
#include <thread>
#include <atomic>

#include "ClientClass.hpp"

class LoggerManager;

struct ThreadState {
    std::unique_ptr<sf::UdpSocket> socket;
    std::thread thread;
    std::atomic<int> clientCount;
    std::vector<ClientClass*> clients;

    ThreadState() : clientCount(0) {}
    // Supprimer le constructeur de copie et l'opérateur d'affectation pour atomic
    ThreadState(const ThreadState&) = delete;
    ThreadState& operator=(const ThreadState&) = delete;
    // Supporter le déplacement
    ThreadState(ThreadState&&) noexcept = default;
    ThreadState& operator=(ThreadState&&) noexcept = default;
};

class GameServerClass {
private:
    std::unique_ptr<LoggerManager> loggerManager;
    std::vector<std::unique_ptr<ThreadState>> threadStates;
    int maxClientsPerThread;
    std::atomic<bool> alive;

    bool initRecvSockets();
    ThreadState* findAvailableThreadState();

    int currentClientCount;
    
public:
    GameServerClass();
    ~GameServerClass();
    bool init(int maxClients);
    void run();
    void stop();
    void addLoggedClient(ClientClass* client);
    void receiveData(ThreadState* state);
    int getCurrentClientCount() const;
};

#endif // GAMESERVERCLASS_HPP
