#version 120 // GLSL version pour SFML 2.0

uniform sampler2D texture;
uniform float time;
uniform vec2 resolution;

void main() {
    // Inverser l'axe Y pour les coordonnées UV
    vec2 uv = gl_FragCoord.xy / resolution.xy;
    uv.y = 1.0 - uv.y; 

    // Logique de glitch
    float glitchAmount = sin(time) * sin(time * 20.0) * 0.002;
    vec2 redUV = uv + vec2(glitchAmount, 0.0);
    vec2 greenUV = uv;
    vec2 blueUV = uv - vec2(glitchAmount, 0.0);

    vec3 color;
    color.r = texture2D(texture, redUV).r;
    color.g = texture2D(texture, greenUV).g;
    color.b = texture2D(texture, blueUV).b;

    gl_FragColor = vec4(color, 1.0);
}

/* Implémentation :

sf::Shader glitchShader;
if (!glitchShader.loadFromFile("glitch.frag", sf::Shader::Fragment)) {
            // Handle shader loading error
}
sf::Clock clock;
glitchShader.setUniform("time", clock.getElapsedTime().asSeconds());
glitchShader.setUniform("resolution", sf::Vector2f(this->renderWindow->getSize()));


while (this->renderWindow->isOpen()) {
            glitchShader.setUniform("time", clock.getElapsedTime().asSeconds());
            sf::RenderStates states;
            states.shader = &glitchShader;
            this->render->render(states);
        }
    } else {
        std::cout << "Render is not initialized!" << std::endl;
    }
}*/