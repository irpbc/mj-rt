//
// Created by Ivan Rašić on 5.5.18..
//

#include <printf.h>

#include <inttypes.h>
#ifdef __APPLE__
#include <mach-o/getsect.h>
#include <mach-o/ldsyms.h>
#else
// TODO: Windows
#endif

void mjrt_init_runtime(void *);

void _premain() {

    uint8_t *_LLVM_StackMaps = NULL;

#ifdef __APPLE__
    unsigned long size = 0;
    _LLVM_StackMaps = getsectiondata(&_mh_execute_header, "__LLVM_STACKMAPS", "__llvm_stackmaps", &size);
#else
    // TODO: Windows
#endif

    mjrt_init_runtime(_LLVM_StackMaps);
}
