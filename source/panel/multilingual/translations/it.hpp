// Copyright (C) 2026 - zsliu98
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
    inline constexpr std::array kTexts = {
        // band basic controls
        "Attiva/disattiva il bypass della banda.",
        "Attiva/disattiva l'ascolto in solo della banda.",
        "Seleziona il tipo di filtro.",
        "Seleziona la pendenza del filtro. Una pendenza maggiore produce una curva di risposta in frequenza più ripida.",
        "Seleziona la modalità del canale stereo.",
        "Regola la frequenza centrale o di taglio.",
        "Regola il guadagno di base e il guadagno target dinamico.",
        "Regola il fattore Q. Un valore di Q maggiore restringe la larghezza di banda.",
        "Attiva/disattiva il comportamento dinamico di equalizzazione per questa banda.",
        "Disattiva questa banda.",
        "Seleziona la banda precedente.",
        "Seleziona la banda successiva.",
        "Attiva/disattiva la scalatura passante per i filtri passa-basso e passa-alto.",
        "Regola la scala di ampiezza del filtro passante.",
        // band dynamic controls
        "Attiva/disattiva il bypass dell'elaborazione dinamica per questa banda.",
        "Attiva/disattiva l'apprendimento automatico della soglia dinamica. Disattivandolo imposta automaticamente Soglia e Ginocchio (Knee).",
        "Attiva/disattiva la modalità a soglia relativa per l'elaborazione dinamica.",
        "Attiva/disattiva lo scambio dei canali stereo della side-chain.",
        "Regola la soglia dell'elaborazione dinamica.",
        "Regola l'ampiezza del ginocchio (Knee) dell'elaborazione dinamica.",
        "Regola il tempo di attacco dell'elaborazione dinamica.",
        "Regola il tempo di rilascio dell'elaborazione dinamica.",
        "Collega frequenza e fattore Q del filtro della side-chain a questa banda.",
        "Seleziona il tipo di filtro della side-chain.",
        "Seleziona la pendenza del filtro della side-chain.",
        "Regola la frequenza del filtro della side-chain.",
        "Regola il fattore Q del filtro della side-chain.",
        "Mostra/nasconde le impostazioni aggiuntive di rilevamento dinamico.",
        "Regola la lunghezza della finestra di integrazione RMS per il rilevamento del livello.",
        "Regola il bilanciamento tra il rilevamento di picco e il rilevamento di livello RMS.",
        "Regola la quantità di smussamento applicata all'inviluppo dinamico.",
        // top controls
        "Mostra/nasconde il pannello delle impostazioni dell'interfaccia.",
        "Mostra/nasconde il browser dei preset.",
        "Mostra/nasconde il pannello delle impostazioni dell'analizzatore.",
        "Mostra/nasconde il pannello delle impostazioni di uscita.",
        "Seleziona la struttura del filtro.",
        "Struttura digitale classica standard. Non adatta ad automazioni aggressive.",
        "Struttura stabile usata nei filtri dei synth e nei crossover. Adatta ad automazioni aggressive, ma introduce un forte sfasamento.",
        "Elabora filtri Shelf e Peak dolci in parallelo per un'elaborazione dinamica efficiente e naturale.",
        "Emula la risposta in ampiezza e fase dei prototipi analogici. Non automatizzare i parametri del filtro in questa modalità.",
        "Emula la risposta in ampiezza dei prototipi analogici e ripulisce la fase sulle alte frequenze. Non automatizzare i parametri del filtro in questa modalità.",
        "Emula la risposta in ampiezza dei prototipi analogici e ripulisce la fase sulle medie e alte frequenze. Non automatizzare i parametri del filtro in questa modalità.",
        "Attiva/disattiva il bypass del plugin.",
        "Attiva/disattiva l'ingresso side-chain esterno.",
        "Mostra/nasconde il pannello EQ Match.",
        // preset browser
        "Apre la cartella dei preset nel gestore file di sistema.",
        "Elimina il gruppo di preset selezionato.",
        "Elimina il preset selezionato.",
        "Chiude il browser dei preset.",
        // output controls
        "Regola il guadagno di uscita aggiuntivo.",
        "Regola la scala complessiva del guadagno di base e target di tutti i filtri.",
        "Attiva/disattiva la Compensazione Statica del Guadagno (SGC). La SGC è approssimata, ma non influisce sulla dinamica.",
        "Attiva/disattiva l'adattamento del guadagno basato sul loudness. Attivandolo misura il loudness di ingresso e uscita; disattivandolo imposta il guadagno di uscita sulla loro differenza.",
        "Attiva/disattiva la Compensazione Automatica del Guadagno (AGC). L'AGC è più accurata nel lungo periodo, ma influisce sulla dinamica.",
        "Attiva/disattiva l'inversione di fase del segnale in uscita.",
        "Regola il tempo di lookahead del segnale della side-chain.",
        // analyzer controls
        "Mostra/nasconde lo spettro del segnale in ingresso.",
        "Mostra/nasconde lo spettro del segnale in uscita.",
        "Mostra/nasconde lo spettro del segnale della side-chain.",
        "Seleziona la velocità di decadimento dell'analizzatore di spettro.",
        "Seleziona la pendenza di inclinazione (tilt) dell'analizzatore di spettro.",
        "Seleziona l'ampiezza di smussamento in frequenza dell'analizzatore di spettro.",
        "Seleziona la modalità di smussamento in frequenza.\nOCT: smussamento costante per frazioni di ottava.\nERB: smussamento per larghezza di banda rettangolare equivalente.",
        "Attiva/disattiva il congelamento dello spettro al passaggio del puntatore sull'analizzatore.",
        "Seleziona la modalità del canale stereo visualizzata dall'analizzatore di spettro.",
        "Attiva/disattiva il rilevamento delle collisioni di frequenza.",
        "Regola l'intensità del rilevamento delle collisioni.",
        "Mostra/nasconde la lettura numerica dei valori.",
        "Mostra/nasconde il pannello dei misuratori di livello.",
        // curve scale & piano roll
        "Seleziona l'intervallo di visualizzazione in decibel per la curva di risposta dell'equalizzatore.",
        "Seleziona il limite superiore in decibel per la visualizzazione dell'analizzatore di spettro.",
        "Seleziona il livello minimo in decibel per la visualizzazione dell'analizzatore di spettro.",
        "Mostra/nasconde il piano roll.",
        // eq match controls
        "Salva la curva target in un file preset.",
        "Attiva/disattiva il disegno manuale sulla curva di differenza.",
        "Seleziona la sorgente della curva target.",
        "Apprende la curva target dal segnale della side-chain.",
        "Imposta la curva target su una linea piatta.",
        "Carica la curva target da un file preset.",
        "Imposta la curva target su una curva bilanciata.",
        "Imposta la curva target su una curva naturale.",
        "Regola lo spostamento verticale della curva di differenza.",
        "Regola la scala verticale della curva di differenza.",
        "Regola la pendenza di inclinazione della curva di differenza.",
        "Seleziona l'ampiezza massima consentita del guadagno per EQ Match.",
        "Avvia l'adattamento delle bande di equalizzazione alla curva di differenza.",
        "Regola il numero di bande di equalizzazione usate per l'adattamento.",
        "Analisi dei segnali main-chain e side-chain in corso.\nSeleziona la sorgente della curva target e regola la curva di differenza una volta stabilizzata prima di avviare l'adattamento.",
        "Adattamento delle bande di equalizzazione alla curva di differenza in corso. Potrebbe richiedere alcuni secondi.",
        "Adattamento completato. Se necessario, puoi modificare il numero di bande adattate.",
        " "
    };
}
