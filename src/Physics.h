#pragma once
#include <SFML/Graphics.hpp>

/**
 * Physics utility class containing constants and helper functions
 * for astronomical calculations and unit conversions
 */
class Physics {
public:
    // Physical constants
    static constexpr double G = 6.67430e-11;           // Gravitational constant (m^3 kg^-1 s^-2)
    static constexpr double AU = 1.496e11;             // Astronomical Unit in meters
    static constexpr double EARTH_MASS = 5.972e24;     // Earth mass in kg
    static constexpr double SUN_MASS = 1.989e30;       // Sun mass in kg
    static constexpr double SECONDS_PER_DAY = 86400.0; // Seconds in a day
    static constexpr double SECONDS_PER_YEAR = 31557600.0; // Seconds in a year

    // Simulation scaling factors
    static constexpr double DISTANCE_SCALE = 1e-9;     // Scale factor for distances (1 pixel = 1e9 meters)
    static constexpr double TIME_SCALE = 86400.0;      // Time scale factor (1 simulation second = 1 day)

    /**
     * Calculate gravitational force between two bodies
     * @param m1 Mass of first body (kg)
     * @param m2 Mass of second body (kg)
     * @param r Distance between bodies (meters)
     * @return Force magnitude (N)
     */
    static double calculateGravitationalForce(double m1, double m2, double r);

    /**
     * Calculate gravitational force vector between two positions
     * @param m1 Mass of first body (kg)
     * @param pos1 Position of first body
     * @param m2 Mass of second body (kg)
     * @param pos2 Position of second body
     * @return Force vector on body 1 due to body 2
     */
    static sf::Vector2f calculateGravitationalForceVector(double m1, const sf::Vector2f& pos1,
                                                         double m2, const sf::Vector2f& pos2);

    /**
     * Convert real-world distance (meters) to simulation pixels
     * @param meters Distance in meters
     * @return Distance in simulation pixels
     */
    static float metersToPixels(double meters);

    /**
     * Convert simulation pixels to real-world distance (meters)
     * @param pixels Distance in simulation pixels
     * @return Distance in meters
     */
    static double pixelsToMeters(float pixels);

    /**
     * Calculate orbital velocity for circular orbit
     * @param centralMass Mass of central body (kg)
     * @param distance Orbital distance (meters)
     * @return Orbital velocity (m/s)
     */
    static double calculateOrbitalVelocity(double centralMass, double distance);

    /**
     * Calculate escape velocity from a body
     * @param mass Mass of the body (kg)
     * @param radius Distance from center (meters)
     * @return Escape velocity (m/s)
     */
    static double calculateEscapeVelocity(double mass, double radius);

    /**
     * Calculate distance between two points
     * @param p1 First point
     * @param p2 Second point
     * @return Distance
     */
    static float distance(const sf::Vector2f& p1, const sf::Vector2f& p2);

    /**
     * Normalize a vector
     * @param vec Vector to normalize
     * @return Normalized vector
     */
    static sf::Vector2f normalize(const sf::Vector2f& vec);

    /**
     * Get magnitude of a vector
     * @param vec Vector
     * @return Magnitude
     */
    static float magnitude(const sf::Vector2f& vec);
};
