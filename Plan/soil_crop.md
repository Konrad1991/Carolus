# Soil-Bilanz (soil_water, soil_minerals, soil_temp)

Aus `src/soil/soil.c`, Update pro Frame mit `dt = game_delta_time()`. Alle
Terme unten sind mit `dt` zu multiplizieren (pro Frame skaliert).

soil_water ist L (0..SOIL_WATER_CAPACITY_L = 200), soil_minerals ist g
(0..SOIL_MINERAL_CAPACITY_G = 100) - echte Mengen, keine 0..1-Anteile.
Tile-Flaeche ist als 1m² angenommen, deshalb ist Wasserhoehe in mm
zahlengleich mit Litern (1mm ueber 1m² = 1L). Beide Konstanten stehen in
soil.h.

Saettigungs-Schwellen (Init-Werte, Proximity-Floor, Keimung, Wetter-Szenario-
Trigger) skalieren mit SOIL_WATER_CAPACITY_L. Der tatsaechliche Pflanzen-
Wasserbedarf (crop_growth.c, target_water_uptake) bleibt dagegen fix in
Litern, unabhaengig von der Speichergroesse - ein groesserer Speicher gibt
also mehr Puffer aus frueheren Regenfaellen, ohne dass Pflanzen mehr
"trinken" muessten.

## Soil Water

    dW = (W_ziel - W) * r_W + proximity_term

- Bei Regen: W_ziel = 200L, r_W = 0.03
- Sonst: W_ziel = 0L, r_W = 0.033 * max(soil_temp / 20, 0)
- proximity_term: nur wenn W < 120L UND Tile grenzt an Wasser
  -> (120L - W) * 0.02
  sonst 0

## Soil Minerals

    dM = 0.03 * max(soil_temp / 20, 0) - regen_leach - anbau_entzug

- Term 1: passive Regeneration (rein, 0.03g/s), abhängig von Bodentemperatur
- regen_leach: nur bei Regen -> (M - 15g) * r_leach, sonst 0 (Auswaschung, zieht Richtung Floor 15g)
  - r_leach = 0.0015 ueberall als Basis (bewachsen/gedeckt - Cover-Crop-Studien
    zeigen ca. 40-50% weniger Auswaschung unter Bewuchs, siehe soil_crop_refs.bib)
  - +0.0015 extra (`leach_bare_fields`) nur auf Feldern in PLOWED/SOWED-Zustand
    (frisch gepfluegt/gesaet, noch kein Bewuchs) -> zusammen 0.003, wie kahler
    Boden. Wachsende Felder, ruhende Felder und Wildnis bleiben bei der Basis-Rate.
- anbau_entzug: nur bei aktiv wachsenden Feldern (SMALL/MEDIUM/LARGE_GREEN_PLANTS) -> 0.08g/s, sonst 0
- ruhe_bonus (`regenerate_resting_fields`): auf Feld-Tiles, die *nicht* aktiv wachsen -> +0.01g/s,
  kleiner Zusatz obendrauf zur Basis-Regeneration (Term 1) fuer Reststoff-Verrottung. Die Basis-Rate
  allein braucht schon ca. 11 Monate um ein leeres Feld voll aufzufuellen (100g/0.03g/s) - der Bonus
  soll das nur leicht beschleunigen, nicht verdoppeln. Nicht weil eine Pflanze die Mineralisierung
  selbst bremst (Studien zeigen eher das Gegenteil), sondern weil Brache den Ernte-Entzug wegfallen
  laesst und zusaetzlich von Reststoff-Verrottung profitiert (Yao et al. 2025, siehe soil_crop_refs.bib)

## Soil Temperature

    dT = (T_luft - T_soil) * 0.1 / (1 + 2 * wetness)

- wetness = soil_water / SOIL_WATER_CAPACITY_L (0..1), nur hier auf Anteil normiert
- Reine Angleichung an Lufttemperatur, keine Zufluss/Abfluss-Terme
- Nasser Boden (hohes wetness) bremst die Angleichung (höhere thermische Trägheit)

## Farming-bedingte Entzüge (negative Bilanzterme)

Alle folgenden Terme werden durch Farming (statt Wetter) angetrieben. Aus
`src/farming/crop_growth.c`, `update_wheat_tuft_growth` - pro Wheat-Tuft und
Frame, nur solange soil_temp im Stage-Fenster liegt (min_temp/max_temp):

    water_absorbed    += min(rate_W * dt * vigor, soil_water)   -> soil_water -= gleicher Betrag
    minerals_absorbed += min(rate_M * dt * vigor, soil_minerals) -> soil_minerals -= gleicher Betrag

rate_W/rate_M sind L/s bzw. g/s, direkt gegen das Spielgefühl getunt (nicht
durch WHEAT_TUFTS_PER_TILE geteilt - siehe Kommentar in crop_growth.c).
target_water_uptake/target_minerals_uptake (die kumulativen Ziele je Tuft)
sind durch WHEAT_TUFTS_PER_TILE=7 geteilt, weil sich alle 7 Tufts eines
Tiles denselben soil_water/soil_minerals-Wert teilen.

- Beide Entzüge sind auf das tatsächlich vorhandene soil_water/soil_minerals
  am Tuft-Tile gedeckelt (`soil_deplete_water`/`soil_deplete_minerals` in
  soil.c) - ein leerer Boden bremst also die Aufnahme und damit das
  Wachstum, statt nur eine Schwelle zu unterschreiten.
- Stage-Wechsel (YOUNG->MIDDLE->LARGE_GREEN->RIPE) erst wenn Zeit-Timer
  **und** water_absorbed **und** minerals_absorbed ihr jeweiliges Ziel
  erreicht haben.
- Zusätzlich weiterhin: **anbau_entzug** (soil_minerals, 0.08g/Tile/s,
  siehe oben) - flacher Grundentzug für jedes Tile eines aktiv wachsenden
  Feldes, unabhängig von den einzelnen Pflanzen.
- soil_temp bleibt reines Zeitfenster-Gate (kein Verbrauchsstoff, 1.
  Hauptsatz) - kein Entzug, nur Bedingung für den Aufnahme-Loop.

## Kopplungen

- soil_temp -> dry_factor -> soil_water-Abfluss
- soil_temp -> regen_factor -> soil_minerals-Zufluss
- soil_water (als wetness) -> Trägheit von soil_temp
- Regen -> soil_water-Zufluss und soil_minerals-Abfluss gleichzeitig
- Wachsende Felder -> zusätzlicher soil_water- und soil_minerals-Abfluss (pro Tuft) + soil_minerals-Grundabfluss (pro Tile) -> viele Ernten in Folge ohne Brache/Regen erschöpfen den Boden spürbar
