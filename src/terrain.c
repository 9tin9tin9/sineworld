#include <raymath/raymath.h>

#include "terrain.h"

static float noise(Vector2 p) {
    return sinf(p.x) + sinf(p.y);
}

static float fbm(Vector2 p) {
    const float N_OCTAVES = 8;
    float res = 0.f;
    float amp = 0.5;
    float freq = 1.95;
    for (int i = 0; i < N_OCTAVES; i++) {
        res += amp * noise(p);
        amp *= 0.5;
        p = Vector2Scale(p, freq);
        p = Vector2Rotate(p, PI / 4.f);
        p = Vector2AddValue(p, res * 0.6);
    }
    return res;
}

float terrain(Vector2 p) {
    float d = 0;
    d += 5 * fbm(Vector2Scale(p, .2f));
    d += 0.2 * fbm(Vector2Scale(p, 5));
    d -= 2.3 * noise(Vector2Scale(p, 0.02));
    d += 4.5 * noise(Vector2Scale(p, 0.1));
    return d;
}
