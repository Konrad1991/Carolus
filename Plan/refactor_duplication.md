# Duplication cleanup (queued from code audit 2026-08-01)

Six duplication spots found during a full-codebase audit, in priority order.
Not started yet — pick these up next session.

## 1. Route setup functions in `src/units/units.c`

`start_harvest_route`, `start_dig_route`, `start_sow_route` are byte-for-byte
identical except which route struct they write into:

- `start_plow_route`   lines 72-103  (core dup: 77-99, 23 lines)
- `start_harvest_route` lines 150-188 (core dup: 155-179, 25 lines)
- `start_dig_route`     lines 190-223 (core dup: 195-219, 25 lines)
- `start_sow_route`     lines 225-259 (core dup: 231-255, 25 lines; one extra
  grain-cost line)

All four compute `min_tx/max_tx/min_ty/max_ty` from `field->corners_field`,
pick the entry corner nearest the figure, set
`row_along_tx = (max_tx-min_tx) >= (max_ty-min_ty)`, then branch on
`row_along_tx` to init row/cursor/step_dir/sweep_dir the same way.
`start_plow_route` uses `step_coord`/`sweep_positive` instead of
`row`/`cursor`/`sweep_dir` but is the same algorithm.

Idea: a shared helper that takes the field + figure position and returns the
bounds/row_along_tx/entry point, called by all four, each then filling in
its own route-specific fields (timers, phase enum, etc).

## 2. Row-sweep cursor advance in `src/drawing/update_figure.c`

- `advance_harvest_cursor` lines 459-478 (20 lines)
- `advance_sow_cursor`     lines 556-573 (18 lines)
- `advance_dig_cursor`     lines 601-620 (20 lines)
- `harvest_route_field_tile` (237-240) / `dig_route_field_tile` (596-599):
  identical 4-line cursor/row -> tx/ty mapping (also inlined in
  `advance_sow_cursor` 570-571)

All three: try `cursor + sweep_dir` within row bounds; if out of range, step
`row + step_dir` (fail if exhausted), flip `sweep_dir`; else advance
`cursor`; map `(row, cursor)` back to `(tx, ty)`.

Idea: one `advance_row_sweep_cursor(...)` used by Harvest/Sow/Dig.
`PlowRoute`'s `advance_plow_route` (203-234) is a related but structurally
different variant (whole-row-at-a-time, not tile-by-tile) — leave that one
alone unless it turns out to fit the same shape.

## 3. Tile-rect highlight draw loop

Same iso-projection + diamond-draw block repeated wholesale:

- `build_object.c:324-332`  (BUILD_MANSUS ghost preview)
- `build_object.c:338-352`  (BUILD_LIVING_HOUSE/BARN eligibility highlight)
- `build_object.c:445-453`  (BUILD_FIELD ghost preview)
- `build_object.c:468-483`  (field-assignment mansus eligibility highlight)
- `drawing.c:494-502`   (`draw_mansus_assign_flash`)
- `drawing.c:511-520`, `525-535` (`draw_mansus_selection_highlight`, yard +
  field)
- `build_road.c:9-20` (`draw_path_preview`, single-tile variant)
- `drawing.c:549-550` (`draw_field_status_icon`, single-tile variant)

Shape: `for fy in [min,max] for fx in [min,max] { tile = map_tile; sx,sy =
iso projection with z offset; draw_diamond(...) }`.

Worth noting: the projection math here duplicates `iso_to_screen()`
(`drawing_helper.c:28-33`) but with a different y-convention (z-height
subtraction, no `+TILE_H`) — none of these sites call it. Check whether
`iso_to_screen` should just grow a z-aware variant instead of leaving this
reimplemented 8 times.

Idea: `highlight_tile_rect(TileState, Map*, min_tx,max_tx,min_ty,max_ty,
fill_color, outline?, outline_color)`.

## 4. Toggle-button widget

- `draw_scenario_button` — `weather_scenario.c:137-149`
- `draw_speed_button` — `game_time.c:21-35`
- `draw_overlay_button` — `soil_overlay.c:15-26`

Identical bodies (hover via `CheckCollisionPointRec`, active/hovered/default
fill, rounded rect + border, centered label, return hovered). Differences are
cosmetic (`draw_speed_button` takes a `vscale` for font size;
`draw_overlay_button` hardcodes font size).

Paired "commit selection on click" loops, same shape
(`if released: for i in COUNT: if hovered[i] { current = i; ...; return }`):

- `update_weather_scenario_state` — weather_scenario.c:167-175
- `update_soil_overlay_state` — soil_overlay.c:53-61
- `update_game_speed_state` — game_time.c:57-66
- `update_md` — mode.c:3-9 (variant: toggles back to MOVEMENT on re-click)
- tail of `update_weather_state` — weather.c:244-249

Idea: one `draw_toggle_button(...)` + one `pick_hovered(bool *hovered, int
count, int *out_index)`.

## 5. Icon button with press-offset

- `sidebar.c:draw_icon` lines 78-113
- `weather.c:draw_weather_icon_frame` lines 53-80

Same hover/press/color logic (including identical hardcoded RGB triples for
default/pressed/hovered), same offset-on-press behaviour, same border draw.

Idea: shared `draw_pressable_icon(...)` helper.

## 6. Field-tile iteration in `src/soil/soil.c`

- `drain_cultivated_fields` lines 166-184
- `regenerate_resting_fields` lines 186-204
- `leach_bare_fields` lines 210-228

Identical mansus -> field -> tile-bounds triple loop, differing only in the
predicate (`field_is_actively_growing` / negated / `field_is_bare`) and the
per-tile effect.

The inner tile-bounds double loop (`for y in field bounds: for x in field
bounds: tile = map_tile(...)`) alone is duplicated further in:

- `weather_scenario.c:54-55` (`average_field_soil_water`)
- `crop_growth.c:66-67` (`compute_field_averages`)
- `crop_growth.c:260-261` (`update_fallow_overgrowth`)
- `minimap.c:39-40` (`paint_fields`)
- `build_object.c:72-73` (`delete_field`)

Idea: a `for_each_field_tile(GameState*, Map*, predicate, per_tile_fn, ctx)`
iterator (callback-based, since C has no closures) for the outer
mansus/field/predicate wrapper, and consider whether the inner bounds loop
alone deserves its own small helper too (used in 5 places beyond soil.c).

---

Not queued (deferred, not forgotten):
- item 7 from the audit (array push/remove/free duplication in arrays.c,
  `object_in_field` predicate copy-pasted 4x, `figure_walk_to` reset tail,
  `load_signs` 6x texture-load block) — smaller wins, pick up separately.
- the dead-field audit (`Familia`/`mansus_level` chain, `Field.crop`,
  `PathResult.cost`, `FloodField.len`, `Texture_State.earth`, unused `Goods`
  fields) — separate topic from duplication, still open.
