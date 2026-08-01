#ifndef CLOUDS_TYPES_H
#define CLOUDS_TYPES_H

// Clouds
// ----------------------
#define CLOUD_COUNT 6

typedef struct {
  float x;
  float y;
  float speed;
  float phase;
} Cloud;

typedef struct {
  Cloud clouds[CLOUD_COUNT];
} CloudState;

#endif // !CLOUDS_TYPES_H
