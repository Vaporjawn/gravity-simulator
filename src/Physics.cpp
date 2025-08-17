#include "Physics.h"
#include <cmath>

double Physics::calculateGravitationalForce(double m1, double m2, double r) {
    if (r <= 0.0) return 0.0;
    return G * m1 * m2 / (r * r);
}

sf::Vector2f Physics::calculateGravitationalForceVector(double m1, const sf::Vector2f& pos1,
                                                       double m2, const sf::Vector2f& pos2) {
    // Calculate distance vector
    sf::Vector2f deltaPos = pos2 - pos1;
    double distance = static_cast<double>(magnitude(deltaPos));

    // Prevent division by zero
    if (distance <= 0.0) return sf::Vector2f(0.0f, 0.0f);

    // Calculate force magnitude
    double forceMagnitude = calculateGravitationalForce(m1, m2, distance);

    // Calculate unit direction vector
    sf::Vector2f direction = normalize(deltaPos);

    // Apply force in the direction of the other body
    return direction * static_cast<float>(forceMagnitude);
}

float Physics::metersToPixels(double meters) {
    return static_cast<float>(meters * DISTANCE_SCALE);
}

double Physics::pixelsToMeters(float pixels) {
    return static_cast<double>(pixels) / DISTANCE_SCALE;
}

double Physics::calculateOrbitalVelocity(double centralMass, double distance) {
    if (distance <= 0.0) return 0.0;
    return std::sqrt(G * centralMass / distance);
}

double Physics::calculateEscapeVelocity(double mass, double radius) {
    if (radius <= 0.0) return 0.0;
    return std::sqrt(2.0 * G * mass / radius);
}

float Physics::distance(const sf::Vector2f& p1, const sf::Vector2f& p2) {
    float dx = p2.x - p1.x;
    float dy = p2.y - p1.y;
    return std::sqrt(dx * dx + dy * dy);
}

sf::Vector2f Physics::normalize(const sf::Vector2f& vec) {
    float mag = magnitude(vec);
    if (mag == 0.0f) return sf::Vector2f(0.0f, 0.0f);
    return vec / mag;
}

float Physics::magnitude(const sf::Vector2f& vec) {
    return std::sqrt(vec.x * vec.x + vec.y * vec.y);
}
