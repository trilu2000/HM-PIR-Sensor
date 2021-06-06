# HM-PIR-Sensor

dieses Repo beinhaltet alle Infos und Dateien die es zum Nachbau eines Homematic kompatiblen PIR Sensors braucht.

Der PIR-Sensor ist streng genommen ein selbstgedrucktes Gehäuse und eine Zusatzplatine für den HMSensor (https://asksinpp.de/Projekte/psi/HMSensor/). 

Die Software ist eine Abwandlung vom HM-SEC-MDIR Sketch aus den Beispielen der Asksin Library. 

![GitHub Logo](/Pictures/IMG_20210605_190158.jpg)


Ein wesentliches Highlight des PIR Sensors ist der verbaute Helligkeitssensor - Ich nutze den OPT3001 und wandle den gemessenen LUX Wert in einen HM Helligkeitswert.
Die HM Helligkeitswerte habe ich in einer Meßreihe mit einem selbstgebautem LUX-Sensor und einem HomeMatic MDIR-WM-55 ermittelt.
Das Ergebnis findet ihr in der Exceltabelle im GIT-Hub (https://github.com/trilu2000/HM-PIR-Sensor/blob/main/DOCU/PIR-Addon.xlsm)

## Benötigtes Material


HM-Sensor Platine, bis auf die LED und Vorwiderstände, bestückt

PIR Addon Platine, bis auf den AD5247, bestückt

Die Gehäuse Files gedruckt - ich habe sie aus PLA mit 25% infill gedruckt

PIR Optische Fresnel-Linse 23.3*23.3*14,7mm (https://de.aliexpress.com/item/1005001664621201.html)

Batterie-Federn (https://de.aliexpress.com/item/4001112851052.html)

