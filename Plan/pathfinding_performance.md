- Problem: with many figures moving at once, path finding causes a noticeable
  hang. `a_star()` currently runs once per figure (see `command_selected_units`
  in `units.c`), and every call allocates fresh `pi`, `prev`, `processed`
  arrays plus the binary heap's `arr`/`pos` arrays. Map is 400x400 = 160k
  tiles, so this is not free even before the search itself runs.

- Considered: persistent scratch buffers (allocate `pi`/`prev`/`processed`/
  heap arrays once, e.g. at map init, free at shutdown) plus a generation
  counter instead of clearing `processed`/`pi` with an O(n) loop each call.
  Still a valid idea in principle, but becomes secondary once flow fields
  remove the "one A* run per figure" pattern - see below.

- Considered: multithreading the per-figure A* calls. Also secondary once
  flow fields remove the need to run pathfinding once per figure in the
  first place.

- Chosen direction: switch the multi-figure-to-shared-destination case
  (`command_selected_units` in `units.c`) from N independent A* calls to a
  single flow field.

## Flow field design

- Normal A*/Dijkstra answers "what is the best path from A to B" - one
  request, one start, one goal. A flow field answers "what is the best
  direction toward B" for every tile on the map at once. That fits the
  case where several selected figures all move toward the same clicked
  target region.

- Two phases:
  1. **Integration field = `pi`**: run the same Dijkstra/priority-queue
     expansion already in `a_star()` (`heap_extract_min`, `tile_cost`,
     neighbor expansion), but started from the destination instead of from
     the figure, with no heuristic (a full expansion is needed anyway, so
     A*'s heuristic buys nothing here), run until the heap is empty or a
     bounded region has been covered. This fills `pi[]` completely -
     cost-to-goal for every reachable tile - instead of stopping once one
     target is found.
  2. **Flow field derived from `pi`**: one pass over `pi[]`. For every
     tile on the map (not just figures' start tiles), look at its
     neighbors and record the direction toward the neighbor with the
     lowest `pi` value (steepest descent). Compare `pi` values directly,
     no extra "+cost" term, since `pi` already includes the cost of
     stepping into that neighbor. Exclude neighbors with
     `pi == TILE_IMPASSABLE`, and re-apply the diagonal corner-cutting
     check from `a_star()` (`flank_a`/`flank_b`, lines 190-196) so figures
     can't cut through blocked corners. Tiles with `pi == TILE_IMPASSABLE`
     themselves get no direction. Ties between neighbors can be broken
     arbitrarily (e.g. by their order in `dx[8]`/`dy[8]`). Store one
     direction per tile, reusing the existing `FigureDirection` enum
     already used for sprites.

- This gives one direction per tile for the whole map, computed once per
  command - not per figure and not from any particular figure's start
  tile. Figures stop walking a `solution[]` array via `path_index`.
  Instead, at every tick, each figure looks up the direction stored for
  *whichever tile it currently stands on* (that lookup tile changes as it
  moves) - O(1) per figure per step, no per-figure search at all.

- Multi-source variant: `command_selected_units` already computes distinct
  free target tiles per figure via `map_free_tiles_near`. With a flow
  field, seed the integration field with *all* of those free tiles at once
  (multi-source Dijkstra) instead of assigning one specific tile per figure
  up front. Every figure then just flows toward whichever free tile ends
  up nearest along the field - no explicit assignment step needed.

## Open questions / next steps

- How far to flood-fill: whole map vs. a bounded region around the start
  positions + destination. In the end only the figures' current start
  tiles need a resolved direction (the rest gets filled in as a
  byproduct of reaching those). So the flood fill can early-exit once
  every figure's start tile has been reached/`processed`, instead of
  always running until the heap is empty - same idea as the existing
  early exit in `a_star()`, just against a set of targets instead of one.
  Requires knowing all relevant figures' start tiles up front, before the
  flood fill begins.
- Storage/lifetime of the field: recomputed per command (destination
  changes each time), so the scratch-buffer idea above can still apply to
  the integration array itself.
- `update_figure.c` movement logic needs to change from path-array
  stepping to flow-field direction lookup.
- Local collision avoidance between figures sharing one field (steering /
  separation so they don't visually stack) - separate topic, layered on
  top later.
