//
// Created by Michael Shires on 11/11/24.
//

#ifndef PROTECTIVEACTIONLOGIC_H
#define PROTECTIVEACTIONLOGIC_H



class ProtectiveActionLogic {
public:
    ProtectiveActionLogic();

    void evaluateConditions(double coreTemperature, double coolantFlowRate);
    bool isScramInitiated() const;

private:
    bool scramInitiated;
    double responseTime{}; // in milliseconds
};



#endif //PROTECTIVEACTIONLOGIC_H
