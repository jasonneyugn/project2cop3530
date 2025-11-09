OpenFlights Shortest Path Visualizer!!!

Overview: Using the mercator projection map (and formula), we visualize the shortest flight path between airports around the world using Dijkstra's Algorithm and the A* algorithm with a heuristic value. Using the OpenFlights dataset for routes and airports, along with generating routes to reach the threshold of 100,000 values, we created a connected graph of all airports.

Requirements / Setup:

** C++ Compiler (clang++ was used) ** SFML2@ was used (SFML 2 or later)

These are the only hard requirements and the code should work after installing both and setupping up the cmake file. I tested it once on mine so it should work.

Functionality: Left click + drag allows you to pan around the screen Left click highlights an airport and shows its connected routes Middle click displays name of airport R resets view to base Rightclick first to select a start and then rightclick again to select and end, This runs dijkstra's algorithm A to select start and then A to select end. This runs the A* algorithm

Contributers - Jason Nguyen
