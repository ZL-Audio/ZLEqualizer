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

namespace zlgui::multilingual::es {
    static constexpr std::array kTexts = {
        "Presiona: activa la banda seleccionada.\nSuelta: omite la banda seleccionada.",
        "Presiona: activa el solo del audio afectado por la banda de frecuencia seleccionada.",
        "Elige el tipo de la banda de frecuencia seleccionada. Peak, Low Shelf, Low Pass (pasa bajos), High Shelf, High Pass (pasa altos), Notch, Band Pass (pasa banda), Tilt Shelf.",
        "Elige la pendiente de la banda de frecuencia seleccionada. Una pendiente mayor hará que la curva de respuesta del filtro cambie de forma más pronunciada.",
        "Elige el modo estéreo de la banda de frecuencia seleccionada. Stereo (estéreo), Left (izquierda), Right (derecha), Mid (central), Side (lateral).",
        "Ajusta la frecuencia central de la banda de frecuencia seleccionada.",
        "Ajusta la ganancia de la banda de frecuencia seleccionada.",
        "Ajusta el factor Q de la banda de frecuencia seleccionada. Cuanto mayor sea el valor Q, más estrecho será el ancho de banda.",
        "Elige la banda de frecuencia con las flechas izquierda/derecha.",
        "Presiona: activa el comportamiento dinámico de la banda de frecuencia seleccionada.",
        "Presiona: activa el umbral automático de la banda de frecuencia seleccionada.\nSuelta: desactiva el umbral automático de la banda de frecuencia seleccionada y aplica el umbral aprendido.",
        "Haz clic: desactiva la banda de frecuencia seleccionada.",
        "Suelta: omite el comportamiento dinámico de la banda seleccionada.",
        "Presiona: activa el solo del audio de la side-chain de la banda seleccionada.",
        "Presiona: configura el umbral dinámico en modo relativo.\nSuelta: configura el umbral dinámico en modo absoluto.",
        "Presiona: intercambia la side-chain para configurarla en el modo estéreo opuesto.",
        "Ajusta el umbral del comportamiento dinámico de la banda seleccionada.",
        "Ajusta el ancho del knee del comportamiento dinámico de la banda seleccionada. El ancho real del knee es el valor mostrado × 60 dB.",
        "Ajusta el tiempo de ataque del comportamiento dinámico de la banda seleccionada.",
        "Ajusta el tiempo de liberación del comportamiento dinámico de la banda seleccionada.",
        "Ajusta la frecuencia central del filtro paso banda aplicado a la señal de side-chain.",
        "Ajusta el valor Q del filtro paso banda aplicado a la señal de side-chain.",
        "Presiona: utiliza la side-chain externa.\nSuelta: utiliza la side-chain interna.",
        "Presiona: activa la compensación estática de ganancia (SGC). El SGC estima la cantidad de compensación a partir de los parámetros del filtro. El SGC no es preciso, pero NO afecta la dinámica de la señal.",
        "Presiona: omite el plugin.",
        "Ajusta la escala de todos los parámetros de ganancia.",
        "Presiona: invierte la fase de la señal de salida.",
        "Presiona: activa la compensación automática de ganancia (AGC). El AGC es preciso a largo plazo, pero afectará la dinámica de la señal.",
        "Presiona: al presionarlo, comienza a medir la sonoridad integrada de la señal de entrada y de salida.\nSuelta: al soltarlo, desactiva el AGC y actualiza la ganancia de salida con la diferencia entre ambas sonoridades.",
        "Ajusta la ganancia de salida del plugin.",
        "Elige el estado del analizador de espectro de entrada. FRZ puede congelar el analizador.",
        "Elige el estado del analizador de espectro de salida. FRZ puede congelar el analizador.",
        "Elige el estado del analizador de espectro de la side-chain. FRZ puede congelar el analizador.",
        "Ajusta la velocidad de decaimiento del analizador de espectro.",
        "Ajusta la pendiente del analizador de espectro (no afecta la señal real).",
        "Ajusta el tiempo de lookahead de la señal de side-chain (introduciendo latencia).",
        "Ajusta la longitud del RMS. A medida que aumenta el RMS, el ataque y la liberación serán más lentos y suaves.",
        "Ajusta la suavidad de la liberación.",
        "Elige el estado de la opción de Alta Calidad. Cuando está activada, la dinámica ajusta el estado del filtro por muestra, lo que suaviza los efectos dinámicos y reduce los artefactos.",
        "Elige el estado de la detección de colisiones.",
        "Ajusta la intensidad de la detección de colisiones. Si aumentas la intensidad, las áreas detectadas se volverán más grandes y oscuras.",
        "Ajusta la escala de la detección de colisiones. Si aumentas la escala, las áreas detectadas se volverán más oscuras.",
        "Elige la estructura de los filtros.",
        "Elige el estado de Cero Latencia. Cuando Cero Latencia está activado, la latencia extra es cero, pero el tamaño del búfer afectará ligeramente los efectos dinámicos y la automatización de parámetros.",
        "Elige la curva objetivo.\nSide: aprendida a partir de la señal de side-chain.\nPreset: cargada desde un archivo de preajuste.\nFlat: configurada como una línea plana de -4,5 dB/oct.",
        "Ajusta el peso del modelo de aprendizaje de la curva. Cuanto mayor sea el peso, más aprenderá el modelo de la parte más fuerte de la señal.",
        "Haz clic: inicia el aprendizaje de la curva.",
        "Haz clic: guarda la curva objetivo actual como un archivo de preajuste.",
        "Ajusta la suavidad de la curva de diferencia.",
        "Ajusta la pendiente de la curva de diferencia.",
        "Elige el algoritmo de ajuste.\nLD: algoritmo basado en gradiente local (rápido).\nGN: algoritmo global sin gradiente (recomendado).\nGN+: algoritmo global sin gradiente (lento, permite que los filtros tengan pendientes más pronunciadas).",
        "Haz clic: inicia el ajuste de la curva.",
        "Ajusta el número de bandas utilizadas para el ajuste. Cuando se complete el ajuste, el modelo de ajuste de curva sugerirá el número de bandas, el cual podrás modificar posteriormente.",
        "Elige la escala en dB de las curvas de respuesta de los filtros (izquierda) y de los analizadores de espectro (derecha).",
        "Haz clic: abre el panel Equalizer Match. El Equalizer Match consta de cuatro pasos:\n1. Elige la curva objetivo.\n2. Inicia el aprendizaje.\n3. Ajusta la diferencia.\n4. Inicia el ajuste.",
        "Estructura de filtro más común en ecualizadores. La respuesta del filtro es muy similar al prototipo analógico. Adecuado para modulación.",
        "Estructura de filtro más común en crossovers. El filtro provoca un mayor cambio de fase. Adecuado para modulación rápida.",
        "Los filtros Peak/Shelf están dispuestos en paralelo. La respuesta del filtro es diferente a la curva mostrada. Adecuado para modulación.",
        "Estructura de fase mínima con correcciones adicionales. La respuesta del filtro es casi idéntica al prototipo analógico. NO es adecuado para modulación.",
        "Estructura de fase mínima con correcciones adicionales. El filtro tiene una respuesta en magnitud similar al prototipo analógico y menor cambio de fase en altas frecuencias. NO es adecuado para modulación.",
        "El filtro tiene una respuesta en magnitud similar al prototipo analógico y una respuesta de fase cero. NO es adecuado para modulación. El efecto dinámico no funciona.",
        "Doble clic: abre la Configuración de la Interfaz de Usuario (UI Settings)."
    };
}
