#pragma once
#include <SFML/Graphics.hpp>
#include <string>

/**
 * Simple 3D vector structure for 3D simulation mode
 */
struct Vector3f {
    float x, y, z;

    Vector3f() : x(0), y(0), z(0) {}
    Vector3f(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
    Vector3f(const sf::Vector2f& vec2d) : x(vec2d.x), y(vec2d.y), z(0) {}

    Vector3f operator+(const Vector3f& other) const { return Vector3f(x + other.x, y + other.y, z + other.z); }
    Vector3f operator-(const Vector3f& other) const { return Vector3f(x - other.x, y - other.y, z - other.z); }
    Vector3f operator*(float scalar) const { return Vector3f(x * scalar, y * scalar, z * scalar); }
    Vector3f operator/(float scalar) const { return Vector3f(x / scalar, y / scalar, z / scalar); }
    Vector3f& operator+=(const Vector3f& other) { x += other.x; y += other.y; z += other.z; return *this; }
    Vector3f operator-() const { return Vector3f(-x, -y, -z); }

    float magnitude() const { return std::sqrt(x*x + y*y + z*z); }
    Vector3f normalized() const { float mag = magnitude(); return mag > 0 ? *this / mag : Vector3f(); }

    // Convert to 2D for rendering (orthographic projection)
    sf::Vector2f to2D() const { return sf::Vector2f(x, y); }
};

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

    // 3D physics properties
    Vector3f getPosition3D() const { return position3D_; }
    Vector3f getVelocity3D() const { return velocity3D_; }
    void setPosition3D(const Vector3f& position) {
        position3D_ = position;
        position_ = position.to2D();
    }
    void setVelocity3D(const Vector3f& velocity) {
        velocity3D_ = velocity;
        velocity_ = velocity.to2D();
    }

    void setPosition(const sf::Vector2f& position) { position_ = position; }
    void setVelocity(const sf::Vector2f& velocity) { velocity_ = velocity; }

    // Add force to the body (for gravity calculations)
    void addForce(const sf::Vector2f& force);
    void addForce3D(const Vector3f& force);

    // Update position based on velocity and forces (physics integration)
    void update(double deltaTime);
    void update3D(const std::vector<CelestialBody>& bodies, float deltaTime);

    // 3D distance calculation
    float getDistanceFrom3D(const CelestialBody& other) const;

    // Previous position for Verlet integration in 3D
    Vector3f getPreviousPosition3D() const { return previousPosition3D_; }
    void setPreviousPosition3D(const Vector3f& pos) { previousPosition3D_ = pos; }

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
    static Vector3f calculateGravitationalForce3D(const CelestialBody& body1,
                                                  const CelestialBody& body2);

private:
    std::string name_;
    double mass_;           // Mass in kg
    double radius_;         // Physical radius in meters
    sf::Vector2f position_; // Position in meters (simulation space - 2D)
    sf::Vector2f velocity_; // Velocity in m/s (2D)
    sf::Vector2f force_;    // Accumulated force for current physics step (2D)

    // 3D simulation support
    Vector3f position3D_;           // Position in 3D space
    Vector3f velocity3D_;           // Velocity in 3D space
    Vector3f force3D_;              // Accumulated force in 3D space
    Vector3f previousPosition3D_;   // Previous position for Verlet integration

    sf::Color color_;       // Visual color
    float visualRadius_;    // Visual radius for rendering (may be scaled)

    // Constants
    static constexpr double G = 6.67430e-11; // Gravitational constant
};
