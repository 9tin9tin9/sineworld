#pragma once

#include <ansipixel/ansipixel.h>
#include <flecs/flecs.h>
#include <raymath/raymath.h>

#define TERRAIN_SCALE 0.05

typedef struct {
    float lower;
    float upper;
} FunctionRange;

float Terrain_terrain(Vector2 p);
extern FunctionRange Terrain_terrain_range;

typedef enum {
    BIOME_SEA,
    BIOME_ICE,
    BIOME_SWAMP,
    BIOME_MEADOW,
    BIOME_JUNGLE,
    BIOME_HIGHLAND,
    BIOME_SNOW,
    BIOME_DESSERT,
    BIOME_VOLCANO,
    BIOME_END,
} TerrainBiome;

TerrainBiome Terrain_biome(Vector2 p, float height);
AP_ColorRgb Terrain_biomeMapColor(
    float pixelHeight,
    float middleHeight,
    float min,
    float max,
    TerrainBiome biome);
