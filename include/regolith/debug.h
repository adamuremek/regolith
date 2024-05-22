#ifndef REGOLITH_DEBUG_H
#define REGOLITH_DEBUG_H

#include <mutex>

class rDebug {
private:
    // Mutex for thread-safe logging
    static inline std::mutex log_mutex;

    // Default logging functions
    static void defaultLog(const char *message) {
        std::lock_guard<std::mutex> lock(log_mutex);
        std::cout << "LOG: " << message << std::endl;
    }

    static void defaultErr(const char *message) {
        std::lock_guard<std::mutex> lock(log_mutex);
        std::cerr << "ERROR: " << message << std::endl;
    }

public:
    // Function pointers for logging
    REGOLITH_API static inline std::function<void(const char *)> logFunc = defaultLog;
    REGOLITH_API static inline std::function<void(const char *)> errFunc = defaultErr;

    REGOLITH_API static void log(const char *format, ...) {
        va_list args;
                va_start(args, format);
        char buffer[107] = "[rLOG] ";
        vsnprintf(buffer + 7, sizeof(buffer) - 7, format, args);
                va_end(args);

        logFunc(buffer);
    }

    REGOLITH_API static void err(const char *format, ...) {
        va_list args;
                va_start(args, format);
        char buffer[107] = "[rERR] ";
        vsnprintf(buffer + 7, sizeof(buffer) - 7, format, args);
                va_end(args);

        errFunc(buffer);
    }


};

#endif //REGOLITH_DEBUG_H
