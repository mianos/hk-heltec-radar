#include "WiFiProv.h"
#include "WiFi.h"
#include "settings.h"
#include "display.h"

int prov_on_flag = 0;
int prov_end = 0;
int prov_err = 0;

void SysProvEvent(arduino_event_t *sys_event)
{
    switch (sys_event->event_id) {
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
        Serial.print("\nConnected IP address : ");
        Serial.println(IPAddress(sys_event->event_info.got_ip.ip_info.ip.addr));
        break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        Serial.println("\nDisconnected. Connecting to the AP again... ");
        break;
    case ARDUINO_EVENT_PROV_START:
        Serial.println("\nProvisioning started\nGive Credentials of your access point using \" Android app \"");
        break;
    case ARDUINO_EVENT_PROV_CRED_RECV: 
        Serial.println("\nReceived Wi-Fi credentials");
        Serial.print("\tSSID : ");
        Serial.println((const char *) sys_event->event_info.prov_cred_recv.ssid);
        Serial.print("\tPassword : ");
        Serial.println((char const *) sys_event->event_info.prov_cred_recv.password);
        break;
    case ARDUINO_EVENT_PROV_CRED_FAIL: 
        Serial.println("\nProvisioning failed!\nPlease reset to factory and retry provisioning\n");
        if(sys_event->event_info.prov_fail_reason == WIFI_PROV_STA_AUTH_ERROR) 
            Serial.println("\nWi-Fi AP password incorrect");
        else
            Serial.println("\nWi-Fi AP not found....Add API \" nvs_flash_erase() \" before beginProvision()");        
        break;
    case ARDUINO_EVENT_PROV_CRED_SUCCESS:
        Serial.println("\nProvisioning Successful");
        break;
    case ARDUINO_EVENT_PROV_END:
        Serial.println("\nProvisioning Ends");
        break;
    case ARDUINO_EVENT_PROV_INIT:
        Serial.printf("Prov: Init\n");
        break;
    case ARDUINO_EVENT_PROV_DEINIT:
        Serial.printf("Prov: Stopped\n");
        break;
    case ARDUINO_EVENT_WIFI_SCAN_DONE:
        printf("Prov: scan done\n");
        break;
    case SYSTEM_EVENT_STA_START:
        printf("Station start\n");
        break;
    default:
        Serial.printf("Some other event %d\n", sys_event->event_id);
        break;
    }
}

SettingsManager *settingsManager;

#if defined(BOOT_BUTTON)
constexpr int PROG_BUTTON_PIN = BOOT_BUTTON;
#else
constexpr int PROG_BUTTON_PIN = 0; // GPIO0
#endif

String getMacAddress() {
  uint8_t mac[6];
  WiFi.macAddress(mac);
  char macStr[18] = { 0 };
  sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X",  mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(macStr);
}

void wifi_connect(Display* display) {
//  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  WiFi.mode(WIFI_STA);
  WiFi.onEvent(SysProvEvent);
  WiFi.disconnect();  // Disconnect from the WiFi
  Serial.printf("mac '%s'\n", getMacAddress().c_str());
  String name = String("faba_");
  name += String(random(0xffff), HEX);
  printf("prov name: %s\n",name.c_str());

  display->taf("press and hold prog now to reset .... \n");
  display->scroll_now();
  delay(1000);
  if (digitalRead(PROG_BUTTON_PIN) == LOW) {
    display->taf("Reset button low Resetting\n");
    display->scroll_now();
    WiFi.disconnect(false,true); 
    delay(500);
    ESP.restart();
  }
  display->taf("Begin Provisioning using Soft AP\n");
  display->scroll_now();
  const char * pop = "abcd1234"; 
  const char * service_name = "PROV_RDR";
  const char * service_key = NULL; 

  WiFiProv.beginProvision(WIFI_PROV_SCHEME_SOFTAP, WIFI_PROV_SCHEME_HANDLER_NONE, WIFI_PROV_SECURITY_1, pop, service_name);
  while (WiFi.status() != WL_CONNECTED) {
    display->taf("Connecting to WiFi...\n");
    display->scroll_now();
  }
  Serial.printf("COnnected\n");
}
