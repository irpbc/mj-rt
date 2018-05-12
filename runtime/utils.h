//
// Created by Ivan Rašić on 9.5.18..
//

#ifndef MJ_RT_UTILS_H
#define MJ_RT_UTILS_H

#ifndef NDEBUG
//#define DEBUG_PRINT(format, ...) printf((format),#endif __VA_ARGS__)
#define DEBUG(...) printf(__VA_ARGS__)
#define DEBUG_EXPR(_expression_) _expression_
#else
#define DEBUG(...)
#define DEBUG_EXPR(_expression_)
#endif

#endif //MJ_RT_UTILS_H
