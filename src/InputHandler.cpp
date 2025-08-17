#include "InputHandler.h"
#include <iostream>

InputHandler::InputHandler(sf::RenderWindow& window, SolarSystem& solarSystem, Renderer& renderer)
    : window_(window), solarSystem_(solarSystem), renderer_(renderer), shouldExit_(false),
      mousePressed_(false), dragging_(false), cameraSpeed_(100.0f), zoomSpeed_(2.0f),
      zoomStep_(0.1f), timeScaleStep_(0.5f) {
}

void InputHandler::handleEvents() {
    sf::Event event;
    while (window_.pollEvent(event)) {
        switch (event.type) {
            case sf::Event::Closed:
                shouldExit_ = true;
                break;

            case sf::Event::KeyPressed:
                handleKeyPressed(event);
                break;

            case sf::Event::KeyReleased:
                handleKeyReleased(event);
                break;

            case sf::Event::MouseButtonPressed:
                handleMousePressed(event);
                break;

            case sf::Event::MouseButtonReleased:
                handleMouseReleased(event);
                break;

            case sf::Event::MouseMoved:
                handleMouseMoved(event);
                break;

            case sf::Event::MouseWheelScrolled:
                handleMouseWheelScrolled(event);
                break;

            case sf::Event::Resized:
                handleWindowResized(event);
                break;

            default:
                break;
        }
    }
}

void InputHandler::update(double deltaTime) {
    // Handle continuous input
    handleCameraMovement(deltaTime);
    handleZoomInput(deltaTime);
    handleTimeScaleInput(deltaTime);
}

void InputHandler::handleKeyPressed(const sf::Event& event) {
    switch (event.key.code) {
        case sf::Keyboard::Escape:
            shouldExit_ = true;
            break;

        case sf::Keyboard::Space:
            solarSystem_.togglePause();
            break;

        case sf::Keyboard::R:
            solarSystem_.reset();
            break;

        case sf::Keyboard::T:
            renderer_.setShowTrails(!renderer_.getShowTrails());
            if (!renderer_.getShowTrails()) {
                renderer_.clearTrails();
            }
            break;

        case sf::Keyboard::L:
            renderer_.setShowLabels(!renderer_.getShowLabels());
            break;

        case sf::Keyboard::G:
            renderer_.setShowGrid(!renderer_.getShowGrid());
            break;

        case sf::Keyboard::V:
            renderer_.setShowVelocityVectors(!renderer_.getShowVelocityVectors());
            break;

        case sf::Keyboard::C:
            centerOnSun();
            break;

        case sf::Keyboard::H:
            showHelpMessage();
            break;

        case sf::Keyboard::Num0:
            resetCamera();
            break;

        // Camera movement
        case sf::Keyboard::W:
        case sf::Keyboard::Up:
            keyState_.up = true;
            break;

        case sf::Keyboard::S:
        case sf::Keyboard::Down:
            keyState_.down = true;
            break;

        case sf::Keyboard::A:
        case sf::Keyboard::Left:
            keyState_.left = true;
            break;

        case sf::Keyboard::D:
        case sf::Keyboard::Right:
            keyState_.right = true;
            break;

        // Zoom
        case sf::Keyboard::Q:
            keyState_.zoomOut = true;
            break;

        case sf::Keyboard::E:
            keyState_.zoomIn = true;
            break;

        // Time scale
        case sf::Keyboard::Equal: // + key
        case sf::Keyboard::Add:
            keyState_.speedUp = true;
            break;

        case sf::Keyboard::Hyphen: // - key
        case sf::Keyboard::Subtract:
            keyState_.slowDown = true;
            break;

        default:
            break;
    }
}

void InputHandler::handleKeyReleased(const sf::Event& event) {
    switch (event.key.code) {
        case sf::Keyboard::W:
        case sf::Keyboard::Up:
            keyState_.up = false;
            break;

        case sf::Keyboard::S:
        case sf::Keyboard::Down:
            keyState_.down = false;
            break;

        case sf::Keyboard::A:
        case sf::Keyboard::Left:
            keyState_.left = false;
            break;

        case sf::Keyboard::D:
        case sf::Keyboard::Right:
            keyState_.right = false;
            break;

        case sf::Keyboard::Q:
            keyState_.zoomOut = false;
            break;

        case sf::Keyboard::E:
            keyState_.zoomIn = false;
            break;

        case sf::Keyboard::Equal:
        case sf::Keyboard::Add:
            keyState_.speedUp = false;
            break;

        case sf::Keyboard::Hyphen:
        case sf::Keyboard::Subtract:
            keyState_.slowDown = false;
            break;

        default:
            break;
    }
}

void InputHandler::handleMousePressed(const sf::Event& event) {
    if (event.mouseButton.button == sf::Mouse::Left) {
        mousePressed_ = true;
        lastMousePos_ = sf::Vector2i(event.mouseButton.x, event.mouseButton.y);
        dragging_ = false;
    }
}

void InputHandler::handleMouseReleased(const sf::Event& event) {
    if (event.mouseButton.button == sf::Mouse::Left) {
        mousePressed_ = false;
        dragging_ = false;
    }
}

void InputHandler::handleMouseMoved(const sf::Event& event) {
    if (mousePressed_) {
        sf::Vector2i currentMousePos(event.mouseMove.x, event.mouseMove.y);
        sf::Vector2i mouseDelta = lastMousePos_ - currentMousePos;

        // Convert mouse movement to world coordinates
        float scaleFactor = 1.0f / renderer_.getZoom();
        sf::Vector2f worldDelta(static_cast<float>(mouseDelta.x) * scaleFactor,
                               static_cast<float>(mouseDelta.y) * scaleFactor);

        renderer_.moveCamera(worldDelta);

        lastMousePos_ = currentMousePos;
        dragging_ = true;
    }
}

void InputHandler::handleMouseWheelScrolled(const sf::Event& event) {
    float zoomDelta = event.mouseWheelScroll.delta * zoomStep_;
    renderer_.adjustZoom(zoomDelta);
}

void InputHandler::handleWindowResized(const sf::Event& event) {
    // Update the window view when resized
    sf::FloatRect visibleArea(0.0f, 0.0f,
                             static_cast<float>(event.size.width),
                             static_cast<float>(event.size.height));
    window_.setView(sf::View(visibleArea));
}

void InputHandler::handleCameraMovement(double deltaTime) {
    sf::Vector2f movement(0.0f, 0.0f);

    if (keyState_.left) movement.x -= 1.0f;
    if (keyState_.right) movement.x += 1.0f;
    if (keyState_.up) movement.y -= 1.0f;
    if (keyState_.down) movement.y += 1.0f;

    if (movement.x != 0.0f || movement.y != 0.0f) {
        // Normalize diagonal movement
        float length = std::sqrt(movement.x * movement.x + movement.y * movement.y);
        movement /= length;

        // Scale by camera speed and zoom level
        float speed = cameraSpeed_ * static_cast<float>(deltaTime) / renderer_.getZoom();
        movement *= speed;

        renderer_.moveCamera(movement);
    }
}

void InputHandler::handleZoomInput(double deltaTime) {
    if (keyState_.zoomIn) {
        float zoomDelta = zoomSpeed_ * static_cast<float>(deltaTime);
        renderer_.adjustZoom(zoomDelta);
    }

    if (keyState_.zoomOut) {
        float zoomDelta = -zoomSpeed_ * static_cast<float>(deltaTime);
        renderer_.adjustZoom(zoomDelta);
    }
}

void InputHandler::handleTimeScaleInput(double deltaTime) {
    if (keyState_.speedUp) {
        double currentTimeScale = solarSystem_.getTimeScale();
        double newTimeScale = currentTimeScale + timeScaleStep_ * deltaTime;
        solarSystem_.setTimeScale(std::min(10.0, newTimeScale));
    }

    if (keyState_.slowDown) {
        double currentTimeScale = solarSystem_.getTimeScale();
        double newTimeScale = currentTimeScale - timeScaleStep_ * deltaTime;
        solarSystem_.setTimeScale(std::max(0.1, newTimeScale));
    }
}

void InputHandler::resetCamera() {
    renderer_.setCenter(sf::Vector2f(0.0f, 0.0f));
    renderer_.setZoom(1.0f);
}

void InputHandler::centerOnSun() {
    CelestialBody* sun = solarSystem_.getCentralBody();
    if (sun) {
        renderer_.setCenter(sun->getPosition());
    }
}

void InputHandler::showHelpMessage() {
    std::cout << "\n=== Solar System Gravity Simulator Help ===\n";
    std::cout << "Camera Controls:\n";
    std::cout << "  WASD / Arrow Keys: Move camera\n";
    std::cout << "  Q/E: Zoom out/in\n";
    std::cout << "  Mouse wheel: Zoom\n";
    std::cout << "  Left click + drag: Pan camera\n";
    std::cout << "  0: Reset camera to origin\n";
    std::cout << "  C: Center on Sun\n\n";

    std::cout << "Simulation Controls:\n";
    std::cout << "  Space: Pause/Resume simulation\n";
    std::cout << "  R: Reset to initial conditions\n";
    std::cout << "  +/-: Increase/Decrease time scale\n\n";

    std::cout << "Visual Options:\n";
    std::cout << "  T: Toggle orbital trails\n";
    std::cout << "  L: Toggle planet labels\n";
    std::cout << "  G: Toggle grid\n";
    std::cout << "  V: Toggle velocity vectors\n\n";

    std::cout << "Other:\n";
    std::cout << "  H: Show this help\n";
    std::cout << "  ESC: Exit simulation\n";
    std::cout << "=========================================\n\n";
}
