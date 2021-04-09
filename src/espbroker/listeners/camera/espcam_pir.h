/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-cam-take-photo-display-web-server/

  IMPORTANT!!!
   - Select Board "AI Thinker ESP32-CAM"
   - GPIO 0 must be connected to GND to upload a sketch
   - After connecting GPIO 0 to GND, press the ESP32-CAM on-board RESET button to put your board in flashing mode

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/
#include "pin_def.h"
#include "ui.h"

#include "WiFi.h"
#include "esp_camera.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "driver/rtc_io.h"
#include <ESPAsyncWebServer.h>
#include <StringArray.h>
#include <SPIFFS.h>
#include <FS.h>

#include "../base.h"

namespace Modules {

#define MAX_CAPTURE 1000
int capture_increment = 0;
// = "/photo.jpg";


AsyncWebServer server(80); // Create AsyncWebServer object on port 80

class ESPCamPIRCombo: public Listener {
  public:
  String FILE_PHOTO = "";
  
  const char* ssid = "mbayar0"; // Replace with your network credentials
  const char* password = "qwertyuu";

  boolean takeNewPhoto = false;
  unsigned long timestamp;
  unsigned long prev_timestamp;

  ESPCamPIRCombo(): Listener(ListenerType::EXTENDED) {}

  private:
  String incomingByte;
  unsigned long currentTime;
  unsigned long prevTime = 10000;
  unsigned long interval = 10000;
  // Photo File Name to save in SPIFFS
  
  String filePhotoName() {
    if (timestamp > 0 && prev_timestamp != timestamp) {
      prev_timestamp = timestamp;
      return "/photo-" + String(timestamp) + ".jpg";
    }

    if (capture_increment < MAX_CAPTURE) {
      return "/photo-" + String(capture_increment++) + ".jpg";
    }

    return "";
  }

  void cameraInit()
  {
    // OV2640 camera module
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
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;

    // pinMode(GPIO_NUM_4, INPUT);

    if (psramFound()) {
      config.frame_size = FRAMESIZE_UXGA;
      config.jpeg_quality = 10;
      config.fb_count = 2;
    } else {
      config.frame_size = FRAMESIZE_SVGA;
      config.jpeg_quality = 12;
      config.fb_count = 1;
    }
    // Camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
      Serial.printf("Camera init failed with error 0x%x", err);
      ESP.restart();
    }
  }

  void serverInit() {
    // Route for root / web page
    server.on("/", HTTP_GET, [&](AsyncWebServerRequest * request) {
      request->send_P(200, "text/html", index_html);
    });

    server.on("/capture", HTTP_GET, [&](AsyncWebServerRequest * request) {
      takeNewPhoto = true;
      Serial.println("Capturing");
      request->send_P(200, "text/plain", "Taking Photo");
    });

    server.on("/photo", HTTP_GET, [&](AsyncWebServerRequest * request) {
      if (request->hasParam("i")) {
        AsyncWebParameter* p = request->getParam("i");
        String imgName = "/photo-"+p->value()+".jpg";
        request->send(SPIFFS, imgName.c_str(), "image/jpg", false);
      } else {
        request->send(SPIFFS, FILE_PHOTO.c_str(), "image/jpg", false);
      }
    });

    // Start server
    server.begin();
  }

  // Check if photo capture was successful
  bool checkPhoto( fs::FS &fs ) {
    File f_pic = fs.open( FILE_PHOTO );
    unsigned int pic_sz = f_pic.size();
    return ( pic_sz > 100 );
  }

  // Capture Photo and Save it to SPIFFS
  void capturePhotoSaveSpiffs( void ) {
    // rtc_gpio_hold_dis(GPIO_NUM_4);

    camera_fb_t * fb = NULL; // pointer
    bool ok = 0; // Boolean indicating if the picture has been taken correctly

    do {
      digitalWrite(GPIO_NUM_4, HIGH);
      // Take a photo with the camera
      Serial.println("Taking a photo...");

      fb = esp_camera_fb_get();
      if (!fb) {
        Serial.println("Camera capture failed");
        return;
      }

      // Photo file name
      Serial.printf("Picture file name: %s\n", FILE_PHOTO.c_str());
      File file = SPIFFS.open(FILE_PHOTO.c_str(), FILE_WRITE);

      // Insert the data in the photo file
      if (!file) {
        Serial.println("Failed to open file in writing mode");
      }
      else {
        file.write(fb->buf, fb->len); // payload (image), payload length
        Serial.print("The picture has been saved in ");
        Serial.print(FILE_PHOTO);
        Serial.print(" - Size: ");
        Serial.print(file.size());
        Serial.println(" bytes");
      }
      // Close the file
      file.close();
      esp_camera_fb_return(fb);

      digitalWrite(GPIO_NUM_4, LOW);
      // rtc_gpio_hold_en(GPIO_NUM_4);

      // check if file has been correctly saved in SPIFFS
      ok = checkPhoto(SPIFFS);
    } while ( !ok );
  }

  public:
  void initListener() override {
    Serial.setDebugOutput(true);

    pinMode(ANALOG_PIN, OUTPUT);
    // Connect to Wi-Fi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.println("Connecting to WiFi...");
    }

    if (!SPIFFS.begin(true)) {
      Serial.println("An Error has occurred while mounting SPIFFS");
      ESP.restart();
    } else {
      delay(500);
      Serial.println("SPIFFS mounted successfully");
    }

    // Print ESP32 Local IP Address
    Serial.print("IP Address: http://");
    Serial.println(WiFi.localIP());

    // Turn-off the 'brownout detector'
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
    
    cameraInit();
    
    serverInit();

    EspNowMQ.on("CAMERA_TAKE", [&](const uint8_t* data, uint8_t size){
        memcpy(&rx_Msg, data, size);
        Serial.println(rx_Msg.data.name);
        
        takePhoto(rx_Msg.data.getValue<bool>());
    });

    EspNowMQ.on("TIMESTAMP", [&](const uint8_t* data, uint8_t size){
        memcpy(&rx_Msg, data, size);
        Serial.println(rx_Msg.data.name);

        timestamp = rx_Msg.data.toInt();
    });
  }

  void alertIntruder() {
    EspNowMQ.send("CAMERA_INTRUDER", masterMac, "INTRUDER", (int *)1);
    Serial.println("Intruder");
    takePhoto();
  }

  void getTimestamp() {
    EspNowMQ.send("CAMERA_TIMESTAMP", masterMac, "TIMESTAMP", (int *)1);
  }

  void takePhoto(bool take = true) {
    takeNewPhoto = take;
  }

  void loop() override {
    currentTime = millis();
    if (digitalRead(INPUT_PIN_1) == HIGH) {
      if ((unsigned long) currentTime - prevTime >= interval) {
        alertIntruder();
        prevTime = currentTime;
      } else if (digitalRead(ANALOG_PIN) == HIGH) digitalWrite(ANALOG_PIN, LOW);
    } else {
      if (digitalRead(ANALOG_PIN) == HIGH) digitalWrite(ANALOG_PIN, LOW);
      // Serial.println("no Intruder");
    }

    if (takeNewPhoto) {
      FILE_PHOTO = filePhotoName();
      if (!FILE_PHOTO.equals("")) {
        capturePhotoSaveSpiffs();
      }
      takeNewPhoto = false;
    }

    delay(10);
  }
};

}