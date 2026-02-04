# HomeControl

Proyecto de domótica basado en **ESP32** que permite **leer datos de temperatura y humedad** desde un sensor DHT11, controlar un **relé** y enviar **códigos IR** para controlar dispositivos remotos.

Este proyecto se desarrolló con fines de aprendizaje y experimentación en IoT, enfocándose en la adquisición de datos desde hardware físico, su procesamiento en el microcontrolador y el control de actuadores.

## Table of Contents

1. [Descripción](#descripción)  
2. [Características](#características)  
3. [Arquitectura del sistema](#arquitectura-del-sistema)  
4. [Requisitos de hardware](#requisitos-de-hardware)  
5. [Instalación](#instalación)  
6. [Configuración](#configuración)  
7. [Uso](#uso)

---

## Descripción

Este repositorio contiene:
- Código para el **ESP32** que obtiene datos de un sensor DHT11 (temperatura y humedad).
- Control de un **relé** para accionar dispositivos eléctricos.
- Emisión de **códigos infrarrojos (IR)** para controlar dispositivos como aires acondicionados, ventiladores o equipos IR.
- Archivos de datos y configuración para prueba del sistema. :contentReference[oaicite:1]{index=1}

## Características

- Lectura periódica de valores de **temperatura y humedad**.  
- Control digital de **relé** para activar/desactivar un dispositivo.  
- Emisión de señales **IR** preconfiguradas.  
- Estructura simple y didáctica para aprendizaje de IoT con ESP32. :contentReference[oaicite:2]{index=2}

## Arquitectura del sistema

ESP32
├─ DHT11 (sensor de temperatura y humedad)
├─ Relé (control de dispositivos)
└─ LED IR (emisión de comandos infrarrojos)

1. El ESP32 lee sensores y ejecuta lógica de control.  
2. Los datos de sensores pueden almacenarse o transmitirse según futuras mejoras.  
3. El relé y los códigos IR permiten interactuar con dispositivos físicos. :contentReference[oaicite:3]{index=3}

## Requisitos de hardware

Para replicar este proyecto necesitarás:

- ESP32 (cualquier modelo compatible).  
- Sensor DHT11 o DHT22.  
- Módulo de relé de 5V.  
- LED emisor IR (junto con resistencia apropiada).  
- Cables dupont y fuente de alimentación adecuada.

> **Nota:** Verifica el voltaje de operación de los módulos para evitar dañar el ESP32.

## Instalación

1. Clona este repositorio:

```bash```
```git clone https://github.com/TBO57/HomeControl.git```
```cd HomeControl```

2. Abre el archivo HomeControl.ino con Arduino IDE o VS Code + PlatformIO.
3. Instala las dependencias necesarias:
 - Biblioteca para DHT (DHT sensor library).
 - Biblioteca para emisión de IR (IRremoteESP8266 o similar).
4. Selecciona la placa ESP32 en el IDE y configura el puerto correcto.

## Configuración

Antes de cargar el código:
  - Ajusta los pines del sensor DHT y del relé en el archivo HomeControl.ino.
  - Asegúrate de que los pines no estén en conflicto con otros periféricos del ESP32.

Ejemplo de configuración de pines:

```#define PIN_DHT 4```
```#define PIN_RELE 16```
```#define PIN_IR 17```

(Ajusta según tu conexión física)

## Uso

1. Conecta el ESP32 a tu computadora.
2. Compila y sube el programa al dispositivo.
3. Abre el monitor serie del IDE para visualizar salidas de temperatura y humedad.
4. Observa el estado del relé y la emisión de IR en función de tu lógica.

Este proyecto no incluye servidor web, API o almacenamiento remoto aún, pero puede extenderse para enviar datos vía Wi-Fi o MQTT en futuras versiones.

