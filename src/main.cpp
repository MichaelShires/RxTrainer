#include "MainSimulation.h"
#include "Visualization.h"
#include "Core.h"
#include <thread>

int main() {
    // Create core and coolant loop
    Core core(10, 10, 10);
    CoolantLoop coolantLoop(100);

    // Atomic flag to control running state
    std::atomic<bool> running(true);

    // Create the visualization object
    Visualization visualization(core, coolantLoop, running);

    // Start the simulation in a separate thread
    MainSimulation simulation(core, coolantLoop, running);
    std::thread simulationThread(&MainSimulation::runSimulation, &simulation);

    // Start the visualization on the main thread
    visualization.start(); // This will block until the window is closed

    // Wait for the simulation to complete
    simulationThread.join();

    return 0;
}