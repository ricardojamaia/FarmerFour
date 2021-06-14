#include <TimeLib.h>

#ifndef FARMFOUR_DEBUG_H
#define FARMFOUR_DEBUG_H

#define DEBUG_OFF 0
#define DEBUG_ON 1

#define DEBUG_HIGH 30
#define DEBUG_MEDIUM 20
#define DEBUG_LOW 10

/**
 * DEBUG_MODE
 * Turn on and of the debug
 **/
#define DEBUG_MODE DEBUG_ON

/** Set the maximum debug level for the whole code */
#define DEBUG_LEVEL DEBUG_HIGH

/** Set the actual debug level for each module */
#define DEBUG_FF_MAIN DEBUG_ON
#define DEBUG_FF_UI DEBUG_ON
#define DEBUG_FF_CTRL DEBUG_ON

#if DEBUG_MODE

#define DPRINT(debug_module, debug_level, ...)        \
    if (debug_module && (debug_level <= DEBUG_LEVEL)) \
    {                                                 \
        Serial.print(__VA_ARGS__);                    \
    }

#define DPRINTLN(debug_module, debug_level, ...)      \
    if (debug_module && (debug_level <= DEBUG_LEVEL)) \
    {                                                 \
        Serial.println(__VA_ARGS__);                  \
        Serial.flush();                               \
    }

#define DPRINT_DATE(debug_module, debug_level, ...) \
    if (debug_module && (debug_level <= DEBUG_LEVEL)) \
    {                                                 \
        time_t current_time = now();                  \
        char message[20]; \
        sprintf(message, "%02u/%02u/%u %02u:%02u: ", day(current_time), month(current_time), year(current_time), hour(current_time), minute(current_time)); \
        Serial.print(message);                      \
        Serial.print(__VA_ARGS__);                  \
        Serial.flush();                               \
    }

#define DPRINT_HH_MM(debug_module, debug_level, hh, mm) \
    if (debug_module && (debug_level <= DEBUG_LEVEL)) \
    {                                                 \
        char message[6];                              \
        sprintf(message, "%02u:%02u", hh, mm);        \
        Serial.print(message);                        \
        Serial.flush();                               \
    }

#define DPRINTLN_DATE(debug_module, debug_level, ...) \
    if (debug_module && (debug_level <= DEBUG_LEVEL)) \
    {                                                 \
        time_t current_time = now();                  \
        char message[20]; \
        sprintf(message, "%02u/%02u/%u %02u:%02u: ", day(current_time), month(current_time), year(current_time), hour(current_time), minute(current_time)); \
        Serial.print(message);                      \
        Serial.println(__VA_ARGS__);                  \
        Serial.flush();                               \
    }

#define DEBUG(x) x

#else
#define DPRINT(debug_module, debug_level, ...) \
    do                                         \
    {                                          \
    } while (0)
#define DPRINTLN(debug_module, debug_level, ...) \
    do                                           \
    {                                            \
    } while (0)
#define DEBUG(x) \
    do           \
    {            \
    } while (0)
#endif

#endif /* FARMFOUR_DEBUG_H */