#ifndef REGOLITH_DEBUG_H
#define REGOLITH_DEBUG_H

#include <mutex>

class REGOLITH_API rDebug {
public:
    // Mutex for thread-safe logging
    static inline std::mutex log_mutex;

    // Default logging functions
    static void defaultLog(const char* message) {
        std::lock_guard<std::mutex> lock(log_mutex);
        std::cout << "LOG: " << message << std::endl;
    }

    static void defaultErr(const char* message) {
        std::lock_guard<std::mutex> lock(log_mutex);
        std::cerr << "ERROR: " << message << std::endl;
    }

    // Function pointers for logging
    static inline std::function<void(const char*)> log = defaultLog;
    static inline std::function<void(const char*)> err = defaultErr;
};

#endif //REGOLITH_DEBUG_H
