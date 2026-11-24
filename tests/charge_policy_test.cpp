#include "../ChargeLimiter/charge_policy.h"
#include <cassert>
#include <iostream>

static cl::Input normal() {
    cl::Input in = {};
    in.connected = true;
    in.capacityValid = in.temperatureValid = true;
    in.capacity = 45;
    in.lower = 40;
    in.upper = 50;
    in.temperature = 25;
    in.cool = 30;
    in.hot = 35;
    in.plugMode = true;
    return in;
}

int main() {
    cl::State state;
    auto in = normal();
    assert(cl::evaluate(in, state).decision == cl::Keep);
    in.newlyConnected = true;
    assert(cl::evaluate(in, state).decision == cl::Charge);
    in.plugMode = false;
    assert(cl::evaluate(in, state).decision == cl::Stop);
    in.capacity = 40;
    assert(cl::evaluate(in, state).decision == cl::Charge);
    in.capacity = 50;
    assert(cl::evaluate(in, state).decision == cl::Stop);
    in.capacity = 75;
    assert(cl::evaluate(in, state).decision == cl::Stop);

    // Every supported limit must stop at the threshold, in either mode.
    for (int upper = 6; upper <= 100; ++upper) {
        for (int capacity = 0; capacity <= 100; ++capacity) {
            for (bool plug : {false, true}) {
                in = normal();
                in.upper = upper;
                in.lower = upper - 1;
                in.capacity = capacity;
                in.plugMode = plug;
                state = cl::State();
                auto result = cl::evaluate(in, state);
                if (capacity >= upper) assert(result.decision == cl::Stop);
                else assert(result.decision == cl::Charge);
            }
        }
    }

    // Low charge and a reconnect must not bypass temperature hysteresis.
    in = normal();
    in.capacity = 3;
    in.temperatureEnabled = true;
    in.temperature = 36;
    state = cl::State();
    assert(cl::evaluate(in, state).decision == cl::Stop);
    in.temperature = 33;
    in.newlyConnected = true;
    assert(cl::evaluate(in, state).decision == cl::Stop);
    in.temperature = 30;
    assert(cl::evaluate(in, state).decision == cl::Charge);
    in.capacity = 60;
    assert(cl::evaluate(in, state).decision == cl::Stop);
    in.temperatureValid = false;
    assert(cl::evaluate(in, state).reason == cl::InvalidReading);
    in.temperatureValid = true;
    in.capacityValid = false;
    assert(cl::evaluate(in, state).decision == cl::Keep);
    in.capacityValid = true;
    in.connected = false;
    assert(cl::evaluate(in, state).reason == cl::Unplugged);

    // Legacy iPhones use legacy defaults; keep newer/unknown families intact.
    for (const char* model : {"iPhone7,2", "iPhone8,1", "iPhone8,4", "iPhone9,1"})
        assert(!cl::defaultPredictive(15, model));
    for (const char* model : {"iPhone10,1", "iPhone10,6", "iPhone16,2", "iPad13,4", "unknown"})
        assert(cl::defaultPredictive(15, model));
    assert(!cl::defaultPredictive(12, "iPhone10,1"));
    assert(!cl::validLimits(50, 50));
    assert(!cl::validLimits(-1, 50));
    assert(!cl::validLimits(40, 101));
    assert(cl::validLimits(40, 50));
    assert(cl::retryDue(0, -1));
    assert(!cl::retryDue(14, 0));
    assert(cl::retryDue(15, 0));
    assert(!cl::commandDue(15, 0, 0));
    assert(!cl::commandDue(119, 0, 0));
    assert(cl::commandDue(120, 0, 0));
    assert(!cl::commandDue(14, 0, -2));
    assert(cl::commandDue(15, 0, -2));
    assert(!cl::stopConfirmed(true, 500, false));
    assert(cl::stopConfirmed(true, -100, true));
    assert(cl::stopConfirmed(true, 5, true));
    assert(!cl::stopConfirmed(false, 0, true));
    assert(cl::stopConfirmed(false, 0, false));
    std::cout << "Charging policy: threshold matrix, thermal recovery, model defaults and verification passed\n";
}
