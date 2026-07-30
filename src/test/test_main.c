#include "test.h"
#include "test_soil.h"

int tests_failed = 0;
int tests_run = 0;

int main(void) {
  test_soil_deplete_water_clamps_to_available();
  printf("%d/%d passed\n", tests_run - tests_failed, tests_run);
  return tests_failed > 0 ? 1 : 0;
}
