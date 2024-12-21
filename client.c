#include <WiFi.h>
#include <HTTPClient.h>
#include <HardwareSerial.h>

// Thông tin WiFi
const char *ssid = "SUBOY";
const char *password = "suboy123";

// Địa chỉ IP của server
const char *serverIP = "192.168.1.76";

// Cấu hình chân cảm biến siêu âm
#define TRIG_PIN 18
#define ECHO_PIN 5

// Cấu hình chân LED cảnh báo
#define GREEN_LED_PIN 2
#define RED_LED_PIN 4

// Cấu hình ngưỡng khoảng cách
#define CALL_THRESHOLD 10
#define SMS_THRESHOLD 30

// Định nghĩa thông tin cho Module SIM
#define simSerial Serial2
#define ESP_SIM_BAUDRATE 115200
#define ESP_SIM_TX_PIN 17
#define ESP_SIM_RX_PIN 16

// Biến trạng thái để tránh spam lệnh
bool isCalling = false;
bool isSendingSMS = false;
unsigned long lastActionTime = 0;
const unsigned long actionCooldown = 5000; // 5 giây

// Hàm đo khoảng cách bằng cảm biến siêu âm
long measureDistance()
{
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(5);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    long duration = pulseIn(ECHO_PIN, HIGH, 30000); // Timeout 30ms
    return (duration == 0) ? 9999 : (duration * 0.034) / 2;
}

// Gọi điện đến số điện thoại được chỉ định
void makeCall(const char *phoneNumber)
{
    simSerial.print("ATD ");
    simSerial.print(phoneNumber);
    simSerial.println(";");
    Serial.println("Calling...");
    isCalling = true;
    lastActionTime = millis();
}

// Gửi tin nhắn cảnh báo đến số điện thoại
void sendSMS(const char *phoneNumber, const char *message)
{
    simSerial.println("AT+CMGF=1"); // Chế độ văn bản
    delay(500);
    simSerial.print("AT+CMGS=\"");
    simSerial.print(phoneNumber);
    simSerial.println("\"");
    delay(500);
    simSerial.print(message);
    simSerial.write(0x1A); // Gửi Ctrl+Z để gửi tin nhắn
    delay(1000);
    Serial.println("SMS sent.");
    isSendingSMS = true;
    lastActionTime = millis();
}

void setup()
{
    Serial.begin(115200);
    Serial.println("\nSystem initializing...");

    // Cấu hình chân cảm biến siêu âm
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    // Cấu hình chân LED cảnh báo
    pinMode(GREEN_LED_PIN, OUTPUT);
    pinMode(RED_LED_PIN, OUTPUT);

    // Kết nối WiFi
    Serial.print("Connecting to WiFi...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected.");
    Serial.print("Client IP: ");
    Serial.println(WiFi.localIP());

    // Khởi tạo giao tiếp với SIM module
    simSerial.begin(ESP_SIM_BAUDRATE, SERIAL_8N1, ESP_SIM_RX_PIN, ESP_SIM_TX_PIN);
    delay(8000); // Chờ SIM module khởi động
    Serial.println("System ready.");
}

void loop()
{
    unsigned long currentMillis = millis();

    // Đo khoảng cách
    long distance = measureDistance();
    Serial.print("Measured distance: ");
    Serial.print(distance);
    Serial.println(" cm");

    // Kiểm tra và xử lý dựa trên khoảng cách
    if (distance < CALL_THRESHOLD && !isCalling && (currentMillis - lastActionTime > actionCooldown))
    {
        digitalWrite(GREEN_LED_PIN, LOW);
        digitalWrite(RED_LED_PIN, HIGH);
        makeCall("0334109129");
    }
    else if (distance >= CALL_THRESHOLD && distance < SMS_THRESHOLD && !isSendingSMS && (currentMillis - lastActionTime > actionCooldown))
    {
        digitalWrite(RED_LED_PIN, LOW);
        digitalWrite(GREEN_LED_PIN, HIGH);
        sendSMS("0334109129", "Canh bao: Khoang cach qua gan!");
    }
    else if (distance >= SMS_THRESHOLD)
    {
        digitalWrite(GREEN_LED_PIN, LOW);
        digitalWrite(RED_LED_PIN, LOW);
        Serial.println("Safe distance.");
        isCalling = false;
        isSendingSMS = false;
    }

    // Gửi dữ liệu tới server qua WiFi
    if (WiFi.status() == WL_CONNECTED)
    {
        HTTPClient http;
        String Str_distance = String(distance); // Chuyển đổi giá trị khoảng cách thành chuỗi

        String serverURL = String("http://") + serverIP + "/update?data=" + Str_distance;
        http.begin(serverURL);
        int httpResponseCode = http.GET();

        if (httpResponseCode > 0)
        {
            Serial.println("Data sent successfully.");
            Serial.println("Response from server: " + http.getString());
        }
        else
        {
            Serial.println("Error sending data.");
        }
        http.end();
    }

    delay(2000); // Chu kỳ đo lường
}
