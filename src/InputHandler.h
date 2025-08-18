#pragma once
#include <SFML/Graphics.hpp>
#include "SolarSystem.h"
#include "Renderer.h"

/**
 * Handles user input for controlling the simulation
 */
class InputHandler {
public:
    InputHandler(sf::RenderWindow& window, SolarSystem& solarSystem, Renderer& renderer);
    ~InputHandler() = default;

    // Process events
    void handleEvents();

    // Update continuous input (called every frame)
    void update(double deltaTime);

    // Check if user wants to exit
    bool shouldExit() const { return shouldExit_; }

    // Helper methods
    void showHelpMessage();

private:
    sf::RenderWindow& window_;
    SolarSystem& solarSystem_;
    Renderer& renderer_;
    bool shouldExit_;

    // Input state
    bool mousePressed_;
    sf::Vector2i lastMousePos_;
    bool dragging_;

    // Keyboard state for continuous input
    struct KeyState {
        bool up = false;
        bool down = false;
        bool left = false;
        bool right = false;
        bool zoomIn = false;
        bool zoomOut = false;
        bool speedUp = false;
        bool slowDown = false;
        // 3D movement keys
        bool moveUp3D = false;     // Page Up - move camera up in Z
        bool moveDown3D = false;   // Page Down - move camera down in Z
        bool rotateUp = false;     // I key - rotate camera up (pitch)
        bool rotateDown = false;   // K key - rotate camera down (pitch)
        bool rotateLeft = false;   // J key - rotate camera left (yaw)
        bool rotateRight = false;  // L key - rotate camera right (yaw)
    } keyState_;

    // Input sensitivity settings
    float cameraSpeed_;
    float zoomSpeed_;
    float zoomStep_;
    float timeScaleStep_;

    // Event handlers
    void handleKeyPressed(const sf::Event& event);
    void handleKeyReleased(const sf::Event& event);
    void handleMousePressed(const sf::Event& event);
    void handleMouseReleased(const sf::Event& event);
    void handleMouseMoved(const sf::Event& event);
    void handleMouseWheelScrolled(const sf::Event& event);
    void handleWindowResized(const sf::Event& event);

    // Continuous input handlers
    void handleCameraMovement(double deltaTime);
    void handleZoomInput(double deltaTime);
    void handleTimeScaleInput(double deltaTime);
    void handle3DCameraMovement(double deltaTime);

    // Helper methods
    void toggleVisualOption(const std::string& option);
    void resetCamera();
    void centerOnSun();
};
