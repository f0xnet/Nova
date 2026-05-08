#ifndef PACKETCLASS_HPP
#define PACKETCLASS_HPP

#include <SFML/Network.hpp>
#include <stack>
#include <mutex>
#include <queue>
#include <iostream>
#include <string>
#include <thread>

class PacketClass {
    std::stack<sf::Packet> stack;
    mutable std::mutex mutex;
    mutable std::mutex logMutex;

    void log(const std::string& type, const std::string& message);

public:
    void add(const sf::Packet& packet);
    bool get(sf::Packet& packet);
    bool empty() const;
    size_t size() const;

    //set queue variable    
    std::queue<sf::Packet> queue;
    
};

#endif // PACKETCLASS_HPP