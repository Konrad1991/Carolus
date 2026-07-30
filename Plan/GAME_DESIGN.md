# Game Design

High-level design notes on core gameplay loops - the "why" behind TODO.md's
feature list. Closest reference points: Sierra's Pharaoh/Caesar/Zeus city
builders (production-chain depth) and Age of Empires/Settlers (visible
units doing assigned work).

## MVP (Minimum Viable Product) / Build Order

Everything below this section is the full design. To get to something
playable first, the initial build cuts scope down to the core loop only:

- **Fixed starting area** - no Zoning system yet, no forest clearing. The
  map/land available for Mansen and fields is predefined for the MVP.
- **Mansen + Fields + Task assignment only** - settle familias onto
  Mansen, assign them fields, watch the automatic stage chain
  (plow -> sow -> grow -> harvest -> thresh -> mill) play out. This is
  the "farmers settle and farm" loop.
- **Deferred to later:** Zoning (Allmende/forest), Promotion, Obligations,
  Trade, Research, Yield Variance beyond the basics, Random Events. These
  layer on top once the core assignment loop feels right.

## Player Activities (meta level)

1. **Zoning**
   divide land into:
    * commons (Allmende)
    * forest
      * plain
      * Allmende-Wald
      * Sall-Wald
2. **Building**
   place houses, barns, church, mill, etc.
3. **Task planning**
    * Assign a number of tasks to familias (see below)
4. **Promotion**
  * When the requirements for a promotion are met the player has to approve this.
5. **Obligations to higher authority**
   * deliver warriors
     Idea: the higher a farmer's social rank, the better their required
     armament (this was historically mandated - free full-hide farmers
    (Vollhufner) had to fight, unfree laborers did not). This creates a
     second axis of pressure: assign resources well enough that your
    families can actually afford what's demanded of their rank.
  * deliver goods from the Mansen to the Demesne
  * Thus, the player has to balance goods between Mansen and Demesne
6. **Trade**
  * decide what gets sold and what needs to be bought in
7. **Research**
  * In monasteries one has a small research tree (new buildings, plants, devices)

## Zoning

- The player defines specific zones:
  * Allmende
  * forest

Forest splits into three zones:
- **forest** (plain, unzoned forest) - can be cleared. Clearing a Wald tile
  is the core growth mechanic: it frees up space and delivers wood as a raw resource.
- **Allmende-Wald** (commons forest) - reserved for pannage (Schweinemast,
  fattening pigs on acorns/beechmast), petty/low hunting rights
  (niedere Jagd), gathering firewood
  --> available to the Familia.
- **Sall-Wald** (demesne forest) presumably high hunting (hohe Jagd)
  as the noble counterpart to Allmende-Wald's niedere Jagd (not yet detailed).
  --> available for the *lord*

- Thus, forest is a resource in three ways.
  * forest: can be cleared and delivers timber and space.
  * Allmende-Wald: required for pannage, firewood, and low hunting
  * Sall-Wald: high hunting. Which delivers food but is also an event

## Buildings

- Mansen and the Herrenhof are not buildings itself.
  Instead the player defines a zone for the Herrenhof and individual zones for the Mansen.
  Afterwards, one familia is assigned to each Mansus.
  The idea is that the player begins with a group of people/familias standing in the mid of the forest.
  The player sets buildings on area of the Herrenhof or the Mansus respectively. At the beginning
  a house where the people are living is placed. In order to promote a Mansus or the Herrenhof
  certain houses such as a barn or a well are required.
- Furthermore, fields are assigned to each Mansus and to the Herrenhof.
  A field is always split in two parts. One for growing plants and the other is used for animals.

## Task planning

**Chosen approach:** a household ("Familia") abstraction.

- The Villikation has a bunch of Mansen.
- The player assigns each Mansus a set of tasks:
    * Familia 1: commons-apples, commons-fishing; common-Zeidler; Sall-Field1;
    * Familia 2: common-cherries; Clear wood; blacksmith; Sall-Field2;
    (Not 100% historically accurate, but a game needs playable constraints.)
- Thus, each familia has certain task which potentially run forever for example:
  commons-fishing, and commons-apples. On the other hand tasks like clear a certain
  area needs a certain amount of time but is afterwards finished. By assigning clear wood
  the familia itself gets timber. But a certain proportion of the timber goes to the Herrenhof.
  Maybe, the player decides how much goes to the Herrenhof.
- The player can also tell familias to restart a field. For example the winter wheat is destroyed
  by bad weather, then it's the responsibility of the player to see this and tell the familia
  to grow summer wheat on it.
- However, all the steps related to fields such as(plow, sow, grow, harvest, thresh, mill) runs automatically.
- **Plowing is tier-dependent.** Häusler and Viertel-Hufner have no draft animal (see
  Promotion below), so they plow by hand (spade/hoe) - historically accurate, since even
  Mansen with an ox rarely used it on a small garden plot anyway. Only from Vollhufner on
  (first tier with "one ox for the plow") does ox-drawn plowing apply.
  Visual fallback if an ox+plow+farmer composite sprite proves too hard to generate cleanly:
  animate the ox+plow as its own sprite moving across the field, and place the farmer
  standing near the field rather than visually guiding the plow. Not 100% accurate, but the
  part that matters gameplay-wise (the ox is doing the pulling) still reads correctly.

## Promotion

- The following states for Mansen are planned (not set in stone)
  * Häusler:
    buildings: house
    land: garden
    animals: beehive, few sheep, goats, pigs, and/or cows
    war equipment: nothing
  * Viertel-Hufner:
    buildings: well
    land: medium garden, very small field
    animals: same as Häusler
    war equipment: spear
  * Halb-Hufner:
    buildings: stable
    land: large garden, small field
    animals: small herd of pigs, and some cows
    war equipment: spear + shield
  * Vollhufner:
    buildings: mill
    land: large garden, large field
    animals: medium herd of pigs, one ox for the plow
    war equipment: spear + shield, sword, chain mail
  * Rich farmer:
    buildings:
    land: large garden, several large fields
    animals: huge herd of pigs, at least 2 oxen, at least one horse
    war equipment: same as Vollhufner + horse

- States of the Herrenhof
  * Simple Herrenhof:
    buildings: One large house, Chapel
    land: garden, several fields
    animals: huge herd of pigs, sheep, cows, and 2 oxen, at least two horses
    war equipment: same as rich farmer
  * Medium Herrenhof:
    buildings: well, mill, bakery, stables, barn
  * Large Herrenhof:
    buildings: weavery, medium Church

- States of Monastery
  * simple Monastery: same as simple Herrenhof
  * medium Monastery: same as medium Herrenhof + Medium Church + Herb Garden
  * large monastery:
    buildings: weavery, large Church, Skriptorium


## Tribute & Demesne (Salland) System

Deliveries are owed upward to two destinations: the monastery (Kloster) and/or
the manor house (Herrenhof). On top of goods tribute, labor is owed on
the lord's own demesne land (Salland) - historically worked via
obligatory labor service (Frondienst), not owned/farmed by any one
Familia.

- **Key economic rule:** only what's produced on the Salland, plus the
  Familias' Abgaben (tribute deliveries), is available to the player for
  trade.
- **Relief valve:** if a Familia is doing badly (low stores/starvation
  risk), the player needs a way to waive their Abgaben. If he does not waive
  then the farmers will eventually start a rebellion.
- **Soldiers** the lord itself has supply soldiers to the king, or in general
  a higher standing person. This is another motivation to promote Mansen.

## Yield Variance

Harvests must fluctuate:
- weather
  * wet, dry, normal
- soil moisture
- soil mineral content
- soil structure (light sandy, middle, heavy clay)
- the soil mineral content will regenerate faster if the player has animals on the Brache
- if the same crop is used again and again the mineral content will decrease
- some plants require wet soil some not. Good planning required.
- sometimes dry out of soil is required by building a drainage.

Other factors affecting the soil quality and plants on the fields:
- Fehde (feud) - open question whether this is a relevant mechanic for the early
  medieval period, needs verification.
- Raids by Vikings, Saracens, or Hungarians - this is also the natural place to make
  the armament obligation (see Player Activities/Obligations above) actually *matter*
  as a played moment rather than a background resource tax: a raid should be an event
  the player responds to using the Familias' required armament, with a real risk of
  loss (people, livestock, buildings, stores) if under-equipped, not just a static
  requirement check.
- Inventions such as the heavy plow will increase the soil quality (this could be
  invented in a monastery)
- Livestock disease (a Familia's animals sicken/die off)
- Fire (a building or field is damaged/destroyed)

