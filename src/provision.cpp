#include "WiFiProv.h"
#include "WiFi.h"
#include "settings.h"
#include "display.h"

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
        Serial.println("\nProvisioning started\nGive Credentials of your access point using smartphone app");
        break;
    case ARDUINO_EVENT_PROV_CRED_RECV: {
        Serial.println("\nReceived Wi-Fi credentials");
        Serial.print("\tSSID : ");
        Serial.println((const char *) sys_event->event_info.prov_cred_recv.ssid);
        Serial.print("\tPassword : ");
        Serial.println((char const *) sys_event->event_info.prov_cred_recv.password);
        break;
    }
    case ARDUINO_EVENT_PROV_CRED_FAIL: {
        Serial.println("\nProvisioning failed!\nPlease reset to factory and retry provisioning\n");
        if(sys_event->event_info.prov_fail_reason == WIFI_PROV_STA_AUTH_ERROR)
            Serial.println("\nWi-Fi AP password incorrect");
        else
            Serial.println("\nWi-Fi AP not found....Add API \" nvs_flash_erase() \" before beginProvision()");
        break;
    }
    case ARDUINO_EVENT_PROV_CRED_SUCCESS:
        Serial.println("\nProvisioning Successful");
        break;
    case ARDUINO_EVENT_PROV_END:
        Serial.println("\nProvisioning Ends");
        break;
    default:
        break;
    }
}

SettingsManager *settingsManager;

constexpr int PROG_BUTTON_PIN = 0; // GPIO0

void wifi_connect(Display* display) {
  const char * pop = "abcd1234"; // Proof of possession - otherwise called a PIN - string provided by the device, entered by user in the phone app
  const char * service_name = "PROV_RDR"; // Name of your device (the Espressif apps expects by default device name starting with "Prov_")
  const char * service_key = NULL; // Password used for SofAP method (NULL = no password needed)
  bool reset_provisioned = false; // When true the library will automatically delete previously provisioned data.

  pinMode(PROG_BUTTON_PIN, INPUT_PULLUP); // Set the PROG button as an input with pull-up resistor
  WiFi.onEvent(SysProvEvent);

  display->taf("press and hold prog now to reset .... \n");
  display->scroll_now();
  delay(3000);
  if (digitalRead(PROG_BUTTON_PIN) == LOW) {
    display->taf("Resetting\n");
    display->scroll_now();
    reset_provisioned = true;
  }
  display->taf("Begin Provisioning using Soft AP\n");
  display->scroll_now();
  uint8_t uuid[16] = {0xb4, 0xdf, 0x5a, 0x1c, 0x3f, 0x6b, 0xf4, 0xbf,
                     0xea, 0x4a, 0x82, 0x03, 0x04, 0x90, 0x1a, 0x02 };
  WiFiProv.beginProvision(WIFI_PROV_SCHEME_SOFTAP,
                          WIFI_PROV_SCHEME_HANDLER_NONE,
                          WIFI_PROV_SECURITY_1,
                          pop,
                          service_name,
                          service_key,
                          uuid,
                          reset_provisioned);

//  log_d("wifi qr");
//  WiFiProv.printQR(service_name, pop, "softap");

  while (WiFi.status() != WL_CONNECTED) {
    display->taf("Connecting to WiFi...");
    display->scroll_now();
  }
}
