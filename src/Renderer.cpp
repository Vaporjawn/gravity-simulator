#include "Renderer.h"
#include "Physics.h"
#include <iostream>
#include <sstream>
#include <iomanip>

Renderer::Renderer(sf::RenderWindow& window)
    : window_(window), zoom_(1.0f), center_(0.0f, 0.0f),
      showTrails_(true), showLabels_(true), showVelocityVectors_(false),
      showForceVectors_(false), showGrid_(false), maxTrailLength_(1000) {

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

    // Render trails first (so they appear behind bodies)
    if (showTrails_) {
        for (size_t i = 0; i < trails_.size(); ++i) {
            renderTrail(i);
        }
    }

    // Render celestial bodies
    const auto& bodies = solarSystem.getBodies();
    for (size_t i = 0; i < bodies.size(); ++i) {
        renderCelestialBody(*bodies[i], i);
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

void Renderer::renderCelestialBody(const CelestialBody& body, size_t bodyIndex) {
    sf::Vector2f pos = body.getPosition();
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
    ss << "Status: " << (solarSystem.isPaused() ? "PAUSED" : "RUNNING") << "\n\n";

    ss << "Controls:\n";
    ss << "WASD: Move camera\n";
    ss << "Mouse wheel: Zoom\n";
    ss << "Space: Pause/Resume\n";
    ss << "R: Reset simulation\n";
    ss << "T: Toggle trails\n";
    ss << "L: Toggle labels\n";
    ss << "G: Toggle grid\n";
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
