#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>

struct Airport {
    int id;
    std::string name;
    double lat;
    double lon;
};


static std::string get_field(std::stringstream& s) { // helper to parse CSV
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


sf::Vector2f toPixel(double lat, double lon, float width, float height) {
    double latRad = lat * M_PI / 180.0 ;  //mercator projection formula from stack overflow
    double mercN = std::log(std::tan((M_PI / 4.0) + (latRad / 2.0)));
    double y = (height / 2.0) - (width * mercN / (2.0 * M_PI));
    double x = (lon + 180.0) * (width / 360.0);
    return sf::Vector2f(static_cast<float>(x), static_cast<float>(y));
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

    sf::RenderWindow window(sf::VideoMode(1200, 1200), "OpenFlights Viewer"); // window for map
    window.setFramerateLimit(60);

    sf::Texture mapTexture;
    if (!mapTexture.loadFromFile("./data/world_map.jpg")) {
        std::cerr << "Error: could not load map image.\n";
    }

    sf::Sprite map(mapTexture);
    float mapW = mapTexture.getSize().x;
    float mapH = mapTexture.getSize().y;

    std::vector<sf::CircleShape> dots;
    dots.reserve(airports.size());
    for (auto& airport : airports) {
        auto p = toPixel(airport.lat, airport.lon, mapW, mapH);
        sf::CircleShape dot(1.5f);
        dot.setPosition(p.x -1.5f, p.y-1.5f);
        dot.setFillColor(sf::Color::Red);
        dots.push_back(dot);
    }

    sf::View view = window.getDefaultView();
    float zoom = 1.0f;
    bool dragging = false;
    sf::Vector2i mousePos;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
                window.close();
            }
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::R) {
                view = window.getDefaultView();
                zoom = 1.0f;
                window.setView(view);
            }
            if (event.type == sf::Event::MouseWheelScrolled) {
                if (event.mouseWheelScroll.delta > 0) {
                    zoom *= .9f;
                }
                else if (event.mouseWheelScroll.delta < 0) {
                    zoom *= 1.1f;
                }
                if (zoom < .05f)
                    zoom = .05f;
                else if (zoom > 2.f)
                    zoom = 2.f;
                view.setSize(window.getDefaultView().getSize() * zoom);
                window.setView(view);
            }
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {   // mouse panning from stack overflow
                 dragging = true;
                mousePos = sf::Mouse::getPosition(window);
            }
            else if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
                dragging = false;
            }
            else if (event.type == sf::Event::MouseMoved && dragging) {
                sf::Vector2i currMouse = sf::Mouse::getPosition(window);
                sf::Vector2f delta = window.mapPixelToCoords(mousePos, view) - window.mapPixelToCoords(currMouse, view);
                view.move(delta);
                window.setView(view);
                mousePos = currMouse;
            }

            window.clear(sf::Color::Black);
            window.draw(map);
            for (auto dot: dots)
                window.draw(dot);
            window.display();
        }
    }
}
