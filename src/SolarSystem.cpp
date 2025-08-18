#include "SolarSystem.h"
#include "Physics.h"
#include <iostream>
#include <cmath>

SolarSystem::SolarSystem() : paused_(false), timeScale_(1.0), is3DMode_(false) {
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

    // Add moons to planets
    addMoonsToEarth();
    addMoonsToMars();
    addMoonsToJupiter();
    addMoonsToSaturn();
    // Uranus and Neptune moons are quite small, so we'll skip them for now

    storeInitialConditions();

    std::cout << "Solar system initialized with " << bodies_.size() << " celestial bodies." << std::endl;
    std::cout << "Starting in 2D mode. Press M to toggle to 3D mode." << std::endl;
}void SolarSystem::update(double deltaTime) {
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

std::vector<CelestialBody> SolarSystem::getRawBodies() const {
    std::vector<CelestialBody> rawBodies;
    rawBodies.reserve(bodies_.size());

    for (const auto& body : bodies_) {
        rawBodies.push_back(*body);
    }

    return rawBodies;
}

void SolarSystem::set3DMode(bool enable) {
    is3DMode_ = enable;  // Actually set the mode flag!

    if (enable) {
        // Initialize 3D positions from 2D positions when switching to 3D
        for (auto& body : bodies_) {
            sf::Vector2f pos2D = body->getPosition();
            body->setPosition3D(Vector3f(pos2D.x, pos2D.y, 0.0f));

            sf::Vector2f vel2D = body->getVelocity();
            body->setVelocity3D(Vector3f(vel2D.x, vel2D.y, 0.0f));

            // Set previous position for Verlet integration
            body->setPreviousPosition3D(body->getPosition3D() - body->getVelocity3D() * 0.016f);
        }
    }
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
    if (is3DMode_) {
        // 3D physics update
        for (auto& body : bodies_) {
            body->update3D(getRawBodies(), static_cast<float>(deltaTime));
        }
    } else {
        // 2D physics update
        // Calculate gravitational forces between all bodies
        calculateGravitationalForces();

        // Update all bodies
        for (auto& body : bodies_) {
            body->update(deltaTime);
            body->resetForces();
        }
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

    std::cout << "Created Sun at center with visual radius 20.0 pixels" << std::endl;

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
    float finalVisualRadius = std::max(4.0f, baseVisualRadius * visualScale);
    planet->setVisualRadius(finalVisualRadius);

    // Debug output
    std::cout << "Created planet " << name
              << " at distance " << simDistance << " pixels"
              << " with visual radius " << finalVisualRadius << " pixels" << std::endl;

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

void SolarSystem::createMoon(const std::string& moonName, double mass, double radius,
                            const std::string& parentPlanetName, double orbitDistance,
                            double orbitVelocity, const sf::Color& color, float visualScale) {
    // Find the parent planet
    CelestialBody* parentPlanet = findBody(parentPlanetName);
    if (!parentPlanet) {
        std::cerr << "Warning: Could not find parent planet " << parentPlanetName
                  << " for moon " << moonName << std::endl;
        return;
    }

    // Calculate moon's position relative to parent planet
    sf::Vector2f parentPos = parentPlanet->getPosition();
    sf::Vector2f parentVel = parentPlanet->getVelocity();

    // Convert orbital distance to simulation coordinates
    float simDistance = Physics::metersToPixels(orbitDistance);

    // Place moon initially to the right of the planet
    sf::Vector2f moonPosition = parentPos + sf::Vector2f(simDistance, 0.0f);

    // Calculate orbital velocity around the parent planet
    double planetMass = parentPlanet->getMass();
    double moonOrbitalSpeed = Physics::calculateOrbitalVelocity(planetMass, orbitDistance);

    // Add parent planet's velocity plus moon's orbital velocity (perpendicular)
    sf::Vector2f moonVelocity = parentVel + sf::Vector2f(0.0f, static_cast<float>(moonOrbitalSpeed));

    auto moon = std::make_unique<CelestialBody>(
        moonName,
        mass,
        radius,
        moonPosition,
        moonVelocity,
        color
    );

    // Scale visual radius for better visibility (moons are usually very small)
    float baseVisualRadius = Physics::metersToPixels(radius);
    moon->setVisualRadius(std::max(2.0f, baseVisualRadius * visualScale));

    addBody(std::move(moon));
}

void SolarSystem::addMoonsToEarth() {
    // Luna (Earth's Moon)
    createMoon("Luna", 7.342e22, 1737.4e3, "Earth", 384400e3, 1.022e3,
               sf::Color::White, 8.0f);
}

void SolarSystem::addMoonsToMars() {
    // Phobos
    createMoon("Phobos", 1.0659e16, 11.1e3, "Mars", 9376e3, 2.138e3,
               sf::Color(139, 139, 139), 15.0f);

    // Deimos
    createMoon("Deimos", 1.4762e15, 6.2e3, "Mars", 23463e3, 1.351e3,
               sf::Color(105, 105, 105), 18.0f);
}

void SolarSystem::addMoonsToJupiter() {
    // Io
    createMoon("Io", 8.9319e22, 1821.6e3, "Jupiter", 421700e3, 17.334e3,
               sf::Color(255, 255, 0), 4.0f);

    // Europa
    createMoon("Europa", 4.7998e22, 1560.8e3, "Jupiter", 671034e3, 13.740e3,
               sf::Color(173, 216, 230), 4.5f);

    // Ganymede
    createMoon("Ganymede", 1.4819e23, 2634.1e3, "Jupiter", 1070412e3, 10.880e3,
               sf::Color(139, 119, 101), 3.0f);

    // Callisto
    createMoon("Callisto", 1.0759e23, 2410.3e3, "Jupiter", 1882709e3, 8.204e3,
               sf::Color(64, 64, 64), 3.2f);
}

void SolarSystem::addMoonsToSaturn() {
    // Titan
    createMoon("Titan", 1.3452e23, 2574e3, "Saturn", 1221830e3, 5.57e3,
               sf::Color(255, 165, 0), 3.5f);

    // Enceladus (smaller but interesting)
    createMoon("Enceladus", 1.08022e20, 252.1e3, "Saturn", 238020e3, 12.635e3,
               sf::Color::White, 12.0f);
}

void SolarSystem::addMoonsToUranus() {
    // Add major moons if desired - currently empty
}

void SolarSystem::addMoonsToNeptune() {
    // Add Triton if desired - currently empty
}
