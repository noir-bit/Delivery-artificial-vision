# ESP32-P4 AI Obstacle & Person Detection Pipeline (Rama: live_video_feed)

Esta rama contiene la estructura del proyecto y los desarrollos experimentales para intentar integrar **video en vivo en tiempo real** utilizando el microcontrolador **ESP32-P4**, una cámara compatible con **Raspberry Pi v1.3 (OV5647)** y la biblioteca de Deep Learning **ESP-DL** de Espressif.

> [!WARNING]
> **Estado de la rama**: Según la documentación interna de este desarrollo (`live_video_feed/README_live.md`), **la conexión activa de la cámara no se logró de manera totalmente exitosa** debido a problemas de bajo nivel (I2C/SCCB timeouts, inestabilidad en el reloj master e incompatibilidad del driver MIPI CSI en el SDK de Espressif). Esta rama sirve como registro técnico e histórico del código base y controladores desarrollados para futuros intentos de integración.

---

## 📂 Estructura General del Repositorio en esta Rama

```text
Proyecto_Final_IA/ (Rama: live_video_feed)
├── README.md                                 # Esta documentación (Actualizada)
├── .gitignore                                # Filtros Git para evitar subir archivos pesados o temporales
├── Reporte_Proyecto_Vision_Artificial.pdf     # Reporte en PDF del proyecto
├── live_video_feed/                          # [CÓDIGO EXPERIMENTAL DEL LIVE FEED]
│   ├── CMakeLists.txt                        # Configuración CMake con rutas de componentes relativas (Portables)
│   ├── partitions.csv                        # Tabla de particiones de memoria flash
│   ├── sdkconfig.defaults                    # Parámetros por defecto para compilar en ESP32-P4
│   ├── README_live.md                        # Registro original de limitaciones y problemas de la cámara
│   ├── main/
│   │   ├── idf_component.yml                 # Declaración de componentes de sensores de cámara
│   │   ├── main.cpp                          # Bucle experimental de captura, debayerización, inferencia y envío serial
│   │   ├── mipi_camera.c                     # Driver de bajo nivel para MIPI CSI en ESP32-P4 (Experimental)
│   │   └── mipi_camera.h                     # Cabecera del driver de cámara
│   └── pc_visualizer/
│       └── live_feed.py                      # Script de Python para decodificación Base64 y visualización con OpenCV
├── Proyecto_Final_IA/                        # [CÓDIGO DE SIMULACIÓN DE IMÁGENES FIJAS - ESTABLE]
│   └── esp-dl/                               # Repositorio base de ESP-DL
│       ├── examples/
│       │   └── obstacle_person_detect/       # Inferencia local sobre imágenes fijas cargadas vía SPIFFS
│       └── models/
│           └── obstacle_person_detect/       # Modelo quantizado .espdl (Pico 224x224, Obstáculos y Personas)
└── Vision_Carro_Delivery_dataset_adquisition/# Metadatos y MATLAB script para preparación del dataset
```

---

## ⚠️ Registro de Limitaciones y Errores de la Cámara (MIPI CSI)

Tal como se documenta en [README_live.md](file:///C:/Users/Sr.p/Desktop/Proyecto_Final_IA/live_video_feed/README_live.md), los principales obstáculos encontrados al intentar correr el flujo de video en vivo fueron:

1. **Error de Sonda de Cámara (I2C/SCCB)**: Fallos constantes del tipo `Camera Probe Failed` o lecturas de timeout al intentar inicializar los registros del sensor OV5647 en la dirección `0x36` por el bus SCCB (pines SDA `GPIO 7` y SCL `GPIO 8`).
2. **Inestabilidad del Reloj Maestro (XCLK)**: Dificultades para enrutar una señal de reloj limpia y estable de 10MHz a 24MHz desde el chip de la ESP32-P4 al cristal del sensor de imagen.
3. **Picos de Corriente y Alimentación**: Caídas de tensión (brownouts) del microcontrolador al alimentar la cámara directamente de los rieles de 3.3V de la placa.
4. **Capturas Negras y Timeouts**:
   * En el software, la llamada a `mipi_camera_capture()` frecuentemente arrojaba tiempos de espera agotados: `Camera capture timeout! Frame didn't finish.`
   * Adicionalmente, el código incluye una verificación de depuración en `main.cpp` para reportar si la cámara envía imágenes completamente negras (promedio de píxeles en cero).

---

## 🛠️ Instrucciones de Compilación y Visualización

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
   *(Las dependencias del modelo de IA apuntan de forma relativa a `Proyecto_Final_IA/esp-dl` para que la compilación sea completamente portable sin rutas absolutas de usuario).*

### 2. Ejecución del Visualizador de PC
1. Instale las dependencias de Python:
   ```bash
   pip install opencv-python pyserial numpy
   ```
2. Abra el script `live_video_feed/pc_visualizer/live_feed.py` y modifique la línea 9 con el puerto COM de su placa:
   ```python
   SERIAL_PORT = 'COM4'  # Reemplace por su puerto COM (ej. COM3, COM5)
   ```
3. Asegúrese de **cerrar cualquier monitor serial** (para liberar el puerto de comunicación).
4. Ejecute el script:
   ```bash
   python live_video_feed/pc_visualizer/live_feed.py
   ```
5. Presione la tecla `q` en la ventana de OpenCV para salir del visualizador.
