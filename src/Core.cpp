//
// Created by Michael Shires on 11/11/24.
//

#include "Core.h"
#include <iostream>

Core::Core(int xSize, int ySize, int zSize)
    : xSize(xSize), ySize(ySize), zSize(zSize), controlRodInsertion(0.0) {
    elements.resize(xSize * ySize * zSize);
    initializeCore();
}

void Core::initializeCore() {
    for (int x = 0; x < xSize; ++x) {
        for (int y = 0; y < ySize; ++y) {
            for (int z = 0; z < zSize; ++z) {
                MaterialType material;

                if (x == 0 || x == xSize - 1 || y == 0 || y == ySize - 1 || z == 0 || z == zSize - 1) {
                    material = MaterialType::Vessel; // Outer layers are vessel material
                } else {
                    material = MaterialType::Fuel; // Inner elements are fuel
                }

                elements[index(x, y, z)] = CoreElement(material, 300.0);
            }
        }
    }
}

void Core::calculateCoreThermals(double deltaTime) {
    // Step 1: Calculate reactivity for each element
#pragma omp parallel for collapse(3) schedule(static)
    for (int x = 0; x < xSize; ++x) {
        for (int y = 0; y < ySize; ++y) {
            for (int z = 0; z < zSize; ++z) {
                CoreElement& element = elements[index(x, y, z)];

                if (element.getMaterial() != MaterialType::Vessel) {
                    auto neighbors = getNeighbors(x, y, z);
                    element.calculateReactivity(neighbors);
                }
            }
        }
    }

    // Step 2: Update neutron population and temperature
#pragma omp parallel for collapse(3) schedule(static)
    for (int x = 0; x < xSize; ++x) {
        for (int y = 0; y < ySize; ++y) {
            for (int z = 0; z < zSize; ++z) {
                CoreElement& element = elements[index(x, y, z)];

                if (element.getMaterial() == MaterialType::Fuel) {
                    double neutronPopulation = element.getNeutronPopulation();
                    double reactivity = element.getReactivity();

                    // Simplified neutron population update
                    double newNeutronPopulation = neutronPopulation * (1 + reactivity);
                    element.setNeutronPopulation(newNeutronPopulation);

                    // Heat generated is proportional to neutron population
                    double heatGenerated = newNeutronPopulation * 1000.0; // Arbitrary scaling

                    // Update temperature
                    element.updateTemperature(heatGenerated, deltaTime);
                }
            }
        }
    }
}

void Core::updateNeutronPopulation() {
    // Additional neutron population updates can be implemented here if needed
}

void Core::insertControlRods() {
    // Insert control rods to reduce reactivity

    // For simplicity, insert control rods in every other column
    for (int x = 1; x < xSize - 1; x += 2) {
        for (int y = 1; y < ySize - 1; y += 2) {
            for (int z = 1; z < zSize - 1; ++z) {
                CoreElement& element = elements[index(x, y, z)];
                if (element.getMaterial() == MaterialType::Fuel) {
                    element = CoreElement(MaterialType::ControlRod, element.getTemperature());
                }
            }
        }
    }
}

std::vector<CoreElement>& Core::getElements() {
    return elements;
}

void Core::setControlRodInsertion(double insertionDepth) {
    controlRodInsertion = insertionDepth;

    // Update the material of elements based on insertion depth
    // For simplicity, assume control rods insert from the top (z-direction)
    int maxInsertionLevel = static_cast<int>(insertionDepth * zSize);

    for (int x = 0; x < xSize; ++x) {
        for (int y = 0; y < ySize; ++y) {
            for (int z = zSize - 1; z >= zSize - maxInsertionLevel; --z) {
                CoreElement& element = elements[index(x, y, z)];
                if (element.getMaterial() == MaterialType::Fuel) {
                    element.setMaterial(MaterialType::ControlRod);
                }
            }
        }
    }

    // Reset elements above the insertion level back to fuel (if previously set to control rod)
    for (int x = 0; x < xSize; ++x) {
        for (int y = 0; y < ySize; ++y) {
            for (int z = 0; z < zSize - maxInsertionLevel; ++z) {
                CoreElement& element = elements[index(x, y, z)];
                if (element.getMaterial() == MaterialType::ControlRod) {
                    element.setMaterial(MaterialType::Fuel);
                }
            }
        }
    }
}

void Core::increaseReactivity(double delta) {
    // Increase the reactivity of all fuel elements
    for (auto& element : elements) {
        if (element.getMaterial() == MaterialType::Fuel) {
            double newReactivity = element.getReactivity() + delta;
            element.setReactivity(newReactivity);
        }
    }
}

void Core::calculateMultiGroupNeutronFlux(double deltaTime) {
    // Create copies of current fluxes for each group
    std::vector<std::vector<double>> newFluxes(numEnergyGroups, std::vector<double>(elements.size(), 0.0));

    // Spatial steps
    double dx = 1.0;
    double dy = 1.0;
    double dz = 1.0;

    // Loop over energy groups
    for (int g = 0; g < numEnergyGroups; ++g) {
        // Parameters for group g (define based on materials)
        double D_g = 1.0; // Diffusion coefficient for group g

        // Loop over all grid points
        for (int x = 1; x < xSize - 1; ++x) {
            for (int y = 1; y < ySize - 1; ++y) {
                for (int z = 1; z < zSize - 1; ++z) {
                    int idx = index(x, y, z);
                    CoreElement& element = elements[idx];

                    if (element.getMaterial() == MaterialType::Fuel) {
                        // Get neighboring fluxes for group g
                        double phi_center = element.getNeutronFlux(g);
                        double phi_x_plus = elements[index(x + 1, y, z)].getNeutronFlux(g);
                        double phi_x_minus = elements[index(x - 1, y, z)].getNeutronFlux(g);
                        double phi_y_plus = elements[index(x, y + 1, z)].getNeutronFlux(g);
                        double phi_y_minus = elements[index(x, y - 1, z)].getNeutronFlux(g);
                        double phi_z_plus = elements[index(x, y, z + 1)].getNeutronFlux(g);
                        double phi_z_minus = elements[index(x, y, z - 1)].getNeutronFlux(g);

                        // Laplacian for group g
                        double laplacian = (phi_x_plus - 2 * phi_center + phi_x_minus) / (dx * dx)
                                         + (phi_y_plus - 2 * phi_center + phi_y_minus) / (dy * dy)
                                         + (phi_z_plus - 2 * phi_center + phi_z_minus) / (dz * dz);

                        // Absorption term
                        double Sigma_a_g = element.getSigmaA(g);
                        double absorption = -Sigma_a_g * phi_center;

                        // Scattering term
                        double scattering = 0.0;
                        for (int g_prime = 0; g_prime < numEnergyGroups; ++g_prime) {
                            if (g_prime != g) {
                                double Sigma_s_gp_to_g = element.getSigmaS(g_prime, g);
                                double phi_gp = element.getNeutronFlux(g_prime);
                                scattering += Sigma_s_gp_to_g * phi_gp;
                            }
                        }

                        // Fission source term
                        double fission_source = 0.0;
                        for (int g_prime = 0; g_prime < numEnergyGroups; ++g_prime) {
                            double nuSigma_f_gp = element.getSigmaF(g_prime); // Assuming nu included
                            double phi_gp = element.getNeutronFlux(g_prime);
                            fission_source += element.getChi(g) * nuSigma_f_gp * phi_gp;
                        }

                        // Right-hand side for group g
                        double rhs = D_g * laplacian + absorption + scattering + fission_source;

                        // Update flux
                        newFluxes[g][idx] = phi_center + deltaTime * rhs;
                    } else {
                        newFluxes[g][idx] = 0.0;
                    }
                }
            }
        }
    }

    // Update fluxes for all groups
    for (int g = 0; g < numEnergyGroups; ++g) {
        for (size_t i = 0; i < elements.size(); ++i) {
            elements[i].setNeutronFlux(g, newFluxes[g][i]);
        }
    }
}

void Core::updateFuelBurnup(double delta_time) {
    for (auto& element : elements) {
        if (element.getMaterial() == MaterialType::Fuel) {
            element.updateBurnup(delta_time);
        }
    }
}

std::vector<CoreElement*> Core::getNeighbors(int x, int y, int z) {
    std::vector<CoreElement*> neighbors;

    const int dx[] = { -1, 1, 0, 0, 0, 0 };
    const int dy[] = { 0, 0, -1, 1, 0, 0 };
    const int dz[] = { 0, 0, 0, 0, -1, 1 };

    for (int i = 0; i < 6; ++i) {
        int nx = x + dx[i];
        int ny = y + dy[i];
        int nz = z + dz[i];

        if (nx >= 0 && nx < xSize && ny >= 0 && ny < ySize && nz >= 0 && nz < zSize) {
            neighbors.push_back(&elements[index(nx, ny, nz)]);
        }
    }

    return neighbors;
}