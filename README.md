# Solar System Gravity Simulator

A real-time gravity simulation of our solar system built with C++ and SFML. Experience the beauty of orbital mechanics as planets orbit around the Sun with realistic gravitational forces.

## Features

- **Realistic Physics**: Uses Newton's law of universal gravitation with proper orbital mechanics
- **Visual Solar System**: All 8 planets with realistic relative masses and orbital distances
- **Interactive Controls**: Pan, zoom, and control simulation speed
- **Visual Effects**: Orbital trails, planet labels, and velocity vectors
- **Real-time Simulation**: Smooth 60 FPS rendering with stable physics integration

## Screenshots

The simulator shows the Sun at the center with all planets in their respective orbits. Colorful trails show the orbital paths, and labels identify each celestial body.

## Controls

### Camera Controls
- **WASD / Arrow Keys**: Move camera
- **Q/E**: Zoom out/in
- **Mouse wheel**: Zoom
- **Left click + drag**: Pan camera
- **0**: Reset camera to origin
- **C**: Center on Sun

### Simulation Controls
- **Space**: Pause/Resume simulation
- **R**: Reset to initial conditions
- **+/-**: Increase/Decrease time scale

### Visual Options
- **T**: Toggle orbital trails
- **L**: Toggle planet labels
- **G**: Toggle grid
- **V**: Toggle velocity vectors

### Other
- **H**: Show help in console
- **ESC**: Exit simulation

## Requirements

- **CMake** 3.21 or higher
- **C++17** compatible compiler
- **Git** (for downloading SFML automatically)

### Platform-specific Requirements

#### macOS
No additional requirements - CMake will handle SFML automatically.

#### Linux (Ubuntu/Debian)
```bash
sudo apt update
sudo apt install \
    libxrandr-dev \
    libxcursor-dev \
    libxi-dev \
    libudev-dev \
    libfreetype-dev \
    libflac-dev \
    libvorbis-dev \
    libgl1-mesa-dev \
    libegl1-mesa-dev
```

#### Windows
CMake and Visual Studio with C++ support. SFML will be downloaded automatically.

## Building and Running

1. **Clone the repository:**
   ```bash
   git clone <repository-url>
   cd gravity-simulator
   ```

2. **Build with CMake:**
   ```bash
   cmake -B build -DCMAKE_BUILD_TYPE=Release
   cmake --build build --config Release
   ```

3. **Run the simulation:**
   ```bash
   ./build/GravitySimulator      # Linux/macOS
   ./build/Release/GravitySimulator.exe  # Windows
   ```

## Technical Details

### Physics Implementation
- Uses Newton's law of universal gravitation: F = G × m₁ × m₂ / r²
- Verlet integration for numerical stability
- Real astronomical data for planetary masses and orbital distances
- Adaptive time scaling for comfortable viewing speeds

### Rendering
- SFML 2.6 for cross-platform graphics
- Efficient trail rendering with fade effects
- Adaptive visual scaling based on zoom level
- Real-time UI displaying simulation statistics

### Architecture
- **CelestialBody**: Individual planets and the Sun with physics properties
- **SolarSystem**: Manages all bodies and physics calculations
- **Renderer**: Handles all visual rendering and camera controls
- **InputHandler**: Processes user input and controls
- **Physics**: Utility functions for calculations and constants

## Customization

You can easily modify the simulation by editing `SolarSystem.cpp`:

- Add new celestial bodies (moons, asteroids, comets)
- Modify planetary properties (mass, size, orbital distance)
- Adjust simulation parameters (time scale, gravitational constant)
- Add new visual effects

## Known Limitations

- Simplified to 2D orbital plane (no orbital inclinations)
- Planets don't affect each other gravitationally (only the Sun's gravity matters)
- No relativistic effects
- Circular orbits assumed for initial conditions

## Contributing

Feel free to contribute by:
- Adding new features (3D visualization, more celestial bodies)
- Improving physics accuracy (planet-planet interactions, elliptical orbits)
- Enhancing visuals (better graphics, particle effects)
- Optimizing performance

## License

This project is open source. See LICENSE file for details.

## Acknowledgments

- SFML development team for the excellent multimedia library
- NASA for providing accurate astronomical data
- Physics simulation techniques from numerical methods literature
