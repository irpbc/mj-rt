//
// Created by Ivan Rašić on 4.5.18..
//

#ifndef MJ_RT_HEAP_H
#define MJ_RT_HEAP_H

#include "../statepoint_utils/statepoint.h"

struct ObjectDescriptor {
    int32_t objectSize;
    int32_t numRefFields;
    int32_t refFieldOffsets[];
};

enum class Status : uint8_t {
    UNSCANED, SCANING, LIVE
};

struct ObjectBase {
    ObjectDescriptor* descriptor;
    ObjectBase* forward;
    Status status;

    inline ObjectBase* getRefFieldValue(int32_t offset) {
        return *(ObjectBase**)(reinterpret_cast<uint8_t*>(this) + offset);
    }

    inline void setRefFieldValue(int32_t offset, ObjectBase* child) {
        *(ObjectBase**)(reinterpret_cast<uint8_t*>(this) + offset) = child;
    }
};

struct Heap {
    uint8_t* heapStart;
    uint8_t* heapEnd;
    uint8_t* heapMax;

    StatepointTable* spTable;

    void initialize(uint32_t bytes, uint8_t* LLVM_StackMaps);

    ObjectBase* allocateObject(ObjectDescriptor* meta, void* compiledCodeFrameAddress);

    bool collectGarbage(void* compiledCodeFrameAddress);

    void findLiveObjects(void* compiledCodeFrameAddress);

    uint32_t computeForwardReferences();

    void updateReferences(void* compiledCodeFrameAddress);

    void relocateLiveObjects();

    void updateRefsOnHeap();

    void updateRefsOnStack(void* compiledCodeFrameAddress);
};


#endif //MJ_RT_HEAP_H
