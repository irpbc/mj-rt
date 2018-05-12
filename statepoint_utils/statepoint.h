#ifndef __LLVM_STATEPOINT_UTILS_API__
#define __LLVM_STATEPOINT_UTILS_API__

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

/**** Types ****/

struct PointerSlot {
    // kind < 0 means this is a base pointer
    // kind >= 0 means this is a pointer derived from base pointer in slot number "kind"
    int32_t kind;

    // offsets are relative to the base of a frame. 
    // See Figure 1 below for our defintion of "base"
    int32_t offset;
};

/*

                 FIGURE 1
    Stack grows towards low addresses.

high addresses

 ... ETC ...
-------------- <- base2
frame 2's ret addr
-------------- <- start of frame 2 (computed with: base1 + base1's frameSize)

frame 1's contents

-------------- <- base1, aka base for offsets into frame 1 (8 bytes above start of frame 1)
frame 1's ret addr
-------------- <- start of frame 1 (what you get immediately after a callq)

low addresses

*/

struct FrameInfo {
    // NOTE flags & calling convention didn't seem useful to include in the map.
    uint64_t retAddr;
    uint64_t frameSize;     // in bytes

    // all base pointers come before derived pointers in the slot array. you can use this
    // fact to quickly update the derived pointers by referring back to the base pointers
    // while scanning the slots.
    uint16_t numSlots;
    PointerSlot slots[];

    size_t size();

    // the function print_table uses to print an individual frame, useful for debugging.
    void print(FILE* stream);
};


struct TableBucket {
    uint16_t numEntries;
    size_t sizeOfEntries; // total memory footprint of the entries
    FrameInfo* entries;
};

struct StatepointTable {
    uint64_t size;
    TableBucket* buckets;

    /**
     * An amortized O(1) return address lookup function for garbage collectors.
     *
     * table - table generated by generate_table
     * retAddr - the key (a return address) corresponding to the frame you need info about.
     *
     * Returns NULL if the address was not found in the table.
     */
    FrameInfo* lookupReturnAddress(uint64_t retAddr);

    /**
     * Given an LLVM generated Stack Map, will returns a hash table mapping return addresses
     * to a frame_info_t struct that provides information about live pointer locations within
     * that stack frame.
     *
     * - The map is the LLVM Stack Map generated via gc.statepoint.
     * - The load factor allows you to tune the amount of hash collisions in the table. Lower
     * values help prevent collisions, helping lookup times, at the cost of increasing
     * the size of the table.
     */
    static StatepointTable* generateTable(void* map, float load_factor);


    /**
     * Frees _all_ allocated memory reachable from the table. Thus, any
     * pointers returned from a previous lookup are invalid after this call.
     */
    ~StatepointTable();


    /* Insert a custom key value pair.
       NOTE the value _must_ be a malloc'd pointer, because insert_key
       will attempt to free it after it's been inserted.
     */
    void insertKey(uint64_t key, FrameInfo* value);



    /**** Debugging Functions ****/

    // skip_empty will skip printing out empty buckets
    void print(FILE* stream, bool skip_empty);
};


#endif /* __LLVM_STATEPOINT_UTILS_API__ */
#ifndef __LLVM_STATEPOINT_UTILS_HASH_TABLE__
#define __LLVM_STATEPOINT_UTILS_HASH_TABLE__

#define __STDC_FORMAT_MACROS 1

#include <inttypes.h>

#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

/** Functions **/

StatepointTable* new_table(float loadFactor, uint64_t expectedElms);

/* lookup_return_address & insert_key is declared in api.h */

size_t size_of_frame(uint16_t numSlots);

// returns the next frame relative the current frame
FrameInfo* next_frame(FrameInfo* cur);

#endif /* __LLVM_STATEPOINT_UTILS_HASH_TABLE__ */
#ifndef __LLVM_STATEPOINT_UTILS_STACKMAP__
#define __LLVM_STATEPOINT_UTILS_STACKMAP__

#include <stdint.h>
#include <stddef.h>

/** 
 * LLVM's Documentation: http://llvm.org/docs/StackMaps.html#stack-map-format
 *
 *  "The runtime must be able to interpret the stack map record given only the ID,
 *  offset, and the order of the locations, which LLVM preserves."
 *
 *  We interpret "order of the locations" to mean that not only are callsite records
 *  cooresponding to a function grouped together and ordered from least to greatest
 *  offset, but these callsite groups are also in the same order as the array of
 *  function stack size records. 
 * 
 *  This appears to be the case in LLVM, and indeed, these assumptions are nessecary to 
 *  figure out what groups correspond to which functions (without abusing the ID field
 *  with a post processing script) to compute the return addresses.
 */

/******** LAYOUT ********

stackmap_header_t;

function_info_t[numFunctions];

uint64_t[numConstants];

numRecords of the following {
   callsite_header_t;

   value_location_t[numLocations];

   << upto 4 bytes of padding, as needed, to achieve 8 byte alignment >>

   liveout_header_t;

   liveout_location_t[numLiveouts];

   << upto 4 bytes of padding, as needed, to achieve 8 byte alignment >>
}

******** END OF LAYOUT ********/

#ifdef _MSC_VER
#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop) )
#else
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#endif

PACK(struct stackmap_header_t {
    uint8_t version;
    uint8_t reserved1;
    uint16_t reserved2;
    uint32_t numFunctions;
    uint32_t numConstants;
    uint32_t numRecords;
});

PACK(struct function_info_t {
    uint64_t address;
    uint64_t stackSize;
    uint64_t callsiteCount;   // see https://reviews.llvm.org/D23487
});

PACK(struct callsite_header_t {
    uint64_t id;
    uint32_t codeOffset;  // from the entry of the function
    uint16_t flags;
    uint16_t numLocations;
});

enum location_kind_t {
    Register = 0x1,
    Direct = 0x2,
    Indirect = 0x3,
    Constant = 0x4,
    ConstIndex = 0x5
};

PACK(struct value_location_t {
    uint8_t kind;       // possibilities come from location_kind_t, but is one byte in size.
    uint8_t flags;      // expected to be 0
    uint16_t locSize;
    uint16_t regNum;    // Dwarf register num
    uint16_t reserved;  // expected to be 0
    int32_t offset;     // either an offset or a "Small Constant"
});

PACK(struct liveout_header_t {
    uint16_t padding;
    uint16_t numLiveouts;
});

PACK(struct liveout_location_t {
    uint16_t regNum;    // Dwarf register num
    uint8_t flags;
    uint8_t size;       // in bytes
});

#endif /* __LLVM_STATEPOINT_UTILS_STACKMAP__ */
