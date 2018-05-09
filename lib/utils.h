//
// Created by Ivan Rašić on 9.5.18..
//

#ifndef MJ_RT_UTILS_H
#define MJ_RT_UTILS_H

#ifndef NDEBUG
//#define DEBUG_PRINT(format, ...) printf((format),#endif __VA_ARGS__)
#define DEBUG(...) printf(__VA_ARGS__)
#else
#define DEBUG(...)
#endif

#endif //MJ_RT_UTILS_H
