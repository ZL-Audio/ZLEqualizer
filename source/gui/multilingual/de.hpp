// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS of A PARTICULAR PURPOSE. See the GNU Affero General Public License of more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <array>

namespace zlgui::multilingual::de {
    static constexpr std::array kTexts = {
        "Drücken: Schalte das ausgewählte Band ein.\nLoslassen: Bypasse das ausgewählte Band.",
        "Drücken: Aktiviere Solo für das vom ausgewählten Frequenzband betroffene Audio.",
        "Wählen Sie den Typ des ausgewählten Frequenzbands. Peak、Low Shelf、Low Pass（Tiefpass）、High Shelf、High Pass（Hochpass）、Notch、Band Pass（Bandpass）、Tilt Shelf.",
        "Wählen Sie die Steilheit des ausgewählten Frequenzbands. Eine höhere Steilheit bewirkt, dass die Antwortkurve des Filters steiler verläuft.",
        "Wählen Sie den Stereo-Modus des ausgewählten Frequenzbands. Stereo（Stereo）、Left（links）、Right（rechts）、Mid（Mitte）、Side（Seiten）.",
        "Stellen Sie die Mittenfrequenz des ausgewählten Frequenzbands ein.",
        "Stellen Sie den Gain des ausgewählten Frequenzbands ein.",
        "Stellen Sie den Q-Faktor des ausgewählten Frequenzbands ein. Je größer der Q-Wert, desto schmaler ist die Bandbreite.",
        "Wählen Sie das Frequenzband mit den Pfeiltasten links/rechts.",
        "Drücken: Schalte das dynamische Verhalten des ausgewählten Frequenzbands ein.",
        "Drücken: Schalte den automatischen Schwellenwert des ausgewählten Frequenzbands ein.\nLoslassen: Schalte den automatischen Schwellenwert des ausgewählten Frequenzbands aus und übernehme den erlernten Schwellenwert.",
        "Klicken: Schalte das ausgewählte Frequenzband aus.",
        "Loslassen: Bypasse das dynamische Verhalten des ausgewählten Bands.",
        "Drücken: Aktiviere das Solo des Side-Chain-Audios des ausgewählten Bands.",
        "Drücken: Stelle den dynamischen Schwellenwert auf den relativen Modus ein.\nLoslassen: Stelle den dynamischen Schwellenwert auf den absoluten Modus ein.",
        "Drücken: Wechsel die Side-Chain, um sie im entgegengesetzten Stereo-Modus einzustellen.",
        "Stellen Sie den Schwellenwert des dynamischen Verhaltens des ausgewählten Bands ein.",
        "Stellen Sie die Kniebreite des dynamischen Verhaltens des ausgewählten Bands ein. Die tatsächliche Kniebreite = angezeigter Wert × 60 dB.",
        "Stellen Sie die Attack-Zeit des dynamischen Verhaltens des ausgewählten Bands ein.",
        "Stellen Sie die Release-Zeit des dynamischen Verhaltens des ausgewählten Bands ein.",
        "Stellen Sie die Mittenfrequenz des Bandpassfilters ein, der auf das Side-Chain-Signal angewendet wird.",
        "Stellen Sie den Q-Wert des Bandpassfilters ein, der auf das Side-Chain-Signal angewendet wird.",
        "Drücken: Verwenden Sie die externe Side-Chain.\nLoslassen: Verwenden Sie die interne Side-Chain.",
        "Drücken: Aktivieren Sie die Static Gain Compensation (SGC). SGC schätzt die Kompensationsmenge anhand der Filterparameter. SGC ist ungenau, beeinflusst jedoch NICHT die Dynamik des Signals.",
        "Drücken: Bypassen Sie das Plugin.",
        "Stellen Sie die Skala aller Gain-Parameter ein.",
        "Drücken: Kehren Sie die Phase des Ausgangssignals um.",
        "Drücken: Aktivieren Sie die Automatic Gain Compensation (AGC). AGC ist langfristig genau, wirkt sich jedoch auf die Dynamik des Signals aus.",
        "Drücken: Beim Drücken beginnt die Messung der integrierten Lautstärke des Eingangssignals und des Ausgangssignals.\nLoslassen: Beim Loslassen wird AGC deaktiviert und der Ausgangs-Gain auf die Differenz der beiden Lautstärkewerte aktualisiert.",
        "Stellen Sie den Ausgangs-Gain des Plugins ein.",
        "Wählen Sie den Zustand des Eingangsspektrumanalysators. FRZ kann den Analysator einfrieren.",
        "Wählen Sie den Zustand des Ausgangsspektrumanalysators. FRZ kann den Analysator einfrieren.",
        "Wählen Sie den Zustand des Side-Chain-Spektrumanalysators. FRZ kann den Analysator einfrieren.",
        "Stellen Sie die Abklinggeschwindigkeit des Spektrumanalysators ein.",
        "Stellen Sie die Steilheit des Spektrumanalysators ein (hat keinen Einfluss auf das tatsächliche Signal).",
        "Stellen Sie die Lookahead-Zeit des Side-Chain-Signals ein (durch Einführen von Latenz).",
        "Stellen Sie die Länge des RMS ein. Mit zunehmendem RMS werden Attack und Release langsamer und sanfter.",
        "Stellen Sie die Sanftheit des Releases ein.",
        "Wählen Sie den Zustand der High-Quality-Option. Wenn High-Quality aktiviert ist, passt das dynamische Verhalten den Filterstatus pro Sample an, was dynamische Effekte glättet und Artefakte reduziert.",
        "Wählen Sie den Zustand der Kollisionserkennung.",
        "Stellen Sie die Stärke der Kollisionserkennung ein. Bei erhöhter Stärke werden die erkannten Bereiche größer und dunkler.",
        "Stellen Sie die Skalierung der Kollisionserkennung ein. Erhöht man die Skalierung, werden die erkannten Bereiche dunkler.",
        "Wählen Sie die Struktur der Filter.",
        "Wählen Sie den Zustand von Zero Latency. Ist Zero Latency aktiviert, beträgt die zusätzliche Latenz null, aber die Puffergröße wirkt sich leicht auf dynamische Effekte und die Parameterautomatisierung aus.",
        "Wählen Sie die Zielkurve.\nSide: aus dem Side-Chain-Signal erlernt.\nPreset: aus einer Preset-Datei geladen.\nFlat: eingestellt als eine flache -4,5 dB/oct-Linie.",
        "Stellen Sie das Gewicht des Kurvenlernmodells ein. Je höher das Gewicht, desto stärker lernt das Modell aus dem lauteren Teil des Signals.",
        "Klicken: Starten Sie das Kurvenlernen.",
        "Klicken: Speichern Sie die aktuelle Zielkurve als Preset-Datei.",
        "Stellen Sie die Glätte der Differenzkurve ein.",
        "Stellen Sie die Steilheit der Differenzkurve ein.",
        "Wählen Sie den Fitting-Algorithmus.\nLD: lokaler, gradientbasierter Algorithmus (schnell).\nGN: globaler, gradientsfreier Algorithmus (empfohlen).\nGN+: globaler, gradientsfreier Algorithmus (langsam, erlaubt Filter mit höheren Steigungen).",
        "Klicken: Starten Sie das Kurvenfitting.",
        "Stellen Sie die Anzahl der für das Fitting verwendeten Bänder ein. Nach Abschluss des Fittings schlägt das Kurvenfitting-Modell eine Anzahl von Bändern vor, die anschließend geändert werden kann.",
        "Wählen Sie die dB-Skala der Filter-Antwortkurven (links) und der Spektrumanalysatoren (rechts).",
        "Klicken: Öffnen Sie das Equalizer-Match-Panel. Das Equalizer-Match umfasst vier Schritte:\n1. Wählen Sie die Zielkurve.\n2. Starten Sie das Lernen.\n3. Passen Sie die Differenz an.\n4. Starten Sie das Fitting.",
        "Die gebräuchlichste Filterstruktur bei Equalizern. Die Filterantwort liegt dem analogen Prototyp sehr nahe. Geeignet für Modulation.",
        "Die gebräuchlichste Filterstruktur bei Crossovers. Der Filter verursacht mehr Phasenverschiebung. Geeignet für schnelle Modulation.",
        "Peak/Shelf-Filter werden parallel geschaltet. Die Filterantwort weicht von der angezeigten Kurve ab. Geeignet für Modulation.",
        "Minimum-Phase-Struktur mit zusätzlichen Korrekturen. Die Filterantwort ist nahezu identisch mit dem analogen Prototyp. NICHT geeignet für Modulation.",
        "Minimum-Phase-Struktur mit zusätzlichen Korrekturen. Der Filter weist eine amplitudentreue Antwort des analogen Prototyps auf und verursacht weniger Phasenverschiebung im Hochfrequenzbereich. NICHT geeignet für Modulation.",
        "Der Filter hat eine amplitudentreue Antwort des analogen Prototyps und eine Null-Phasen-Antwort. NICHT geeignet für Modulation. Dynamische Effekte funktionieren nicht.",
        "Doppelklick: Öffnen Sie die UI-Einstellungen."
    };
}
