#define LIGHT_INTENSITY 1.0
#define LIGHT_RADIUS 0.4
#define AMBIENT_LIGHT 0.2 // Lumière ambiante pour éclairer toute la scène

uniform float iTime;
uniform vec2 iLightPosition; // Position de l'objet lumineux
uniform vec2 iResolution;
uniform sampler2D texture; // Texture à laquelle le shader sera appliqué

void main() {
    vec2 uv = gl_FragCoord.xy / iResolution;
    vec4 color = texture2D(texture, uv);

    vec2 lightPos = iLightPosition / iResolution;
    float distance = distance(uv, lightPos);
    float lightFactor = 1.0 - smoothstep(0.0, LIGHT_RADIUS, distance);
    lightFactor = pow(lightFactor, LIGHT_INTENSITY);

    // Ajouter une lumière ambiante pour assurer que la scène reste visible
    color.rgb *= (AMBIENT_LIGHT + lightFactor * (1.0 - AMBIENT_LIGHT));

    gl_FragColor = color;
}

/* IMPLEMENTATION :
void Nova::Game() {
    if (this->render->newWindow(1980, 1080, "Nova", false, false)) {
        this->renderWindow->setFramerateLimit(60);

        // Charger le shader
        sf::Shader shader;
        if (!shader.loadFromFile("assets/shaders/light.frag", sf::Shader::Fragment)) {
            std::cout << "Failed to load shader" << std::endl;
        }

        // Charger la texture de fond
        sf::Texture backgroundTexture;
        if (!backgroundTexture.loadFromFile("wallpaper.jpeg")) {
            std::cerr << "Failed to load background texture" << std::endl;
            return;
        }

        // Créer un sprite pour la texture de fond
        sf::Sprite backgroundSprite(backgroundTexture);
        backgroundSprite.setScale(
            static_cast<float>(this->renderWindow->getSize().x) / backgroundTexture.getSize().x,
            static_cast<float>(this->renderWindow->getSize().y) / backgroundTexture.getSize().y);

        sf::Clock clock;

        while (this->renderWindow->isOpen()) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(*this->renderWindow);
            shader.setUniform("iTime", clock.getElapsedTime().asSeconds());
            shader.setUniform("iLightPosition", sf::Vector2f(mousePos.x, mousePos.y));
            shader.setUniform("iResolution", sf::Vector2f(this->renderWindow->getSize().x, this->renderWindow->getSize().y));

            sf::Event event;
            while (this->renderWindow->pollEvent(event)) {
                if (event.type == sf::Event::Closed)
                    this->renderWindow->close();
            }

            this->renderWindow->clear(sf::Color::Transparent);

            // Appliquer le shader au sprite de fond
            sf::RenderStates states;
            states.shader = &shader;
            this->renderWindow->draw(backgroundSprite, states);

            this->renderWindow->display();
        }
    } else {
        std::cout << "Render is not initialized!" << std::endl;
    }
}*/