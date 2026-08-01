# Building Blocks (Brain-Dump 2026-07-29)

## Harvest Rework

- [x] Add carrying animation.
- [x] If someone was picked the farmer is walking with the basket on the back.
      Similar to pressing S + walking the farmer is sowing during walking.
- [x] Add a mechanism to assign a farmer to a mansus. This is necessary
      as the farmer has to know which fields he is allowed to harvest and
      to which mansus the yield has to be transported to.
      The farmer can also only transport the yield in case a barn was already build.
- [x] Add transport away - farmer carries bundle into the storehouse.
- [x] Update harvest:
    * [x] mowing. Currently, the farmer is standing on the middle of the tile
          which gets mowed. Instead the farmer should stand on the edge of a
          neighbour tile which is either a grass, road, or already harvested
          field tile.
    * [x] transport away. The farmer should collect the wheat. This is visualized
          by 'farmer picks sack from floor and puts it into his basket'.
          Then the farmer is walking towards the barn.
- [x] Currently, wheat plants of any stage which do not get ripe turn
      at some point into overripe plants. This does not make sense.
      Instead they should turn into destroyed plants.

## Tree Felling

- [x] Create tree-felled images.
- [x] Add mechanism: after a certain time of chopping, the tree falls.
- [ ] Transport wood to storage, analogous to the grain transport.

## Sowing & Soil

- [x] Sowing analogous to mowing: farmer walks sowing in a meandering pattern
    across the field. Only then do the plants start to grow.
- [ ] Add ploughing soil effect: increase in mineral content by a certain
    amount. If plants were not harvested, the increase is higher than if
    only grain stubble is present.
- [x] If the field is not tended, the farmland slowly turns green.

## Grain Economy

- [x] In the GameState, assign each Mansus a certain number of liters of
    summer grain.
- [ ] Sowing consumes wheat + continuous consumption through eating (not
    visualized) + harvest increases the wheat stock (threshing would be
    cool, but not for now). Wheat doesn't keep forever, let's say 2 years,
    then it's spoiled.

## House Building: Timber Frame

- [ ] The residential house is still missing a stage where only the
    foundation is shown - needs to be created.
- [ ] Farmer transports wood from storage to the foundation.
- [ ] Farmer hammers on beams. Afterwards the house transitions to the
    timber-frame stage, showing only the beams.

## Willow & Wattle

- [ ] Create images for willow (tree type).
- [ ] Farmer animation images for harvesting willow branches.
- [ ] Farmer transports willow branches to storage.
- [ ] Farmer digs and then transports clay to storage.
- [ ] Storage now includes: wheat grains, straw (increases analogous to the
    grain level), wood, willow branches, and clay. Next, the farmer
    transports willow branches to the residential house, which currently
    still consists of beams.
- [ ] Wattling animation plays - this reveals the wattle walls between the
    beams.

## House Building: Clay

- [ ] Transport clay to the construction site.
- [ ] Create clay-slapped-onto-wall animation.
- [ ] Farmer slaps clay onto the wall - this makes the wattle disappear and
    brown clay walls become visible.

## House Building: Roof

- [ ] Farmer transports straw to the house.
- [ ] Farmer-thatches-straw animation - this adds the thatched roof.

## Lime Kiln

- [ ] Create rock (specifically: limestone) as an object.
- [ ] Create limestone mining animation with a pickaxe.
- [ ] Create lime kiln building images.
- [ ] Farmer mines limestone.
- [ ] Farmer delivers limestone to storage.
- [ ] Farmer delivers limestone and wood to the lime kiln.
- [ ] Kiln burns - simply show smoke.
- [ ] Create well asset.
- [ ] Farmer draws water from the well and transports it to the lime kiln.
- [ ] Steam cloud appears, illustrating the slaking.
- [ ] Farmer transports slaked lime to storage.
- [ ] Farmer transports slaked lime to the construction site.
- [ ] Create plastering animation.
- [ ] Farmer plasters the wall, which finishes the house and gives it white
    walls.

## Grass Trampling

- [ ] Create trampled grass and wheat (sprite/asset) --> maybe later
- [ ] Add trampling mechanic caused by figures. --> maybe later

## Conclusion

In summary, we now have the following resources in storage: wood, water,
lime, wheat, straw, clay, etc.
