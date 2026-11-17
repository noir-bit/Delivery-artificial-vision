# Sistema Embebido Edge AI: Detección de Obstáculos y Peatones (ESP32-P4 + OV5647)

Proyecto Final de Inteligencia Artificial (2026-2) — Ingeniería Mecatrónica (UMNG).

## ¿Qué incluye esta actualización (`feature/live-mipi-csi-abet`)?
Esta rama contiene la versión completa y funcional del sistema de percepción **Edge AI** ejecutándose **100% dentro del microcontrolador RISC-V ESP32-P4**:

1. **Video en vivo MIPI-CSI + Hardware ISP (`main/app_main.cpp`):**
   - Captura en tiempo real desde cámara física **OV5647** (Raspberry Pi Camera rev 1.3) a `800×640` RAW8 (50 FPS).
   - Conversión `RAW8 -> RGB565` por Hardware ISP + DMA hacia PSRAM (alineación de caché de 128 bytes).
   - Calibración I2C/SCCB del sensor (`VTS=3500` líneas para evitar *exposure wrap-around* y `0x5000=0x07` para desactivar LENC sin calibrar).
   - Balance de blancos *Gray-World* en punto fijo Q8 (`<= 3.5x`).
   - Modo de respaldo automático (*fallback*) a las 12 imágenes del campus UMNG en SPIFFS si no se detecta cámara.
2. **Inferencia CNN (`ESPDet Pico 224×224 INT8`):**
   - Ejecución en hardware con `ESP-DL` y aceleración SIMD PIE (~106 ms por inferencia).
3. **Telemetría de Alta Velocidad y Dashboard (`dashboard_viewer.py`):**
   - Transmisión UART0 a **921600 baudios** con video `266×212` en Base64 + JSON de detecciones.
   - Filtro perceptual `LAB-CLAHE` + filtro bilateral en PC.
   - Métricas de validación ABET sobre *Ground Truth* (`mAP@0.50: 85.4%`, `Mean IoU: 76.8%`, `Precision/Recall/F1: 88.9%`).
4. **Documentación y Evidencias (`Doc/`):**
   - Plantilla oficial ABET diligenciada, Anexo Técnico E1–E5 con gráficas y capturas reales, y Guía de Sustentación.

---

## Cómo ejecutar el Dashboard (si la placa ya tiene el firmware)
1. Conecta la ESP32-P4 por USB (`COM5` o puerto detectado automáticamente).
2. Ejecuta `esp-dl/examples/obstacle_person_detect/INICIAR_DASHBOARD.bat`.

## Cómo compilar y cargar el firmware (ESP-IDF v5.3.5)
```cmd
cd esp-dl/examples/obstacle_person_detect
idf.py set-target esp32p4
idf.py build flash
```
*(También puedes usar `COMPILAR_Y_CARGAR_ESP32.bat` en la raíz).*

---

## Cómo subir esta carpeta a una nueva rama en GitHub
Si ya tienes el repositorio de GitHub creado, abre una terminal dentro de esta carpeta y ejecuta:
```bash
git remote add origin https://github.com/TU_USUARIO/TU_REPOSITORIO.git
git push -u origin feature/live-mipi-csi-abet
```
