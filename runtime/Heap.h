//
// Created by Ivan Rašić on 4.5.18..
//

#ifndef MJ_RT_HEAP_H
#define MJ_RT_HEAP_H

#include "../statepoint_utils/statepoint.h"

enum class TypeKind : uint8_t {
    OBJECT = 0x0,
    ARRAY_OF_PRIMITIVES = 0x1,
    ARRAY_OF_REFERENCES = 0x2,

    ANY_ARRAY = (uint8_t)ARRAY_OF_PRIMITIVES | (uint8_t)ARRAY_OF_REFERENCES
};

inline int operator &(TypeKind left, TypeKind right) {
    return (uint8_t)left & (uint8_t)right;
}

struct ObjectDescriptor {
    TypeKind typeKind;
    uint8_t arrayElemSize;
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
        return *(ObjectBase**)((uint8_t*)this + offset);
    }

    inline void setRefFieldValue(int32_t offset, ObjectBase* child) {
        *(ObjectBase**)((uint8_t*)this + offset) = child;
    }
};

/**
 * IMPORTANT: Fields of ObjectBase are copied over because
 * inheritance or nesting produces different alignment
 */
struct ArrayBase {
    // ----- ObjectBase ---------- //
    ObjectDescriptor* descriptor;
    ObjectBase* forward;
    Status status;
    // ----- End ObjectBase ------ //

    uint32_t length;
    char elems[];

    inline ObjectBase* firstArrayElemenet() {
        return (ObjectBase*)((uint8_t*)this + sizeof(ArrayBase));
    }
};

struct Heap {
    uint8_t* heapStart;
    uint8_t* heapEnd;
    uint8_t* heapMax;

    StatepointTable* spTable;

    void initialize(uint32_t bytes, uint8_t* LLVM_StackMaps);

    ObjectBase* allocateObject(ObjectDescriptor* meta, void* compiledCodeFrameAddress);

    ObjectBase* allocateArray(ObjectDescriptor* meta, int32_t length, void* compiledCodeFrameAddress);

    bool collectGarbage(void* compiledCodeFrameAddress);

    void findLiveObjects(void* compiledCodeFrameAddress);

    uint32_t computeForwardReferences();

    void updateReferences(void* compiledCodeFrameAddress);

    void relocateLiveObjects();

    void updateRefsOnHeap();

    void updateRefsOnStack(void* compiledCodeFrameAddress);
};


#endif //MJ_RT_HEAP_H
