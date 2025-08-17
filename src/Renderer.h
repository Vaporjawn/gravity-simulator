#pragma once
#include <SFML/Graphics.hpp>
#include "SolarSystem.h"
#include <vector>
#include <deque>

/**
 * Handles all rendering operations for the gravity simulation
 */
class Renderer {
public:
    Renderer(sf::RenderWindow& window);
    ~Renderer() = default;

    // Main rendering method
    void render(const SolarSystem& solarSystem, double deltaTime);

    // Camera controls
    void setZoom(float zoom);
    float getZoom() const { return zoom_; }
    void adjustZoom(float deltaZoom);

    void setCenter(const sf::Vector2f& center);
    sf::Vector2f getCenter() const { return center_; }
    void moveCamera(const sf::Vector2f& offset);

    // Convert between screen coordinates and world coordinates
    sf::Vector2f screenToWorld(const sf::Vector2i& screenPos) const;
    sf::Vector2i worldToScreen(const sf::Vector2f& worldPos) const;

    // Visual settings
    void setShowTrails(bool show) { showTrails_ = show; }
    bool getShowTrails() const { return showTrails_; }

    void setShowLabels(bool show) { showLabels_ = show; }
    bool getShowLabels() const { return showLabels_; }

    void setShowVelocityVectors(bool show) { showVelocityVectors_ = show; }
    bool getShowVelocityVectors() const { return showVelocityVectors_; }

    void setShowForceVectors(bool show) { showForceVectors_ = show; }
    bool getShowForceVectors() const { return showForceVectors_; }

    void setShowGrid(bool show) { showGrid_ = show; }
    bool getShowGrid() const { return showGrid_; }

    // Trail management
    void clearTrails();
    void setMaxTrailLength(size_t maxLength) { maxTrailLength_ = maxLength; }
    size_t getMaxTrailLength() const { return maxTrailLength_; }

    // Get camera bounds for optimization
    sf::FloatRect getViewBounds() const;

    // Visual scaling for different zoom levels
    float getVisualScale() const;

private:
    sf::RenderWindow& window_;
    sf::View view_;
    sf::Font font_;

    // Camera properties
    float zoom_;
    sf::Vector2f center_;

    // Visual options
    bool showTrails_;
    bool showLabels_;
    bool showVelocityVectors_;
    bool showForceVectors_;
    bool showGrid_;

    // Trail system
    struct TrailPoint {
        sf::Vector2f position;
        sf::Color color;
        float alpha;
    };
    std::vector<std::deque<TrailPoint>> trails_; // One trail per celestial body
    size_t maxTrailLength_;

    // Rendering shapes (reused for performance)
    sf::CircleShape circleShape_;
    sf::RectangleShape lineShape_;
    sf::Text labelText_;

    // Private methods
    void renderCelestialBody(const CelestialBody& body, size_t bodyIndex);
    void renderTrail(size_t bodyIndex);
    void renderLabel(const CelestialBody& body);
    void renderVelocityVector(const CelestialBody& body);
    void renderForceVector(const CelestialBody& body);
    void renderGrid();
    void renderUI(const SolarSystem& solarSystem, double deltaTime);

    void updateTrails(const SolarSystem& solarSystem);
    void updateView();

    // Helper methods
    bool loadFont();
    float calculateBodyVisualRadius(const CelestialBody& body) const;
    sf::Color adjustColorAlpha(const sf::Color& color, sf::Uint8 alpha) const;
};
