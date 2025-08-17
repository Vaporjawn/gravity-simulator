#pragma once
#include <SFML/Graphics.hpp>
#include <string>

/**
 * Represents a celestial body in the solar system with physical properties
 * and visual representation
 */
class CelestialBody {
public:
    CelestialBody(const std::string& name, double mass, double radius,
                  const sf::Vector2f& position, const sf::Vector2f& velocity,
                  const sf::Color& color = sf::Color::White);

    // Physics properties
    double getMass() const { return mass_; }
    double getRadius() const { return radius_; }
    sf::Vector2f getPosition() const { return position_; }
    sf::Vector2f getVelocity() const { return velocity_; }
    std::string getName() const { return name_; }
    sf::Color getColor() const { return color_; }

    void setPosition(const sf::Vector2f& position) { position_ = position; }
    void setVelocity(const sf::Vector2f& velocity) { velocity_ = velocity; }

    // Add force to the body (for gravity calculations)
    void addForce(const sf::Vector2f& force);

    // Update position based on velocity and forces (physics integration)
    void update(double deltaTime);

    // Reset forces (called after each physics update)
    void resetForces();

    // Visual properties for rendering
    float getVisualRadius() const { return visualRadius_; }
    void setVisualRadius(float radius) { visualRadius_ = radius; }

    // Check if a point is within this body (for mouse interaction)
    bool contains(const sf::Vector2f& point) const;

    // Get distance to another body
    double distanceTo(const CelestialBody& other) const;

    // Static method to calculate gravitational force between two bodies
    static sf::Vector2f calculateGravitationalForce(const CelestialBody& body1,
                                                   const CelestialBody& body2);

private:
    std::string name_;
    double mass_;           // Mass in kg
    double radius_;         // Physical radius in meters
    sf::Vector2f position_; // Position in meters (simulation space)
    sf::Vector2f velocity_; // Velocity in m/s
    sf::Vector2f force_;    // Accumulated force for current physics step
    sf::Color color_;       // Visual color
    float visualRadius_;    // Visual radius for rendering (may be scaled)

    // Constants
    static constexpr double G = 6.67430e-11; // Gravitational constant
};
