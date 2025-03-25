//
// Created by Michael Shires on 11/11/24.
//

// CoolantLoop.cpp

#include "CoolantLoop.h"

CoolantLoop::CoolantLoop(int chunkCount)
    : hasLeak(false) {
    // Initialize coolant chunks with initial temperature
    for (int i = 0; i < chunkCount; ++i) {
        chunks.emplace_back(300.0); // Starting temperature 300K
    }
}

void CoolantLoop::advanceLoop() {
    if (hasLeak && !chunks.empty()) {
        // Remove a chunk to simulate coolant loss
        chunks.pop_back();
    }

    // Simulate coolant movement by rotating the chunks
    if (!chunks.empty()) {
        CoolantChunk frontChunk = chunks.front();
        chunks.pop_front();
        chunks.push_back(frontChunk);
    }
}

void CoolantLoop::updateCoolantChunks() {
    // Simulate heat exchange in the steam generator
    double heatLossPerChunk = 5000.0; // Arbitrary value representing heat given to the secondary loop

    for (auto& chunk : chunks) {
        chunk.absorbHeat(-heatLossPerChunk); // Negative heat to represent cooling
    }
}

CoolantChunk& CoolantLoop::getUpperChunk() {
    // Assuming upper chunk is at index chunks.size() / 2
    std::size_t index = chunks.size() / 2;
    return chunks[index];
}

CoolantChunk& CoolantLoop::getLowerChunk() {
    // Assuming lower chunk is at index 0
    return chunks.front();
}

void CoolantLoop::setLeak(bool cond) {
    hasLeak = cond;
}
