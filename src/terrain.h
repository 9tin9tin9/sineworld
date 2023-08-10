#pragma once

#include <ansipixel/ansipixel.h>
#include <flecs/flecs.h>
#include <raymath/raymath.h>

#define TERRAIN_SCALE 0.05

float Terrain_terrain(Vector2 p);

typedef enum {
    BIOME_SEA,
    BIOME_JUNGLE,
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
