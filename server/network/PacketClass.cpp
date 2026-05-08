#include "headers/PacketClass.hpp"

void PacketClass::add(const sf::Packet& packet) {
    std::lock_guard<std::mutex> lock(mutex);
    stack.push(packet);
}

bool PacketClass::get(sf::Packet& packet) {
    std::lock_guard<std::mutex> lock(mutex);
    if (stack.empty()) {
        return false;
    }
    packet = std::move(stack.top());
    stack.pop();
    return true;
}

bool PacketClass::empty() const {
    std::lock_guard<std::mutex> lock(mutex);
    return stack.empty();
}

size_t PacketClass::size() const {
    std::lock_guard<std::mutex> lock(mutex);
    return stack.size();
}

void PacketClass::log(const std::string& type, const std::string& message) {
    std::thread([this, type, message]() {
        std::lock_guard<std::mutex> guard(this->logMutex); // Verrouillage du mutex
        // Affichage du message avec le type
        std::cout << "[" << type << "] " << message << std::endl;
    }).detach(); // Détachement de la thread pour permettre son exécution en arrière-plan
}
