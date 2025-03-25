//
// Created by Michael Shires on 11/11/24.
//

// MainSimulation.cpp

#include "MainSimulation.h"
#include <iostream>
#include <chrono>
#include <thread>
#include "Core.h"

MainSimulation::MainSimulation(Core& core, CoolantLoop& coolantLoop, std::atomic<bool>& running)
    : core(core),
      coolantLoop(coolantLoop),
      running(running),
      paused(false) {
    // Start the input thread
    inputThread = std::thread(&MainSimulation::handleUserInput, this);
}


MainSimulation::~MainSimulation() {
    if (inputThread.joinable()) {
        inputThread.join();
    }
}


void MainSimulation::runSimulation() {
    // Target iteration time in milliseconds
    const double targetIterationTime = 33.0; // For ~30 FPS

    while (running.load()) {
        if (paused.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        const double minDeltaTime = 0.01; // 10 ms
        const double maxDeltaTime = 0.05; // 50 ms

        auto startTime = std::chrono::high_resolution_clock::now();


        iterate();
        updateDisplay();

        auto endTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsedTime = endTime - startTime;

        // Update deltaTime based on actual elapsed time
        deltaTime = elapsedTime.count(); // deltaTime in seconds

        // After updating deltaTime
        if (deltaTime < minDeltaTime) {
            deltaTime = minDeltaTime;
        } else if (deltaTime > maxDeltaTime) {
            deltaTime = maxDeltaTime;
        }

        // Sleep if necessary to maintain target iteration time
        double sleepTime = targetIterationTime / 1000.0 - deltaTime; // Convert target to seconds

        if (sleepTime > 0) {
            std::this_thread::sleep_for(std::chrono::duration<double>(sleepTime));
        } else {
            std::cout << "Warning: Simulation iteration took longer than target time by "
                      << -sleepTime * 1000.0 << " ms." << std::endl;
        }
    }
}

void MainSimulation::iterate() {
    {
        std::lock_guard<std::mutex> lock(core.getMutex());
        // Calculate neutron flux
        core.calculateMultiGroupNeutronFlux(deltaTime);

        // Update burnup (fuel depletion)
        core.updateFuelBurnup(deltaTime);

        // Update core thermals (temperature calculations)
        core.calculateCoreThermals(deltaTime);
    }

    {
        std::lock_guard<std::mutex> lock(coolantLoop.getMutex());
        // Advance coolant loop and update chunks
        coolantLoop.advanceLoop();
        coolantLoop.updateCoolantChunks();
    }


    // Exchange heat between core and coolant
    exchangeHeat();

    // Evaluate protective actions
    evaluateProtection();

    double maxCoreTemp = getMaxCoreTemperature();
    double upperCoolantTemp = coolantLoop.getUpperChunk().getTemperature();
    double lowerCoolantTemp = coolantLoop.getLowerChunk().getTemperature();

    {
        std::lock_guard<std::mutex> lock(ioMutex);
        //std::cout << "Iteration: Max Core Temp = " << maxCoreTemp
        //          << ", Upper Coolant Temp = " << upperCoolantTemp
        //          << ", Lower Coolant Temp = " << lowerCoolantTemp << std::endl;
    }
}

void MainSimulation::displayStatus() const {
    double maxTemperature = getMaxCoreTemperature();
    double upperCoolantTemp = coolantLoop.getUpperChunk().getTemperature();
    double lowerCoolantTemp = coolantLoop.getLowerChunk().getTemperature();

    std::cout << "\nSimulation Status:\n"
              << " - Max Core Temperature: " << maxTemperature << " K\n"
              << " - Upper Coolant Temperature: " << upperCoolantTemp << " K\n"
              << " - Lower Coolant Temperature: " << lowerCoolantTemp << " K\n"
              << " - Control Rod Insertion: " << (core.getControlRodInsertion() * 100) << "%\n"
              << " - Coolant Chunks: " << coolantLoop.getChunkCount() << "\n";
}

// MainSimulation.cpp

void MainSimulation::exchangeHeat() {
    // Simplified heat exchange between core and coolant
    double totalHeatGenerated = 0.0;

    auto& elements = core.getElements();
    int xSize = core.getXSize();
    int ySize = core.getYSize();
    int zSize = core.getZSize();

    // Accumulate total heat generated
#pragma omp parallel for collapse(3) reduction(+:totalHeatGenerated) schedule(static)
    for (int x = 0; x < xSize; ++x) {
        for (int y = 0; y < ySize; ++y) {
            for (int z = 0; z < zSize; ++z) {
                CoreElement& element = elements[core.index(x, y, z)];
                if (element.getMaterial() == MaterialType::Fuel) {
                    double neutronPopulation = element.getNeutronPopulation();
                    double heatGenerated = neutronPopulation * 1000.0; // Scaling factor
                    totalHeatGenerated += heatGenerated;

                    // Cool the fuel element
                    double heatRemoved = heatGenerated * 0.5; // Half the heat removed by coolant
                    element.updateTemperature(-heatRemoved, deltaTime);
                }
            }
        }
    }

    // Transfer heat to coolant chunks
    double totalHeatTransferred = totalHeatGenerated * 0.5; // Total heat transferred to coolant
    double heatPerChunk = totalHeatTransferred / 2.0;       // Split between upper and lower chunks

    coolantLoop.getUpperChunk().absorbHeat(heatPerChunk);
    coolantLoop.getLowerChunk().absorbHeat(heatPerChunk);

    // No need for further temperature updates here
}

double MainSimulation::calculateHeatTransferCoefficient(double density, double heatCapacity) const {
    // Implement your calculation here
    // For example:
    return 1000.0; // Placeholder value
}


void MainSimulation::evaluateProtection() {
    // Get maximum core temperature
    double maxCoreTemperature = getMaxCoreTemperature();

    // Simulate coolant flow rate (for this example, assume constant)
    double coolantFlowRate = 1.0;

    // Evaluate protective actions
    protectiveLogic.evaluateConditions(maxCoreTemperature, coolantFlowRate);

    if (protectiveLogic.isScramInitiated()) {
        std::cout << "Scram initiated due to unsafe conditions!" << std::endl;
        core.insertControlRods();
    }
}

void MainSimulation::adjustControlRods(double insertionDepth) const {
    if (insertionDepth < 0.0 || insertionDepth > 1.0) {
        std::cout << "Insertion depth must be between 0.0 and 1.0.\n";
        return;
    }

    // Pass the insertion depth to the core
    core.setControlRodInsertion(insertionDepth);
    std::cout << "Control rods adjusted to " << (insertionDepth * 100) << "% insertion.\n";
}

void MainSimulation::initiateCasualty(const std::string& casualtyType) {
    if (casualtyType == "leak") {
        // Simulate a coolant leak
        coolantLoop.setLeak(true);
        std::cout << "Coolant leak initiated.\n";
    } else if (casualtyType == "power surge") {
        // Simulate a sudden increase in reactivity
        core.increaseReactivity(0.1); // Increase by 10%
        std::cout << "Power surge initiated.\n";
    } else {
        std::cout << "Unknown casualty type.\n";
    }
}

double MainSimulation::getMaxCoreTemperature() const {
    double maxTemperature = 0.0;

    const auto& elements = core.getElements();

#pragma omp parallel for reduction(max:maxTemperature)
    for (size_t i = 0; i < elements.size(); ++i) {
        double temp = elements[i].getTemperature();
        if (temp > maxTemperature) {
            maxTemperature = temp;
        }
    }

    return maxTemperature;
}

void MainSimulation::handleUserInput() {
    try {
        while (running.load()) {
            std::string command;

            // Lock mutex only when writing to std::cout
            {
                std::lock_guard<std::mutex> lock(ioMutex);
                std::cout << "\nEnter command (type 'help' for options): ";
                std::cout.flush(); // Ensure prompt is displayed immediately
            }

            // Do not hold the mutex while waiting for input
            std::getline(std::cin, command);

            // Process the command
            if (command == "help") {
                std::lock_guard<std::mutex> lock(ioMutex);
                std::cout << "Available commands:\n"
                          << " - adjust rods [depth]: Adjust control rod insertion depth (0.0 to 1.0)\n"
                          << " - initiate casualty [type]: Initiate a casualty ('leak', 'power surge')\n"
                          << " - exit: Stop the simulation\n";
            } else if (command.find("adjust rods") == 0) {
                // Extract depth value
                double depth = 0.0;
                try {
                    depth = std::stod(command.substr(12));
                    adjustControlRods(depth);
                } catch (...) {
                    std::lock_guard<std::mutex> lock(ioMutex);
                    std::cout << "Invalid depth value.\n";
                }
            } else if (command.find("initiate casualty") == 0) {
                // Extract casualty type
                std::string casualtyType = command.substr(18);
                initiateCasualty(casualtyType);
            } else if (command == "exit") {
                running.store(false);
            } else if (command == "pause") {
                paused.store(true);
                std::lock_guard<std::mutex> lock(ioMutex);
                std::cout << "Simulation paused.\n";
            } else if (command == "resume") {
                paused.store(false);
                std::lock_guard<std::mutex> lock(ioMutex);
                std::cout << "Simulation resumed.\n";
            } else if (command == "status") {
                displayStatus();
            } else {
                std::lock_guard<std::mutex> lock(ioMutex);
                std::cout << "Unknown command. Type 'help' for options.\n";
            }

            // Small delay to prevent busy-waiting
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception in handleUserInput: " << e.what() << std::endl;
        running.store(false);
    } catch (...) {
        std::cerr << "Unknown exception in handleUserInput." << std::endl;
        running.store(false);
    }
}

void MainSimulation::updateDisplay() {
    // For now, output key parameters to the console
    std::lock_guard<std::mutex> lock(ioMutex);
    double maxTemperature = getMaxCoreTemperature();
    double upperCoolantTemp = coolantLoop.getUpperChunk().getTemperature();
    double lowerCoolantTemp = coolantLoop.getLowerChunk().getTemperature();

    //std::cout << "Max Core Temperature: " << maxTemperature << " K"
    //          << ", Upper Coolant Temp: " << upperCoolantTemp << " K"
    //          << ", Lower Coolant Temp: " << lowerCoolantTemp << " K" << std::endl;
}