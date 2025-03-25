//
// Created by Michael Shires on 11/11/24.
//

#ifndef COOLANTCHUNK_H
#define COOLANTCHUNK_H



class CoolantChunk {
public:
    explicit CoolantChunk(double temperature);

    //Getters and Setters
    [[nodiscard]] double getTemperature() const;
    void setTemperature(double temperature);

    //Methods
    void absorbHeat(double heatEnergy);

    [[nodiscard]] double getDensity() const;
    [[nodiscard]] static double getHeatCapacity() ;
    [[nodiscard]] static double getViscosity() ;

private:
    double temperature;

    // Constants for property calculations
    static constexpr double density0 = 1000.0; // kg/m^3 at T0
    static constexpr double beta = 0.3;        // Coefficient for density change
    static constexpr double T0 = 300.0;        // Reference temperature
};



#endif //COOLANTCHUNK_H
