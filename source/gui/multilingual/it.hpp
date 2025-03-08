// Copyright (C) 2025 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS of A PARTICULAR PURPOSE. See the GNU Affero General Public License of more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZL_INTERFACE_MULTILINGUAL_IT_HPP
#define ZL_INTERFACE_MULTILINGUAL_IT_HPP

#include <array>

namespace zlInterface::multilingual::it {
    static constexpr std::array texts = {
        "Premi: attiva la banda selezionata.\nRilascia: bypassa la banda selezionata.",
        "Premi: attiva il solo dell’audio interessato dalla banda di frequenza selezionata.",
        "Scegli il tipo della banda di frequenza selezionata. Peak（picco）、Low Shelf（scaffale basso）、Low Pass（passa basso）、High Shelf（scaffale alto）、High Pass（passa alto）、Notch（notch）、Band Pass（passa banda）、Tilt Shelf（shelf inclinato）.",
        "Scegli la pendenza della banda di frequenza selezionata. Una pendenza maggiore renderà la curva di risposta del filtro più ripida.",
        "Scegli la modalità stereo della banda di frequenza selezionata. Stereo（stereo）、Left（sinistra）、Right（destra）、Mid（centrale）、Side（side）.",
        "Regola la frequenza centrale della banda di frequenza selezionata.",
        "Regola il guadagno della banda di frequenza selezionata.",
        "Regola il fattore Q della banda di frequenza selezionata. Un valore di Q maggiore comporta una larghezza di banda più stretta.",
        "Scegli la banda di frequenza con le frecce sinistra/destra.",
        "Premi: attiva il comportamento dinamico della banda di frequenza selezionata.",
        "Premi: attiva la soglia automatica della banda di frequenza selezionata.\nRilascia: disattiva la soglia automatica e applica la soglia appresa.",
        "Clicca: disattiva la banda di frequenza selezionata.",
        "Rilascia: bypassa il comportamento dinamico della banda selezionata.",
        "Premi: attiva il solo dell’audio della side-chain della banda selezionata.",
        "Premi: imposta la soglia dinamica in modalità relativa.\nRilascia: imposta la soglia dinamica in modalità assoluta.",
        "Premi: scambia la side-chain per impostarla nella modalità stereo opposta.",
        "Regola la soglia del comportamento dinamico della banda selezionata.",
        "Regola la larghezza del knee del comportamento dinamico della banda selezionata. La larghezza effettiva del knee = valore visualizzato × 60 dB.",
        "Regola il tempo di attacco del comportamento dinamico della banda selezionata.",
        "Regola il tempo di rilascio del comportamento dinamico della banda selezionata.",
        "Regola la frequenza centrale del filtro passa-banda applicato al segnale della side-chain.",
        "Regola il valore Q del filtro passa-banda applicato al segnale della side-chain.",
        "Premi: utilizza la side-chain esterna.\nRilascia: utilizza la side-chain interna.",
        "Premi: attiva la compensazione statica del guadagno (SGC). L'SGC stima la quantità di compensazione in base ai parametri del filtro. L'SGC non è preciso, ma NON influisce sulla dinamica del segnale.",
        "Premi: bypassa il plugin.",
        "Regola la scala di tutti i parametri di guadagno.",
        "Premi: inverte la fase del segnale di uscita.",
        "Premi: attiva la compensazione automatica del guadagno (AGC). L'AGC è accurato a lungo termine, ma influirà sulla dinamica del segnale.",
        "Premi: quando premi, inizia a misurare la loudness integrata del segnale in ingresso e del segnale in uscita.\nRilascia: quando rilasci, disattiva l'AGC e aggiorna il guadagno d'uscita con la differenza tra le due loudness.",
        "Regola il guadagno di uscita del plugin.",
        "Scegli lo stato dell'analizzatore di spettro in ingresso. FRZ può congelare l'analizzatore.",
        "Scegli lo stato dell'analizzatore di spettro in uscita. FRZ può congelare l'analizzatore.",
        "Scegli lo stato dell'analizzatore di spettro della side-chain. FRZ può congelare l'analizzatore.",
        "Regola la velocità di decadimento dell'analizzatore di spettro.",
        "Regola la pendenza dell'analizzatore di spettro (non influisce sul segnale reale).",
        "Regola il tempo di lookahead del segnale della side-chain (introducendo latenza).",
        "Regola la lunghezza del RMS. Aumentando l'RMS, l'attacco e il rilascio saranno più lenti e fluidi.",
        "Regola la fluidità del rilascio.",
        "Scegli lo stato dell'opzione Alta Qualità. Quando attiva, il comportamento dinamico regola lo stato del filtro per ogni campione, rendendo gli effetti più fluidi e riducendo gli artefatti.",
        "Scegli lo stato della Rilevazione delle Collisioni.",
        "Regola l'intensità della Rilevazione delle Collisioni. Aumentando l'intensità, le aree rilevate diventeranno più grandi e più scure.",
        "Regola la scala della Rilevazione delle Collisioni. Aumentando la scala, le aree rilevate diventeranno più scure.",
        "Scegli la struttura dei filtri.",
        "Scegli lo stato della modalità Zero Latency. Quando Zero Latency è attivo, la latenza extra è pari a zero, ma la dimensione del buffer influirà leggermente sugli effetti dinamici e sull'automazione dei parametri.",
        "Scegli la curva target.\nSide: appresa dal segnale della side-chain.\nPreset: caricata da un file preset.\nFlat: impostata come una linea piatta a -4,5 dB/oct.",
        "Regola il peso del modello di apprendimento della curva. Un peso maggiore fa sì che il modello apprenda maggiormente dalle parti più forti del segnale.",
        "Clicca: avvia l'apprendimento della curva.",
        "Clicca: salva la curva target attuale come file preset.",
        "Regola la fluidità della curva di differenza.",
        "Regola la pendenza della curva di differenza.",
        "Scegli l'algoritmo di fitting.\nLD: algoritmo basato sul gradiente locale (veloce).\nGN: algoritmo globale senza gradiente (consigliato).\nGN+: algoritmo globale senza gradiente (lento, consente filtri con pendenze più elevate).",
        "Clicca: avvia il fitting della curva.",
        "Regola il numero di bande utilizzate per il fitting. Quando il fitting è completato, il modello suggerirà il numero di bande, che potrai modificare successivamente.",
        "Scegli la scala in dB delle curve di risposta dei filtri (sinistra) e degli analizzatori di spettro (destra).",
        "Clicca: apri il pannello Equalizer Match. L'Equalizer Match si compone di quattro fasi:\n1. Scegli la curva target.\n2. Avvia l'apprendimento.\n3. Regola la differenza.\n4. Avvia il fitting.",
        "Struttura di filtro più comune negli equalizzatori. La risposta del filtro è molto vicina al prototipo analogico. Adatta per la modulazione.",
        "Struttura di filtro più comune nei crossover. Il filtro introduce un maggiore sfasamento. Adatta per modulazioni veloci.",
        "I filtri Peak/Shelf sono disposti in parallelo. La risposta del filtro differisce dalla curva visualizzata. Adatta per la modulazione.",
        "Struttura a Fase Minima con correzioni aggiuntive. La risposta del filtro è quasi identica al prototipo analogico. NON adatta per la modulazione.",
        "Struttura a Fase Minima con correzioni aggiuntive. Il filtro ha una risposta in ampiezza simile al prototipo analogico e un minore sfasamento alle alte frequenze. NON adatta per la modulazione.",
        "Il filtro ha una risposta in ampiezza analoga al prototipo analogico e una risposta a fase zero. NON adatto per la modulazione. L'effetto dinamico non funziona.",
        "Doppio clic: apri le impostazioni dell'interfaccia utente (UI Settings)."
    };
}

#endif //ZL_INTERFACE_MULTILINGUAL_IT_HPP
