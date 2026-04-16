// wakatime display
#include <Arduino.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <WiFi.h>
#include <SPI.h>
#include <WiFiClientSecure.h>

const char* SSID = "";
const char* PASSWORD = "";
const char* WAKA_API_KEY = "";

#define TFT_CS 1
#define TFT_RST 2
#define TFT_DC 3
#define TFT_SCLK 4
#define TFT_MOSI 5

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

WiFiClientSecure _client;

// WakaTime API
const char* WAKA_HOST = "wakatime.com";
const int   WAKA_PORT = 443;
const char* WAKA_TODAY_PATH = "/api/v1/users/current/status_bar/today";
const char* WAKA_7DAYS_PATH = "/api/v1/users/current/stats/last_7_days";

// Gotten with $ openssl s_client -connect wakatime.com:443 -showcerts </dev/null 2>/dev/null \ | openssl x509 -outform PEM > site_cert.pem
// WT doesn't like HTTP requests so we need this to send HTTPS requests properly
const char* WAKA_ROOT_CA = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFBTCCA+2gAwIBAgISBRjKUoBggp3a+b7Hv73HWyNUMA0GCSqGSIb3DQEBCwUA
MDMxCzAJBgNVBAYTAlVTMRYwFAYDVQQKEw1MZXQncyBFbmNyeXB0MQwwCgYDVQQD
EwNSMTMwHhcNMjYwNDEzMDcwMTM4WhcNMjYwNzEyMDcwMTM3WjAXMRUwEwYDVQQD
Ewx3YWthdGltZS5jb20wggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQC9
RxyAuXgcnHklY6BU73tSTzGt3pu+/3aTc7V1+oE2f/psAO41Ix/RA1tPORGoVTT6
JMIBBhb3yRPuIDuBZinKM++WQYXVJCfnnFj0jYnvg66ziTRAHC7wLJm2hJntT4eU
QoBZjHuDG27mKeNbWrmFArSyij9yCow1aU6mOtXQSPx5pTmV5eCwYZwenY0hpW/C
wKLB27xhSowrjOKXqPS/NJmE1oBS9qRaCcdAAnwZ/x/qNXXBFVtGDUW1miDpFxeR
pF+WiGOdcGW5RnRw6f5m+NqRqz8mbtHaavE93PGvdLzzPRTXSldla/YHws6zPH1P
OD7zdSXcBOzsx73jSJKFAgMBAAGjggItMIICKTAOBgNVHQ8BAf8EBAMCBaAwEwYD
VR0lBAwwCgYIKwYBBQUHAwEwDAYDVR0TAQH/BAIwADAdBgNVHQ4EFgQUyE+pAcQV
bAh1gXWtBfHEREIe7EYwHwYDVR0jBBgwFoAU56ufDywzoFPTXk94yLKEDjvWkjMw
MwYIKwYBBQUHAQEEJzAlMCMGCCsGAQUFBzAChhdodHRwOi8vcjEzLmkubGVuY3Iu
b3JnLzApBgNVHREEIjAgggx3YWthdGltZS5jb22CEHd3dy53YWthdGltZS5jb20w
EwYDVR0gBAwwCjAIBgZngQwBAgEwLgYDVR0fBCcwJTAjoCGgH4YdaHR0cDovL3Ix
My5jLmxlbmNyLm9yZy84NS5jcmwwggENBgorBgEEAdZ5AgQCBIH+BIH7APkAdwCv
Z4g7V7BO3Y+m2X72LqjrgQrHcWDwJF5V1gwv54WHOgAAAZ2F2uWkAAAEAwBIMEYC
IQDQRZQedmg1CQqhnjOxkCTjfuEaRuQ9V8HoCiILOxmQoAIhAJKTQHxUfKu85OkX
+sHSYqIq0aDnSEQb4EOxT5yn4Bd4AH4AJuNkblhpISO8ND9HJDWbN5LNJFqI2BXT
kzP9mRirRyMAAAGdhdrk9gAIAAAFAAbADsYEAwBHMEUCIQCUSri+82DOOKCsYO36
6+xE4sIG3J0dhdHrzcmliaSu9gIgLu7Yn1oB0wGy7cPbk55R2TfH+75FkRdN/ow5
lq61Z6gwDQYJKoZIhvcNAQELBQADggEBAHQuzaYDguS3vpfwxGZSRPdFxxplzByS
6iANYPllV1SociYeUPg/2GmsDeyQkw4xsx2ksE1zEvtvyMIlML9ox+lKiGL2duRp
BQVuno8D/ll23O494gQqR2OTk571FdFovwzdIgKC+gFTSnT6nBO8caemMR0Vr/YW
iVx+uapFL9jZ082od5KW0D8nd9t1eWPiMK2ERWRQPagWJuUtf0TsAmRREP1pXmXH
FRJVs5wvmZuey7UMHz4ryXA8XESMk1FDpRDloDORGcUwKCANPjkqAkXgboqb+BXT
CxdS4bhQunkp5rWgxCWWeNvMW8Sx/1NV+nJYGn9YVQ6qOBNixVazw0Y=
-----END CERTIFICATE-----
)EOF";

#define FETCH_INTERVAL_MS (5UL * 60UL * 1000UL)  // 5 minutes

typedef struct {
  JsonDocument data;
  int status_code;
} response;

response wakaGet(const char* path) {
  response res;
  int status_code = -1;

  _client.setCACert(WAKA_ROOT_CA);

  if (!_client.connect(WAKA_HOST, WAKA_PORT)) {
    Serial.println("HTTPS connection failed");
    return res;
  }

  // ?api_key=<key>
  String query = "?api_key=";
  query += WAKA_API_KEY;

  // Send HTTP/1.1 request
  String requestLine = String("GET ") + path + query + " HTTP/1.1";
  _client.println(requestLine);
  _client.print("Host: ");
  _client.println(WAKA_HOST);
  _client.println("Connection: close");
  _client.println("Accept: application/json");
  _client.println("User-Agent: WakaTime-ESP32"); // uusally APIs require a user agent
  _client.println();  // end of headers

  // ── Read the status line ────────────────────────────────────────────────
  String statusLine = _client.readStringUntil('\n');
  // e.g. "HTTP/1.1 200 OK\r"
  int firstSpace  = statusLine.indexOf(' ');
  int secondSpace = statusLine.indexOf(' ', firstSpace + 1);
  if (firstSpace < 0 || secondSpace < 0) {
    Serial.println("Malformed status line");
    _client.stop();
    return res;
  }
  res.status_code = statusLine.substring(firstSpace + 1, secondSpace).toInt();
  Serial.printf("HTTP status: %d\n", res.status_code);

  while (_client.connected()) {
    String headerLine = _client.readStringUntil('\n');
    if (headerLine == "\r") break;  // blank line → body follows
  }

  DeserializationError jsonErr = deserializeJson(res.data, _client);
  if (jsonErr) {
    Serial.printf("JSON parse error: %s\n", jsonErr.c_str());
    res.status_code = -1;
  }

  _client.stop();
  return res;
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(SSID, PASSWORD);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");

  tft.initR(INITR_BLACKTAB); // the type of screen
  tft.setRotation(1); // this makes the screen landscape! remove this line for portrait
  Serial.println("TFT Initialized!");
  tft.fillScreen(ST77XX_BLACK); // make sure there is nothing in the buffer

  tft.setCursor(0,0); // make the cursor at the top left
}

void loop() {
  static unsigned long lastFetch = 0;

  if (millis() - lastFetch >= FETCH_INTERVAL_MS) {
    lastFetch = millis();
    Serial.println("Fetching WakaTime data...");
    response res_today = wakaGet(WAKA_TODAY_PATH);
    response res_week = wakaGet(WAKA_7DAYS_PATH);

    if (res_today.status_code == 200 && res_week.status_code == 200) {
      renderStats(res_today.data, res_week.data);
    } else {
      // fail
    }
  }
  delay(1000);
}

void renderStats(JsonDocument& data_today, JsonDocument& data_week) {
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextSize(1);

  tft.setCursor(4, 4); // padding
  tft.setTextColor(ST77XX_CYAN);
  tft.print("WakaTime ");

  const char* week_time_text = data_week["data"]["human_readable_total"] | "? hours";

  tft.setTextColor(ST77XX_WHITE);
  tft.print("Last 7 days: ");
  tft.print(week_time_text);

  tft.drawFastHLine(0, 10, tft.width(), ST77XX_CYAN);

  const char* time_text = data_today["data"]["grand_total"]["text"] | "? hours";

  tft.setCursor(4, 16);
  tft.setTextSize(2);
  tft.setTextWrap(true)
  tft.setTextColor(ST77XX_YELLOW);
  tft.println(time_text);

  tft.setCursor(4, 22);
  tft.setTextSize(1);

  unsigned long uptime_ms = millis() / 1000;

  tft.setCursor(4, tft.height() - 10);
  tft.printf("uptime %luh %02lum", s / 3600, (s % 3600) / 60);
}

