#include <raymath/raymath.h>

#include <ansipixel/ansipixel.h>
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

float Terrain_terrain(Vector2 p) {
    float d = 0;
    d += 5 * fbm(Vector2Scale(p, .2f));
    d += 0.5 * fbm(Vector2Scale(p, 3));
    d -= 2.3 * noise(Vector2Scale(p, 0.02));
    d += 4.5 * noise(Vector2Scale(p, 0.1));
    return d;
}

// float temperature(Vector2 p) {
// 
// }

TerrainBiome Terrain_biome(Vector2 p, float height) {
    if (height < 0) {
        return BIOME_SEA;
    } else {
        return BIOME_JUNGLE;
    }
}

static float RemapQuadratic(
    float value,
    float inputStart, float inputEnd,
    float outputStart, float outputEnd) {
    float r = (value - inputStart) / (inputEnd - inputStart);
    r = r*r;
    r = r * (outputEnd - outputStart) + outputStart;
    return r;
}

AP_ColorRgb Terrain_biomeMapColor(
    float pixelHeight,
    float middleHeight,
    float min,
    float max,
    TerrainBiome biome) {
    // unsigned short c = Remap(pixelHeight, min, max, 0, 255);
    unsigned short c = pixelHeight < middleHeight ?
        RemapQuadratic(pixelHeight, min, middleHeight, 0, 160) :
        RemapQuadratic(pixelHeight, middleHeight, max, 161, 255);
    c = Clamp(c * Remap(middleHeight, -24, 24, 0.5, 1.5), 0, 255);

    switch (biome) {
        case BIOME_SEA: return AP_ColorRgb(0, 0, c);
        case BIOME_JUNGLE: return AP_ColorRgb(0, c, 0);
        case BIOME_SNOW: return AP_ColorRgb(c, c, c);
        case BIOME_DESSERT: return AP_ColorRgb(c, c, 0);
        case BIOME_VOLCANO: return AP_ColorRgb(c, 0, 0);
        case BIOME_END: return AP_ColorRgb(0, 0, 0);
    }
}
