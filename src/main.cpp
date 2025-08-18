#include <SFML/Graphics.hpp>
#include <iostream>
#include <chrono>

#include "SolarSystem.h"
#include "Renderer.h"
#include "InputHandler.h"

int main() {
    std::cout << "Starting Solar System Gravity Simulator...\n" << std::endl;

    // Create window
    const unsigned int WINDOW_WIDTH = 1200;
    const unsigned int WINDOW_HEIGHT = 800;
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT),
                           "Solar System Gravity Simulator",
                           sf::Style::Default);

    // Enable VSync for smooth animation
    window.setVerticalSyncEnabled(true);

    // Create simulation components
    SolarSystem solarSystem;
    Renderer renderer(window);
    InputHandler inputHandler(window, solarSystem, renderer);

    // Initialize the solar system
    solarSystem.initialize();

    // Set up initial camera view to show the entire solar system
    renderer.setZoom(0.5f);  // Zoom out to see more of the solar system
    renderer.setCenter(sf::Vector2f(0.0f, 0.0f));  // Center on the Sun

    // Show help message
    inputHandler.showHelpMessage();

    // Set up timing for physics updates
    auto lastTime = std::chrono::high_resolution_clock::now();
    const double MAX_DELTA_TIME = 1.0 / 30.0; // Cap delta time to prevent instability

    std::cout << "Simulation started! Press H for help.\n" << std::endl;

    // Main simulation loop
    while (window.isOpen() && !inputHandler.shouldExit()) {
        // Calculate delta time
        auto currentTime = std::chrono::high_resolution_clock::now();
        double deltaTime = std::chrono::duration<double>(currentTime - lastTime).count();
        lastTime = currentTime;

        // Cap delta time to prevent physics instability during window resize or lag spikes
        deltaTime = std::min(deltaTime, MAX_DELTA_TIME);

        // Handle input events
        inputHandler.handleEvents();
        inputHandler.update(deltaTime);

        // Update physics simulation
        solarSystem.update(deltaTime);

        // Render everything
        renderer.render(solarSystem, deltaTime);

        // Check if window was closed
        if (!window.isOpen()) {
            break;
        }
    }

    std::cout << "\nThank you for using the Solar System Gravity Simulator!" << std::endl;
    return 0;
}
