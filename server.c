#include <WiFi.h>
#include <WebServer.h>

// Thông tin WiFi
const char *ssid = "SUBOY";        // Tên WiFi của bạn
const char *password = "suboy123"; // Mật khẩu WiFi của bạn

// Tạo web server trên cổng 80
WebServer server(80);

// Biến để lưu dữ liệu nhận được từ client
String receivedData = "No data received yet.";

// Xử lý yêu cầu trang HTML
void handleRoot()
{
    String html = "<!DOCTYPE html><html>";
    html += "<head><title>Water Level Warning System</title>";
    html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
    html += "<style>";
    html += "body { background-color: white; font-family: Arial, sans-serif; color: #333; }";
    html += "h1 { color: #1E90FF; text-align: center; }";
    html += "p { font-size: 18px; color: #1E90FF; text-align: center; }";
    html += "#data { font-size: 20px; color: #000080; font-weight: bold; text-align: center; padding: 10px; border: 2px solid #1E90FF; margin: 20px auto; width: 80%; }";
    html += "button { background-color: #1E90FF; color: white; border: none; padding: 10px 20px; text-align: center; font-size: 16px; cursor: pointer; border-radius: 5px; margin: 10px 0; }";
    html += "button:hover { background-color: #4682B4; }";
    html += "footer { font-size: 14px; color: #808080; text-align: center; margin-top: 20px; }";
    html += "</style>";
    html += "<script>";
    html += "function fetchData() {";
    html += "  var xhttp = new XMLHttpRequest();";
    html += "  xhttp.onreadystatechange = function() {";
    html += "    if (this.readyState == 4 && this.status == 200) {";
    html += "      document.getElementById('data').innerHTML = this.responseText;";
    html += "    }";
    html += "  };";
    html += "  xhttp.open('GET', '/data', true);";
    html += "  xhttp.send();";
    html += "}";
    html += "setInterval(fetchData, 1000);"; // Cập nhật dữ liệu mỗi giây
    html += "</script></head>";
    html += "<body>";
    html += "<h1>Water Level Warning System</h1>";
    html += "<p>Water Level current</p>";
    html += "<div id='data'>" + receivedData + "</div>"; // Dữ liệu sẽ được cập nhật tại đây
    html += "<footer>ESP32 Web Server - Developed by Dat Vuong</footer>";
    html += "</body></html>";
    server.send(200, "text/html", html);
}

// Xử lý yêu cầu dữ liệu
void handleData()
{
    server.send(200, "text/plain", receivedData);
}

// Xử lý khi client gửi dữ liệu
void handleUpdate()
{
    if (server.hasArg("data"))
    {
        receivedData = server.arg("data");
        Serial.println("Updated Data: " + receivedData);
        server.send(200, "text/plain", "Data updated successfully.");
    }
    else
    {
        server.send(400, "text/plain", "No data provided.");
    }
}

void setup()
{
    Serial.begin(115200);

    // Kết nối WiFi
    Serial.print("Connecting to WiFi...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected.");
    Serial.println("IP Address: ");
    Serial.println(WiFi.localIP());

    // Cấu hình route
    server.on("/", handleRoot);         // Route cho trang chính
    server.on("/data", handleData);     // Route cung cấp dữ liệu real-time
    server.on("/update", handleUpdate); // Route để cập nhật dữ liệu từ client

    // Bắt đầu server
    server.begin();
    Serial.println("Server started.");
}

void loop()
{
    server.handleClient(); // Lắng nghe yêu cầu từ client
}
