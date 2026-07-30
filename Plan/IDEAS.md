# Ideas

Rough, undeveloped gameplay ideas — not yet checked against GAME_DESIGN.md
for consistency, not yet vetted or scoped. Park loose thoughts here before
they're mature enough for TODO.md.

## Mansus labor as directly-controlled Siedler-style workers (2026-07-16)

Two-tier economy, player as steward/administrator rather than farmer:

- Mansen (peasant farms) are self-sufficient: the player only builds
  houses/barns and assigns fields to a Mansus. The people living there tend
  their own fields automatically — no direct player control over
  subsistence farming. Field state advances via the season/time clock, not
  via a player-driven work tick.
- Each Mansus spawns one or more workers ("Arbeiter") at its location,
  scaled by `MansusLevel` (higher level = more workers). These are the only
  figures the player directly commands — reuses the
  selection/swarm-move/contextual-action system already built
  (`selection/`, `units/command_selected_units`) to have them develop the
  player's own manor (Herrenhof): chopping wood, building, etc.
- Possible mechanic: a spawned worker is only available for 1-3 months,
  then needs replacing — ties worker turnover to the season/time clock.
- Design intent: upgrading a Mansus should feel directly rewarding (more
  workers = more capacity for the player's own projects), not just an
  abstract number going up.
- Depends on: Seasons & Time clock (for both field auto-progression and
  worker turnover).
- Open question: how this reconciles with GAME_DESIGN.md's existing
  Familia/task-assignment model (task lists per familia) — this idea
  proposes direct unit control instead, at least for the player's own
  workers. Not yet reconciled.

## Rough, attribute-based combat (2026-07-16)

Keep it simple: figure hits figure, whoever has the better attributes wins.
No tactical depth planned.

- Reuses the exact interaction pattern already built for chopping wood:
  right-click an enemy -> selected figures walk adjacent (same
  `map_free_tiles_near` ring search) -> on arrival, `pending_action`
  triggers a fight instead of `FIGURE_ACTION_CHOP`.
- New pieces actually needed: (1) some faction/hostility marker on Object
  (nothing like this exists yet — everything is implicitly the player's);
  (2) a number to compare for "better attributes wins."
- For (2): don't invent a new attribute system — hang combat strength off
  the armament-by-social-rank idea already in GAME_DESIGN.md ("the higher a
  farmer's social rank, the better their required armament"). Reuses an
  existing design thread instead of adding a parallel one.
- Losing figure is removed the same way `delete_object` already
  swap-removes objects.
- Open question: who's the enemy (bandits, rival village, a historical
  event trigger)? Not decided.

## Weather: state on objects/tiles, not a floating overlay (2026-07-16)

Trigger: Pharaoh's drifting cloud shadows were maddening; rain was worse
(at least it was toggleable). Credit where due — still subtly done. Goal
for Carolus: avoid the "foreign layer floating over the scene" feeling
entirely, rather than just toning it down.

Split weather into two different kinds of effect instead of one global
overlay pass:

- **State (slow, persistent)** — e.g. wet ground. A moisture/puddle value
  per tile that builds up during rain and fades after, shown as tile
  variant swaps (same mechanism as the existing `tile_variant` terrain
  system). No animation required, just a state.
- **Motion (fast, transient)** — falling raindrops, blowing dust/debris
  ("wie im Western"). Drawn very close over the objects/tiles themselves,
  as part of their normal draw call in `draw_scene`'s existing per-tile/
  per-object sorted drawing — not a separate global overlay pass. No
  clouds drawn at all.
- Wind as tile-variant cycling (e.g. 4 "swayed grass" variants) needs a
  traveling wave, not independent per-tile randomness — e.g.
  `sin(x*a + y*b - time*speed)` picking which band of tiles currently
  shows the swayed variant. Independent per-tile flicker reads as noise,
  not wind.
- Which tiles/objects show a motion effect (a drop, a dust puff) on a
  given frame should come from a stable hash of position + a slowly
  advancing time phase (same principle as `tile_variant`'s position hash),
  not fresh randomness every frame — otherwise it flickers instead of
  reading as passing gusts/showers.
- Trade-off vs. the overlay+color-tint approach already sketched in
  TODO.md's Weather section: more art needed (swayed-grass variants,
  wet-tile variants, per-object motion sprites) but avoids the exact
  "detached floating layer" feeling that's explicitly the thing to avoid
  here. TODO.md's Weather section should be revisited/rewritten once this
  direction firms up.

## Living house: bigger, plus chimney smoke as a wind indicator (2026-07-17)

Living house needs a size increase anyway (independent of weather). Once
it's bigger, give it a chimney with smoke — the smoke's drift direction
doubles as a passive wind indicator, no extra UI needed.

- Explicitly **not** a particle system — same asset-driven animation
  pattern already built for grass/oak wind-sway: pre-rendered frame
  sequences per compass direction, loaded and swapped like
  `grass_tuft_sway`/`oak_sway`.
- Different loop shape than grass/oak though: those are one-way
  press-and-release cycles that only play while windy. Smoke needs to rise
  continuously and seamlessly regardless of wind, with only the drift
  angle changing per `wind_direction` — plus a separate calm/straight-up
  loop for no-wind, since none of the 8 lean directions represent "rises
  straight up."
- Depends on: house resize art existing first, then a smoke frame set per
  compass direction (`Images/houses/living_house/animations/smoke/<dir>/
  frame_000..008.png`) plus a `calm/` variant, mirroring the grass/oak
  loader pattern in `textures.c`.
- Parked until wheat's animation set is in good shape first.
