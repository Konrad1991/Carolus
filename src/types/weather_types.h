#ifndef WEATHER_TYPES_H
#define WEATHER_TYPES_H

// Weather
// ----------------------
typedef enum {
  WEATHER_SUNNY,
  WEATHER_WINDY,
  WEATHER_RAIN,
  WEATHER_COUNT
} Weather;

typedef struct {
  Weather current;
  bool hovered[WEATHER_COUNT];
  FigureDirection wind_direction;
  bool wind_lever_hovered;
  float temperature_celsius;
  float water_level_z;
} WeatherState;

typedef enum {
  WEATHER_SCENARIO_NONE,
  WEATHER_SCENARIO_PERFECT_YEAR,
  WEATHER_SCENARIO_GOOD_YEAR,
  WEATHER_SCENARIO_MEDIOCRE_YEAR,
  WEATHER_SCENARIO_BAD_YEAR,
  WEATHER_SCENARIO_COUNT
} WeatherScenario;

typedef struct {
  WeatherScenario current;
  float elapsed;
  float sub_timer;
  int last_seen_month; // -1 until first update; used to detect the calendar wrapping into a new year
  bool hovered[WEATHER_SCENARIO_COUNT];
} WeatherScenarioState;

#endif // !WEATHER_TYPES_H
