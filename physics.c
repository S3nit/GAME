#include "physics.h"

float SmoothLerp(float start, float end, float amount) {
    return start + amount * (end - start);
}