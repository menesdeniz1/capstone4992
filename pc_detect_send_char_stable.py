"""
pc_detect_send_char_stable.py
----------------------------------------------------------------
Detects color using YOLOv8 and sends the most frequently
detected class in the last 8 frames (with >60% confidence)
to the Arduino as a single character (R, G, B, Y, O, N, M)
every 1.5 seconds.
"""

import cv2, time, serial, numpy as np, torch, collections
from ultralytics import YOLO

# ─── Configuration ─────────────────────────────────────────────────────────────
MODEL_PATH      = "en/epo100batch16mm881.pt"
PORT            = "COM13"
BAUD_RATE       = 9600
CONF_TH         = 0.60     # confidence threshold
SEND_INTERVAL_S = 1.5      # serial send interval in seconds
WINDOW_SIZE     = 8        # sliding window size for majority voting

CLASS2CHAR = {
    "red":    "R",
    "green":  "G",
    "blue":   "B",
    "yellow": "Y",
    "orange": "O",
    "brown":  "N",
    "non-mm": "M",
}

# ─── Load model and warm-up ────────────────────────────────────────────────────
model = YOLO(MODEL_PATH)
model.predict(np.zeros((640, 480, 3), dtype=np.uint8), verbose=False)

# ─── Open camera ───────────────────────────────────────────────────────────────
cap = cv2.VideoCapture(1, cv2.CAP_DSHOW)
cap.set(cv2.CAP_PROP_FRAME_WIDTH , 640)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)
if not cap.isOpened():
    raise SystemExit("Camera could not be opened")

# ─── Open serial port ──────────────────────────────────────────────────────────
ser = serial.Serial(PORT, BAUD_RATE, timeout=1)
time.sleep(2)
print("Serial connection established")

# ─── State variables ───────────────────────────────────────────────────────────
history         = collections.deque(maxlen=WINDOW_SIZE)
last_char       = ""
last_send_time  = 0.0

# ─── Main loop ─────────────────────────────────────────────────────────────────
while True:
    frame_start = time.time()
    ok, frame = cap.read()
    if not ok:
        print("Camera read error")
        break

    res = model(frame, verbose=False)
    boxes = [b for b in (res[0].boxes if res and res[0].boxes else [])
             if float(b.conf[0]) >= CONF_TH]

    if boxes:
        best = max(boxes, key=lambda b: float(b.conf[0]))
        cls  = model.names[int(best.cls[0])]
        history.append(cls)

        if len(history) == WINDOW_SIZE:
            majority = max(set(history), key=history.count)
            char     = CLASS2CHAR.get(majority, "")

            now = time.time()
            if char and char.isalpha() and len(char) == 1 and char != last_char and (now - last_send_time) >= SEND_INTERVAL_S:
                try:
                    ser.write((char + "\n").encode())
                    ser.flush()
                    print(f"Sent: {char} ({majority})")
                    last_char, last_send_time = char, now
                except serial.SerialException as e:
                    print("Serial write error:", e)
                    ser.reset_output_buffer()

        # draw box (optional)
        x1, y1, x2, y2 = map(int, best.xyxy[0])
        cv2.rectangle(frame, (x1, y1), (x2, y2), (0, 255, 0), 2)
        cv2.putText(frame, cls, (x1, y1 - 10),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)

    # display frame
    cv2.imshow("Detect", frame)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

    del res, boxes
    torch.cuda.empty_cache()
    fps = 1 / (time.time() - frame_start)
    print(f"FPS: {fps:.1f}", end="\r")

# ─── Cleanup ───────────────────────────────────────────────────────────────────
cap.release()
cv2.destroyAllWindows()
ser.close()
