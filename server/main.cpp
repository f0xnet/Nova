#include <SFML/Network.hpp>
#include <iostream>
#include <string>
#include <vector>
#include "network/headers/NetworkManager.hpp"

int main() 
{
    NetworkManager networkManager(53000);
    networkManager.init();
    networkManager.run();

    return 0;
}