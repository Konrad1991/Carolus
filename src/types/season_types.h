#ifndef SEASON_TYPES_H
#define SEASON_TYPES_H

// Seasons
// ----------------------
#define SECONDS_PER_MONTH 600
typedef enum {
  WINTER,
  SPRING,
  SUMMER,
  AUTUMN,
  SEASON_COUNT
} Season;

typedef struct {
  int month;
  float month_progress;
  bool month_up_hovered;
  bool month_down_hovered;
  int year;
} SeasonState;

typedef struct {
  Season base;
  Season fade_in;
  float alpha;
} SeasonBlend;

#endif // !SEASON_TYPES_H
