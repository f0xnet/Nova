#ifndef NETWORK_MANAGER_HPP
#define NETWORK_MANAGER_HPP

#include <string>
#include <memory>
#include <thread>
#include <mutex>
#include <nlohmann/json.hpp>
#include <SFML/Network.hpp>

using json = nlohmann::json;

class NetworkManager {
private:
    sf::UdpSocket socket;
    std::unique_ptr<std::thread> packetHandlerThread;
    bool running = false;
    std::mutex logMutex;
    unsigned short port;
    std::string ip;
    unsigned short localPort;
    unsigned short gameServerPort;

    void handlePackets();
    void log(const std::string& type, const std::string& message);

public:
    NetworkManager();
    ~NetworkManager();
    bool Init(const std::string& ip, unsigned short port);
    bool Connect();
    void disconnect();
};

#endif // NETWORK_MANAGER_HPP
