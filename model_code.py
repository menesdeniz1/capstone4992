# Install dependencies
!pip install roboflow
!pip install ultralytics

# Import libraries
from roboflow import Roboflow
from ultralytics import YOLO
import torch

# Load dataset from Roboflow
rf = Roboflow(api_key="hdqSvyPvtqTfMrOkiEyU")
project = rf.workspace("mam-nv1e6").project("capstonefixed")
version = project.version(10)
dataset = version.download("yolov8")
                
# Load YOLOv8 segmentation model
model = YOLO("yolov8x-seg.pt")

# Train the model
results = model.train(
    data=f"{dataset.location}/data.yaml",
    epochs=100,            # Start with 100 epochs
    imgsz=640,             # Keep standard YOLOv8 resolution
    batch=16,              # Good batch size for GPU memory management
)
