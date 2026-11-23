#include <RTClib.h>

#include "DHT.h"

#define DHTPIN 2     // DHT传感器接数字引脚2
#define DHTTYPE DHT11   // 传感器型号，使用DHT22可自行修改

#define LED_PIN 13  // LED引脚，默认使用Arduino板载LED，可自行修改

// 温度-闪烁间隔映射参数，可按需调整
#define TEMP_MIN 20.0    // 最低参考温度（℃），对应最慢闪烁
#define TEMP_MAX 35.0    // 最高参考温度（℃），对应最快闪烁
#define INTERVAL_MAX 1000  // 最慢闪烁间隔（毫秒）
#define INTERVAL_MIN 200   // 最快闪烁间隔（毫秒）

DHT dht(DHTPIN, DHTTYPE);

// 非阻塞运行变量
unsigned long previousReadMillis = 0;
const long readInterval = 2000;  // 每2秒读取一次传感器

unsigned long previousLedMillis = 0;
long blinkInterval = INTERVAL_MAX;
bool ledState = LOW;

void setup() {
  Serial.begin(9600);
  Serial.println(F("DHT Sensor Test - Group: zhaojunmin"));
  
  pinMode(LED_PIN, OUTPUT);
  dht.begin();
}

void loop() {
  unsigned long currentMillis = millis();

  // 定时读取传感器（非阻塞，不影响LED闪烁）
  if (currentMillis - previousReadMillis >= readInterval) {
    previousReadMillis = currentMillis;

    // 读取温湿度
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    float f = dht.readTemperature(true);

    // 读取失败处理
    if (isnan(h) || isnan(t) || isnan(f)) {
      Serial.println(F("Group: zhaojunmin | Failed to read from DHT sensor!"));
      blinkInterval = INTERVAL_MAX; // 读取失败时保持最慢闪烁
      return;
    }

    // 计算热指数
    float hif = dht.computeHeatIndex(f, h);
    float hic = dht.computeHeatIndex(t, h, false);

    // 根据温度线性计算闪烁间隔：温度越高，间隔越小，闪烁越快
    if (t <= TEMP_MIN) {
      blinkInterval = INTERVAL_MAX;
    } else if (t >= TEMP_MAX) {
      blinkInterval = INTERVAL_MIN;
    } else {
      blinkInterval = map(t, TEMP_MIN, TEMP_MAX, INTERVAL_MAX, INTERVAL_MIN);
    }

    // 串口输出：开头增加组号
    Serial.print(F("Group: zhaojunmin | "));
    Serial.print(F("Humidity: "));
    Serial.print(h);
    Serial.print(F("%  Temperature: "));
    Serial.print(t);
    Serial.print(F("°C "));
    Serial.print(f);
    Serial.print(F("°F  Heat index: "));
    Serial.print(hic);
    Serial.print(F("°C "));
    Serial.print(hif);
    Serial.println(F("°F"));
  }

  // LED持续闪烁（独立运行，不受读取间隔影响）
  if (currentMillis - previousLedMillis >= blinkInterval) {
    previousLedMillis = currentMillis;
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
  }
}
