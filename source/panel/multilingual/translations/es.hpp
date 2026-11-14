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

namespace zlpanel::multilingual::es {
    inline constexpr std::array kTexts = {
        // band basic controls
        "Activa/desactiva el bypass de la banda.",
        "Activa/desactiva la escucha en solo de la banda.",
        "Selecciona el tipo de filtro.",
        "Selecciona la pendiente del filtro. Una pendiente mayor produce una curva de respuesta en frecuencia más abrupta.",
        "Selecciona el modo de canal estéreo.",
        "Ajusta la frecuencia central o de corte.",
        "Ajusta la ganancia base y la ganancia objetivo dinámica.",
        "Ajusta el factor Q. Un valor Q mayor estrecha el ancho de banda.",
        "Activa/desactiva el comportamiento dinámico de ecualización para esta banda.",
        "Desactiva esta banda.",
        "Selecciona la banda anterior.",
        "Selecciona la banda siguiente.",
        "Activa/desactiva el escalado de paso para los filtros paso bajo y paso alto.",
        "Ajusta la escala de magnitud del filtro de paso.",
        // band dynamic controls
        "Activa/desactiva el bypass del procesamiento dinámico para esta banda.",
        "Activa/desactiva el aprendizaje automático del umbral dinámico. Al desactivarlo se ajustan automáticamente el umbral y el codo (Knee).",
        "Activa/desactiva el modo de umbral relativo para el procesamiento dinámico.",
        "Activa/desactiva el intercambio de los canales estéreo del side-chain.",
        "Ajusta el umbral del procesamiento dinámico.",
        "Ajusta la anchura del codo (Knee) del procesamiento dinámico.",
        "Ajusta el tiempo de ataque del procesamiento dinámico.",
        "Ajusta el tiempo de relajación del procesamiento dinámico.",
        "Vincula la frecuencia y el factor Q del filtro de side-chain con esta banda.",
        "Selecciona el tipo de filtro del side-chain.",
        "Selecciona la pendiente del filtro del side-chain.",
        "Ajusta la frecuencia del filtro del side-chain.",
        "Ajusta el factor Q del filtro del side-chain.",
        "Muestra/oculta los ajustes adicionales de detección dinámica.",
        "Ajusta la duración de la ventana de integración RMS para la detección de nivel.",
        "Ajusta la mezcla entre la detección de nivel por pico y por RMS.",
        "Ajusta el grado de suavizado aplicado a la envolvente dinámica.",
        // top controls
        "Muestra/oculta el panel de ajustes de la interfaz.",
        "Muestra/oculta el explorador de preajustes.",
        "Muestra/oculta el panel de ajustes del analizador.",
        "Muestra/oculta el panel de ajustes de salida.",
        "Selecciona la estructura del filtro.",
        "Estructura digital clásica estándar. No apta para automatización agresiva.",
        "Estructura estable usada en filtros de sintetizador y crossovers. Apta para automatización agresiva, aunque introduce un gran desfase.",
        "Procesa filtros Shelf y Peak suaves en paralelo para un procesamiento dinámico eficiente y natural.",
        "Emula la respuesta de magnitud y fase de prototipos analógicos. No automatice parámetros del filtro en este modo.",
        "Emula la respuesta de magnitud de prototipos analógicos y limpia la fase en agudos. No automatice parámetros del filtro en este modo.",
        "Emula la respuesta de magnitud de prototipos analógicos y limpia la fase en medios y agudos. No automatice parámetros del filtro en este modo.",
        "Activa/desactiva el bypass del plugin.",
        "Activa/desactiva la entrada de side-chain externo.",
        "Muestra/oculta el panel de EQ Match.",
        // preset browser
        "Abre la carpeta de preajustes en el administrador de archivos del sistema.",
        "Elimina el grupo de preajustes seleccionado.",
        "Elimina el preajuste seleccionado.",
        "Cierra el explorador de preajustes.",
        // output controls
        "Ajusta la ganancia de salida adicional.",
        "Ajusta la escala general de la ganancia base y objetivo de todos los filtros.",
        "Activa/desactiva la Compensación de Ganancia Estática (SGC). SGC es aproximada, pero no afecta a la dinámica.",
        "Activa/desactiva la igualación de ganancia por sonoridad. Al activarla mide la sonoridad de entrada y salida; al desactivarla ajusta la ganancia de salida según su diferencia.",
        "Activa/desactiva la Compensación Automática de Ganancia (AGC). AGC es más precisa a largo plazo, pero afecta a la dinámica.",
        "Activa/desactiva la inversión de fase de la señal de salida.",
        "Ajusta el tiempo de anticipación (lookahead) de la señal de side-chain.",
        // analyzer controls
        "Muestra/oculta el espectro de la señal de entrada.",
        "Muestra/oculta el espectro de la señal de salida.",
        "Muestra/oculta el espectro de la señal de side-chain.",
        "Selecciona la velocidad de decaimiento del analizador de espectro.",
        "Selecciona la pendiente de inclinación (tilt) del analizador de espectro.",
        "Selecciona el ancho de suavizado frecuencial del analizador de espectro.",
        "Selecciona el modo de suavizado frecuencial.\nOCT: suavizado por fracción de octava constante.\nERB: suavizado por ancho de banda rectangular equivalente.",
        "Activa/desactiva la congelación del espectro al pasar el puntero sobre el analizador.",
        "Selecciona el modo de canal estéreo mostrado por el analizador de espectro.",
        "Activa/desactiva la detección de colisiones de frecuencia.",
        "Ajusta la intensidad de la detección de colisiones.",
        "Muestra/oculta la lectura numérica de valores.",
        "Muestra/oculta el panel de medidores de nivel.",
        // curve scale & piano roll
        "Selecciona el rango de visualización en decibelios para la curva de respuesta del ecualizador.",
        "Selecciona el límite superior en decibelios para la visualización del analizador de espectro.",
        "Selecciona el nivel mínimo en decibelios para la visualización del analizador de espectro.",
        "Muestra/oculta el piano roll.",
        // eq match controls
        "Guarda la curva objetivo en un archivo de preajuste.",
        "Activa/desactiva el dibujo manual sobre la curva de diferencia.",
        "Selecciona el origen de la curva objetivo.",
        "Aprende la curva objetivo a partir de la señal de side-chain.",
        "Establece la curva objetivo como una línea plana.",
        "Carga la curva objetivo desde un archivo de preajuste.",
        "Establece la curva objetivo como una curva equilibrada.",
        "Establece la curva objetivo como una curva natural.",
        "Ajusta el desplazamiento vertical de la curva de diferencia.",
        "Ajusta la escala vertical de la curva de diferencia.",
        "Ajusta la pendiente de inclinación de la curva de diferencia.",
        "Selecciona la magnitud máxima de ganancia permitida para EQ Match.",
        "Inicia el ajuste de bandas de ecualización a la curva de diferencia.",
        "Ajusta el número de bandas de ecualización usadas para el ajuste.",
        "Analizando las señales principal y de side-chain.\nSeleccione el origen de la curva objetivo y ajuste la curva de diferencia cuando se estabilice antes de iniciar el ajuste.",
        "Ajustando bandas de ecualización a la curva de diferencia. Esto puede tardar unos segundos.",
        "Ajuste completado. Puede modificar el número de bandas ajustadas si es necesario.",
        " "
    };
}
