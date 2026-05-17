#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <Base64.h>

// ==================== WiFi Credentials ====================
const char* ssid = "YOUR_WIFI_SSID";        // Change this
const char* password = "YOUR_WIFI_PASSWORD"; // Change this

// ==================== Hugging Face API ====================
const char* api_url = "https://api-inference.huggingface.co/models/facebook/detr-resnet-50";
const char* api_key = "YOUR_HUGGING_FACE_API_KEY"; // Change this

// ==================== Mode Selection ====================
#define DEFAULT_MODE 0      // Vehicle detection mode
#define MAINTENANCE_MODE 1  // Web server mode
int currentMode = DEFAULT_MODE;

// ==================== GPIO Pins ====================
#define PIR_SENSOR_PIN 13   // PIR Motion Sensor
#define BUZZER_PIN 15       // Buzzer
#define LED_PIN 2           // Status LED
#define FLASH_PIN 4         // Camera Flash
#define MODE_SWITCH_PIN 12  // Physical mode switch (optional)

// ==================== Camera Pin Configuration ====================
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// ==================== Global Variables ====================
WebServer server(80);
int httpFailCount = 0;
const int maxHttpFails = 3;
bool objectDetected = false;
unsigned long lastDetectionTime = 0;
unsigned long lastMotionTime = 0;
const unsigned long motionCooldown = 5000; // 5 seconds cooldown

// ==================== Setup Function ====================
void setup() {
  Serial.begin(115200);
  Serial.println("\n\n=================================");
  Serial.println("ESP32-CAM Vehicle Detection System");
  Serial.println("=================================\n");

  // Initialize GPIO pins
  pinMode(PIR_SENSOR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(FLASH_PIN, OUTPUT);
  pinMode(MODE_SWITCH_PIN, INPUT_PULLUP);
  
  // Initial LED indication
  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);
  delay(500);
  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);

  // Initialize Camera
  if (!initCamera()) {
    Serial.println("Camera initialization failed!");
    while (true) {
      digitalWrite(LED_PIN, HIGH);
      delay(200);
      digitalWrite(LED_PIN, LOW);
      delay(200);
    }
  }

  // Connect to WiFi
  connectToWiFi();

  // Setup web server routes
  setupWebServer();

  Serial.println("\n✅ System Ready!");
  Serial.print("🌐 Web Interface: http://");
  Serial.println(WiFi.localIP());
  Serial.println("📹 Mode: Vehicle Detection (Default)");
  Serial.println("🔄 Press GPIO12 button or use web interface to switch modes\n");
}

// ==================== Camera Initialization ====================
bool initCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_QVGA;
  config.jpeg_quality = 12;
  config.fb_count = 1;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;

  if (psramFound()) {
    config.jpeg_quality = 10;
    config.fb_count = 2;
    config.grab_mode = CAMERA_GRAB_LATEST;
    config.frame_size = FRAMESIZE_SVGA;
  } else {
    config.fb_location = CAMERA_FB_IN_DRAM;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed: 0x%x", err);
    return false;
  }

  sensor_t *s = esp_camera_sensor_get();
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
  
  return true;
}

// ==================== WiFi Connection ====================
void connectToWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  WiFi.setSleep(false);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n❌ WiFi connection failed!");
  }
}

// ==================== Web Server Setup ====================
void setupWebServer() {
  server.on("/", handleRoot);
  server.on("/mode", handleMode);
  server.on("/stream", handleStream);
  server.on("/capture", handleCapture);
  server.on("/status", handleStatus);
  server.onNotFound(handleNotFound);
  server.begin();
}

// ==================== Main Loop ====================
void loop() {
  server.handleClient();
  
  // Check physical mode switch
  if (digitalRead(MODE_SWITCH_PIN) == LOW) {
    if (currentMode != MAINTENANCE_MODE) {
      currentMode = MAINTENANCE_MODE;
      Serial.println("🔄 Switched to Maintenance Mode (Physical Switch)");
      digitalWrite(LED_PIN, HIGH);
      delay(200);
      digitalWrite(LED_PIN, LOW);
    }
  } else {
    if (currentMode != DEFAULT_MODE && currentMode != MAINTENANCE_MODE) {
      currentMode = DEFAULT_MODE;
      Serial.println("🔄 Switched to Vehicle Detection Mode (Physical Switch)");
    }
  }

  // Run vehicle detection only in DEFAULT mode
  if (currentMode == DEFAULT_MODE) {
    runVehicleDetection();
  } else {
    // In maintenance mode, blink LED slowly to indicate
    static unsigned long lastBlink = 0;
    if (millis() - lastBlink > 2000) {
      digitalWrite(LED_PIN, !digitalRead(LED_PIN));
      lastBlink = millis();
    }
  }
  
  delay(100);
}

// ==================== Vehicle Detection Logic ====================
void runVehicleDetection() {
  // Check PIR sensor with cooldown
  if (digitalRead(PIR_SENSOR_PIN) == HIGH && (millis() - lastMotionTime > motionCooldown)) {
    lastMotionTime = millis();
    Serial.println("\n🔍 Motion detected! Analyzing...");
    
    // Turn on flash and LED
    digitalWrite(FLASH_PIN, HIGH);
    digitalWrite(LED_PIN, HIGH);
    delay(500);
    
    // Capture image
    camera_fb_t *fb = esp_camera_fb_get();
    digitalWrite(FLASH_PIN, LOW);
    
    if (!fb) {
      Serial.println("❌ Camera capture failed");
      digitalWrite(LED_PIN, LOW);
      return;
    }

    Serial.println("📸 Image captured, sending to AI...");
    
    // Convert to Base64
    String imageBase64 = base64::encode(fb->buf, fb->len);
    esp_camera_fb_return(fb);

    // Send to Hugging Face API
    if (sendToAI(imageBase64)) {
      Serial.println("✅ AI analysis complete");
    } else {
      Serial.println("❌ AI analysis failed");
    }
    
    digitalWrite(LED_PIN, LOW);
  }
}

// ==================== Send to Hugging Face AI ====================
bool sendToAI(String imageBase64) {
  HTTPClient http;
  http.begin(api_url);
  http.addHeader("Authorization", String("Bearer ") + api_key);
  http.addHeader("Content-Type", "application/json");

  String payload = "{\"inputs\":\"" + imageBase64 + "\",\"parameters\":{\"threshold\":0.5}}";
  int httpResponseCode = http.POST(payload);

  if (httpResponseCode == 200) {
    String response = http.getString();
    Serial.println("AI Response: " + response);
    
    // Check for vehicles
    if (response.indexOf("\"car\"") >= 0 || 
        response.indexOf("\"bus\"") >= 0 ||
        response.indexOf("\"truck\"") >= 0 || 
        response.indexOf("\"motorbike\"") >= 0 ||
        response.indexOf("\"bicycle\"") >= 0 || 
        response.indexOf("\"van\"") >= 0) {
      
      Serial.println("🚗 VEHICLE DETECTED! Activating alarm...");
      objectDetected = true;
      lastDetectionTime = millis();
      
      // Sound alarm and flash lights
      for (int i = 0; i < 10; i++) {
        digitalWrite(BUZZER_PIN, HIGH);
        digitalWrite(LED_PIN, HIGH);
        delay(200);
        digitalWrite(BUZZER_PIN, LOW);
        digitalWrite(LED_PIN, LOW);
        delay(200);
      }
      
      http.end();
      return true;
    } else {
      Serial.println("❌ No vehicle detected");
      objectDetected = false;
      http.end();
      return true;
    }
  } else {
    Serial.printf("HTTP request failed: %d\n", httpResponseCode);
    httpFailCount++;
    
    if (httpFailCount >= maxHttpFails) {
      Serial.println("Too many failures, restarting...");
      ESP.restart();
    }
    
    http.end();
    return false;
  }
}

// ==================== Web Server Handlers ====================
void handleRoot() {
  if (currentMode != MAINTENANCE_MODE) {
    String html = "<!DOCTYPE html><html><head><title>ESP32-CAM</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>body{font-family:Arial;text-align:center;padding:20px;}";
    html += "button{background:#4CAF50;color:white;padding:15px 30px;margin:10px;font-size:18px;border:none;border-radius:5px;cursor:pointer;}";
    html += "button:hover{background:#45a049;}</style></head>";
    html += "<body><h1>🔒 Vehicle Detection Mode Active</h1>";
    html += "<p>System is automatically detecting vehicles using PIR sensor</p>";
    html += "<button onclick=\"location.href='/mode?mode=maintenance'\">🔧 Switch to Maintenance Mode</button>";
    html += "</body></html>";
    server.send(200, "text/html", html);
    return;
  }
  
  // Full web interface for maintenance mode
  String html = getWebpage();
  server.send(200, "text/html", html);
}

void handleMode() {
  if (server.hasArg("mode")) {
    String mode = server.arg("mode");
    if (mode == "maintenance") {
      currentMode = MAINTENANCE_MODE;
      server.send(200, "text/plain", "Switched to Maintenance Mode");
      Serial.println("🔄 Switched to Maintenance Mode (Web)");
    } else if (mode == "default") {
      currentMode = DEFAULT_MODE;
      server.send(200, "text/plain", "Switched to Vehicle Detection Mode");
      Serial.println("🔄 Switched to Vehicle Detection Mode (Web)");
    } else {
      server.send(400, "text/plain", "Invalid mode");
    }
  } else {
    server.send(400, "text/plain", "Mode parameter missing");
  }
}

void handleStream() {
  if (currentMode != MAINTENANCE_MODE) {
    server.send(403, "text/plain", "Access denied");
    return;
  }
  
  WiFiClient client = server.client();
  String response = "HTTP/1.1 200 OK\r\n";
  response += "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
  server.sendContent(response);
  
  while (client.connected()) {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
      break;
    }
    
    String frame = "--frame\r\nContent-Type: image/jpeg\r\n\r\n";
    server.sendContent(frame);
    client.write(fb->buf, fb->len);
    server.sendContent("\r\n");
    
    esp_camera_fb_return(fb);
    delay(50);
  }
}

void handleCapture() {
  if (currentMode != MAINTENANCE_MODE) {
    server.send(403, "text/plain", "Access denied");
    return;
  }
  
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    server.send(500, "text/plain", "Capture failed");
    return;
  }
  
  WiFiClient client = server.client();
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: image/jpeg");
  client.println("Connection: close");
  client.print("Content-Length: ");
  client.println(fb->len);
  client.println();
  client.write(fb->buf, fb->len);
  
  esp_camera_fb_return(fb);
}

void handleStatus() {
  String status = objectDetected ? "Vehicle Detected" : "No Vehicle";
  server.send(200, "text/plain", status);
}

void handleNotFound() {
  server.send(404, "text/plain", "Not found");
}

// ==================== Webpage Generator ====================
String getWebpage() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<title>ESP32-CAM Control Panel</title>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>";
  html += "body{font-family:Arial;text-align:center;margin:0;padding:20px;background:#f0f0f0;}";
  html += ".container{max-width:800px;margin:auto;background:white;padding:20px;border-radius:10px;box-shadow:0 2px 10px rgba(0,0,0,0.1);}";
  html += "button{padding:10px 20px;margin:5px;font-size:16px;background:#4CAF50;color:white;border:none;border-radius:5px;cursor:pointer;}";
  html += "button:hover{background:#45a049;}";
  html += ".mode-buttons{background:#e0e0e0;padding:15px;border-radius:5px;margin:20px 0;}";
  html += ".video-container{margin:20px 0;}";
  html += "img{max-width:100%;border:1px solid #ddd;border-radius:5px;}";
  html += ".status{padding:15px;background:#e0e0e0;border-radius:5px;margin:20px 0;}";
  html += "#statusText{font-weight:bold;font-size:18px;}";
  html += "</style>";
  html += "</head><body>";
  html += "<div class='container'>";
  html += "<h1>🎥 ESP32-CAM Control Panel</h1>";
  
  html += "<div class='mode-buttons'>";
  html += "<h2>System Mode</h2>";
  html += "<button onclick='setMode(\"default\")'>🚗 Vehicle Detection Mode</button>";
  html += "<button onclick='setMode(\"maintenance\")'>🔧 Maintenance Mode (Current)</button>";
  html += "</div>";
  
  html += "<div class='video-container'>";
  html += "<h2>Live Stream</h2>";
  html += "<img id='stream' src='/stream'>";
  html += "<div><button onclick='startStream()'>▶️ Start Stream</button>";
  html += "<button onclick='stopStream()'>⏹️ Stop Stream</button>";
  html += "<button onclick='captureImage()'>📸 Capture Image</button></div>";
  html += "</div>";
  
  html += "<div class='status'>";
  html += "<h2>Detection Status</h2>";
  html += "<p id='statusText'>Checking...</p>";
  html += "<button onclick='checkStatus()'>🔄 Refresh Status</button>";
  html += "</div>";
  
  html += "<p><strong>ESP32-CAM</strong> | Vehicle Detection System</p>";
  html += "</div>";
  
  html += "<script>";
  html += "function setMode(mode){";
  html += "fetch('/mode?mode='+mode).then(r=>r.text()).then(d=>{alert(d);location.reload();});}";
  html += "function startStream(){document.getElementById('stream').src='/stream';}";
  html += "function stopStream(){document.getElementById('stream').src='';}";
  html += "function captureImage(){window.open('/capture');}";
  html += "function checkStatus(){fetch('/status').then(r=>r.text()).then(d=>{";
  html += "document.getElementById('statusText').innerHTML=d;";
  html += "if(d.includes('Detected'))document.getElementById('statusText').style.color='red';";
  html += "else document.getElementById('statusText').style.color='green';});}";
  html += "setInterval(checkStatus,5000);checkStatus();";
  html += "</script>";
  html += "</body></html>";
  
  return html;
}
