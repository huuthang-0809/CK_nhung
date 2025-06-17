#include <ESP8266WiFi.h>
#include <ESP_Mail_Client.h>

#define WIFI_SSID "P 801"
#define WIFI_PASS "801doankethien"

#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465
#define AUTHOR_EMAIL "huuthang08092004@gmail.com"
#define AUTHOR_PASS "klqf wllk pyau vbzd"
#define RECIPIENT_EMAIL "thanglc302@gmail.com"

#define ALERT_PIN D5  // GPIO14

SMTPSession smtp;
Session_Config config;
SMTP_Message message;

bool email_sent = false;

void setup() {
  Serial.begin(9600);
  pinMode(ALERT_PIN, INPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("WiFi connecting...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ Connected to WiFi!");

  // SMTP config
  config.server.host_name = SMTP_HOST;
  config.server.port = SMTP_PORT;
  config.login.email = AUTHOR_EMAIL;
  config.login.password = AUTHOR_PASS;
  config.login.user_domain = "";

  message.sender.name = "Gas Warning System";
  message.sender.email = AUTHOR_EMAIL;
  message.subject = "⚠️ GAS ALERT!";
  message.addRecipient("User", RECIPIENT_EMAIL);
  message.text.content = "Gas level exceeded the safe limit! Please take action!";
  message.text.charSet = "utf-8";
  message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;
}

void loop() {
  static bool last_state = LOW;
  bool current_state = digitalRead(ALERT_PIN);

  // Phát hiện cạnh lên (LOW -> HIGH)
  if (current_state == HIGH && last_state == LOW && !email_sent) {
    Serial.println("🚨 Alert detected. Sending email...");

    // Chỉ connect nếu chưa kết nối
    if (!smtp.connected()) {
      if (!smtp.connect(&config)) {
        Serial.println("❌ Failed to connect SMTP");
        return;
      }
    }

    if (!MailClient.sendMail(&smtp, &message)) {
      Serial.println("❌ Failed to send Email, " + smtp.errorReason());
    } else {
      Serial.println("✅ Email sent!");
      email_sent = true;
    }
  }

  // Reset trạng thái nếu cảnh báo ngắt (cạnh xuống)
  if (current_state == LOW && last_state == HIGH) {
    email_sent = false;
  }

  last_state = current_state;
  delay(200);  // Giảm delay để phản ứng nhanh hơn
}
