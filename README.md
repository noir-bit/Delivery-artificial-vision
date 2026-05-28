# ESP32-P4 AI Obstacle & Person Detection Pipeline (Rama: live_video_feed)

Esta rama contiene la implementación funcional para el procesamiento de **video en vivo en tiempo real** utilizando el microcontrolador **ESP32-P4**, una cámara compatible con **Raspberry Pi v1.3 (OV5647)** y la biblioteca de Deep Learning **ESP-DL** de Espressif.

---

## 📋 Estructura General del Repositorio en esta Rama

```text
Proyecto_Final_IA/ (Rama: live_video_feed)
├── README.md                                 # Esta documentación
├── .gitignore                                # Filtros Git para evitar subir archivos pesados o temporales
├── live_video_feed/                          # [CÓDIGO DE VIDEO EN VIVO]
│   ├── CMakeLists.txt                        # Configuración CMake con rutas de componentes relativas
│   ├── partitions.csv                        # Tabla de particiones de memoria flash
│   ├── sdkconfig.defaults                    # Parámetros por defecto para compilar en ESP32-P4
│   ├── README_live.md                        # Documentación original del live feed
│   ├── main/
│   │   ├── idf_component.yml                 # Declaración de componentes de sensores de cámara
│   │   ├── main.cpp                          # Captura de frames, debayerización, inferencia y envío serial
│   │   ├── mipi_camera.c                     # Driver de bajo nivel para MIPI CSI en ESP32-P4
│   │   └── mipi_camera.h                     # Cabecera del driver de cámara
│   └── pc_visualizer/
│       └── live_feed.py                      # Script de Python para decodificación Base64 y visualización con OpenCV
├── Proyecto_Final_IA/                        # [CÓDIGO DE SIMULACIÓN DE IMÁGENES FIJAS]
│   └── esp-dl/                               # Repositorio base de ESP-DL
│       ├── examples/
│       │   └── obstacle_person_detect/       # Inferencia local sobre imágenes fijas cargadas vía SPIFFS
│       └── models/
│           └── obstacle_person_detect/       # Modelo quantizado .espdl (Pico 224x224, Obstáculos y Personas)
└── Vision_Carro_Delivery_dataset_adquisition/# Metadatos y MATLAB script para preparación del dataset
```

---

## 🚀 Flujo de Trabajo del Live Video Feed

A diferencia de la rama `main` (que emula la cámara procesando fotos estáticas cargadas en SPIFFS), esta rama realiza la captura en tiempo real:

1. **Captura MIPI CSI**: El driver de bajo nivel (`mipi_camera.c`) inicializa la cámara OV5647 de 5MP mediante el bus MIPI CSI y CCI (I2C) a 800x800 píxeles.
2. **Debayerización (RAW8 a RGB888)**: Se ejecuta una rutina de debayerización por software rápida en la PSRAM para transformar los datos de color de Bayer (BGGR) a canales completos RGB888.
3. **Inferencia Local con IA**: El procesador de doble núcleo del ESP32-P4 ejecuta la inferencia mediante el modelo quantizado de ESP-DL (`ESPDetDetect`) para detectar obstáculos y personas en una ventana de 224x224.
4. **Compresión de Imagen**: El frame RGB se comprime a JPEG directamente en hardware utilizando el codificador JPEG integrado de alto rendimiento del ESP32-P4.
5. **Transmisión Base64**: La imagen JPEG comprimida se codifica en Base64 y se envía por consola serial en formato JSON junto con las coordenadas de detección, a una tasa de refresco limitada a **7 FPS** (para evitar saturación del buffer serie):
   ```json
   >>>JSON_START<<<
   {
     "image": "/9j/4AAQSkZJRgABAQEASABIAAD/2wBD...",
     "detections": [
       {"category": 0, "score": 0.88, "box": [120, 45, 180, 200]}
     ]
   }
   >>>JSON_END<<<
   ```
6. **Decodificación y Visualización en PC**: El script `live_feed.py` en la PC lee el flujo serial, decodifica el string Base64 a una matriz de imagen OpenCV, dibuja las cajas delimitadoras de las detecciones (`Obstacle` en Rojo, `Person` en Verde) y las proyecta en una ventana de video en vivo.

---

## 🔧 Instrucciones de Compilación y Ejecución

### 1. Compilación del Firmware
1. Abra su consola con el entorno **ESP-IDF v5.3** activado.
2. Navegue al directorio del live feed:
   ```bash
   cd live_video_feed
   ```
3. Defina la tarjeta objetivo:
   ```bash
   idf.py set-target esp32p4
   ```
4. Compile e instale en la placa conectada:
   ```bash
   idf.py build flash
   ```
   *(La configuración de `CMakeLists.txt` incluye de forma relativa las dependencias del modelo de IA que se encuentran en el directorio `Proyecto_Final_IA/esp-dl` para que la compilación sea completamente portable).*

### 2. Ejecución del Visualizador en PC
1. Instale las dependencias de Python si aún no lo ha hecho:
   ```bash
   pip install opencv-python pyserial numpy
   ```
2. Abra el script `live_video_feed/pc_visualizer/live_feed.py` y modifique la línea 9 con el puerto COM de su placa:
   ```python
   SERIAL_PORT = 'COM4'  # Reemplace por su puerto COM correspondiente (ej. COM3, COM5)
   ```
3. Asegúrese de **cerrar cualquier monitor serial** (para liberar el puerto de comunicación).
4. Ejecute el script:
   ```bash
   python live_video_feed/pc_visualizer/live_feed.py
   ```
5. Presione la tecla `q` en la ventana de OpenCV para salir del reproductor.
