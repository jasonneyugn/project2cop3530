#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>
#include <unordered_map>
#include <queue>
#include <limits>

struct Airport {
    int id;
    std::string name;
    double lat;
    double lon;
};


struct Edge {
    int dest;
    double weight;
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


std::unordered_map<int, Airport*> makeMap(std::vector<Airport>& airports) {
    std::unordered_map<int, Airport*> map;
    for (auto& airport : airports) {
        map[airport.id] = &airport;
    }
    return map;
}


sf::Vector2f toPixel(double lat, double lon, float width, float height) {
    double latRad = lat * M_PI / 180.0 ;  //mercator projection formula from stack overflow
    double mercN = std::log(std::tan((M_PI / 4.0) + (latRad / 2.0)));
    double y = (height / 2.0) - (width * mercN / (2.0 * M_PI));
    double x = (lon + 180.5) * (width / 360.0);
    return sf::Vector2f(static_cast<float>(x), static_cast<float>(y));
}


static double haversine(double lat1, double lon1,    // from geeks for geeks
                        double lat2, double lon2)
{
    double dLat = (lat2 - lat1) *
                  M_PI / 180.0;
    double dLon = (lon2 - lon1) *
                  M_PI / 180.0;

    lat1 = (lat1) * M_PI / 180.0;
    lat2 = (lat2) * M_PI / 180.0;

    double a = pow(sin(dLat / 2), 2) +
               pow(sin(dLon / 2), 2) *
               cos(lat1) * cos(lat2);
    double rad = 6371;
    double c = 2 * asin(sqrt(a));
    return rad * c;
}


void loadRoutes(const std::string& filename, std::unordered_map<int, std::vector<Edge>>& adj,
                const std::unordered_map<int, Airport*>& airportById) {
    std::ifstream routes(filename);
    if (!routes.is_open()) {
        std::cerr << "Error: could not open airports.dat\n";
        return;
    }

    std::string line;
    while (std::getline(routes, line)) {
        std::stringstream ss(line);
        get_field(ss);
        get_field(ss);
        get_field(ss);
        std::string src_id = get_field(ss);
        get_field(ss);
        std::string dest_id = get_field(ss);

        if (src_id.empty() || dest_id.empty()) {
            continue;
        }
        int srcid = 0, destid = 0;
        try {
            srcid = std::stoi(src_id);
            destid = std::stoi(dest_id);
        } catch (...) {
            continue;
        }
        if (!airportById.count(srcid) || !airportById.count(destid))
            continue;

        const Airport* A = airportById.at(srcid);
        const Airport* B = airportById.at(destid);

        double dist = haversine(A->lat, A->lon, B->lat, B->lon);
        adj[srcid].push_back({destid, dist});
      }
}


std::vector<int> dijkstra(int start, int end, const std::unordered_map<int, std::vector<Edge>>& adj) {
    std::unordered_map<int, double> dist;
    std::unordered_map<int, int> prev;
    std::unordered_map<int, bool> visited;

    for (auto& [u, _] : adj) {
        dist[u] = std::numeric_limits<double>::infinity();
        visited[u] = false;
    }
    dist[start] = 0;

    using Node = std::pair<double, int>;
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> pq;

    pq.push({dist[start], start});
    while (!pq.empty()) {
        auto curr = pq.top();
        pq.pop();
        int u = curr.second;
        if (visited[u]) {
            continue;
        }
        visited[u] = true;
        if (u == end) {
            break;
        }
        if (!adj.count(u)) {
            continue;
        }
        for (auto& edge : adj.at(u)) {
            int v = edge.dest;
            double weight = edge.weight;
            double newDist = dist[u] + weight;
            if (newDist < dist[v]) {
                dist[v] = newDist;
                prev[v] = u;
                pq.push({dist[v], v});
            }
        }
    }
    std::vector<int> path;
    if (dist[end] == std::numeric_limits<double>::infinity()) {
        return path;
    }
    for (int at = end; at != start; at = prev[at]) {
        path.push_back(at);
    }
    path.push_back(start);
    std::reverse(path.begin(), path.end());
    return path;
}


int main() {
    std::vector<Airport> airports;
    std::string line;
    std::ifstream file("./data/airports.dat");
    if (!file.is_open()) {
        std::cerr << "Error: could not open airports.dat\n";
        return 1;
    }
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

        if (idS.empty() || latS.empty() || lonS.empty())
            continue;

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


    auto airportById = makeMap(airports);
    std::unordered_map<int, std::vector<Edge>> adj;
    loadRoutes("./data/routes.dat", adj, airportById);


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
        sf::CircleShape dot(2.f);
        dot.setPosition(p.x -2.f, p.y-2.f);
        dot.setFillColor(sf::Color::Red);
        dots.push_back(dot);
    }

    int selectedAirport = -1;
    int startAirport = -1;
    int endAirport = -1;
    std::vector<int> shortestPath;
    std::unordered_map<int, sf::Vector2f> airportPos;

    for (auto& airport : airports) {
        auto p = toPixel(airport.lat, airport.lon, mapW, mapH);
        sf::CircleShape dot(1.5f);
        dot.setPosition(p.x - 1.5f, p.y - 1.5f);
        dot.setFillColor(sf::Color::Red);
        airportPos[airport.id] = p; //
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

                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                sf::Vector2f before = window.mapPixelToCoords(mousePos, view);
                view.setSize(window.getDefaultView().getSize() * zoom);
                sf::Vector2f after = window.mapPixelToCoords(mousePos, view);
                view.move(before - after);
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
            if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f clickPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                float clickRadius = 2.0f;
                selectedAirport = -1;

                for (auto& airport : airports) {
                    sf::Vector2f pos = airportPos[airport.id];
                    if (std::hypot(pos.x - clickPos.x, pos.y - clickPos.y) < clickRadius) {
                        selectedAirport = airport.id;
                    }
                }
            }
            if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Right) {
                sf::Vector2f clickPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                float clickRadius = 2.0f;
                int clickedId = -1;

                for (auto& airport : airports) {
                    sf::Vector2f pos = airportPos[airport.id];
                    if (std::hypot(pos.x - clickPos.x, pos.y - clickPos.y) < clickRadius) {
                        clickedId = airport.id;
                        break;
                    }
                }
                if (clickedId != -1) {
                    if (startAirport ==-1) {
                        startAirport = clickedId;
                        std::cout << airportById.at(clickedId)->name << std::endl;
                    }
                    else if (endAirport ==-1) {
                        endAirport = clickedId;
                        std::cout << airportById.at(clickedId)->name << std::endl;
                        shortestPath = dijkstra(startAirport, endAirport, adj);
                        if (shortestPath.size() > 0) {
                            std::cout << "no path found." << std::endl;
                        } else {
                            std::cout << "Shortest path: " << shortestPath.size() << std::endl;
                        }
                    }
                } else {
                    startAirport = clickedId;
                    endAirport = -1;
                    shortestPath.clear();
                }
            }
            if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Middle) {
                sf::Vector2f clickPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                float clickRadius = 2.0f;
                int clickedId = -1;

                for (auto& airport : airports) {
                    sf::Vector2f pos = airportPos[airport.id];
                    if (std::hypot(pos.x - clickPos.x, pos.y - clickPos.y) < clickRadius) {
                        clickedId = airport.id;
                        break;
                    }
                }
                if (clickedId != -1) {
                    std::cout << "Airport name: " << airportById.at(clickedId)->name << std::endl;
                }
            }
            window.clear(sf::Color::Black);
            window.draw(map);
            for (size_t i = 0; i < airports.size(); i++) {
                auto& airport = airports[i];
                auto dot = dots[i];

                if (airport.id == selectedAirport) {
                    dot.setFillColor(sf::Color::Cyan);
                }
                else
                    dot.setFillColor(sf::Color::Red);

                dot.setScale(zoom, zoom);
                window.draw(dot);
            }
            if (selectedAirport != -1 && adj.count(selectedAirport)) {
                sf::VertexArray lines(sf::Lines);
                for (auto& e : adj[selectedAirport]) {
                    if (!airportPos.count(e.dest)) {
                        continue;
                    }
                    lines.append(sf::Vertex(airportPos[selectedAirport], sf::Color::Yellow));
                    lines.append(sf::Vertex(airportPos[e.dest], sf::Color::Yellow));
                }
                window.draw(lines);
                sf::CircleShape dotnew(2.f);
                dotnew.setFillColor(sf::Color::Blue);
                dotnew.setPosition(airportPos[selectedAirport].x - 2.f, airportPos[selectedAirport].y - 2.f);
                window.draw(dotnew);
            }
            window.display();
        }

    }
}

