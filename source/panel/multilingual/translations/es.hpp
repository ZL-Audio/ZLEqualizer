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

namespace zlpanel::multilingual::es {
    static constexpr std::array kTexts = {
        "Soltar: omitir la banda.",
        "Pulsar: solo de la banda.",
        "Elegir el tipo de filtro.",
        "Elegir la pendiente del filtro. Una pendiente mayor hará que la curva de respuesta del filtro cambie más abruptamente.",
        "Elegir el modo estéreo.",
        "Controlar la frecuencia.",
        "Controlar la ganancia base y la ganancia objetivo.",
        "Controlar el factor de calidad (Q). Cuanto mayor sea el valor Q, más estrecho será el ancho de banda.",
        "Pulsar: activar el comportamiento dinámico.",
        "Clic: desactivar la banda.",
        "Soltar: omitir el comportamiento dinámico.",
        "Pulsar: activar el aprendizaje dinámico.\nSoltar: desactivar el aprendizaje dinámico y ajustar el Umbral y el Codo (Knee).\nVer detalles en el manual.",
        "Pulsar: activar el comportamiento dinámico relativo.\nVer detalles en el manual.",
        "Pulsar: cambiar el modo estéreo del side-chain.",
        "Controlar el umbral del comportamiento dinámico.\nVer detalles en el manual.",
        "Controlar la amplitud del codo (knee) del comportamiento dinámico.\nVer detalles en el manual.",
        "Controlar el tiempo de ataque del comportamiento dinámico.\nVer detalles en el manual.",
        "Controlar el tiempo de relajación del comportamiento dinámico.\nVer detalles en el manual.",
        "Pulsar: enlazar la banda con la banda de side-chain.",
        "Elegir el tipo de filtro del side-chain.",
        "Elegir la pendiente del filtro del side-chain.",
        "Controlar la frecuencia del filtro del side-chain.",
        "Controlar el factor de calidad (Q) del filtro del side-chain.",

        "Soltar: omitir el plugin.",
        "Pulsar: usar side-chain externo.\nSoltar: usar side-chain interno.",

        "Controlar la ganancia de salida adicional.",
        "Controlar la escala de la ganancia base y objetivo de todos los filtros.",
        "Pulsar: activar la Compensación de Ganancia Estática (CGE). La CGE es imprecisa, pero no afecta a la dinámica.",
        "Pulsar: iniciar la medición de sonoridad integrada de la señal de entrada y salida.\nSoltar: desactivar el AGC y actualizar la Ganancia de Salida a la diferencia entre los dos valores de sonoridad.",
        "Pulsar: activar la Compensación Automática de Ganancia (CAG). El CAG es más preciso a largo plazo, pero afecta a la dinámica.",
        "Pulsar: invertir la fase de la señal de salida.",
        "Controlar la anticipación (lookahead) de la señal de side-chain.",

        "Pulsar: activar el analizador de espectro de la señal de entrada",
        "Pulsar: activar el analizador de espectro de la señal de salida",
        "Pulsar: activar el analizador de espectro de la señal de side-chain",
        "Elegir la velocidad de decaimiento de los analizadores de espectro.",
        "Elegir la pendiente de inclinación (tilt) de los analizadores de espectro.",
        "Pulsar: activar la función de congelación. Pase el ratón sobre el analizador para congelar el espectro.",
        "Pulsar: activar la detección de colisiones.",
        "Controlar la sensibilidad de la detección de colisiones.",

        "Elegir la estructura del filtro.",
        "La estructura digital clásica y estándar. No apta para automatización agresiva.",
        "La estructura estable usada en filtros de sintetizador y crossovers. Apta para automatización agresiva. Causa un gran desfase.",
        "Filtros Shelf y Peak suaves procesados en paralelo. Procesamiento dinámico eficiente y natural.",
        "Imita la respuesta de magnitud y fase de prototipos analógicos. NO automatice parámetros del filtro en este modo.",
        "Imita la respuesta de magnitud de prototipos analógicos y limpia la fase en agudos. NO automatice parámetros del filtro en este modo.",
        "Imita la respuesta de magnitud de prototipos analógicos y limpia la fase en medios y agudos. NO automatice parámetros del filtro en este modo.",

        "Doble clic: abrir los ajustes de la interfaz.",

        "Pulsar: abrir el panel de EQ Match.",
        "Clic: guardar la curva objetivo en un archivo de preajuste.",
        "Elegir: seleccionar la curva objetivo.",
        "Aprender la curva objetivo desde la señal de side-chain.",
        "Establecer la curva objetivo como una línea plana.",
        "Cargar la curva objetivo desde un archivo de preajuste.",
        "Controlar el desplazamiento de la curva de diferencia.",
        "Controlar la suavidad de la curva de diferencia.",
        "Controlar la pendiente de la curva de diferencia.",
        "Clic: iniciar el proceso de ajuste.",
        "Controlar el número de bandas.",
        "El modelo de EQ Match está analizando la señal principal y la señal de side-chain.\nLa fuente debe ser la señal principal, pero puede elegir el objetivo.\nLa diferencia entre la fuente y el objetivo es la 'diferencia'.\nUna vez que la 'diferencia' se estabilice, puede ajustarla más antes del proceso de ajuste.",
        "El modelo de EQ Match está ejecutando el proceso de ajuste. Debería tardar unos segundos como máximo.",
        "El modelo de EQ Match ha completado el proceso de ajuste. Ahora puede cambiar el número de bandas usadas para el ajuste.",
        "Presionar: habilitar el dibujo de la curva de diferencia."
    };
}
