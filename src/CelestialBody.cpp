#include "CelestialBody.h"
#include "Physics.h"
#include <cmath>

CelestialBody::CelestialBody(const std::string& name, double mass, double radius,
                           const sf::Vector2f& position, const sf::Vector2f& velocity,
                           const sf::Color& color)
    : name_(name), mass_(mass), radius_(radius), position_(position),
      velocity_(velocity), force_(0.0f, 0.0f), color_(color),
      visualRadius_(static_cast<float>(radius * Physics::DISTANCE_SCALE)) {
}

void CelestialBody::addForce(const sf::Vector2f& force) {
    force_ += force;
}

void CelestialBody::update(double deltaTime) {
    // Apply scaled time factor
    deltaTime *= Physics::TIME_SCALE;

    // Calculate acceleration from force (F = ma, so a = F/m)
    sf::Vector2f acceleration = force_ / static_cast<float>(mass_);

    // Update velocity using Verlet integration for better stability
    velocity_ += acceleration * static_cast<float>(deltaTime);

    // Update position
    position_ += velocity_ * static_cast<float>(deltaTime);
}

void CelestialBody::resetForces() {
    force_ = sf::Vector2f(0.0f, 0.0f);
}

bool CelestialBody::contains(const sf::Vector2f& point) const {
    float dx = point.x - position_.x;
    float dy = point.y - position_.y;
    float distanceSquared = dx * dx + dy * dy;
    float radiusSquared = visualRadius_ * visualRadius_;
    return distanceSquared <= radiusSquared;
}

double CelestialBody::distanceTo(const CelestialBody& other) const {
    return Physics::distance(position_, other.position_);
}

sf::Vector2f CelestialBody::calculateGravitationalForce(const CelestialBody& body1,
                                                       const CelestialBody& body2) {
    // Calculate distance vector
    sf::Vector2f deltaPos = body2.position_ - body1.position_;
    float distance = Physics::magnitude(deltaPos);

    // Prevent division by zero and unrealistic forces at very close distances
    const float minDistance = 1e6f; // Minimum distance in meters
    if (distance < minDistance) {
        distance = minDistance;
    }

    // Convert distance from simulation units to meters for calculation
    double distanceMeters = distance / Physics::DISTANCE_SCALE;

    // Calculate gravitational force magnitude: F = G * m1 * m2 / r^2
    double forceMagnitude = G * body1.mass_ * body2.mass_ / (distanceMeters * distanceMeters);

    // Convert force back to simulation units and apply direction
    float forceSimulation = static_cast<float>(forceMagnitude * Physics::DISTANCE_SCALE);
    sf::Vector2f direction = Physics::normalize(deltaPos);

    return direction * forceSimulation;
}
