//
// Created by Michael Shires on 11/11/24.
//

#ifndef MAINSIMULATION_H
#define MAINSIMULATION_H

class Core;

#include <thread>

#include "CoolantLoop.h"
#include "ProtectiveActionLogic.h"
#include "Visualization.h"


class MainSimulation {
public:
    MainSimulation(Core &core, CoolantLoop &coolantLoop, std::atomic<bool> &running);
    ~MainSimulation();

    void runSimulation();

private:
    Core& core;
    CoolantLoop& coolantLoop;
    ProtectiveActionLogic protectiveLogic;
    double deltaTime{}; // Time step in seconds

    // User input thread
    std::thread inputThread;
    std::atomic<bool>& running; // Flag to control the simulation loop
    std::atomic<bool> paused;
    std::mutex ioMutex;


    void iterate();

    void displayStatus() const;

    void exchangeHeat();

    void handleUserInput();
    void updateDisplay();

    double calculateHeatTransferCoefficient(double density, double heatCapacity) const;

    void evaluateProtection();

    // New methods for user interactions
    void adjustControlRods(double insertionDepth) const;
    void initiateCasualty(const std::string& casualtyType);

    [[nodiscard]] double getMaxCoreTemperature() const;

};



#endif //MAINSIMULATION_H
