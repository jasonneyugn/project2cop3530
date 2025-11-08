#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <SFML/Graphics.hpp>
#include <iostream>


struct Airport {
    int id;
    std::string name;
    double lat;
    double lon;
};

int main() {
    sf::RenderWindow window(sf::VideoMode(1200, 1200), "OpenFlights Viewer");
    window.setFramerateLimit(60);

    sf::Texture mapTexture;
    if (!mapTexture.loadFromFile("./data/world_map.jpg")) {
        std::cerr << "Error: could not load map image.\n";
    }

    sf::Sprite map(mapTexture);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear(sf::Color::Black);
        window.draw(map);
        window.display();
    }
}
