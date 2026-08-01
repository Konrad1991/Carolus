# TODO

## Roads & Paths

- [x] Basic tile-by-tile road building
- [x] Pathfinding (A*) exists and works
- [x] Drag-to-build roads (click + drag instead of tile by tile)
- [x] Pathfinding refinement: produce angular paths that ideally stay on
      roads only (A* itself is done; this specific road-preference behavior
      isn't)

Roads look really good at this point - no further work planned here beyond
what's below (bridges, seasons).

- [ ] Bug: on the soil overlay ("Spezialkarte", `SoilOverlayState`) view,
      a road built by dragging (`update_road_drag`/`place_road_tile`) shows
      its first tile still rendered as a normal road, but the rest of the
      dragged tiles correctly disappear under the overlay tint. Not
      root-caused yet - `draw_tiles()` in `drawing.c` unconditionally skips
      the tile-type switch and draws only the tint diamond once
      `soil_overlay_state.current != SOIL_OVERLAY_NONE`, so the discrepancy
      is more likely in how the drag start tile gets placed/rendered vs.
      the rest of the dragged path (`place_road_tile` pushes an
      `OBJECT_ROAD` object per tile, separate from the tile-type switch -
      worth checking whether the start tile ends up represented twice, or
      through a different code path than the rest of the path).

## Terrain

- Architecture note: only water/ground/road are actual tile *types* -
  everything else (trees, hedges, forest) is scattered as *objects* on top
  of a base tile, same pattern as oak trees/grass tufts. Forest and Hedges
  below should follow this, not become new TileTypes.
- [x] Small walls to separate fields/plots - solved via boundary-stone
      markers at plot corners (non-blocking, purely visual) rather than a
      continuous wall/fence (a full fence ring was tried and looked bad)
- [ ] Forest - in progress, working through the tree species pipeline first
      (see Trees & Shrubs below)
- [x] Tall grass - done via grass tufts
- [ ] Hay: harvesting/drying tall grass into hay - still open
- Hedges - decided against, too complicated for the payoff; won't be done

## Seasons & Time

Done as of 2026-07-22 (core system + grass + oak). Water/road seasonal art
and weather coupling still open, see below.

- [x] Global time/season clock: real-time driven. Month length is
      `SECONDS_PER_MONTH`, currently 5s to see transitions quickly during
      development; will later be set to something like 300s
- [x] Season-specific tile variants for grass - done
- Road: decided - no seasonal variants, roads stay as they are
- Water: undecided. Leaning toward winter-only frozen/ice variant, otherwise
      unchanged across seasons. New idea worth exploring separately: high
      water / flooding (Hochwasser) - not season-bound, but interesting

  Flooding design (revised - supersedes the original
  TILE_GRASS-converts-to-TILE_WATER idea): `Tile.z` is already `float`
  (done). Scope decided: a *bounded* floodplain, not a full flood-fill
  across the map - simpler, and avoids having to remember/restore a tile's
  original type.

  - Water tiles get a global water level (`water_level`, likely a new field
    on `WeatherState` next to `temperature_celsius`) that a small update
    function nudges toward a temperature-dependent target (warmer -> lower)
    and bumps up during `WEATHER_RAIN`. Applied to every `TILE_WATER`
    tile's `z` each frame, replacing the current fixed `z = -0.5`.
  - Neighboring `TILE_GRASS` tiles do NOT convert to `TILE_WATER`. Instead,
    a new `bool floodplain` field on `Tile` is set once at map-gen time (in
    `define_nature()`, right after `carve_pond`/`carve_stream`): any
    `TILE_GRASS` tile adjacent to a `TILE_WATER` tile gets flagged. This is
    a plain bool and deliberately not a new `TileType` - a floodplain tile
    must keep behaving exactly like normal grass everywhere else in the
    code (walkable, buildable, farmable in `map.c`/`path_finding.c`/
    `build_object.c`). Making it its own `TileType` would mean auditing and
    updating every `switch`/`if` on `t->type` in those files to also treat
    it like grass. A bool flag is invisible to all of that - only
    `draw_grass()` needs to look at it.
  - Rendering: in `draw_grass()`, if `t->floodplain && water_level > 0`,
    blend in a wet/mud tint proportional to
    `water_level / FLOOD_MAX_LEVEL` - reusing the tinted-overlay technique
    already used for the season fade (`draw_ground_tile_tinted` +
    `fade_tint`, see `drawing.c`).
  - Open questions: how many tiles deep the floodplain ring should be (just
    direct neighbors, or two tiles out for more visual range); whether
    `water_level` really belongs on `WeatherState` or should get its own
    struct; whether flooded tiles should eventually have gameplay
    consequences (blocked building, damaged crops) or stay purely visual
    for now.
- [x] `Texture_State`: store all 4 seasons' variants at once
      (`[SEASON_COUNT][TILE_VARIANT_COUNT]`), loaded once at startup;
      rendering just indexes by the current season instead of reloading
      textures at runtime
- [x] Tree season states (autumn colors, bare winter look) - done for oak
      (all 4 seasons, incl. sway animation), the template species; other
      species still to follow, see Trees & Shrubs below

## Field Growth States

Blocked on Seasons & Time above (field art is meant to vary by season).
Ripe/mature wheat (LARGE_YELLOW_PLANTS) already exists, but only that one
state - still need season variants for the rest of the growth states.
Currently working on wheat's growth stages.

Design: tied to the real calendar via `season_state.month`, not a separate
per-field timer - same pattern as `SEASON_FOR_MONTH` in `seasons.c`, e.g. a
`FIELD_CONDITION_FOR_MONTH[]` lookup (sow ~March/April, grow through the
green stages, ripe/LARGE_YELLOW_PLANTS by July, matching real summer-grain
timing). On top of that lookup, each field still needs its own small state
to diverge from the calendar default:
- was it sown at all (unsown fields stay GRASS/FALLOW instead of following
  the crop calendar)
- was it harvested in time (if not, it doesn't reset to GRASS/PLOWED for
  the next cycle - it tips into DAMAGED_LARGE_GREEN_PLANTS/
  DAMAGED_LARGE_YELLOW_PLANTS instead)

- [ ] `FieldCondition` enum already exists in `types.h` (GRASS, PLOWED,
      SOWED, SMALL/MEDIUM/LARGE_GREEN_PLANTS, LARGE_YELLOW_PLANTS, FALLOW,
      DAMAGED_LARGE_GREEN_PLANTS, DAMAGED_LARGE_YELLOW_PLANTS) — needs
      sprite assets per state and wiring into `Texture_State`
- [ ] Field state transitions driven by season + farmer action (or
      inaction): untended fields drift toward FALLOW/weeds over time;
      plowing/sowing/harvesting actions advance the state

## Harvest & Trampling

Design discussion resolved 2026-07-29, not yet implemented. Three
mechanics decided together, motivated by the current visual problem where
a standing figure overlaps tall wheat/grass sprites and sorts wrong (see
Rendering/Sorting below) - trampling fixes that as a side effect, since a
figure standing on flattened wheat has no tall sprite left to sort
against.

- [ ] Trampling: any figure standing on a field tile temporarily lowers
      that tile's wheat tufts' `wheat_vigor` (`WheatAttributes`,
      `crop_growth.c`) for as long as the figure remains there - not a new
      state/flag, reuses the existing mass-balance growth model instead of
      a separate "trampled" variable. Needs a flattened/short sprite
      variant for tufts while vigor is suppressed, not just slower growth
      with the same tall sprite.
- [ ] Harvest reworked from the current tile-by-tile cursor
      (`advance_harvest_cursor`/`HarvestRoute` in `update_figure.c` -
      farmer walks onto each tile and mows in place) to edge-mowing: the
      farmer stays outside the field boundary and mows inward, so the
      field shrinks from the outside in with each pass (like real
      scything) instead of the farmer walking through standing crop.
      Needs `HarvestRoute` reworked to track a shrinking boundary instead
      of a fixed row cursor.
- [ ] After mowing, the tile holds a wheat-sheaf bundle (`wheat_sheaf`
      sprite, already exists) that blocks the tile until collected -
      farmer picks it up (animation already exists: bends down, puts it
      in a wicker basket on his back - also an existing asset) and carries
      it to the barn before the tile counts as walkable again / before the
      next tile can be mowed. Open question, not decided yet: basket
      capacity - one bundle per trip to the barn (frequent short trips,
      matches the slow sim pacing) vs. collecting several bundles before
      making the trip (fewer trips, basket visibly fills up).

## Weather

Partially implemented already (ad-hoc, ahead of the Seasons work above):
Sunny/Windy/Rain toggle exists, wind direction affects grass/wheat/tree
sway, clouds drift during rain, puddles spawn/evaporate during rain with
evaporation speed coupled to temperature (player-adjustable via U/D, see
`weather.c`). `WeatherState` has no season dependency at all (verified:
no `Season`/`season` reference anywhere in weather.c/.h, clouds.c,
puddles.c) - the "decouple weather from season" idea from an earlier pass
was already true by the time it got written down, not an open task. Still
open:

- [ ] Cheapest approach found by other isometric sims: screen-space overlay
      (hand-rolled falling particle sprites for rain/snow) plus a
      translucent full-scene color-grade tint (bluish for rain, grey for
      overcast) drawn after the scene — no new tile art required, reuses
      the season system's elapsed-time clock
- [ ] Winter's season tile art can double as "always snowy," so only rain
      needs a dedicated overlay
- [x] Wind/grass-sway animation - done (oak, grass, wheat all sway with
      wind direction/weather already)
- [x] Bug: weather scenarios (`WeatherScenarioState`/`weather_scenario.c`)
      didn't repeat cleanly across consecutive in-game years - reported as
      "perfect year back to back works so-so." Root cause was that none of
      the scenarios ever looped `elapsed` back to the start once the
      calendar rolled into a new year (`update_perfect_year` gave up after
      `SECONDS_PER_MONTH * 4` and forced `WEATHER_SUNNY` forever;
      `run_scenario_phases` flatlined on its last, long sunny phase).
      Fixed: `WeatherScenarioState.last_seen_month` tracks the calendar
      month each frame; when it drops (month wraps from 11 back to 0),
      `update_weather_scenario` resets `elapsed`/`sub_timer`, restarting
      the scenario's script for the new year. `update_weather_scenario`
      now takes `current_month` (passed as `season_state.month` from
      `main.c`).

## Bridges

Bridges are effectively roads now, functionally speaking.

- [x] Characters walk across bridges already - a placed bridge converts the
      water tile underneath to TILE_ROAD, so pathfinding treats it as a
      normal (cheap) walkable tile with no special-casing needed
- [ ] Visual: missing beams/planks visible under the bridge tile - turned
      out trickier than expected, still open

## Characters

- [x] Farmer generated in pixellab.ai and walking/acting in-game
- [ ] Farmer's wife, children, farmhands, maids
- [ ] Steward (Meier/bailiff), ministerials, lords of the manor, clergy

### Farmer actions

- [x] Chop wood, dig, mow (grass), carpenter/assemble beams (hammer), sow -
      all implemented (`FigureAction` in `types.h`)
- [ ] Weaving/wattle (flechten) - needed for willow-based basket/wattle-wall
      work once willow is added (see Trees & Shrubs)
- [ ] Lime burning (Kalk brennen)
- [ ] Longer-term: every character role above (not just the farmer) needs
      access to whichever of these actions makes sense for them, not just
      the current single farmer type

## Social Classes / Estate Structure

Deprioritized for now, later.

Mapping the social strata on the estate, each with a different farm
size/equipment. Foundation exists (`MansusLevel` enum in `types.h`: HOUSLER,
QUARTER_HUFNER, HALF_HUFNER, FULLHUFNER, RICH_FARMER; plots are placed and
bounded with corner boundary-stone markers already), but only the base
Häusler level is actually used/differentiated so far - the other levels
aren't wired to any distinct size/equipment logic yet.

- [ ] Häusler (cottager — only a house, no hide/land share)
- [ ] Halbhufner (half-hide farmer)
- [ ] Vollhufner (full-hide farmer)
- [ ] Rich farmer (multiple hides)
- [ ] Lord of the manor (Gutsherr)
- [ ] Slaves (historically part of the early-medieval manorial system
      (Villikation), even though the topic is uncomfortable — serfs/unfree
      laborers are part of this economic system)

## Trees & Shrubs

Oak is the current template species, done fully before starting any other
species: wind sway [x] done; season variants (autumn colors, bare winter
look) [x] done, all 4 seasons incl. sway; felled/chopped-down state -
currently working on the animations for this. Once the full oak pipeline is
proven out, repeat per species below. Forest (see Terrain above) is being
worked on alongside this, tree-species-first.

- [ ] Shrub
- Deciduous: oak (in progress, see above), beech, maple, ash, elm
- Fruit trees: apple, pear, quince, cherry, plum (Zwetschge), mirabelle plum,
  walnut, hazelnut
- Willow - important beyond just a tree: wattle (geflochten) walls and
  baskets are made from it, ties into the "flechten" farmer action above
- Conifers: spruce (Fichte), larch (Lärche), fir (Tanne), pine (Kiefer)

## Crops

- [x] Summer wheat
- [ ] Winter wheat
- [ ] Lentils, lettuce
- [ ] Turnips/rutabaga, cabbage, onions
- [ ] Fava beans
- [ ] Hops, wine grapes
- [ ] Flax, hemp
- [ ] Woad (dye plant)
- [ ] Herb garden

## Animals & Stables

- [ ] Pigs, goats, sheep, cows
- [ ] Oxen (for pulling the plow), horses
- [ ] Chickens, geese, bees
- [ ] Corresponding stables

## Buildings

Currently working on assets for the buildings below.

- [ ] Fields
- [x] Mansus claim
- [x] Living house
- [x] Barn (Scheune)
- [ ] Church
- [ ] Threshing facility
- [ ] Mill
- [ ] Well
- [ ] Bakehouse (communal oven)
- [ ] Manor house/Fronhof (center of the Villikation)
- [ ] Fish pond (artificially built, more of a manorial feature)
- [ ] Blacksmith — open question whether village-level or centralized at the
      Fronhof; deprioritized for now

## Farming Mechanics

- [x] Sowing should cost seed grain: placing a field
      (`build_object.c`, `update_field_action`) set `field_condition =
      SOWED` for free before, nothing deducted `mansus->goods.grains`, so
      sowing was unlimited regardless of stock. Fixed: field placement now
      costs a flat `SEED_GRAIN_COST` (currently 112, tuned by hand against
      how low harvest yields used to be - still probably on the generous
      side but anything stricter starts feeling too harsh), checked both
      in the Mansus eligibility highlight (red if not enough grain) and
      again before actually placing the field (blocks it if short,
      otherwise deducted). New Mansions start with `SEED_GRAIN_COST`
      grains so the very first field isn't a chicken-and-egg deadlock.
      Still open:
      `FIGURE_ACTION_SOW` only drives the walk animation/speed
      (`update_figure.c`) - it doesn't gate the `field_condition`
      transition, that still happens instantly on field placement rather
      than through an actual farmer sowing action; and there's still no
      re-sowing path once a field goes FALLOW (see Field Growth States
      above), so this only covers the one-time initial sowing per Mansus
      for now.
- [ ] Fallow rotation (Brache): fields are always split in two
      (fallow/cultivated), only half is farmed per year — later maybe a
      three-field system
- [ ] Commons (Allmende): a communally used zone (forest/meadow/river) where
      every farmer of the Villikation may fish, hunt small game, and use
      fruit trees — not a buildable area, but a usage-rights marker on a
      region

## Rendering / Sorting

- [ ] Object draw-order heuristic: `compare_object_order` (drawing.c) falls
      back to a depth-sum tiebreak (`frontmost_point + backmost_point`)
      whenever two objects' diagonal depth ranges overlap. Fine for 1x1
      objects, but can misorder adjacent multi-tile-footprint buildings -
      observed with a barn and living house standing side by side on the
      same Mansus yard, wrong layering right at the seam where they touch.
      Option worth trying: two non-overlapping axis-aligned footprints are
      always separable along at least one axis (x or y), so replacing the
      current range-check + depth-sum combo with a proper separating-axis
      test would be both simpler and exact for footprint objects:
      `oa->max_tx <= ob->min_tx || oa->max_ty <= ob->min_ty` (and the
      mirrored check for b). Needs `ObjectOrder` to carry
      `min_tx/max_tx/min_ty/max_ty` instead of just the combined
      `frontmost_point`/`backmost_point`. Remaining tiebreaks (`z`,
      `is_ground_decor`, `is_figure`, `is_puddle`) still needed as fallback
      for the case where footprints genuinely overlap (e.g. a farmer
      walking through a building's footprint).
- [ ] Building-placement floor-area highlight (yellow/red tint for
      `BUILD_LIVING_HOUSE`/`BUILD_BARN`/`BUILD_FIELD`, drawn via immediate
      `draw_diamond()` calls in `build_object()`/`update_field_action()`) is
      called *after* `draw_scene()` in main.c's frame loop. So the tint
      paints directly on top of whatever `draw_scene()` already rendered,
      including building sprites already standing on that Mansus yard,
      instead of being layered underneath them the way normal ground tiles
      are. Visible result: a building already on a highlighted (red) yard
      ends up only partially tinted - wherever the flat ground diamond
      lands overpaints the sprite, wherever the sprite rises above the
      diamond shape stays untouched. Fix needs the highlight to become part
      of `draw_scene()`'s tile layer (drawn after ground textures, before
      the sorted object pass), not a standalone call issued after the whole
      frame - likely means threading the highlight regions/colors into
      `draw_scene()`/`draw_tiles()` instead of calling `draw_diamond()`
      straight from `build_object()`.

## General

- [ ] `safe_malloc`/`safe_calloc`/`safe_realloc` wrapper family: fail-fast
      (print + `exit(1)`) on OOM instead of the current unchecked calls.
      Roughly 10 raw `malloc`/`calloc`/`realloc` sites today, none check
      their result: `a_star.c`, `flood_fill.c`, `map.c`,
      `containers/arrays.c` (growable-array `realloc`s). Deliberately not
      pairing this with a broader error-code-return convention across
      `void` functions - decided against that, see
      `feedback_error_handling_approach` in Claude's memory for the
      reasoning.
- [ ] White pixels sometimes visible at grass tile seams - root cause found:
      not a rendering bug. The flower/highlight speckles are baked directly
      into the flat grass PNGs (e.g. `Images/meadow_spring/flat/tile_01.png`
      has several white/purple/yellow flower pixels sitting right on the
      tile's alpha edge - 19 of its 34 light-colored pixels are on the
      boundary ring alone). This only became visible because grass tiles
      draw their diamond outline commented-out (`drawing.c:150`,
      `// draw_diamond_outline(...)`) - road (line 174) and dirt (line 215)
      still draw theirs. With the outline gone, two differently-colored
      grass variants butt directly against each other, and an edge flower
      from one tile reads as a stray white speck at the seam. Not fixed yet
      - options: re-enable the grass outline (undoes the seamless-meadow
      look it was presumably removed for), or nudge the flower sprites in
      the source PNGs away from the outermost edge pixels.
