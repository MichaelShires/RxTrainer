//
// Created by Michael Shires on 11/11/24.
//

#include "CoreElement.h"
#include "Constants.h"

const double sigma_a_U235 = 680.0;   // Example microscopic cross-section value in barns
const double sigma_a_Xe135 = 2.65e6; // Example value in barns

// Assuming numEnergyGroups is defined somewhere globally or accessible

CoreElement::CoreElement()
    : material(MaterialType::Vessel), temperature(300.0), reactivity(0.0), neutronPopulation(0.0) {
    // Initialize vectors with the correct size
    initializeVectors();
}

CoreElement::CoreElement(MaterialType material, double temperature)
    : material(material), temperature(temperature), reactivity(0.0), neutronPopulation(0.0) {
    // Initialize vectors with the correct size
    initializeVectors();

    // Initialize neutron population and neutron flux based on material type
    if (material == MaterialType::Fuel) {
        neutronPopulation = 1.0;
        for (int g = 0; g < numEnergyGroups; ++g) {
            neutronFlux[g] = 1.0; // Initial neutron flux
            Sigma_a[g] = 0.01;    // Example value
            Sigma_f[g] = 0.005;   // Example value
            Chi[g] = 1.0;         // All neutrons born in this group
            for (int gp = 0; gp < numEnergyGroups; ++gp) {
                Sigma_s[g][gp] = 0.002; // Example value
            }
        }
    } else {
        neutronPopulation = 0.0;
        for (int g = 0; g < numEnergyGroups; ++g) {
            neutronFlux[g] = 0.0;
            Sigma_a[g] = 0.0;
            Sigma_f[g] = 0.0;
            Chi[g] = 0.0;
            for (int gp = 0; gp < numEnergyGroups; ++gp) {
                Sigma_s[g][gp] = 0.0;
            }
        }
    }
}

void CoreElement::initializeVectors() {
    // Initialize vectors with the correct size
    neutronFlux.resize(numEnergyGroups, 0.0);
    Sigma_a.resize(numEnergyGroups, 0.0);
    Sigma_f.resize(numEnergyGroups, 0.0);
    Chi.resize(numEnergyGroups, 0.0);
    Sigma_s.resize(numEnergyGroups, std::vector<double>(numEnergyGroups, 0.0));
}

MaterialType CoreElement::getMaterial() const {
    return material;
}
double CoreElement::getTemperature() const {
    return temperature;
}

void CoreElement::setTemperature(double temperature) {
    this->temperature = temperature;
}

double CoreElement::getReactivity() const {
    return reactivity;
}

void CoreElement::setReactivity(double reactivity) {
    this->reactivity = reactivity;
}

double CoreElement::getNeutronPopulation() const {
    return neutronPopulation;
}

void CoreElement::setNeutronPopulation(double neutronPopulation) {
    this->neutronPopulation = neutronPopulation;
}

// Methods

void CoreElement::calculateReactivity(const std::vector<CoreElement*>& neighbors) {
    // Simplified reactivity calculation based on neighboring elements

    double reactivityEffect = 0.0;

    for (const auto& neighbor : neighbors) {
        if (neighbor->getMaterial() == MaterialType::Fuel) {
            reactivityEffect += 0.01; // positive reactivity from adjacent fuel
        } else if (neighbor->getMaterial() == MaterialType::ControlRod) {
            reactivityEffect -= 0.02; // negative reactivity from adjacent control rod
        }
        // Vessel material does not affect reactivity
    }

    // Tempeerature feedback (negative reactivity coefficient)
    double temperatureCoefficient = -0.0001; // Negative because higher temp reduces reactivity
    double temperatureReactivity = temperatureCoefficient * (temperature - 300.0); // 300K is nominal temperature

    // Total reactiivty is the sum of neigbhor effects and temperature feedback
    reactivity = reactivityEffect + temperatureReactivity;
}

void CoreElement::updateTemperature(double heatInput, double deltaTime) {
    // Update temperature based on heat input, material properties, and deltaTime

    // Simplified specific heat capacities (J/kg*K)
    double specificHeatCapacity = 0.0;
    double mass = 1.0; // Assume unit mass for simplicity

    switch (material) {
        case MaterialType::Fuel:
            specificHeatCapacity = 300.0;
        break;
        case MaterialType::ControlRod:
            specificHeatCapacity = 500.0;
        break;
        case MaterialType::Vessel:
            specificHeatCapacity = 450.0;
        break;
        default:
            specificHeatCapacity = 400.0;
        break;
    }

    // Temperature change: ΔT = (Q * deltaTime) / (m * c)
    double deltaT = (heatInput * deltaTime) / (mass * specificHeatCapacity);
    temperature += deltaT;
}

void CoreElement::setMaterial(MaterialType material) {
    this->material = material;

    // Adjust neutron population if necessary
    if (material == MaterialType::Fuel) {
        neutronPopulation = 1.0;
    } else {
        neutronPopulation = 0.0;
    }
}

double CoreElement::getNeutronFlux(int group) const {
    return neutronFlux[group];
}

void CoreElement::setNeutronFlux(int group, double flux) {
    neutronFlux[group] = flux;
}

double CoreElement::getSigmaA(int group) const {
    return Sigma_a[group];
}

void CoreElement::setSigmaA(int group, double sigmaA) {
    Sigma_a[group] = sigmaA;
}

double CoreElement::getSigmaF(int group) const {
    return Sigma_f[group];
}

void CoreElement::setSigmaF(int group, double sigmaF) {
    Sigma_f[group] = sigmaF;
}

double CoreElement::getChi(int group) const {
    return Chi[group];
}

double CoreElement::getSigmaA() const {
    const double T = this->getTemperature(); // Current temperature
    constexpr double T0 = 300.0;                 // Reference temperature (K)
    return Sigma_a_0 * sqrt(T0 / T);   // Negative temperature coefficient
}

void CoreElement::updateBurnup(double deltaTime) {
    int group = 0; // Assuming we're using group 0 for burnup calculations
    double phi = this->getNeutronFlux(group);
    double Sigma_f = this->getSigmaF(group); // Fission cross-section
    double fissionRate = Sigma_f * phi;

    // Constants
    double yield_Xe135 = 0.065; // Example value

    // Deplete U-235
    U235_concentration -= fissionRate * deltaTime;

    // Build up Xe-135
    Xe135_concentration += fissionRate * deltaTime * yield_Xe135;

    // Update cross-sections based on new concentrations
    // For example:
    Sigma_a_0 = calculateSigmaA0(U235_concentration, Xe135_concentration);
}

double CoreElement::getU235Concentration() const {
    return U235_concentration;
}

void CoreElement::setU235Concentration(double conc) {
    U235_concentration = conc;
}

double CoreElement::getXe135Concentration() const {
    return Xe135_concentration;
}

void CoreElement::setXe135Concentration(double conc) {
    Xe135_concentration = conc;
}

// Function to calculate base absorption cross-section
double CoreElement::calculateSigmaA0(double U235_conc, double Xe135_conc) {
    // Example calculation using macroscopic cross-section formula
    double Sigma_a_U235 = U235_conc * sigma_a_U235;   // Microscopic cross-section times concentration
    double Sigma_a_Xe135 = Xe135_conc * sigma_a_Xe135;

    return Sigma_a_U235 + Sigma_a_Xe135;
}

double CoreElement::getSigmaS(int fromGroup, int toGroup) const {
    return Sigma_s[fromGroup][toGroup];
}