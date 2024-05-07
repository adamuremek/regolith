#ifndef REGOLITH_DEBUG_H
#define REGOLITH_DEBUG_H

class REGOLITH_API rDebug{
public:
    static void doNothing(const char*){}
    static inline std::function<void(const char*)> log = doNothing;
    static inline std::function<void(const char*)> err = doNothing;
};

#endif //REGOLITH_DEBUG_H
