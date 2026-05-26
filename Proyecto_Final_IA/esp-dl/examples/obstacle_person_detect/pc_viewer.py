import serial
import serial.tools.list_ports
import json
import cv2
import os
import sys

# The folder where original images are stored
IMAGE_DIR = r"main\images"

def find_esp32_port():
    ports = serial.tools.list_ports.comports()
    # If there's only one port, just return it
    if len(ports) == 1:
        return ports[0].device
    # Otherwise print options and ask user
    print("Available COM ports:")
    for p in ports:
        print(f" - {p.device}: {p.description}")
    
    port = input("Enter your ESP32 COM port (e.g., COM3): ").strip()
    return port

def main():
    port = find_esp32_port()
    if not port:
        print("No COM port found.")
        return

    baudrate = 115200
    try:
        ser = serial.Serial(port, baudrate, timeout=1)
        print(f"Connected to {port} at {baudrate} baud.")
        print("Waiting for ESP32 output... (Make sure to close other Serial Monitors!)")
    except Exception as e:
        print(f"Error opening serial port {port}: {e}")
        print("Make sure you closed the ESP-IDF monitor!")
        return

    json_str = ""
    in_json = False

    while True:
        try:
            line = ser.readline().decode('utf-8', errors='ignore').strip()
        except Exception:
            continue

        if not line:
            continue
        
        if line == ">>>JSON_START<<<":
            in_json = True
            json_str = ""
            continue
            
        if line == ">>>JSON_END<<<":
            in_json = False
            try:
                data = json.loads(json_str)
                process_image(data)
            except json.JSONDecodeError as e:
                print(f"Failed to parse JSON: {e}")
                print(f"Raw string: {json_str}")
            continue
            
        if in_json:
            json_str += line + "\n"
        else:
            # Print standard ESP32 logs
            print(f"[ESP32] {line}")

def process_image(data):
    filename = data.get("image")
    detections = data.get("detections", [])
    
    if not filename:
        return
        
    img_path = os.path.join(IMAGE_DIR, filename)
    if not os.path.exists(img_path):
        print(f"Image not found on PC: {img_path}")
        return
        
    print(f"\nAnalyzed {filename}: Found {len(detections)} objects")
    
    # Load image using OpenCV
    img = cv2.imread(img_path)
    if img is None:
        print(f"Failed to load image: {img_path}")
        return
        
    # Draw bounding boxes
    for det in detections:
        cat = det["category"]
        score = det["score"]
        # box format from ESP-DL is usually [x1, y1, x2, y2]
        x1, y1, x2, y2 = det["box"]
        
        # Draw rectangle (Green)
        cv2.rectangle(img, (x1, y1), (x2, y2), (0, 255, 0), 2)
        
        # Map category ID to label name
        class_names = {0: "obstacle", 1: "person"}
        class_name = class_names.get(cat, f"Class {cat}")
        
        # Draw label
        label = f"{class_name} ({score:.2f})"
        cv2.putText(img, label, (x1, max(15, y1 - 10)), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2)
        
    # Show the image
    cv2.imshow("ESP32 Detection Output", img)
    print(">>> Press any key on the image window to continue, or 'q' to quit.")
    
    key = cv2.waitKey(0) & 0xFF
    if key == ord('q'):
        cv2.destroyAllWindows()
        sys.exit(0)

if __name__ == "__main__":
    main()
