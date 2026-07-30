#include "test_soil.h"

void test_soil_deplete_water_clamps_to_available(void) {
  Map map;
  map_init(&map, 2, 2);
  map_tile(&map, 0, 0)->soil_water = 20.0f;

  float taken = soil_deplete_water(&map, 0, 0, 50.0f);

  ASSERT_EQ(taken, 20.0f);
  ASSERT_EQ(map_tile(&map, 0, 0)->soil_water, 0.0f);
  free_map(&map);
}
