//
// Created by Michael Shires on 11/11/24.
//

#ifndef CORE_H
#define CORE_H
#include <vector>

#include "CoreElement.h"

class Core {
public:
    Core(int xSize, int ySize, int zSize);

    void initializeCore();
    void calculateCoreThermals(double deltaTime);
    void updateNeutronPopulation();
    void insertControlRods();

    // Getters
    [[nodiscard]] const std::vector<CoreElement>& getElements() const { return elements; }
    std::vector<CoreElement>& getElements();
    // Add methods to map 3D indices to 1D
    int index(int x, int y, int z) const {
        return x * ySize * zSize + y * zSize + z;
    }

    [[nodiscard]] int getXSize() const { return xSize; }
    [[nodiscard]] int getYSize() const { return ySize; }
    [[nodiscard]] int getZSize() const { return zSize; }

    void setControlRodInsertion(double insertion_depth);

    void increaseReactivity(double delta);

    double getControlRodInsertion() const { return controlRodInsertion; }

    std::mutex& getMutex() const { return coreMutex; }

    void calculateMultiGroupNeutronFlux(double deltaTime);

    void updateFuelBurnup(double delta_time);

private:
    int xSize, ySize, zSize, numEnergyGroups;
    std::vector<CoreElement> elements;
    double controlRodInsertion; // 0.0 to 1.0
    mutable std::mutex coreMutex;

    // Helper functions
    std::vector<CoreElement*> getNeighbors(int x, int y, int z);

};



#endif //CORE_H
