#ifndef PERLIN_H
#define PERLIN_H

#ifdef __cplusplus
extern "C" {
#endif
extern int perlin_noise_seed;
float perlin2d(float x, float y, float freq, int depth);
#ifdef __cplusplus
}
#endif

#endif