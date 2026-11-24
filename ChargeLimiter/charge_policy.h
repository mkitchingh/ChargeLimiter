#ifndef CL_CHARGE_POLICY_H
#define CL_CHARGE_POLICY_H

#include <cstdio>
#include <cstring>

// Platform-independent policy. Hardware writes and their verification live in daemon.mm.
namespace cl {
enum Decision { Keep = -1, Stop = 0, Charge = 1 };
enum Reason { Observe, Unplugged, InvalidReading, CapacityLimit, TemperatureLimit,
              TemperatureWait, LowCapacity, Cooled, Plugged, EdgeWait };
struct Input {
    bool connected, newlyConnected, capacityValid, temperatureValid;
    int capacity, lower, upper;
    bool temperatureEnabled;
    double temperature, cool, hot;
    bool plugMode;
};
struct State { bool temperaturePaused = false; };
struct Result { Decision decision; Reason reason; };

inline Result evaluate(const Input& in, State& state) {
    if (!in.capacityValid || in.lower < 0 || in.lower >= in.upper || in.upper > 100 ||
        (in.temperatureEnabled && (!in.temperatureValid || in.cool >= in.hot)))
        return {Keep, InvalidReading};
    if (!in.connected) return {Keep, Unplugged};
    // A low charge must never bypass an enabled high-temperature stop.
    if (in.temperatureEnabled && in.temperature >= in.hot) {
        state.temperaturePaused = true;
        return {Stop, TemperatureLimit};
    }
    if (in.capacity >= in.upper) return {Stop, CapacityLimit};
    if (in.temperatureEnabled && state.temperaturePaused) {
        if (in.temperature > in.cool) return {Stop, TemperatureWait};
        state.temperaturePaused = false;
        if (in.plugMode) return {Charge, Cooled};
        if (in.capacity > in.lower && in.capacity > 5) return {Keep, EdgeWait};
    }
    if (!in.temperatureEnabled) state.temperaturePaused = false;
    if (in.capacity <= 5 || in.capacity <= in.lower) return {Charge, LowCapacity};
    if (in.plugMode) {
        if (in.temperatureEnabled && in.temperature <= in.cool) return {Charge, Cooled};
        if (in.newlyConnected) return {Charge, Plugged};
    } else if (in.newlyConnected) return {Stop, EdgeWait};
    return {Keep, Observe};
}

inline bool defaultPredictive(int majorOS, const char* model) {
    if (majorOS < 13) return false;
    int generation = 0;
    // iPhone8,x means 6s/SE, not iPhone 8 (iPhone10,x).
    if (model && std::sscanf(model, "iPhone%d,", &generation) == 1)
        return generation >= 10;
    return true; // Preserve existing defaults for iPad and unknown families.
}

inline bool validCapacity(int value) { return value >= 0 && value <= 100; }
inline bool validLimits(int lower, int upper) {
    return lower >= 0 && lower < upper && upper <= 100;
}
inline bool retryDue(double now, double last) { return last < 0 || now - last >= 15; }
inline bool commandDue(double now, double last, int lastError) {
    return last < 0 || now - last >= (lastError == 0 ? 120 : 15);
}
inline bool capacityRising(int baseline, int capacity) {
    return validCapacity(baseline) && validCapacity(capacity) && capacity >= baseline + 2;
}
inline bool stopConfirmed(bool hasCurrent, long current, bool charging, bool rising = false) {
    return !rising && (hasCurrent ? current <= 10 : !charging);
}
}
#endif
