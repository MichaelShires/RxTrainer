//
// Created by Michael Shires on 11/11/24.
//

#ifndef COREELEMENT_H
#define COREELEMENT_H
#include <vector>

enum class MaterialType {
    Vessel,
    Fuel,
    ControlRod
};

class CoreElement {
public:
    CoreElement();

    CoreElement(MaterialType material, double temperature);

    void initializeVectors();

    // Getters and Setters
    [[nodiscard]] MaterialType getMaterial() const;
    [[nodiscard]] double getTemperature() const;
    void setTemperature(double temperature);

    [[nodiscard]] double getReactivity() const;
    void setReactivity(double reactivity);

    [[nodiscard]] double getNeutronPopulation() const;
    void setNeutronPopulation(double neutronPopulation);

    // Methods
    void calculateReactivity(const std::vector<CoreElement*>& neighbors);
    void updateTemperature(double heatInput, double deltaTime);

    void setMaterial(MaterialType material);

    // Multi-group neutron flux
    [[nodiscard]] double getNeutronFlux(int group) const;
    void setNeutronFlux(int group, double flux);

    // Multi-group cross-sections
    [[nodiscard]] double getSigmaA(int group) const;
    void setSigmaA(int group, double sigmaA);

    [[nodiscard]] double getSigmaF(int group) const;
    void setSigmaF(int group, double sigmaF);

    [[nodiscard]] double getChi(int group) const;
    [[nodiscard]] double getSigmaA() const;

    void updateBurnup(double deltaTime);

    // Getters and setters for concentrations
    [[nodiscard]] double getU235Concentration() const;
    void setU235Concentration(double conc);

    [[nodiscard]] double getXe135Concentration() const;
    void setXe135Concentration(double conc);

    static double calculateSigmaA0(double U235_conc, double Xe135_conc);

    [[nodiscard]] double getSigmaS(int fromGroup, int toGroup) const;

private:
    MaterialType material;
    double temperature;
    double reactivity;
    double neutronPopulation;
    double Sigma_a_0{};

    double U235_concentration{};   // U-235 concentration
    double Xe135_concentration{};  // Xe-135 concentration (neutron poison)
    // Other isotopes as needed

    std::vector<double> neutronFlux;          // Neutron flux for each energy group
    std::vector<double> Sigma_a;              // Absorption cross-section per group
    std::vector<double> Sigma_f;              // Fission cross-section per group
    std::vector<double> Chi;                  // Fission spectrum per group
    std::vector<std::vector<double>> Sigma_s; // Scattering cross-section matrix

};



#endif //COREELEMENT_H
