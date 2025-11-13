// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

// This file is also dual licensed under the Apache License, Version 2.0. You may obtain a copy of the License at <http://www.apache.org/licenses/LICENSE-2.0>

// The v1 translation is assisted by the GitHub user https://github.com/pms967.
// Please refer to the v1 commit history for details.

#pragma once

#include <array>

namespace zlpanel::multilingual::it {
    static constexpr std::array kTexts = {
        "Rilascia: bypassa la banda.",
        "Premi: attiva il solo della banda.",
        "Scegli il tipo di filtro.",
        "Scegli la pendenza del filtro. Una pendenza maggiore renderà la curva di risposta del filtro più ripida.",
        "Scegli la modalità stereo.",
        "Controlla la frequenza.",
        "Controlla il guadagno di base e il guadagno target.",
        "Controlla il fattore Q. Un valore di Q maggiore comporta una larghezza di banda più stretta.",
        "Premi: attiva il comportamento dinamico.",
        "Clicca: disattiva la banda.",
        "Rilascia: bypassa il comportamento dinamico.",
        "Premi: attiva il comportamento di apprendimento dinamico.\nRilascia: disattiva l'apprendimento dinamico e imposta Soglia (Threshold) e Ginocchio (Knee).\nConsulta il manuale per i dettagli.",
        "Premi: attiva il comportamento dinamico relativo.\nConsulta il manuale per i dettagli.",
        "Premi: cambia la modalità stereo della side-chain.",
        "Controlla la soglia del comportamento dinamico.\nConsulta il manuale per i dettagli.",
        "Controlla l'ampiezza del ginocchio (knee) del comportamento dinamico.\nConsulta il manuale per i dettagli.",
        "Controlla il tempo di attacco del comportamento dinamico.\nConsulta il manuale per i dettagli.",
        "Controlla il tempo di rilascio del comportamento dinamico.\nConsulta il manuale per i dettagli.",
        "Premi: collega la banda con la banda della side-chain.",
        "Scegli il tipo di filtro della side-chain.",
        "Scegli la pendenza del filtro della side-chain.",
        "Controlla la frequenza del filtro della side-chain.",
        "Controlla il fattore Q del filtro della side-chain.",

        "Rilascia: bypassa il plugin.",
        "Premi: utilizza la side-chain esterna.\nRilascia: utilizza la side-chain interna.",

        "Controlla il guadagno di uscita aggiuntivo.",
        "Controlla la scala del guadagno di base e target di tutti i filtri.",
        "Premi: attiva la compensazione statica del guadagno (SGC). L'SGC non è preciso, ma non influisce sulla dinamica.",
        "Premi: inizia a misurare la loudness integrata dei segnali di ingresso e di uscita.\nRilascia: disattiva l'AGC e aggiorna il guadagno d'uscita in base alla differenza tra le due loudness.",
        "Premi: attiva la compensazione automatica del guadagno (AGC). L'AGC è più accurato a lungo termine, ma influisce sulla dinamica.",
        "Premi: inverte la fase del segnale di uscita.",
        "Controlla il lookahead del segnale della side-chain.",

        "Premi: attiva l'analizzatore di spettro del segnale in ingresso.",
        "Premi: attiva l'analizzatore di spettro del segnale in uscita.",
        "Premi: attiva l'analizzatore di spettro del segnale della side-chain.",
        "Scegli la velocità di decadimento degli analizzatori di spettro.",
        "Scegli la pendenza (tilt) degli analizzatori di spettro.",
        "Premi: attiva la funzione di congelamento (freeze). Passa il mouse sopra l'analizzatore per congelare lo spettro.",
        "Premi: attiva la Rilevazione delle Collisioni.",
        "Controlla l'intensità della Rilevazione delle Collisioni.",

        "Scegli la struttura dei filtri.",
        "La struttura digitale classica standard. Non adatta per automazioni aggressive.",
        "La struttura stabile usata nei filtri dei synth e nei crossover. Adatta per automazioni aggressive. Causa un notevole sfasamento.",
        "Filtri Peak e Shelf 'Gentle' processati in parallelo. Elaborazione dinamica efficiente e naturale.",
        "Imita la risposta in ampiezza e fase del prototipo analogico. NON automatizzare i parametri del filtro in questa modalità.",
        "Imita la risposta in ampiezza del prototipo analogico e corregge la fase sulle alte frequenze. NON automatizzare i parametri del filtro in questa modalità.",
        "Imita la risposta in ampiezza del prototipo analogico e corregge la fase sulle medie e alte frequenze. NON automatizzare i parametri del filtro in questa modalità.",

        "Doppio clic: apri le impostazioni dell'interfaccia utente.",

        "Premi: apri il pannello Corrispondenza EQ.",
        "Clicca: salva la curva target in un file preset.",
        "Scegli: scegli la curva target.",
        "Apprendi la curva target dal segnale della side-chain.",
        "Imposta la curva target come linea piatta.",
        "Carica la curva target da un file preset.",
        "Controlla lo spostamento (shift) della curva di differenza.",
        "Controlla la fluidità della curva di differenza.",
        "Controlla la pendenza della curva di differenza.",
        "Clicca: avvia il processo di fitting.",
        "Controlla il numero di bande.",
        "Il modello Corrispondenza EQ sta analizzando il segnale principale (main-chain) e il segnale della side-chain.\nLa sorgente (source) deve essere il segnale principale, ma puoi scegliere il target.\nLa differenza tra sorgente e target è la curva di differenza.\nUna volta che la curva di differenza è stabile, puoi regolarla ulteriormente prima di avviare il fitting.",
        "Il modello Corrispondenza EQ sta eseguendo il processo di fitting. Dovrebbe impiegare al massimo qualche secondo.",
        "Il modello Corrispondenza EQ ha completato il processo di fitting. Ora puoi cambiare il numero di bande usate per il fitting.",
        "Premere: abilita il disegno della curva di differenza."
    };
}
