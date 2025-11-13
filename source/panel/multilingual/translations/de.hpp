// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

// This file is also dual licensed under the Apache License, Version 2.0. You may obtain a copy of the License at <http://www.apache.org/licenses/LICENSE-2.0>

#pragma once

#include <array>

namespace zlpanel::multilingual::de {
    static constexpr std::array kTexts = {
        "Loslassen: Band umgehen (Bypass).",
        "Drücken: Band solo schalten.",
        "Filtertyp auswählen.",
        "Filterflanke auswählen. Eine höhere Flankensteilheit bewirkt eine steilere Filterkurve.",
        "Stereo-Modus auswählen.",
        "Frequenz einstellen.",
        "Grundverstärkung (Base Gain) und Zielverstärkung (Target Gain) einstellen.",
        "Gütefaktor (Q) einstellen. Je größer der Q-Wert, desto schmaler ist die Bandbreite.",
        "Drücken: Dynamikverhalten einschalten.",
        "Klick: Band ausschalten.",
        "Loslassen: Dynamikverhalten umgehen (Bypass).",
        "Drücken: Dynamisches Lernverhalten einschalten.\nLoslassen: Dynamisches Lernverhalten ausschalten und Threshold/Knee setzen.\nDetails im Handbuch.",
        "Drücken: Relatives Dynamikverhalten einschalten.\nDetails im Handbuch.",
        "Drücken: Side-Chain Stereo-Modus ändern.",
        "Schwellenwert (Threshold) des Dynamikverhaltens einstellen.\nDetails im Handbuch.",
        "Breite der Kennlinie (Knee) des Dynamikverhaltens einstellen.\nDetails im Handbuch.",
        "Attack-Zeit des Dynamikverhaltens einstellen.\nDetails im Handbuch.",
        "Release-Zeit des Dynamikverhaltens einstellen.\nDetails im Handbuch.",
        "Drücken: Band mit dem Side-Chain-Band koppeln.",
        "Side-Chain-Filtertyp auswählen.",
        "Side-Chain-Filterflanke auswählen.",
        "Side-Chain-Filterfrequenz einstellen.",
        "Side-Chain-Filter-Gütefaktor (Q) einstellen.",

        "Loslassen: Plugin umgehen (Bypass).",
        "Drücken: Externe Side-Chain verwenden.\nLoslassen: Interne Side-Chain verwenden.",

        "Zusätzliche Ausgangsverstärkung (Output Gain) einstellen.",
        "Skalierung der Grund- und Zielverstärkung aller Filter einstellen.",
        "Drücken: Statische Gain-Kompensation (SGC) einschalten. SGC ist ungenau, beeinflusst aber nicht die Dynamik.",
        "Drücken: Messung der integrierten Lautheit (Integrated Loudness) des Eingangs- und Ausgangssignals starten.\nLoslassen: AGC ausschalten und Ausgangsverstärkung (Output Gain) auf die Differenz der beiden Lautheitswerte setzen.",
        "Drücken: Automatische Gain-Kompensation (AGC) einschalten. AGC ist langfristig genauer, beeinflusst aber die Dynamik.",
        "Drücken: Phase des Ausgangssignals umkehren.",
        "Lookahead des Side-Chain-Signals einstellen.",

        "Drücken: Spektrumanalysator für Eingangssignal einschalten.",
        "Drücken: Spektrumanalysator für Ausgangssignal einschalten.",
        "Drücken: Spektrumanalysator für Side-Chain-Signal einschalten.",
        "Abklinggeschwindigkeit (Decay) des Spektrumanalysators auswählen.",
        "Neigung (Tilt) des Spektrumanalysators auswählen.",
        "Drücken: Freeze-Funktion einschalten. Maus über den Analysator bewegen, um das Spektrum einzufrieren.",
        "Drücken: Kollisionserkennung einschalten.",
        "Stärke der Kollisionserkennung einstellen.",

        "Filterstruktur auswählen.",
        "Die klassische, digitale Standardstruktur. Nicht für aggressive Automation geeignet.",
        "Die stabile Struktur, die in Synth-Filtern und Frequenzweichen verwendet wird. Geeignet für aggressive Automation. Verursacht große Phasenverschiebungen.",
        "Sanfte Kuhschwanz- (Shelf) und Glockenfilter (Peak) werden parallel verarbeitet. Effiziente & natürliche Dynamikbearbeitung.",
        "Ahmt den Amplituden- und Phasengang analoger Prototypen nach. Filterparameter in diesem Modus NICHT automatisieren.",
        "Ahmt den Amplituden-Gang analoger Prototypen nach und bereinigt die Phase in den Höhen. Filterparameter in diesem Modus NICHT automatisieren.",
        "Ahmt den Amplituden-Gang analoger Prototypen nach und bereinigt die Phase in den Mitten und Höhen. Filterparameter in diesem Modus NICHT automatisieren.",

        "Doppelklick: UI-Einstellungen öffnen.",

        "Drücken: Equalizer-Anpassung-Panel öffnen.",
        "Klick: Zielkurve als Preset-Datei speichern.",
        "Auswählen: Zielkurve auswählen.",
        "Zielkurve vom Side-Chain-Signal lernen.",
        "Zielkurve als flache Linie setzen.",
        "Zielkurve aus einer Preset-Datei laden.",
        "Verschiebung der Differenzkurve einstellen.",
        "Glättung der Differenzkurve einstellen.",
        "Neigung der Differenzkurve einstellen.",
        "Klick: Anpassungsprozess (Fitting) starten.",
        "Anzahl der Bänder einstellen.",
        "Das Equalizer-Anpassung-Modell analysiert das Hauptsignal (Main) und das Side-Chain-Signal.\nDie Quelle (Source) muss das Hauptsignal sein, aber Sie können das Ziel (Target) wählen.\nDie Differenz zwischen Quelle und Ziel ist die Differenzkurve.\nSobald die Differenzkurve stabil ist, können Sie sie vor dem Anpassungsprozess weiter justieren.",
        "Das Equalizer-Anpassung-Modell führt den Anpassungsprozess (Fitting) durch. Dies sollte höchstens einige Sekunden dauern.",
        "Das Equalizer-Anpassung-Modell hat den Anpassungsprozess (Fitting) abgeschlossen. Sie können jetzt die Anzahl der für die Anpassung verwendeten Bänder ändern.",
        "Drücken: Differenzkurvenzeichnung aktivieren."
    };
}
