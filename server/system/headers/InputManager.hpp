#ifndef INPUTMANAGER_HPP
#define INPUTMANAGER_HPP

#include <atomic>
#include <thread>
#include <string>

class InputManager {
public:
    InputManager();
    ~InputManager();

    // Démarrer l'écoute des entrées clavier
    void startListening();

    // Arrêter l'écoute des entrées clavier
    void stopListening();

    std::string key; // Dernière touche pressée

private:
    std::atomic<bool> listening; // Contrôle l'écoute des entrées clavier
    std::thread inputThread; // Thread pour l'écoute des entrées clavier

    // Fonction exécutée dans le thread pour gérer les entrées clavier
    void processInput();
};

#endif // INPUTMANAGER_HPP
