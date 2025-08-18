#include "CelestialBody.h"
#include "Physics.h"
#include <cmath>

CelestialBody::CelestialBody(const std::string& name, double mass, double radius,
                           const sf::Vector2f& position, const sf::Vector2f& velocity,
                           const sf::Color& color)
    : name_(name), mass_(mass), radius_(radius), position_(position),
      velocity_(velocity), force_(0.0f, 0.0f),
      position3D_(Vector3f(position)), velocity3D_(Vector3f(velocity)), force3D_(Vector3f()),
      color_(color), visualRadius_(static_cast<float>(radius * Physics::DISTANCE_SCALE)) {
}

void CelestialBody::addForce(const sf::Vector2f& force) {
    force_ += force;
}

void CelestialBody::addForce3D(const Vector3f& force) {
    force3D_ += force;
    force_ += force.to2D(); // Keep 2D version in sync
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
    force3D_ = Vector3f();
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

Vector3f CelestialBody::calculateGravitationalForce3D(const CelestialBody& body1,
                                                     const CelestialBody& body2) {
    // Calculate distance vector in 3D
    Vector3f deltaPos = body2.position3D_ - body1.position3D_;
    float distance = deltaPos.magnitude();

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
    Vector3f direction = deltaPos.normalized();

    return direction * forceSimulation;
}

void CelestialBody::update3D(const std::vector<CelestialBody>& bodies, float deltaTime) {
    // Only update if not the Sun (index 0 in typical setup)
    if (this->mass_ < 1.989e30) { // Sun's mass threshold
        // Calculate net gravitational force from all other bodies
        Vector3f netForce(0.0f, 0.0f, 0.0f);

        for (const auto& body : bodies) {
            if (&body != this) { // Don't calculate force from itself
                Vector3f force = calculateGravitationalForce3D(*this, body);
                netForce = netForce + force;
            }
        }

        // Calculate acceleration: a = F/m
        Vector3f acceleration = netForce / static_cast<float>(mass_);

        // Store current position before updating
        Vector3f currentPos = position3D_;

        // Use a more stable integration method - simple Euler with smaller effective timestep
        float effectiveDt = deltaTime * 0.1f; // Much smaller timestep for stability

        // Update velocity
        velocity3D_ = velocity3D_ + acceleration * effectiveDt;

        // Update position
        position3D_ = position3D_ + velocity3D_ * effectiveDt;

        // Update previous position for Verlet (if needed later)
        previousPosition3D_ = currentPos;

        // Project 3D position to 2D for rendering
        position_ = sf::Vector2f(position3D_.x, position3D_.y);
    }
}float CelestialBody::getDistanceFrom3D(const CelestialBody& other) const {
    Vector3f diff = position3D_ - other.position3D_;
    return diff.magnitude();
}
