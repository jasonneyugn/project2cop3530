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


static std::string get_field(std::stringstream& s) {
    std::string out;
    if (s.peek() == '"') {
        s.get(); // "
        std::getline(s, out, '"');
        if (s.peek() == ',') s.get();
    } else {
        std::getline(s, out, ',');
    }
    if (out == "\\N") out.clear();
    return out;
}


int main() {
    std::vector<Airport> airports;

    std::ifstream file("./data/airports.dat");
    if (!file.is_open()) {
        std::cerr << "Error: could not open airports.dat\n";
        return 1;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string cell;
        Airport a;

        std::string idS   = get_field(ss);
        std::string nameS = get_field(ss);
        get_field(ss);
        get_field(ss);
        get_field(ss);
        get_field(ss);
        std::string latS = get_field(ss);
        std::string lonS = get_field(ss);

        if (idS.empty() || latS.empty() || lonS.empty()) continue;

        a.id = std::stoi(idS);
        try {
            a.id = std::stoi(idS);
            a.name = nameS;
            a.lat = std::stod(latS);
            a.lon = std::stod(lonS);
        } catch (const std::invalid_argument&) {
            continue;
        }
        airports.push_back(a);
    }

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
