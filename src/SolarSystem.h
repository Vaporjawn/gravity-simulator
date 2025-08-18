#pragma once
#include "CelestialBody.h"
#include <vector>
#include <memory>

/**
 * Manages the solar system simulation including all celestial bodies
 * and their interactions
 */
class SolarSystem {
public:
    SolarSystem();
    ~SolarSystem() = default;

    // Initialize the solar system with realistic data
    void initialize();

    // Update physics for all bodies
    void update(double deltaTime);

    // Get all bodies for rendering
    const std::vector<std::unique_ptr<CelestialBody>>& getBodies() const { return bodies_; }

    // Add a new celestial body
    void addBody(std::unique_ptr<CelestialBody> body);

    // Remove all bodies
    void clear();

    // Get body by index
    CelestialBody* getBody(size_t index);
    const CelestialBody* getBody(size_t index) const;

    // Get number of bodies
    size_t getBodyCount() const { return bodies_.size(); }

    // Find body by name
    CelestialBody* findBody(const std::string& name);

    // Get the central body (usually the Sun)
    CelestialBody* getCentralBody() { return bodies_.empty() ? nullptr : bodies_[0].get(); }

    // Simulation control
    void pause() { paused_ = true; }
    void resume() { paused_ = false; }
    void togglePause() { paused_ = !paused_; }
    bool isPaused() const { return paused_; }

    void setTimeScale(double timeScale) { timeScale_ = timeScale; }
    double getTimeScale() const { return timeScale_; }

    // 3D simulation mode
    void set3DMode(bool enable3D);
    bool is3DMode() const { return is3DMode_; }
    void toggle3DMode() { is3DMode_ = !is3DMode_; }

    // Get raw bodies vector for 3D physics calculations
    std::vector<CelestialBody> getRawBodies() const;

    // Get simulation statistics
    double getTotalEnergy() const;
    sf::Vector2f getCenterOfMass() const;

    // Reset to initial conditions
    void reset();

private:
    std::vector<std::unique_ptr<CelestialBody>> bodies_;
    bool paused_;
    double timeScale_; // Speed multiplier for simulation time
    bool is3DMode_;    // Whether to use 3D simulation mode

    // Physics integration method
    void updatePhysics(double deltaTime);

    // Calculate forces between all bodies
    void calculateGravitationalForces();

    // Helper methods for initialization
    void createSun();
    void createPlanet(const std::string& name, double mass, double radius,
                     double orbitDistance, double orbitVelocity,
                     const sf::Color& color, float visualScale = 1.0f);

    // Moon creation methods
    void createMoon(const std::string& moonName, double mass, double radius,
                   const std::string& parentPlanetName, double orbitDistance,
                   double orbitVelocity, const sf::Color& color, float visualScale = 1.0f);
    void addMoonsToEarth();
    void addMoonsToMars();
    void addMoonsToJupiter();
    void addMoonsToSaturn();
    void addMoonsToUranus();
    void addMoonsToNeptune();

    // Store initial conditions for reset functionality
    void storeInitialConditions();
    void restoreInitialConditions();

    struct InitialCondition {
        sf::Vector2f position;
        sf::Vector2f velocity;
    };
    std::vector<InitialCondition> initialConditions_;
};
