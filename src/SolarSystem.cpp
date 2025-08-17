#include "SolarSystem.h"
#include "Physics.h"
#include <iostream>
#include <cmath>

SolarSystem::SolarSystem() : paused_(false), timeScale_(1.0) {
}

void SolarSystem::initialize() {
    clear();

    // Create the Sun at the center
    createSun();

    // Create planets with realistic orbital data
    // Note: Using simplified circular orbits for stability
    // Distances in AU, masses relative to Earth, radii in km

    // Mercury
    createPlanet("Mercury", 0.0553 * Physics::EARTH_MASS, 2439.7e3,
                0.39 * Physics::AU, 47.87e3, sf::Color(169, 169, 169), 5.0f);

    // Venus
    createPlanet("Venus", 0.815 * Physics::EARTH_MASS, 6051.8e3,
                0.72 * Physics::AU, 35.02e3, sf::Color(255, 198, 73), 4.0f);

    // Earth
    createPlanet("Earth", Physics::EARTH_MASS, 6371e3,
                1.0 * Physics::AU, 29.78e3, sf::Color(100, 149, 237), 4.0f);

    // Mars
    createPlanet("Mars", 0.107 * Physics::EARTH_MASS, 3389.5e3,
                1.52 * Physics::AU, 24.08e3, sf::Color(205, 92, 92), 3.0f);

    // Jupiter
    createPlanet("Jupiter", 317.8 * Physics::EARTH_MASS, 69911e3,
                5.2 * Physics::AU, 13.07e3, sf::Color(255, 165, 0), 2.0f);

    // Saturn
    createPlanet("Saturn", 95.2 * Physics::EARTH_MASS, 58232e3,
                9.5 * Physics::AU, 9.69e3, sf::Color(218, 165, 32), 1.8f);

    // Uranus
    createPlanet("Uranus", 14.5 * Physics::EARTH_MASS, 25362e3,
                19.2 * Physics::AU, 6.81e3, sf::Color(64, 224, 208), 1.5f);

    // Neptune
    createPlanet("Neptune", 17.1 * Physics::EARTH_MASS, 24622e3,
                30.1 * Physics::AU, 5.43e3, sf::Color(65, 105, 225), 1.5f);

    storeInitialConditions();

    std::cout << "Solar system initialized with " << bodies_.size() << " celestial bodies." << std::endl;
}

void SolarSystem::update(double deltaTime) {
    if (!paused_) {
        updatePhysics(deltaTime * timeScale_);
    }
}

void SolarSystem::addBody(std::unique_ptr<CelestialBody> body) {
    bodies_.push_back(std::move(body));
}

void SolarSystem::clear() {
    bodies_.clear();
    initialConditions_.clear();
}

CelestialBody* SolarSystem::getBody(size_t index) {
    if (index >= bodies_.size()) return nullptr;
    return bodies_[index].get();
}

const CelestialBody* SolarSystem::getBody(size_t index) const {
    if (index >= bodies_.size()) return nullptr;
    return bodies_[index].get();
}

CelestialBody* SolarSystem::findBody(const std::string& name) {
    for (auto& body : bodies_) {
        if (body->getName() == name) {
            return body.get();
        }
    }
    return nullptr;
}

double SolarSystem::getTotalEnergy() const {
    double totalEnergy = 0.0;

    // Calculate kinetic energy
    for (const auto& body : bodies_) {
        sf::Vector2f vel = body->getVelocity();
        double kineticEnergy = 0.5 * body->getMass() * (vel.x * vel.x + vel.y * vel.y);
        totalEnergy += kineticEnergy;
    }

    // Calculate potential energy (avoid double counting)
    for (size_t i = 0; i < bodies_.size(); ++i) {
        for (size_t j = i + 1; j < bodies_.size(); ++j) {
            double distance = bodies_[i]->distanceTo(*bodies_[j]);
            double potentialEnergy = -Physics::G * bodies_[i]->getMass() * bodies_[j]->getMass() / distance;
            totalEnergy += potentialEnergy;
        }
    }

    return totalEnergy;
}

sf::Vector2f SolarSystem::getCenterOfMass() const {
    sf::Vector2f centerOfMass(0.0f, 0.0f);
    double totalMass = 0.0;

    for (const auto& body : bodies_) {
        sf::Vector2f pos = body->getPosition();
        double mass = body->getMass();
        centerOfMass += pos * static_cast<float>(mass);
        totalMass += mass;
    }

    if (totalMass > 0.0) {
        centerOfMass /= static_cast<float>(totalMass);
    }

    return centerOfMass;
}

void SolarSystem::reset() {
    restoreInitialConditions();
}

void SolarSystem::updatePhysics(double deltaTime) {
    // Calculate gravitational forces between all bodies
    calculateGravitationalForces();

    // Update all bodies
    for (auto& body : bodies_) {
        body->update(deltaTime);
        body->resetForces();
    }
}

void SolarSystem::calculateGravitationalForces() {
    // Calculate forces between all pairs of bodies
    for (size_t i = 0; i < bodies_.size(); ++i) {
        for (size_t j = i + 1; j < bodies_.size(); ++j) {
            // Calculate gravitational force between body i and body j
            sf::Vector2f force = CelestialBody::calculateGravitationalForce(*bodies_[i], *bodies_[j]);

            // Apply equal and opposite forces (Newton's third law)
            bodies_[i]->addForce(force);
            bodies_[j]->addForce(-force);
        }
    }
}

void SolarSystem::createSun() {
    sf::Vector2f sunPosition(0.0f, 0.0f);  // Center of the solar system
    sf::Vector2f sunVelocity(0.0f, 0.0f);  // Stationary (approximately)

    auto sun = std::make_unique<CelestialBody>(
        "Sun",
        Physics::SUN_MASS,
        696340e3, // Sun radius in meters
        sunPosition,
        sunVelocity,
        sf::Color::Yellow
    );

    // Make the Sun visually larger for better visibility
    sun->setVisualRadius(20.0f);

    addBody(std::move(sun));
}

void SolarSystem::createPlanet(const std::string& name, double mass, double radius,
                              double orbitDistance, double orbitVelocity,
                              const sf::Color& color, float visualScale) {
    // Convert orbital distance to simulation coordinates
    float simDistance = Physics::metersToPixels(orbitDistance);

    // Place planet on positive X axis initially
    sf::Vector2f planetPosition(simDistance, 0.0f);

    // Calculate orbital velocity for stable circular orbit
    double orbitalSpeed = Physics::calculateOrbitalVelocity(Physics::SUN_MASS, orbitDistance);

    // Set velocity tangent to orbit (in positive Y direction for clockwise orbit when viewed from above)
    sf::Vector2f planetVelocity(0.0f, static_cast<float>(orbitalSpeed));

    auto planet = std::make_unique<CelestialBody>(
        name,
        mass,
        radius,
        planetPosition,
        planetVelocity,
        color
    );

    // Scale visual radius for better visibility
    float baseVisualRadius = Physics::metersToPixels(radius);
    planet->setVisualRadius(std::max(3.0f, baseVisualRadius * visualScale));

    addBody(std::move(planet));
}

void SolarSystem::storeInitialConditions() {
    initialConditions_.clear();
    for (const auto& body : bodies_) {
        InitialCondition condition;
        condition.position = body->getPosition();
        condition.velocity = body->getVelocity();
        initialConditions_.push_back(condition);
    }
}

void SolarSystem::restoreInitialConditions() {
    if (initialConditions_.size() != bodies_.size()) return;

    for (size_t i = 0; i < bodies_.size(); ++i) {
        bodies_[i]->setPosition(initialConditions_[i].position);
        bodies_[i]->setVelocity(initialConditions_[i].velocity);
        bodies_[i]->resetForces();
    }
}
