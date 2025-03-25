//
// Created by Michael Shires on 11/11/24.
//


#include "CoolantChunk.h"

CoolantChunk::CoolantChunk(double temperature)
    : temperature(temperature) {}

double CoolantChunk::getTemperature() const {
    return temperature;
}

void CoolantChunk::setTemperature(double temp) {
    temperature = temp;
}

void CoolantChunk::absorbHeat(double heatEnergy) {
    // Update temperature based on absorbed heat

    double specificHeatCapacity = 4200.0; // J/kg*K for water
    double mass = 1.0; // Assume unit mass for simplicity

    double deltaT = heatEnergy / (mass * specificHeatCapacity);
    temperature += deltaT;
}

double CoolantChunk::getDensity() const {
    // Simplified linear model for density as a function of temperature
    return density0 - beta * (temperature - T0);
}

double CoolantChunk::getHeatCapacity() {
    // Simplified model or empirical data
    return 4182.0; // J/(kg·K) for water, adjust as needed
}

double CoolantChunk::getViscosity() {
    // Simplified model or empirical data
    return 0.001; // Pa·s, adjust as needed
}