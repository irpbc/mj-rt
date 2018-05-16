//
// Created by Ivan Rašić on 4.5.18..
//

#include <cstdlib>
#include <cstring>
#include "Heap.h"
#include "utils.h"

inline int Align8Up(int ptr) {
    return ptr + ((~ptr+1) & 7);
}

void Heap::initialize(uint32_t bytes, uint8_t* LLVM_StackMaps) {
    heapStart = (uint8_t*)malloc(bytes + 8); // Add 8 bytes to the end because reasons
    heapEnd = heapStart;
    heapMax = heapStart + bytes;

    memset(heapStart, 0, bytes);

    void* stackmaps = LLVM_StackMaps;

    spTable = StatepointTable::generateTable(stackmaps, 0.5);
	DEBUG_EXPR(spTable->print(stdout, true));
}

ObjectBase* Heap::allocateObject(ObjectDescriptor* meta, void* compiledCodeFrameAddress) {
    int size = meta->objectSize;
    DEBUG("Allocating object of %d bytes\n", size);

    if (heapEnd + size >= heapMax && !collectGarbage(compiledCodeFrameAddress)) {
        fprintf(stderr, "Out of heap space\n");
        exit(42);
    }

    uint8_t* objectPtr = heapEnd;
    heapEnd = heapEnd + size;

    memset(objectPtr, 0, (size_t)size);

    ObjectBase* obj = (ObjectBase*)(objectPtr);
    obj->descriptor = meta;
    obj->status = Status::UNSCANED;

    DEBUG("Heap space left = %" PRIu64 "\n", (uint64_t)(heapMax - heapEnd));

    return obj;
}

ObjectBase* Heap::allocateArray(ObjectDescriptor* meta, int32_t length, void* compiledCodeFrameAddress) {
    int startSize = meta->objectSize;

    int size = startSize + meta->arrayElemSize * length;
    size = Align8Up(size);

    DEBUG("Allocating array of %d elements (%d bytes)\n", length, size);

    if (heapEnd + size >= heapMax && !collectGarbage(compiledCodeFrameAddress)) {
        fprintf(stderr, "Out of heap space\n");
        exit(42);
    }

    uint8_t* objectPtr = heapEnd;
    heapEnd = heapEnd + size;

    memset(objectPtr, 0, (size_t)size);

    ArrayBase* arr = (ArrayBase*)(objectPtr);
    arr->descriptor = meta;
    arr->status = Status::UNSCANED;
    arr->length = (uint32_t)length;

    DEBUG("Heap space left = %" PRIu64 "\n", (uint64_t)(heapMax - heapEnd));

    return (ObjectBase*)arr;
}

bool Heap::collectGarbage(void* compiledCodeFrameAddress) {
    DEBUG("Collecting garbage\n");

    findLiveObjects(compiledCodeFrameAddress);
    uint32_t garbageCount = computeForwardReferences();
    DEBUG("Found %" PRIu32 " dead objects\n", garbageCount);
    if (garbageCount > 0) {
        updateReferences(compiledCodeFrameAddress);
        relocateLiveObjects();
    }

    DEBUG("Collection done\n");
    return garbageCount > 0;
}

int MarkLive(ObjectBase* obj) {
    if (obj->status == Status::UNSCANED) {
        obj->status = Status::SCANING;
        int liveCount = 1;
        ObjectDescriptor* desc = obj->descriptor;
        TypeKind kind = desc->typeKind;
        if (kind == TypeKind::OBJECT) {
            DEBUG("Found object\n");
            int32_t* offsets = desc->refFieldOffsets;
            for (int i = 0, count = desc->numRefFields; i < count; ++i) {
                ObjectBase* child = obj->getRefFieldValue(offsets[i]);
                if (child) {
                    liveCount += MarkLive(child);
                }
            }
        } else if (kind == TypeKind::ARRAY_OF_REFERENCES) {
            DEBUG("Found array of references\n");
            ArrayBase* arr = (ArrayBase*)obj;
            // We add runtime object size instead of sizeof(ArrayBase)
            // to account for alignment of elements that come after
            // the static array data. The runtime size is calculated by LLVM.
            ObjectBase** ptrs = (ObjectBase**)((uint8_t*)arr + desc->objectSize);
            for (int i = 0, len = arr->length; i < len; ++i) {
                ObjectBase* elem = ptrs[i];
                if (elem) {
                    DEBUG("Found non-null array element at index %d\n", i);
                    liveCount += MarkLive(elem);
                }
            }
        }
        obj->status = Status::LIVE;
        return liveCount;
    }
    return 0;
}

void Heap::findLiveObjects(void* compiledCodeFrameAddress) {
    void* returnAddress = *(void**)compiledCodeFrameAddress;

    uint64_t liveCount = 0;
    while (FrameInfo* frame = spTable->lookupReturnAddress((uint64_t)returnAddress)) {
        uint8_t* frameBase = (uint8_t*)compiledCodeFrameAddress + sizeof(uint64_t);
        for (int i = 0; i < frame->numSlots; ++i) {
            PointerSlot slot = frame->slots[i];
            ObjectBase* objPtr = *(ObjectBase**)(frameBase + slot.offset);
            if (objPtr) {
                liveCount += MarkLive(objPtr);
            }
        }

        compiledCodeFrameAddress = frameBase + frame->frameSize;
        returnAddress = *(void**)compiledCodeFrameAddress;
    }

    DEBUG("Found %llu live objects\n\n", liveCount);
}

uint32_t Heap::computeForwardReferences() {
    uint8_t* heapStart = this->heapStart;
    uint8_t* nextForwardAddress = heapStart;

    ObjectBase* currentObj = (ObjectBase*)heapStart;
    ObjectDescriptor* descriptor = currentObj->descriptor;

    uint32_t garbageCount = 0;
    while (descriptor) {
        int32_t objectSize = descriptor->objectSize;
        if ((descriptor->typeKind & TypeKind::ANY_ARRAY) > 0) {
            ArrayBase* arr = (ArrayBase*)currentObj;
            objectSize += arr->length * descriptor->arrayElemSize;
            objectSize = Align8Up(objectSize);
        }
        if (currentObj->status == Status::LIVE) {
            currentObj->forward = (ObjectBase*)nextForwardAddress;
            nextForwardAddress += objectSize;
        } else {
            garbageCount = garbageCount + 1;
        }

        currentObj = (ObjectBase*)((uint8_t*)currentObj + objectSize);
        descriptor = currentObj->descriptor;
    }
    return garbageCount;
}

void updateObjectFields(ObjectBase* obj) {
    ObjectDescriptor* desc = obj->descriptor;
    if (desc->typeKind == TypeKind::OBJECT) {
        for (int i = 0, refs = desc->numRefFields; i < refs; ++i) {
            int32_t offset = desc->refFieldOffsets[i];
            ObjectBase* child = obj->getRefFieldValue(offset);
            if (child) {
                obj->setRefFieldValue(offset, child->forward);
            }
        }
    } else if (desc->typeKind == TypeKind::ARRAY_OF_REFERENCES) {
        ArrayBase* arr = (ArrayBase*)obj;
        // We add runtime object size instead of sizeof(ArrayBase)
        // to account for alignment of elements that come after
        // the static array data. The runtime size is calculated by LLVM.
        ObjectBase** ptrs = (ObjectBase**)((uint8_t*)arr + desc->objectSize);
        for (int j = 0, len = arr->length; j < len; ++j, ++ptrs) {
            ObjectBase* elem = *ptrs;
            if (elem) {
                *ptrs = elem->forward;
            }
        }
    }
}

void Heap::updateReferences(void* compiledCodeFrameAddress) {
    updateRefsOnStack(compiledCodeFrameAddress);
    updateRefsOnHeap();
}

void Heap::updateRefsOnStack(void* compiledCodeFrameAddress) {
    void* returnAddress = *(void**)compiledCodeFrameAddress;

    while (FrameInfo* frame = spTable->lookupReturnAddress((uint64_t)returnAddress)) {
        uint8_t* frameBase = (uint8_t*)compiledCodeFrameAddress + sizeof(uint64_t);
        for (int i = 0; i < frame->numSlots; ++i) {
            PointerSlot slot = frame->slots[i];
            ObjectBase** slotPtr = (ObjectBase**)(frameBase + slot.offset);
            ObjectBase* object = *slotPtr;
            if (object) {
                *slotPtr = object->forward;
            }
        }

        compiledCodeFrameAddress = frameBase + frame->frameSize;
        returnAddress = *(void**)compiledCodeFrameAddress;
    }
}

void Heap::updateRefsOnHeap() {
    uint8_t* heapStart = this->heapStart;

    ObjectBase* currentObj = (ObjectBase*)heapStart;
    ObjectDescriptor* descriptor = currentObj->descriptor;

    while (descriptor) {
        int32_t objectSize = descriptor->objectSize;
        if ((descriptor->typeKind & TypeKind::ANY_ARRAY) > 0) {
            ArrayBase* arr = (ArrayBase*)currentObj;
            objectSize += arr->length * descriptor->arrayElemSize;
            objectSize = Align8Up(objectSize);
        }
        if (currentObj->status == Status::LIVE) {
            updateObjectFields(currentObj);
        }

        currentObj = (ObjectBase*)((uint8_t*)currentObj + objectSize);
        descriptor = currentObj->descriptor;
    }
}

void Heap::relocateLiveObjects() {
    uint8_t* newHeapEnd = heapStart;

    ObjectBase* currentObj = (ObjectBase*)heapStart;
    ObjectDescriptor* descriptor = currentObj->descriptor;

    while (descriptor) {
        int32_t objectSize = descriptor->objectSize;
        if ((descriptor->typeKind & TypeKind::ANY_ARRAY) > 0) {
            ArrayBase* arr = (ArrayBase*)currentObj;
            objectSize += arr->length * descriptor->arrayElemSize;
            objectSize = Align8Up(objectSize);
        }
        if (currentObj->status == Status::LIVE) {
            memmove(newHeapEnd, currentObj, (size_t)objectSize);
            ObjectBase* relocated = (ObjectBase*)newHeapEnd;
            relocated->status = Status::UNSCANED;
            relocated->forward = nullptr;
            newHeapEnd += objectSize;
        }

        currentObj = (ObjectBase*)((uint8_t*)currentObj + objectSize);
        descriptor = currentObj->descriptor;
    }

    this->heapEnd = newHeapEnd;
    memset(newHeapEnd, 0, heapMax - newHeapEnd);
    DEBUG("Heap space left = %" PRIu64 "\n", (uint64_t)(heapMax - newHeapEnd));
}
