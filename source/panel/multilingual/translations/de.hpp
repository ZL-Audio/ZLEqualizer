// Copyright (C) 2026 - zsliu98
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
    inline constexpr std::array kTexts = {
        // band basic controls
        "Aktiviert/deaktiviert den Band-Bypass.",
        "Aktiviert/deaktiviert das Solo-Abhören für dieses Band.",
        "Wählt den Filtertyp aus.",
        "Wählt die Filterflanke aus. Eine höhere Flankensteilheit erzeugt eine steilere Frequenzgangkurve.",
        "Wählt den Stereo-Kanalmodus aus.",
        "Regelt die Mitten- oder Grenzfrequenz.",
        "Regelt die Grundverstärkung und die dynamische Zielverstärkung.",
        "Regelt den Gütefaktor (Q). Ein höherer Q-Wert verringert die Bandbreite.",
        "Aktiviert/deaktiviert das dynamische EQ-Verhalten für dieses Band.",
        "Schaltet dieses Band aus.",
        "Wählt das vorherige Band aus.",
        "Wählt das nächste Band aus.",
        "Aktiviert/deaktiviert die Pass-Skalierung für Tiefpass- und Hochpassfilter.",
        "Regelt die Amplitudenskalierung des Passfilters.",
        // band dynamic controls
        "Aktiviert/deaktiviert den Bypass der Dynamikbearbeitung für dieses Band.",
        "Aktiviert/deaktiviert das dynamische Threshold-Lernen. Beim Deaktivieren werden Threshold und Knee automatisch gesetzt.",
        "Aktiviert/deaktiviert den relativen Schwellenwertmodus für die Dynamikbearbeitung.",
        "Aktiviert/deaktiviert das Vertauschen der Stereo-Kanäle in der Side-Chain.",
        "Regelt den Schwellenwert (Threshold) der Dynamikbearbeitung.",
        "Regelt die Kniebreite (Knee) der Dynamikbearbeitung.",
        "Regelt die Attack-Zeit der Dynamikbearbeitung.",
        "Regelt die Release-Zeit der Dynamikbearbeitung.",
        "Verknüpft Frequenz und Q-Wert des Side-Chain-Filters mit diesem Band.",
        "Wählt den Filtertyp für die Side-Chain aus.",
        "Wählt die Filterflanke für die Side-Chain aus.",
        "Regelt die Frequenz des Side-Chain-Filters.",
        "Regelt den Gütefaktor (Q) des Side-Chain-Filters.",
        "Blendet zusätzliche Einstellungen für die Dynamikerkennung ein/aus.",
        "Regelt die Länge des RMS-Integrationsfensters für die Pegelerkennung.",
        "Regelt das Mischungsverhältnis zwischen Peak- und RMS-Pegelerkennung.",
        "Regelt die Glättung der dynamischen Hüllkurve.",
        // top controls
        "Blendet das Bedienfeld für UI-Einstellungen ein/aus.",
        "Blendet den Preset-Browser ein/aus.",
        "Blendet das Bedienfeld für Analyzer-Einstellungen ein/aus.",
        "Blendet das Bedienfeld für Ausgangseinstellungen ein/aus.",
        "Wählt die Filterstruktur aus.",
        "Klassische digitale Standardstruktur. Nicht für aggressive Automation geeignet.",
        "Stabile Struktur für Synth-Filter und Frequenzweichen. Geeignet für aggressive Automation, verursacht jedoch eine große Phasenverschiebung.",
        "Verarbeitet sanfte Shelf- und Peak-Filter parallel für eine effiziente und natürliche Dynamikbearbeitung.",
        "Bildet Amplituden- und Phasengang analoger Prototypen nach. Filterparameter in diesem Modus nicht automatisieren.",
        "Bildet den Amplitudengang analoger Prototypen nach und bereinigt die Phase in den Höhen. Filterparameter in diesem Modus nicht automatisieren.",
        "Bildet den Amplitudengang analoger Prototypen nach und bereinigt die Phase in den Mitten und Höhen. Filterparameter in diesem Modus nicht automatisieren.",
        "Aktiviert/deaktiviert den Plugin-Bypass.",
        "Aktiviert/deaktiviert den externen Side-Chain-Eingang.",
        "Blendet das EQ-Match-Bedienfeld ein/aus.",
        // preset browser
        "Öffnet den Preset-Ordner im Dateimanager des Systems.",
        "Löscht die ausgewählte Preset-Gruppe.",
        "Löscht das ausgewählte Preset.",
        "Schließt den Preset-Browser.",
        // output controls
        "Regelt die zusätzliche Ausgangsverstärkung.",
        "Regelt die Gesamtskalierung der Grund- und Zielverstärkung aller Filter.",
        "Aktiviert/deaktiviert die statische Gain-Kompensation (SGC). SGC ist näherungsweise, beeinflusst aber nicht die Dynamik.",
        "Aktiviert/deaktiviert die Lautheitsanpassung. Beim Aktivieren wird die Ein- und Ausgangslautheit gemessen; beim Deaktivieren wird die Ausgangsverstärkung auf deren Differenz gesetzt.",
        "Aktiviert/deaktiviert die automatische Gain-Kompensation (AGC). AGC ist langfristig genauer, beeinflusst jedoch die Dynamik.",
        "Aktiviert/deaktiviert die Phaseninvertierung des Ausgangssignals.",
        "Regelt die Lookahead-Zeit des Side-Chain-Signals.",
        // analyzer controls
        "Blendet das Eingangssignal-Spektrum ein/aus.",
        "Blendet das Ausgangssignal-Spektrum ein/aus.",
        "Blendet das Side-Chain-Signal-Spektrum ein/aus.",
        "Wählt die Abklinggeschwindigkeit des Spektrumanalysators aus.",
        "Wählt die Neigung (Tilt) des Spektrumanalysators aus.",
        "Wählt die Frequenzglättungsbreite des Spektrumanalysators aus.",
        "Wählt den Modus der Frequenzglättung aus.\nOCT: konstante Oktavbruchteil-Glättung.\nERB: Glättung nach äquivalenter Rechteckbandbreite.",
        "Aktiviert/deaktiviert das Einfrieren des Spektrums beim Überfahren des Analyzers mit dem Mauszeiger.",
        "Wählt den vom Spektrumanalysator angezeigten Stereo-Kanalmodus aus.",
        "Aktiviert/deaktiviert die Erkennung von Frequenzkollisionen.",
        "Regelt die Stärke der Kollisionserkennung.",
        "Blendet die numerische Wertanzeige ein/aus.",
        "Blendet das Pegelmesser-Bedienfeld ein/aus.",
        // curve scale & piano roll
        "Wählt den Dezibel-Anzeigebereich für die EQ-Frequenzgangkurve aus.",
        "Wählt die obere Dezibel-Grenze für die Analyzer-Anzeige aus.",
        "Wählt die minimale Dezibel-Untergrenze für die Analyzer-Anzeige aus.",
        "Blendet die Piano-Roll ein/aus.",
        // eq match controls
        "Speichert die Zielkurve als Preset-Datei.",
        "Aktiviert/deaktiviert das manuelle Zeichnen der Differenzkurve.",
        "Wählt die Quelle der Zielkurve aus.",
        "Lernt die Zielkurve vom Side-Chain-Signal.",
        "Setzt die Zielkurve auf eine flache Linie.",
        "Lädt die Zielkurve aus einer Preset-Datei.",
        "Setzt die Zielkurve auf eine ausgewogene Kurve.",
        "Setzt die Zielkurve auf eine natürliche Kurve.",
        "Regelt den vertikalen Versatz der Differenzkurve.",
        "Regelt die vertikale Skalierung der Differenzkurve.",
        "Regelt die Neigung (Tilt) der Differenzkurve.",
        "Wählt die maximal zulässige Verstärkungsamplitude für EQ Match aus.",
        "Startet die Anpassung von EQ-Bändern an die Differenzkurve.",
        "Regelt die Anzahl der für die Anpassung verwendeten EQ-Bänder.",
        "Haupt- und Side-Chain-Signale werden analysiert.\nWählen Sie die Quelle der Zielkurve und passen Sie die Differenzkurve an, sobald sie sich stabilisiert hat, bevor Sie die Anpassung starten.",
        "EQ-Bänder werden an die Differenzkurve angepasst. Dies dauert höchstens einige Sekunden.",
        "Anpassung abgeschlossen. Bei Bedarf kann die Anzahl der angepassten Bänder noch geändert werden.",
        " "
    };
}
