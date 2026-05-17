# AI-Illegal-Parking-Detection-ESP32CAM
AI-powered illegal parking detection system using ESP32-CAM, Hugging Face API, Embedded AI, IoT, and Web Dashboard.


📌 Project Overview

This project was developed as a Major Capstone Project in the field of Embedded AI + IoT using the ESP32-CAM module.

The main objective of the system is to detect vehicles in restricted or illegal parking zones using computer vision techniques and generate alerts automatically.

Initially, the project started with an Edge Impulse TinyML approach, where a lightweight object detection model was trained directly for edge deployment. However, due to real-world limitations such as:

-Thermal heating of the ESP32-CAM
-Lighting condition variations
-False detections of dark/black objects
-Limited computational capability of edge hardware
-Submission deadline pressure

the project architecture was redesigned and migrated toward a cloud-assisted AI inference system using the Hugging Face DETR Object Detection API.

This transition transformed the project from a simple embedded system into an AI + IoT based intelligent monitoring system.


🎯 Objectives of the Project

1️⃣ Vehicle Detection for Illegal Parking Monitoring
To design an intelligent surveillance system capable of:

-Detecting vehicles automatically
-Monitoring unauthorized parking activity
-Triggering buzzer and LED alerts
-Capturing real-time images using ESP32-CAM


2️⃣ Integrating Edge Hardware with Cloud AI Services
To explore practical implementation of:

-ESP32-CAM based IoT systems
-Embedded AI concepts
-Cloud AI inference using Hugging Face APIs
-Real-time web monitoring interface


🧠 Project Journey & Learning Experience
This project was not only about building a system but also about understanding how AI can interact with embedded hardware.

As electronics students, we initially entered this domain without deep knowledge of AI or computer vision. We were mainly focused on solving the objective. During the process, we explored multiple technologies, faced failures, debugging challenges, hardware limitations, and eventually completed the project successfully.

🔬 Initial Approach – Edge Impulse TinyML Method
Initially, the project used:

-Edge Impulse
-TinyML object detection
-ESP32-CAM live inference

Dataset:
Around 500+ vehicle images
Labels: bike, car
⚠️ Problems Faced in Edge Impulse Method

Despite successful model training, several practical issues occurred:
❌ False Detection Problems

The model started detecting:

-Black objects
-Shadows
-Dark surfaces
-as vehicles.
❌ Lighting Condition Issues

Detection accuracy changed heavily due to:

-Day/Night conditions
-Outdoor sunlight variations
-Camera exposure instability
❌ Thermal Heating of ESP32-CAM

Continuous live inference caused:

-Board overheating
-Performance instability
-Frequent crashes
❌ Limited Edge Processing Capability

ESP32-CAM has:

-Limited RAM
-Limited compute power
-Difficulty handling real-time object detection reliably
🔄 Shift to Hugging Face API Approach

As the submission phase was approaching, we decided to redesign the architecture.

Instead of performing AI inference on-device, the ESP32-CAM:

-Captures image
-Converts image into Base64
-Sends image to Hugging Face API
-Receives detection result
-Activates buzzer and alert system


🤖 AI Model Used
Hugging Face Model
facebook/detr-resnet-50

This model performs:

-Object detection
-Vehicle recognition
-Bounding box prediction


⚙️ Technologies Used
*Technology*               	*Purpose*
ESP32-CAM	              -  Image Capture
Arduino IDE	              -  Embedded Programming
C++	                      -  Firmware Development
Hugging Face API	      -  AI Vehicle Detection
Edge Impulse	          - Initial TinyML Training
Google Colab	          -  AI Testing & API Experiments
HTML/CSS/JavaScript	      -  Web Dashboard
WiFi IoT Communication	  -  Cloud Connectivity
PIR Sensor	              -  Motion Detection
FFmpeg Stream Concepts	  -  Camera Streaming
Embedded AI	              -  AI + Hardware Integration
ISD1820                   -  Voice Alerts 

🛠️ Features of Final System

✅ Vehicle Detection using AI
✅ Live ESP32-CAM Stream
✅ Maintenance Mode Web Dashboard
✅ PIR Motion Trigger
✅ Buzzer Alert System
✅ Flash LED Alert
✅ Web Interface for Monitoring
✅ IoT-Based AI Communication
✅ Automatic AI Analysis using Cloud API

🌐 Web Interface Features

The ESP32-CAM hosts a web dashboard containing:

-Live camera stream
-Mode switching
-Vehicle detection status
-Capture image option
-Maintenance mode controls


🔌 Hardware Components Used
*Component	Quantity*
ESP32-CAM	-1
PIR Motion Sensor	-1
Buzzer	-1
LEDs	-2
Jumper Wires	-Multiple
FTDI Programmer	-1
Breadboard	-1
isd1820 -1


📚 Key Learnings from the Project
This project provided practical exposure to:

Technical Learnings
ESP32-CAM programming
AI API integration
Hugging Face inference APIs
Edge AI concepts
TinyML workflow
Base64 image encoding
Embedded web servers
IoT communication
Google Colab experimentation
HTML-based control dashboards
Debugging embedded systems
Real-world AI deployment limitations
Personal Learnings
Problem-solving mindset
Project-based learning
Handling failures calmly
Adapting to alternative solutions
Importance of experimentation
Learning through debugging instead of rote learning


💡 Project Reflection

One of the biggest lessons from this project was understanding that engineering is not always about achieving the first planned solution successfully.

Even though the initial Edge Impulse implementation failed in real-world deployment, it gave us:

Vision
Practical knowledge
Confidence in Embedded AI

This project became a turning point toward:

AI + Hardware development
Practical engineering learning
Building systems through experimentation


🙏 Acknowledgment
We sincerely thank:

Our project mentor and guide
Faculty support
Our teammates
Open-source AI communities

for helping us complete this project successfully.

📅 Project Timeline
Event	                        Date
Initial Edge Impulse Training	Early 2025
Transition to Hugging Face API	March 2025
Final Submission	            03-2025
