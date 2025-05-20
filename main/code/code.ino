//*************************************************************************
// Khai báo các thư viện ngoại vi

#include <Arduino.h>              // Thư viện chuẩn của Arduino cung cấp các hàm cơ bản
#include <Wire.h>                 // Thư viện giao tiếp I2C (cho LCD I2C, cảm biến, v.v.)
#include <LiquidCrystal_I2C.h>    // Thư viện điều khiển màn hình LCD qua I2C
#include <ESP32Servo.h>           // Thư viện điều khiển động cơ servo trên ESP32
#include "DHT.h"                  // Thư viện hỗ trợ cảm biến DHT (DHT11, DHT22)
#include "freertos/task.h"        // Thư viện FreeRTOS cho việc quản lý task (nhiệm vụ)

// Khởi tạo đối tượng LCD với địa chỉ I2C 0x27 và kích thước 16x2 ký tự
LiquidCrystal_I2C lcd(0x27, 16, 2);

//*************************************************************************
// Khai báo các chân và đối tượng servo

static const int servoPin = 15;   // Chân kết nối với động cơ servo (GPIO15)
Servo servo1;                     // Khai báo đối tượng servo để điều khiển servo

//*************************************************************************
// Khai báo các chân kết nối với cảm biến khí gas
#define sensorPin1  25           // Chân GPIO25 nối với cảm biến khí gas 1
#define sensorPin2  26           // Chân GPIO26 nối với cảm biến khí gas 2
#define sensorPin3  27           // Chân GPIO27 nối với cảm biến khí gas 3

//*************************************************************************
// Khai báo chân kết nối với cảm biến DHT11

#define DHTPIN1 13               // Chân GPIO13 cho cảm biến DHT11 số 1
#define DHTPIN2 12               // Chân GPIO12 cho cảm biến DHT11 số 2
#define DHTPIN3 14               // Chân GPIO14 cho cảm biến DHT11 số 3

// Định nghĩa loại cảm biến DHT (ở đây là DHT11)
#define DHTTYPE DHT11

// Khởi tạo ba đối tượng DHT cho 3 cảm biến
DHT dht1(DHTPIN1, DHTTYPE);        // Cảm biến DHT số 1
DHT dht2(DHTPIN2, DHTTYPE);        // Cảm biến DHT số 2
DHT dht3(DHTPIN3, DHTTYPE);        // Cảm biến DHT số 3

//*************************************************************************
// Khai báo các chân kết nối với cảm biến phát hiện ngọn lửa

#define Flame1_PIN 5             // Chân GPIO5 cho cảm biến ngọn lửa số 1
#define Flame2_PIN 18            // Chân GPIO18 cho cảm biến ngọn lửa số 2
#define Flame3_PIN 19            // Chân GPIO19 cho cảm biến ngọn lửa số 3

//*************************************************************************
// Khai báo các giá trị toàn cục lưu trữ dữ liệu cảm biến

/**
 * Các biến toàn cục sau dùng để lưu trữ giá trị đọc được từ các cảm biến.
 * Việc sử dụng các biến toàn cục giúp các hàm khác nhau có thể truy cập và xử lý dữ liệu dễ dàng.
 */
uint8_t Flame1_Value = 0;
uint8_t Flame2_Value = 0;
uint8_t Flame3_Value = 0;

uint8_t Temp1_Value = 0;
uint8_t Temp2_Value = 0;
uint8_t Temp3_Value = 0;

uint8_t Hum1_Value = 0;
uint8_t Hum2_Value = 0;
uint8_t Hum3_Value = 0;

uint8_t CO1_Value = 0;
uint8_t CO2_Value = 0;
uint8_t CO3_Value = 0;

//*************************************************************************
// Phần cấu hình APP sử dụng ERa
/* 
   Định nghĩa vị trí máy chủ ERa, trong trường hợp này là Việt Nam.
   Có thể thay đổi bằng cách bỏ comment định nghĩa ERA_LOCATION_SG nếu cần.
*/
#define ERA_LOCATION_VN
// #define ERA_LOCATION_SG

// Mã xác thực (Auth Token) để kết nối với ERa App hoặc Dashboard
#define ERA_AUTH_TOKEN "74cbea38-26e9-4f03-9571-565cc6a5f834"

// Bao gồm các thư viện của ERa cung cấp các tính năng Smart, đồng bộ thời gian, v.v.
#include <ERa.hpp>
#include <Automation/ERaSmart.hpp>
#include <Time/ERaEspTime.hpp>

// Khai báo thông tin kết nối WiFi (SSID và mật khẩu)
const char ssid[] = "Vu Van Thien";
const char pass[] = "0382011198";

// Khởi tạo đối tượng đồng bộ thời gian và đối tượng ERaSmart
ERaEspTime syncTime;             // Đối tượng quản lý thời gian từ ERa
ERaSmart smart(ERa, syncTime);   // Đối tượng ERaSmart sử dụng ERa và đồng bộ thời gian

//*************************************************************************
// Các hàm callback của ERa

/* 
   Hàm này được gọi khi có lệnh gửi tin nhắn SMS từ ERa.
   Nó ghi log thông tin tin nhắn và trả về true để xác nhận đã xử lý.
*/
ERA_WRITE_SMS() {
    ERA_LOG("ERa", "Write SMS to %s: %s", to, message);
    return true;
}

/* 
   Hàm này được gọi mỗi khi thiết bị kết nối thành công với ERa.
   Sử dụng để thông báo hoặc thực hiện các thiết lập sau khi kết nối.
*/
ERA_CONNECTED() {
    ERA_LOG("ERa", "ERa connected!");
}

/* 
   Hàm này được gọi mỗi khi thiết bị mất kết nối với ERa.
   Có thể dùng để cập nhật trạng thái hiển thị hoặc thực hiện thao tác khôi phục.
*/
ERA_DISCONNECTED() {
    ERA_LOG("ERa", "ERa disconnected!");
}


//*********************************************************************************RETURN MQ2 VALUE
/**
 * Hàm lấy và trả về giá trị đo được của cảm biến khí gas
 * Hàm trả về giá trị digital
 * Với mức 1 được cài đặt tương ứng với nồng độ khí gas >= 20%
 */
uint8_t MQ21_getValue() {
  uint8_t value1 = digitalRead(sensorPin1);
  if(value1 == HIGH){
    return 0;
  }
  else{
    return 1;
  }
}
uint8_t MQ22_getValue() {
  uint8_t value2 = digitalRead(sensorPin2);
  if(value2 == HIGH){
    return 0;
  }
  else{
    return 1;
  }
}
uint8_t MQ23_getValue() {
  uint8_t value3 = digitalRead(sensorPin3);
  if(value3 == HIGH){
    return 0;
  }
  else{
    return 1;
  }
}
//*********************************************************************************RETURN DHT VALUE
/**
 * Hàm trả về giá trị độ ẩm và nhiệt độ của cảm biến DHT11
 * Độ ẩm: XX (%)
 * Nhiệt độ XX (*C)
 */
// Hàm đọc độ ẩm từ từng cảm biến
uint8_t hum1_getValue() {
  return dht1.readHumidity();
}
uint8_t hum2_getValue() {
  return dht2.readHumidity();
}
uint8_t hum3_getValue() {
  return dht3.readHumidity();
}

// Hàm đọc nhiệt độ từ từng cảm biến
uint8_t temp1_getValue() {
  return dht1.readTemperature();
}
uint8_t temp2_getValue() {
  return dht2.readTemperature();
}
uint8_t temp3_getValue() {
  return dht3.readTemperature();
}

//***********************************************************************************FLAME SENSOR
/**  
 * Phát hiện ngọn lửa và trả về giá trị 0 & 1 
 * 0 tương ứng với không có lửa
 * 1 tương ứng với có lửa
*/
uint8_t flame1_getValue() {
  int flame_state = digitalRead(Flame1_PIN);

  if (flame_state == HIGH) {
    return 0;
    //Serial.println("No flame1 dected => The fire1 is NOT detected");
  } else {
    return 1;
    //Serial.println("Flame1 dected => The fire1 is detected");
  }
}

uint8_t flame2_getValue() {
  int flame_state = digitalRead(Flame2_PIN);

  if (flame_state == HIGH) {
    return 0;
    //Serial.println("No flame1 dected => The fire1 is NOT detected");
  } else {
    return 1;
    //Serial.println("Flame1 dected => The fire1 is detected");
  }
}

uint8_t flame3_getValue() {
  int flame_state = digitalRead(Flame3_PIN);

  if (flame_state == HIGH) {
    return 0;
    //Serial.println("No flame1 dected => The fire1 is NOT detected");
  } else {
    return 1;
    //Serial.println("Flame1 dected => The fire1 is detected");
  }
}


//****************************************************************************************Hàm SET_UP
void setup() {
  Serial.begin(115200);
  servo1.attach(servoPin);
  pinMode(23, INPUT);
  //*********************************Khoi tao LCD
  lcd.init();        // Khởi tạo LCD
  lcd.backlight();   // Bật đèn nền

  // Hiển thị nội dung
  lcd.setCursor(0, 0);  // Di chuyển con trỏ về đầu dòng 1
  lcd.print("Smart Home......"); 

  lcd.setCursor(0, 1);  // Di chuyển con trỏ về đầu dòng 2
  lcd.print("Initializing");
  lcd.clear();
  Serial.println("DHTxx test!");
  dht1.begin();
  dht2.begin();
  dht3.begin();

  //Khoi tao cac chan cam bien nhan biet ngon lua
  pinMode(Flame1_PIN, INPUT);
  pinMode(Flame2_PIN, INPUT);
  pinMode(Flame3_PIN, INPUT);

  pinMode(sensorPin1, INPUT);
  pinMode(sensorPin2, INPUT);
  pinMode(sensorPin3, INPUT);

  //Khoi tao cac chan BUZZ (2), Light(17)
  pinMode(2, OUTPUT);
  pinMode(17, OUTPUT);
  // Cấu hình chân là INPUT
  pinMode(33, INPUT); 

  ERa.setScanWiFi(true);
  ERa.begin(ssid, pass);
  digitalWrite(17, LOW); // Đảm bảo đèn tắt ban đầu
  //ERa.addInterval(2000L, readSensorData); // Gửi dữ liệu cảm biến mỗi 2 giây
}
/**
 * Function: Hàm đọc giá trị của tất cả các cảm biến
 * Hàm này lưu gía trị từ các hàm trả về vào các biến giá trị thông số toàn cục
 */
void updateSensorValues() {
    Flame1_Value = flame1_getValue();
    Flame2_Value = flame2_getValue();
    Flame3_Value = flame3_getValue();

    Temp1_Value = temp1_getValue();
    Temp2_Value = temp2_getValue();
    Temp3_Value = temp3_getValue();

    Hum1_Value = hum1_getValue();
    Hum2_Value = hum2_getValue();
    Hum3_Value = hum3_getValue();

    CO1_Value = MQ21_getValue();
    CO2_Value = MQ22_getValue();
    CO3_Value = MQ23_getValue();
}

/**
 * Các hàm lệnh gửi dữ liệu lên Era AppApp
 */
// Gửi thông số nhiệt độ lên các virtual pin tương ứng
void sendTemp1() { ERa.virtualWrite(V0, Temp1_Value); }
void sendTemp2() { ERa.virtualWrite(V1, Temp2_Value); }
void sendTemp3() { ERa.virtualWrite(V2, Temp3_Value); }
// Gửi thông số độ ẩm lên các virtual pin tương ứng
void sendHum1() { ERa.virtualWrite(V3, Hum1_Value); }
void sendHum2() { ERa.virtualWrite(V4, Hum2_Value); }
void sendHum3() { ERa.virtualWrite(V5, Hum3_Value); }
// Gửi thông điệp thông báo tình trạng có lửa hay không ở các phòng
void sendFlame1() { ERa.virtualWrite(V6, Flame1_Value ? "WARNING" : "GOOD"); }
void sendFlame2() { ERa.virtualWrite(V7, Flame2_Value ? "WARNING" : "GOOD"); }
void sendFlame3() { ERa.virtualWrite(V8, Flame3_Value ? "WARNING" : "GOOD"); }
// Gửi thông điệp thông báo tình trạng có khí gas hay không ở các phòng
void sendCO1() { ERa.virtualWrite(V9, CO1_Value  ? "WARNING" : "GOOD"); }
void sendCO2() { ERa.virtualWrite(V10, CO2_Value  ? "WARNING" : "GOOD"); }
void sendCO3() { ERa.virtualWrite(V11, CO3_Value  ? "WARNING" : "GOOD"); }

/**
 * Hàm chức năng cho chuông thông báo khi có tình trạng bất thường xảy ra
 * 1. Phát hiện ngọn lửa và chuông thông báo
 * 2. Phát hiện khí gas và chuông thông báo
 */
void handleBuzzer() {
    if (Flame1_Value || Flame2_Value || Flame3_Value) {
        for (int i = 0; i < 10; i++) {
            digitalWrite(2, HIGH);
            vTaskDelay(300 / portTICK_PERIOD_MS);
            digitalWrite(2, LOW);
            vTaskDelay(200 / portTICK_PERIOD_MS);
        }
    }
    else if (CO1_Value || CO2_Value  || CO3_Value ) {
      for (int i = 0; i < 10; i++) {
        digitalWrite(2, HIGH);
        vTaskDelay(300 / portTICK_PERIOD_MS);
        digitalWrite(2, LOW);
        vTaskDelay(200 / portTICK_PERIOD_MS);
      }
    }
    Serial.print(CO1_Value);
    Serial.print(" ");
    Serial.print(CO2_Value);
    Serial.print(" ");
    Serial.println(CO3_Value);
}


/**
 * Hàm thực hiện chức năng đóng mở cửa tự động
 * Input: cảm biến hồng ngoại xác định xem có người tới cửa hay không 
 */
void controlServo() {
  /**
   * Đọc giá trị digital từ cảm biến hồng ngoại
   * Khi có người: state = 0
   * Khi không có người state = 11
   */
    int state = digitalRead(23); 
    // Phát hiện người, cửa mở tự động sau 2,5s thì tự động đóng lại 
    if (state == LOW) {
        for (int posDegrees = 0; posDegrees <= 90; posDegrees++) {
            servo1.write(posDegrees);
            vTaskDelay(20 / portTICK_PERIOD_MS);
        }
        vTaskDelay(2500 / portTICK_PERIOD_MS);
        for (int posDegrees = 90; posDegrees >= 0; posDegrees--) {
            servo1.write(posDegrees);
            vTaskDelay(20 / portTICK_PERIOD_MS);
        }
    } else {
        servo1.write(0);
    }
}

void updateLCD() {
    lcd.setCursor(0, 0);
    lcd.print("Tem(*C):");
    lcd.print(Temp1_Value);
    lcd.print("_");
    lcd.print(Temp2_Value);
    lcd.print("_");
    lcd.print(Temp3_Value);

    lcd.setCursor(0, 1);
    lcd.print("Hum(%) :");
    lcd.print(Hum1_Value);
    lcd.print("_");
    lcd.print(Hum2_Value);
    lcd.print("_");
    lcd.print(Hum3_Value);
}

uint8_t led_state = 0;  // Trạng thái điều khiển LED
const int THRESHOLD = 4000;  // Ngưỡng quang trở

void controlRelay() {
    if (led_state == 0) {  // Chỉ hoạt động khi app không điều khiển đèn
        int sensorValue = analogRead(33);
        if(sensorValue > 4000){
          digitalWrite(17, HIGH);
        }
        else if(sensorValue < 2800){
          digitalWrite(17, LOW);
        }
    }
}

ERA_WRITE(V12) {
    uint8_t value = param.getInt();  // Nhận giá trị từ app

    if (value) {  
        led_state = 1;  // Chặn quang trở
        digitalWrite(17, HIGH);
    } else {  
        led_state = 0;  // Cho phép quang trở điều khiển
        digitalWrite(17, LOW);  // Tắt đèn ngay lập tức
    }

    // Cập nhật trạng thái đèn về app
    ERa.virtualWrite(V12, digitalRead(17));
}

void loop() {
    ERa.run();  // Gọi ERa để xử lý các sự kiện

    updateSensorValues();  // Cập nhật giá trị cảm biến

    // Gửi dữ liệu lên ERa qua các hàm riêng
    sendTemp1();
    sendTemp2();
    sendTemp3();

    sendHum1();
    sendHum2();
    sendHum3();

    sendFlame1();
    sendFlame2();
    sendFlame3();

    sendCO1();
    sendCO2();
    sendCO3();

    handleBuzzer();  // Điều khiển còi
    controlRelay();  // Điều khiển relay
    controlServo();  // Điều khiển servo
    updateLCD();     // Cập nhật màn hình LCD
}
