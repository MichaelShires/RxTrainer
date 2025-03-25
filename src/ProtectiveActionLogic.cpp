//
// Created by Michael Shires on 11/11/24.
//

#include "ProtectiveActionLogic.h"

ProtectiveActionLogic::ProtectiveActionLogic()
    : scramInitiated(false) {}

void ProtectiveActionLogic::evaluateConditions(double coreTemperature, double coolantFlowRate) {
    // Thresholds for protective actions
    const double temperatureThreshold = 2000.0; // Maximum allowable temperature
    const double flowRateThreshold = 0.5;       // Minimum acceptable flow rate

    if (coreTemperature >= temperatureThreshold || coolantFlowRate <= flowRateThreshold) {
        scramInitiated = true;
    }
}

bool ProtectiveActionLogic::isScramInitiated() const {
    return scramInitiated;
}
