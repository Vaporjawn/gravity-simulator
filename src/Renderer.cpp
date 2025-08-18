#include "Renderer.h"
#include "Physics.h"
#include <iostream>
#include <sstream>
#include <iomanip>

Renderer::Renderer(sf::RenderWindow& window)
    : window_(window), zoom_(1.0f), center_(0.0f, 0.0f),
      showTrails_(true), showLabels_(true), showVelocityVectors_(false),
      showForceVectors_(false), showGrid_(false), showSpacetimeWarping_(false),
      maxTrailLength_(1000),
      cameraZ_(-1000.0f), cameraRotationX_(0.0f), cameraRotationY_(0.0f) {

    // Initialize view
    view_ = window_.getDefaultView();
    updateView();

    // Load font
    if (!loadFont()) {
        std::cerr << "Warning: Failed to load font. Labels will not be displayed correctly." << std::endl;
    }

    // Initialize shapes for reuse
    circleShape_.setOrigin(1.0f, 1.0f); // Set origin to center
    lineShape_.setSize(sf::Vector2f(1.0f, 1.0f));
}

void Renderer::render(const SolarSystem& solarSystem, double deltaTime) {
    window_.clear(sf::Color::Black);

    // Update view
    updateView();
    window_.setView(view_);

    // Update trails
    updateTrails(solarSystem);

    // Render grid if enabled
    if (showGrid_) {
        renderGrid();
    }

    // Render spacetime warping grid if enabled
    if (showSpacetimeWarping_) {
        renderSpacetimeWarpingGrid(solarSystem);
    }

    // Render trails first (so they appear behind bodies)
    if (showTrails_) {
        for (size_t i = 0; i < trails_.size(); ++i) {
            renderTrail(i);
        }
    }

    // Render celestial bodies
    const auto& bodies = solarSystem.getBodies();
    for (size_t i = 0; i < bodies.size(); ++i) {
        renderCelestialBody(*bodies[i], i, solarSystem);
    }

    // Render UI elements in screen coordinates
    window_.setView(window_.getDefaultView());
    renderUI(solarSystem, deltaTime);

    window_.display();
}

void Renderer::setZoom(float zoom) {
    zoom_ = std::max(0.01f, std::min(100.0f, zoom)); // Clamp zoom
    updateView();
}

void Renderer::adjustZoom(float deltaZoom) {
    setZoom(zoom_ + deltaZoom);
}

void Renderer::setCenter(const sf::Vector2f& center) {
    center_ = center;
    updateView();
}

void Renderer::moveCamera(const sf::Vector2f& offset) {
    center_ += offset;
    updateView();
}

sf::Vector2f Renderer::screenToWorld(const sf::Vector2i& screenPos) const {
    return window_.mapPixelToCoords(screenPos, view_);
}

sf::Vector2i Renderer::worldToScreen(const sf::Vector2f& worldPos) const {
    return window_.mapCoordsToPixel(worldPos, view_);
}

void Renderer::clearTrails() {
    for (auto& trail : trails_) {
        trail.clear();
    }
}

sf::FloatRect Renderer::getViewBounds() const {
    sf::Vector2f viewSize = view_.getSize();
    sf::Vector2f viewCenter = view_.getCenter();
    return sf::FloatRect(
        viewCenter.x - viewSize.x * 0.5f,
        viewCenter.y - viewSize.y * 0.5f,
        viewSize.x,
        viewSize.y
    );
}

float Renderer::getVisualScale() const {
    // Scale visual elements based on zoom level
    return std::max(0.5f, std::min(3.0f, 1.0f / zoom_));
}

void Renderer::renderCelestialBody(const CelestialBody& body, size_t bodyIndex, const SolarSystem& solarSystem) {
    sf::Vector2f pos;

    if (solarSystem.is3DMode()) {
        // Use 3D projection
        Vector3f pos3D = body.getPosition3D();
        pos = project3DTo2D(pos3D, solarSystem);
    } else {
        // Use 2D position directly
        pos = body.getPosition();
    }

    float radius = calculateBodyVisualRadius(body);

    // Set up circle shape
    circleShape_.setRadius(radius);
    circleShape_.setOrigin(radius, radius);
    circleShape_.setPosition(pos);
    circleShape_.setFillColor(body.getColor());
    circleShape_.setOutlineColor(sf::Color::White);
    circleShape_.setOutlineThickness(0.5f * getVisualScale());

    window_.draw(circleShape_);

    // Render additional visual elements
    if (showLabels_) {
        renderLabel(body);
    }

    if (showVelocityVectors_) {
        renderVelocityVector(body);
    }

    if (showForceVectors_) {
        renderForceVector(body);
    }
}

void Renderer::renderTrail(size_t bodyIndex) {
    if (bodyIndex >= trails_.size() || trails_[bodyIndex].size() < 2) {
        return;
    }

    const auto& trail = trails_[bodyIndex];
    sf::VertexArray lines(sf::LineStrip, trail.size());

    for (size_t i = 0; i < trail.size(); ++i) {
        lines[i].position = trail[i].position;
        lines[i].color = trail[i].color;
        lines[i].color.a = static_cast<sf::Uint8>(trail[i].alpha * 255);
    }

    window_.draw(lines);
}

void Renderer::renderLabel(const CelestialBody& body) {
    if (!font_.getInfo().family.empty()) {
        labelText_.setFont(font_);
        labelText_.setString(body.getName());
        labelText_.setCharacterSize(static_cast<unsigned int>(16 * getVisualScale()));
        labelText_.setFillColor(sf::Color::White);

        sf::Vector2f pos = body.getPosition();
        float radius = calculateBodyVisualRadius(body);
        labelText_.setPosition(pos.x + radius + 5, pos.y - 8);

        window_.draw(labelText_);
    }
}

void Renderer::renderVelocityVector(const CelestialBody& body) {
    sf::Vector2f pos = body.getPosition();
    sf::Vector2f vel = body.getVelocity();

    // Scale velocity for visualization
    float scale = 0.001f * getVisualScale();
    sf::Vector2f endPos = pos + vel * scale;

    // Draw velocity vector as a line
    sf::Vertex line[] = {
        sf::Vertex(pos, sf::Color::Green),
        sf::Vertex(endPos, sf::Color::Green)
    };

    window_.draw(line, 2, sf::Lines);
}

void Renderer::renderForceVector(const CelestialBody& body) {
    // This would require access to current forces, which we don't have in the rendering phase
    // Could be implemented by storing force information in the celestial body
}

void Renderer::renderGrid() {
    sf::FloatRect bounds = getViewBounds();
    float gridSpacing = 50.0f / zoom_; // Adaptive grid spacing

    // Vertical lines
    float startX = std::floor(bounds.left / gridSpacing) * gridSpacing;
    for (float x = startX; x < bounds.left + bounds.width; x += gridSpacing) {
        sf::Vertex line[] = {
            sf::Vertex(sf::Vector2f(x, bounds.top), sf::Color(64, 64, 64)),
            sf::Vertex(sf::Vector2f(x, bounds.top + bounds.height), sf::Color(64, 64, 64))
        };
        window_.draw(line, 2, sf::Lines);
    }

    // Horizontal lines
    float startY = std::floor(bounds.top / gridSpacing) * gridSpacing;
    for (float y = startY; y < bounds.top + bounds.height; y += gridSpacing) {
        sf::Vertex line[] = {
            sf::Vertex(sf::Vector2f(bounds.left, y), sf::Color(64, 64, 64)),
            sf::Vertex(sf::Vector2f(bounds.left + bounds.width, y), sf::Color(64, 64, 64))
        };
        window_.draw(line, 2, sf::Lines);
    }
}

void Renderer::renderUI(const SolarSystem& solarSystem, double deltaTime) {
    if (font_.getInfo().family.empty()) return;

    std::stringstream ss;
    ss << std::fixed << std::setprecision(2);
    ss << "Solar System Gravity Simulator\n";
    ss << "Bodies: " << solarSystem.getBodyCount() << "\n";
    ss << "Time Scale: " << solarSystem.getTimeScale() << "x\n";
    ss << "Zoom: " << zoom_ << "x\n";
    ss << "Mode: " << (solarSystem.is3DMode() ? "3D" : "2D") << "\n";
    ss << "Spacetime: " << (showSpacetimeWarping_ ? "ON" : "OFF") << "\n";
    if (solarSystem.is3DMode()) {
        ss << "Camera Z: " << cameraZ_ << "\n";
        ss << "Pitch: " << (cameraRotationX_ * 180.0f / 3.14159f) << "°\n";
        ss << "Yaw: " << (cameraRotationY_ * 180.0f / 3.14159f) << "°\n";
    }
    ss << "Status: " << (solarSystem.isPaused() ? "PAUSED" : "RUNNING") << "\n\n";

    ss << "Controls:\n";
    ss << "WASD: Move camera\n";
    ss << "Mouse wheel: Zoom\n";
    if (solarSystem.is3DMode()) {
        ss << "Page Up/Down: Move Z-axis\n";
        ss << "I/K: Pitch, J/O: Yaw\n";
    }
    ss << "Space: Pause/Resume\n";
    ss << "R: Reset simulation\n";
    ss << "T: Toggle trails\n";
    ss << "L: Toggle labels\n";
    ss << "G: Toggle grid\n";
    ss << "M: Toggle 2D/3D mode\n";
    ss << "X: Toggle spacetime warping\n";
    ss << "+/-: Adjust time scale\n";
    ss << "ESC: Exit\n";

    labelText_.setFont(font_);
    labelText_.setString(ss.str());
    labelText_.setCharacterSize(14);
    labelText_.setFillColor(sf::Color::White);
    labelText_.setPosition(10, 10);

    window_.draw(labelText_);
}

void Renderer::updateTrails(const SolarSystem& solarSystem) {
    const auto& bodies = solarSystem.getBodies();

    // Resize trails vector if needed
    if (trails_.size() != bodies.size()) {
        trails_.resize(bodies.size());
    }

    // Add current positions to trails
    for (size_t i = 0; i < bodies.size(); ++i) {
        TrailPoint point;
        point.position = bodies[i]->getPosition();
        point.color = bodies[i]->getColor();
        point.alpha = 1.0f;

        trails_[i].push_back(point);

        // Remove old trail points
        if (trails_[i].size() > maxTrailLength_) {
            trails_[i].pop_front();
        }

        // Update alpha values for fade effect
        for (size_t j = 0; j < trails_[i].size(); ++j) {
            float t = static_cast<float>(j) / static_cast<float>(trails_[i].size());
            trails_[i][j].alpha = t * t; // Quadratic fade
        }
    }
}

void Renderer::updateView() {
    sf::Vector2u windowSize = window_.getSize();
    view_.setSize(static_cast<float>(windowSize.x) / zoom_, static_cast<float>(windowSize.y) / zoom_);
    view_.setCenter(center_);
}

bool Renderer::loadFont() {
    // Try to load system fonts (this is platform dependent)
    // On macOS, we can try some common system fonts
    const std::vector<std::string> fontPaths = {
        "/System/Library/Fonts/Arial.ttf",
        "/System/Library/Fonts/Helvetica.ttc",
        "/System/Library/Fonts/Times New Roman.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", // Linux
        "C:/Windows/Fonts/arial.ttf" // Windows
    };

    for (const auto& path : fontPaths) {
        if (font_.loadFromFile(path)) {
            return true;
        }
    }

    return false;
}

float Renderer::calculateBodyVisualRadius(const CelestialBody& body) const {
    float baseRadius = body.getVisualRadius();
    float scaledRadius = baseRadius * getVisualScale();

    // Ensure minimum visibility
    return std::max(2.0f, scaledRadius);
}

sf::Color Renderer::adjustColorAlpha(const sf::Color& color, sf::Uint8 alpha) const {
    sf::Color adjustedColor = color;
    adjustedColor.a = alpha;
    return adjustedColor;
}

float Renderer::calculateSpacetimeCurvature(const sf::Vector2f& position,
                                          const SolarSystem& solarSystem) const {
    const auto& bodies = solarSystem.getBodies();
    float totalCurvature = 0.0f;

    // Calculate gravitational field strength at this point
    for (const auto& body : bodies) {
        sf::Vector2f bodyPos = body->getPosition();
        sf::Vector2f diff = position - bodyPos;
        float distance = Physics::magnitude(diff);

        // Prevent division by zero
        if (distance < 1.0f) {
            distance = 1.0f;
        }

        // Schwarzschild radius approximation for curvature visualization
        // This is a simplified model for educational purposes
        float mass = body->getMass();
        float schwarzschildRadius = 2.0f * mass / (3e8 * 3e8); // Simplified calculation

        // Calculate curvature based on gravitational potential
        float curvature = schwarzschildRadius / (distance * distance);
        totalCurvature += curvature;
    }

    // Normalize curvature for visualization
    return std::min(1.0f, totalCurvature * 1e15f); // Scale factor for visibility
}

void Renderer::renderSpacetimeWarpingGrid(const SolarSystem& solarSystem) {
    if (!showSpacetimeWarping_) {
        return;
    }

    sf::FloatRect bounds = getViewBounds();
    float gridSpacing = 100.0f / zoom_; // Adaptive grid spacing for spacetime

    // Create vertex array for the warped grid
    sf::VertexArray grid(sf::Lines);

    // Draw vertical grid lines with curvature
    float startX = std::floor(bounds.left / gridSpacing) * gridSpacing;
    for (float x = startX; x < bounds.left + bounds.width; x += gridSpacing) {
        std::vector<sf::Vertex> vertices;

        float startY = bounds.top;
        float endY = bounds.top + bounds.height;
        int segments = 20; // Number of segments per line for smooth curves

        for (int i = 0; i <= segments; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(segments);
            float y = startY + t * (endY - startY);

            // Calculate curvature at this point
            sf::Vector2f gridPoint(x, y);
            float curvature = calculateSpacetimeCurvature(gridPoint, solarSystem);

            // Apply warping effect to x position
            float warpOffset = curvature * 50.0f * std::sin(t * 3.14159f); // Sine wave warping
            float warpedX = x + warpOffset;

            // Color based on curvature intensity
            sf::Uint8 intensity = static_cast<sf::Uint8>(255 * (1.0f - curvature));
            sf::Color lineColor(intensity, intensity, 255, 128); // Blue tinted, semi-transparent

            vertices.push_back(sf::Vertex(sf::Vector2f(warpedX, y), lineColor));
        }

        // Add vertices to grid
        for (size_t i = 0; i < vertices.size() - 1; ++i) {
            grid.append(vertices[i]);
            grid.append(vertices[i + 1]);
        }
    }

    // Draw horizontal grid lines with curvature
    float startY = std::floor(bounds.top / gridSpacing) * gridSpacing;
    for (float y = startY; y < bounds.top + bounds.height; y += gridSpacing) {
        std::vector<sf::Vertex> vertices;

        float startX2 = bounds.left;
        float endX = bounds.left + bounds.width;
        int segments = 20; // Number of segments per line for smooth curves

        for (int i = 0; i <= segments; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(segments);
            float x = startX2 + t * (endX - startX2);

            // Calculate curvature at this point
            sf::Vector2f gridPoint(x, y);
            float curvature = calculateSpacetimeCurvature(gridPoint, solarSystem);

            // Apply warping effect to y position
            float warpOffset = curvature * 50.0f * std::sin(t * 3.14159f); // Sine wave warping
            float warpedY = y + warpOffset;

            // Color based on curvature intensity
            sf::Uint8 intensity = static_cast<sf::Uint8>(255 * (1.0f - curvature));
            sf::Color lineColor(intensity, intensity, 255, 128); // Blue tinted, semi-transparent

            vertices.push_back(sf::Vertex(sf::Vector2f(x, warpedY), lineColor));
        }

        // Add vertices to grid
        for (size_t i = 0; i < vertices.size() - 1; ++i) {
            grid.append(vertices[i]);
            grid.append(vertices[i + 1]);
        }
    }

    window_.draw(grid);
}

// 3D Camera control methods
void Renderer::setCameraPosition3D(float x, float y, float z) {
    center_ = sf::Vector2f(x, y);
    cameraZ_ = z;
}

void Renderer::moveCameraZ(float deltaZ) {
    cameraZ_ += deltaZ;
    // Clamp camera Z to reasonable bounds
    cameraZ_ = std::max(100.0f, std::min(10000.0f, cameraZ_));
}

void Renderer::rotateCameraX(float deltaAngle) {
    cameraRotationX_ += deltaAngle;
    // Clamp rotation to reasonable bounds (-90 to 90 degrees)
    cameraRotationX_ = std::max(-1.57f, std::min(1.57f, cameraRotationX_));
}

void Renderer::rotateCameraY(float deltaAngle) {
    cameraRotationY_ += deltaAngle;
    // Keep rotation in 0-2π range
    while (cameraRotationY_ > 6.28f) cameraRotationY_ -= 6.28f;
    while (cameraRotationY_ < 0.0f) cameraRotationY_ += 6.28f;
}

sf::Vector2f Renderer::project3DTo2D(const Vector3f& position3D, const SolarSystem& solarSystem) const {
    if (!solarSystem.is3DMode()) {
        // In 2D mode, just return the X,Y coordinates
        return sf::Vector2f(position3D.x, position3D.y);
    }

    // Apply 3D camera transformations
    // First translate by camera position
    float x = position3D.x - center_.x;
    float y = position3D.y - center_.y;
    float z = position3D.z - cameraZ_;

    // Apply rotations
    // Rotation around X-axis (pitch)
    float cosX = std::cos(cameraRotationX_);
    float sinX = std::sin(cameraRotationX_);
    float y_rotX = y * cosX - z * sinX;
    float z_rotX = y * sinX + z * cosX;

    // Rotation around Y-axis (yaw)
    float cosY = std::cos(cameraRotationY_);
    float sinY = std::sin(cameraRotationY_);
    float x_rot = x * cosY + z_rotX * sinY;
    float z_rot = -x * sinY + z_rotX * cosY;

    // Perspective projection
    float perspective_distance = 1000.0f; // Distance to "screen"
    if (z_rot < -perspective_distance) z_rot = -perspective_distance + 1.0f; // Prevent division by zero

    float depth_factor = perspective_distance / (perspective_distance + z_rot);

    // Apply the depth factor and add back the camera center offset
    float projected_x = x_rot * depth_factor + center_.x;
    float projected_y = y_rotX * depth_factor + center_.y;

    return sf::Vector2f(projected_x, projected_y);
}
