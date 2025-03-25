//
// Created by Michael Shires on 11/11/24.
//

#ifndef COOLANTLOOP_H
#define COOLANTLOOP_H
#include <deque>

#include "CoolantChunk.h"


class CoolantLoop {
public:
    CoolantLoop(int chunkCount);

    void advanceLoop();
    void updateCoolantChunks();
    CoolantChunk& getUpperChunk();
    CoolantChunk& getLowerChunk();

    void setLeak(bool cond);

    int getChunkCount() const { return chunks.size(); }
    std::mutex& getMutex() const { return coolantMutex; }
    const std::deque<CoolantChunk>& getChunks() const { return chunks; }

private:
    bool hasLeak;
    std::deque<CoolantChunk> chunks;
    mutable std::mutex coolantMutex;
};



#endif //COOLANTLOOP_H
