// header ต่าง ๆ นานา ๆ
#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include "esp_http_server.h"
#include "esp_camera.h"
#include <ESP32Servo.h>
// นิยาม Pin กล้องต่าง ๆ
#define PWDN_GPIO_NUM -1
#define RESET_GPIO_NUM -1

#define XCLK_GPIO_NUM 15
#define SIOD_GPIO_NUM 4
#define SIOC_GPIO_NUM 5

#define Y9_GPIO_NUM 16
#define Y8_GPIO_NUM 17
#define Y7_GPIO_NUM 18
#define Y6_GPIO_NUM 12
#define Y5_GPIO_NUM 10
#define Y4_GPIO_NUM 8
#define Y3_GPIO_NUM 9
#define Y2_GPIO_NUM 11

#define VSYNC_GPIO_NUM 6
#define HREF_GPIO_NUM 7
#define PCLK_GPIO_NUM 13
#define STREAM_BOUNDARY "frame"

// นิยาม pin PCA
#define SDA 14
#define SCL 21
// ประกาศตัวแปรไว้ล็อกอิน
char ssid[32] = {0};
char pass[32] = {0};

// ไว้เช็คว่าเราต่อไวไฟแล้วหรือยัง
enum DeviceState
{
    WIFI_MANAGER,
    WIFI_CONNECTING,
    CONTROL_MODE
};

// เซ็ตค่าเริ่มต้น
DeviceState deviceState = WIFI_MANAGER;

// เริ่มต้นเซ็ตข้อมูลของ Server HTTP ที่จะเปิดที่ Port 80
httpd_handle_t server = NULL;
httpd_config_t http_cfg = HTTPD_DEFAULT_CONFIG();

//เริ่มต้น pwm ของ PCA
Servo s1;
Servo s2;
// หน้าเว็บเพจของการล็อกอินไวไฟ
static const char WIFI_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>SMART BIN WiFi</title>
    <script>
        setInterval(async () => {
            try {
                const r = await fetch('/wifi/status');
                const j = await r.json();
                if (j.connected) {
                    document.body.innerHTML =
                        `<h2>Connected!<br> Please go to</h2>

         <a href="http://${j.ip}" style="font-size: 3rem;">${j.ip}</a>
         <h2>Please change this device's wifi to the one you entered.</h2>`;
         
                }
            } catch (e) { }
        }, 2000);
    </script>
    <style>
        :root {
            --primary: #0ea5e9;
            --primary-hover: #0284c7;
            --bg: #f0f9ff;
            --text: #0c4a6e;
        }

        * {
            box-sizing: border-box;
            transition: all 0.2s ease-in-out;
        }

        body {
            margin: 0;
            height: 100vh;
            display: grid;
            place-items: center;
            background-color: var(--bg);
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
            color: var(--text);
        }

        .card {
            background: white;
            padding: 2.5rem;
            border-radius: 20px;
            width: 90%;
            max-width: 700px;

            box-shadow: 0 10px 25px -5px rgba(0, 0, 0, 0.05),
                0 8px 10px -6px rgba(0, 0, 0, 0.05);
        }

        h1 {
            margin: 0 0 0.5rem 0;
            font-size: 3rem;
            font-weight: 700;
        }

        p {
            color: #64748b;
            font-size: 1.25rem;
            margin-bottom: 2rem;
        }

        .field {
            margin-bottom: 1.25rem;
            text-align: left;
        }

        label {
            display: block;
            margin-bottom: 6px;
            font-size: 1.5rem;
            font-weight: 600;
            color: #475569;
        }

        input {
            width: 100%;
            padding: 12px 16px;
            border-radius: 10px;
            border: 1px solid #e2e8f0;
            background: #fdfdfd;
            font-size: 1.15rem;
            outline: none;
        }

        input:focus {
            border-color: var(--primary);
            box-shadow: 0 0 0 4px rgba(99, 102, 241, 0.1);
            background: white;
        }

        .btn {
            width: 100%;
            padding: 14px;
            margin-top: 10px;
            border: none;
            border-radius: 10px;
            background: var(--primary);
            color: white;
            font-size: 16px;
            font-weight: 600;
            cursor: pointer;
            box-shadow: 0 4px 6px -1px rgba(99, 102, 241, 0.2);
        }

        .btn:hover {
            background: var(--primary-hover);
            transform: translateY(-1px);
            box-shadow: 0 10px 15px -3px rgba(99, 102, 241, 0.3);
        }

        .btn:active {
            transform: translateY(0);
        }
    </style>
</head>

<body>

    <div class="card">
        <h1>Welcome! ยิงลีต้อนรับ</h1>
        <p>Please enter your desired WiFi credentials for SMART BIN's connection.
        </p>
        <form action="/wifi/save" method="POST">
            <div class="field">
                <label for="ssid">WiFi Name (SSID)</label>
                <input type="text" id="ssid" name="ssid" placeholder="Enter your WiFi name" required>
            </div>

            <div class="field">
                <label for="pass">Password</label>
                <input type="password" id="pass" name="pass" placeholder="Enter your WiFi password" required>
            </div>

            <button class="btn">Go</button>
        </form>
    </div>

</body>

</html>
)rawliteral";

// หน้าเว็บเพจของการควบคุม
static const char CONTROL_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html>

<head>
    <title>SMART BIN Control</title>
    <meta charset="UTF-8">

    <script>
        console.log("SCRIPT LOADED");

        function cmd(c) {
            console.log("CLICKED:", c);
            fetch('/control/' + c);
        }
        document.addEventListener("DOMContentLoaded", function () {
            document.getElementById("cam").src =
                "http://" + location.hostname + ":81/stream";
        });
    </script>
    <style>
        :root {
            --primary: #0284c7;
            --primary-hover: #0070a8;
            --bg: #f0f9ff;
            --text: #0c4a6e;
        }

        body {
            margin: 0;
            height: 100vh;
            background-color: var(--bg);
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
            color: var(--text);
            display: flex;
            align-items: center;
            justify-content: center;
        }


        button {
            padding: 14px;
            margin-top: 10px;
            border: none;
            border-radius: 10px;
            background: var(--primary);
            color: #ebf8ff;
            font-size: 30px;
            font-weight: 600;

            cursor: pointer;
            box-shadow: 0 4px 6px -1px rgba(99, 102, 241, 0.4);
        }

        button:hover {
            background: var(--primary-hover);
            transform: translateY(-1px);
            box-shadow: 0 10px 15px -3px rgba(99, 102, 241, 0.3);
        }

        button:active {
            transform: translateY(0);
        }

        .main {
            display: flex;
        }

        img {
            width: 60vh;
            border-width: 1px;
            border: #e2e8f0;
            border-style: solid;
            border-radius: 10px;
            box-shadow: 0 10px 25px -5px rgba(0, 0, 0, 0.1),
                0 8px 10px -6px rgba(0, 0, 0, 0.1);
        }

        .g {
            background: white;
            border-radius: 20px;
            padding: 25px;
            box-shadow: 0 10px 25px -5px rgba(0, 0, 0, 0.1),
                0 8px 10px -6px rgba(0, 0, 0, 0.1);
        }

        .left {
            display: grid;
            grid-template-columns: 60px 60px 60px;
            grid-template-rows: 60px 60px 60px;
            gap: 10px;
            justify-content: center;
            align-items: center;
            padding: 10px;
            padding-right: 20px;
            padding-left: 0;

        }

        .left button:nth-child(1) {
            grid-column: 2;
            grid-row: 1;
        }

        /* Up */
        .left button:nth-child(2) {
            grid-column: 1;
            grid-row: 2;
        }

        /* Left */
        .left button:nth-child(3) {
            grid-column: 2;
            grid-row: 2;
            background-color: #228B22;
            color: #def7de;
        }

        .left button:nth-child(3):hover {
            background-color: #187a18;

        }

        /* Stop */
        .left button:nth-child(4) {
            grid-column: 3;
            grid-row: 2;
        }

        /* Right */
        .left button:nth-child(5) {
            grid-column: 2;
            grid-row: 3;
        }

        /* Down */
        h1 {
            font-size: 3rem;
            margin: 0%;
            margin-bottom: 20px;
        }

        .parentleft {
            display: flex;
            align-items: center;
            justify-content: center;
        }
    </style>
    <script>
        console.log("SCRIPT LOADED");

        function cmd(c) {
            console.log("CLICKED:", c);
            fetch('/control/' + c);
        }
        document.addEventListener("DOMContentLoaded", function () {
            document.getElementById("cam").src =
                "http://" + location.hostname + ":81/stream";
        });
    </script>


</head>

<body>
    <div class="g">
        <div style="display: grid; place-items: center;">
            <h1>Control Web</h1>
        </div>
        <div class="main">
            <div class="parentleft">
                <div class="left">
                    <button onclick="cmd('forward')">▲</button>
                    <button onclick="cmd('left')">◀</button>
                    <button onclick="cmd('stop')">◼</button>
                    <button onclick="cmd('right')">▶</button>
                    <button onclick="cmd('reverse')">▼</button>
                </div>
            </div>
            <div class="right">
                <img id="cam">
            </div>
        </div>
    </div>
</body>

</html>
)rawliteral";

// ควบคุมการไหลร่วมกับ Device State เพื่อเลือกการแสดงผล
esp_err_t root_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");

    if (deviceState == CONTROL_MODE)
    {
        httpd_resp_send(req, CONTROL_HTML, HTTPD_RESP_USE_STRLEN);
    }
    else
    {
        httpd_resp_send(req, WIFI_HTML, HTTPD_RESP_USE_STRLEN);
    }
    return ESP_OK;
}

//ปุ่มกดเซฟไวไฟ และต่อไวไฟของ ESP32
esp_err_t wifi_save_handler(httpd_req_t *req)
{
    char buf[128];
    int len = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (len <= 0)
        return ESP_FAIL;

    buf[len] = 0;

    sscanf(buf, "ssid=%31[^&]&pass=%31s", ssid, pass);

    Serial.printf("Connecting to %s\n", ssid);

    WiFi.begin(ssid, pass);
    deviceState = WIFI_CONNECTING;

    return ESP_OK;
}

//ไว้ให้เว็บเราเช็คว่าเราต่อไวไฟเสร็จแล้วหรือยัง
esp_err_t wifi_status_handler(httpd_req_t *req)
{
    char resp[128];

    if (WiFi.status() == WL_CONNECTED)
    {
        snprintf(resp, sizeof(resp),
                 "{\"connected\":true,\"ip\":\"%s\"}",
                 WiFi.localIP().toString().c_str());
    }
    else
    {
        snprintf(resp, sizeof(resp),
                 "{\"connected\":false}");
    }

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

//การควบคุมมอเตอร์ไปข้างหน้า
esp_err_t ctrl_forward(httpd_req_t *req)
{
    Serial.println("MOTOR: FORWARD");
    s1.writeMicroseconds(1000);
    s2.writeMicroseconds(1850);
    httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

//การควบคุมมอเตอร์ไปทางซ้าย
esp_err_t ctrl_left(httpd_req_t *req)
{
    Serial.println("MOTOR: LEFT");
    s1.writeMicroseconds(1500);
    s2.writeMicroseconds(1150);
    httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

//การควบคุมมอเตอร์ไปทางขวา
esp_err_t ctrl_right(httpd_req_t *req)
{
    Serial.println("MOTOR: RIGHT");
    s1.writeMicroseconds(2000);
    s2.writeMicroseconds(1500);
    httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

//การควบคุมมอเตอร์ถอยหลัง
esp_err_t ctrl_reverse(httpd_req_t *req)
{
    Serial.println("MOTOR: REVERSE");
    s1.writeMicroseconds(2000);
    s2.writeMicroseconds(1150);
    httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

//การควบคุมมอเตอร์ให้หยุด
esp_err_t ctrl_stop(httpd_req_t *req)
{
    Serial.println("MOTOR: STOP");
    s1.writeMicroseconds(1500);
    s2.writeMicroseconds(1500);
    httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

//รีจิสเตอร์ เราท์ของแต่ละ URI
void register_routes()
{
    httpd_uri_t root = {"/", HTTP_GET, root_handler, NULL};
    httpd_uri_t save = {"/wifi/save", HTTP_POST, wifi_save_handler, NULL};
    httpd_uri_t status = {"/wifi/status", HTTP_GET, wifi_status_handler, NULL};

    httpd_uri_t fwd = {"/control/forward", HTTP_GET, ctrl_forward, NULL};
    httpd_uri_t left = {"/control/left", HTTP_GET, ctrl_left, NULL};
    httpd_uri_t right = {"/control/right", HTTP_GET, ctrl_right, NULL};
    httpd_uri_t rev = {"/control/reverse", HTTP_GET, ctrl_reverse, NULL};
    httpd_uri_t stop = {"/control/stop", HTTP_GET, ctrl_stop, NULL};

    httpd_register_uri_handler(server, &root);
    httpd_register_uri_handler(server, &save);
    httpd_register_uri_handler(server, &status);

    httpd_register_uri_handler(server, &fwd);
    httpd_register_uri_handler(server, &left);
    httpd_register_uri_handler(server, &right);
    httpd_register_uri_handler(server, &rev);
    httpd_register_uri_handler(server, &stop);
}

//จัดการเกี่ยวกับกล้อง
void init_camera()
{
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
    config.fb_count = 2;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;

    if (!psramFound())
    {
        Serial.println("❌ PSRAM NOT FOUND — streaming will fail");
    }

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK)
    {
        Serial.printf("Camera init failed: %d\n", err);
    }
}

//นำภาพไปแปะไว้ที่ เราท์ /stream
esp_err_t stream_handler(httpd_req_t *req)
{
    camera_fb_t *fb;
    char part_buf[64];

    httpd_resp_set_type(req, "multipart/x-mixed-replace;boundary=frame");

    while (httpd_req_to_sockfd(req) >= 0)
    {

        fb = esp_camera_fb_get();
        if (!fb)
        {
            vTaskDelay(1);
            continue;
        }

        int hlen = snprintf(part_buf, sizeof(part_buf),
                            "--frame\r\nContent-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n",
                            fb->len);

        if (httpd_resp_send_chunk(req, part_buf, hlen) != ESP_OK || httpd_resp_send_chunk(req, (const char *)fb->buf, fb->len) != ESP_OK || httpd_resp_send_chunk(req, "\r\n", 2) != ESP_OK)
        {

            esp_camera_fb_return(fb);
            break;
        }

        esp_camera_fb_return(fb);

        vTaskDelay(1);
    }

    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}
//เปิดเซอร์เวอร์สำหรับการสตรีมบนเซอร์เวอร์ port 81
httpd_handle_t stream_server = NULL;
void start_stream_server()
{
    
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    config.server_port = 81; //ของอีกเซอร์เวอร์คือ 81
    config.max_open_sockets = 2;
    config.stack_size = 8192; 
    config.ctrl_port = 32769; //ของอีกเซอร์เวอร์คือ 32768

    if (httpd_start(&stream_server, &config) == ESP_OK)
    {

        httpd_uri_t stream_uri = {
            .uri = "/stream",
            .method = HTTP_GET,
            .handler = stream_handler,
            .user_ctx = NULL};

        httpd_register_uri_handler(stream_server, &stream_uri);

        Serial.println("Stream server started on port 81");
    }
    else
    {
        Serial.println("Stream server failed");
    }
}

void setup()
{
    //Serial
    Serial.begin(115200);
    delay(3000);
    Serial.println("BOOT");

    //เริ่มต้นการสื่อสารกับ Servo สองอันที่ 14 21
    s1.attach(14);
    s2.attach(21);
    init_camera();
    //เปิดไวไฟให้คนต่อ   
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP("SMARBIN-SETUP-AP");

    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());

    //setting server HTTP ที่ พอร์ท 80
    http_cfg.stack_size = 8192;
    http_cfg.server_port = 80;
    http_cfg.max_open_sockets = 10;
    http_cfg.lru_purge_enable = true;
    http_cfg.max_open_sockets = 8;
    http_cfg.max_uri_handlers = 16;
    http_cfg.lru_purge_enable = true;
    http_cfg.task_priority = tskIDLE_PRIORITY + 5;

    httpd_start(&server, &http_cfg);
    register_routes();

    Serial.println("HTTP READY");
    start_stream_server();
    
}
void loop()
{
    //เซ็ตเป็น control mode ถ้าต่อเวลาแล้ว
    if (deviceState == WIFI_CONNECTING && WiFi.status() == WL_CONNECTED)
    {
        deviceState = CONTROL_MODE;
    }
    delay(100);
}
