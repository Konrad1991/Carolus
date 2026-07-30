# Bauketten (Brain-Dump 2026-07-29)

## Grastrampeln & Bäume

1. Gras und Weizen plattgetrampelt erstellen (Sprite/Asset) - muss warten,
   bis das pixellab.ai-Kontingent wieder aufgefüllt ist.
2. Größere Bäume erstellen, damit sie nicht so pixelig sind.
3. Plattgetrampelt-Mechanik durch Figuren einbauen.

## Ernte-Überarbeitung

4. Größeren Bauer-Asset erstellen.
5. Trageanimation einbauen.
6. Abtransport einbauen - Bauer trägt Bündel ins Lagerhaus.
7. Ernte updaten: mähen, abtransportieren, weitermähen etc.

## Baumfällen

8. Baum-gefällt-Bilder erstellen.
9. Mechanismus einbauen: nach gewisser Zeit Hacken fällt der Baum.
10. Abtransport von Holz zum Lager, analog zum Getreide-Transport.

## Aussaat & Boden

11. Aussäen analog zum Mähen: Bauer läuft säend mäanderförmig übers Feld.
    Erst dadurch fangen die Pflanzen an zu wachsen.
12. Bilder/Tiles für Ackerboden mit Getreidestoppeln erstellen.
13. Pflügen-visuellen-Effekt einbauen: egal was vorher auf dem Feld war,
    wird es zu unbebautem Ackerland.
14. Pflügen-Bodeneffekt einbauen: Anstieg des Mineralgehalts um einen
    bestimmten Wert. Wenn Pflanzen nicht abgeerntet wurden, ist der Anstieg
    höher als wenn nur Getreidestoppeln da sind.
15. Bilder/Tiles für Ackerboden mit Getreidestoppeln + etwas Gras erstellen.
16. Wenn das Feld nicht bestellt wird, wird das Ackerland langsam grün.

## Getreide-Wirtschaft

17. Im GameState jedem Mansus eine gewisse Anzahl Liter Sommergetreide
    zuschreiben.
18. Durch Aussaat wird Weizen verbraucht + kontinuierlicher Verbrauch durchs
    Essen (nicht visualisiert) + Ernte erhöht den Weizenvorrat (Dreschen
    wäre cool, aber erstmal nicht). Weizen ist nicht ewig haltbar, sagen wir
    2 Jahre, dann ist es kaputt.

## Hausbau: Fachwerk

19. Für das Wohnhaus fehlt noch ein Stadium, wo nur die Grundfläche
    dargestellt wird - muss erstellt werden.
20. Bauer transportiert Holz aus dem Lager zur Grundfläche.
21. Bauer haut auf Balken. Danach geht das Haus in den Fachwerk-Stand über,
    in dem nur die Balken gezeigt werden.

## Weide & Flechtwerk

22. Bilder für Weide (Baumart) erstellen.
23. Bauer-Animationsbilder zum Ernten von Weidenzweigen.
24. Bauer transportiert Weidenzweige zum Lager.
25. Bauer gräbt und transportiert dann Lehm zum Lager.
26. Das Lager beinhaltet jetzt: Weizenkörner, Stroh (steigt analog zum
    Getreidestand an), Holz, Weidenzweige und Lehm. Als Nächstes
    transportiert der Bauer Weidenzweige zum Wohnhaus, das aktuell noch aus
    Balken besteht.
27. Flechtanimation wird abgespielt - dadurch sieht man jetzt die
    Flechtwände zwischen den Balken.

## Hausbau: Lehm

28. Transport von Lehm zur Baustelle.
29. Lehm-an-Wand-klatschen-Animation erstellen.
30. Bauer klatscht Lehm an die Wand - dadurch verschwindet das Flechtwerk,
    man sieht jetzt braune Lehmwände.

## Hausbau: Dach

31. Bauer transportiert Stroh zum Haus.
32. Bauer-Flechtet-Stroh-Animation - dadurch wird das Strohdach hinzugefügt.

## Kalkbrennerei

33. Gestein (genauer: Kalkstein) als Objekt erstellen.
34. Abbau-Animation von Kalkstein mit Spitzhacke erstellen.
35. Kalkbrennofen-Gebäudebilder erstellen.
36. Bauer baut Kalkstein ab.
37. Bauer liefert Kalkstein ins Lager.
38. Bauer liefert Kalkstein und Holz zum Kalkbrennofen.
39. Ofen brennt - einfach Rauch zeigen.
40. Brunnen-Asset erstellen.
41. Bauer schöpft Wasser aus dem Brunnen und transportiert es zum
    Kalkofen.
42. Dampfwolke entsteht, veranschaulicht das Löschen.
43. Bauer transportiert gelöschten Kalk zum Lager.
44. Bauer transportiert gelöschten Kalk zur Baustelle.
45. Animation fürs Verputzen erstellen.
46. Bauer verputzt die Wand, wodurch das Haus fertig wird und weiße Wände
    bekommt.

## Fazit

Zusammengefasst haben wir damit folgende Ressourcen im Lager: Holz, Wasser,
Kalk, Weizen, Stroh, Lehm, etc.
