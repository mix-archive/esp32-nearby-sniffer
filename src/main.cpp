#include <map>

#include <WiFi.h>
#include <Arduino_GFX_Library.h>
#include "esp_wifi.h"

#include "pins.h"
#include "constants.h"

const wifi_promiscuous_filter_t filter = {
    .filter_mask = WIFI_PROMIS_FILTER_MASK_MGMT | WIFI_PROMIS_FILTER_MASK_DATA,
};

typedef struct
{
    unsigned protocol : 2;
    unsigned type : 2;
    unsigned subtype : 4;
    unsigned to_ds : 1;
    unsigned from_ds : 1;
    unsigned more_frag : 1;
    unsigned retry : 1;
    unsigned pwr_mgmt : 1;
    unsigned more_data : 1;
    unsigned wep : 1;
    unsigned strict : 1;
} FrameControl;

typedef struct
{
    uint8_t mac[6];

    String toString() const
    {
        char buf[] = "00:00:00:00:00:00";
        sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        return String(buf);
    }
} MacAddr;

typedef struct
{
    FrameControl frame_control;
    unsigned duration : 16;
    MacAddr destination_address;
    MacAddr source_address;
    MacAddr bss_id;
    int16_t sequence_control;
    unsigned char payload[];
} WifiMgmtHdr;

std::map<String, unsigned long> mac_addresses;

void sniffer(void *buf, wifi_promiscuous_pkt_type_t type)
{
    if (type != WIFI_PKT_MGMT)
        return;

    digitalWrite(PIN_BUILTIN_LED, HIGH);

    auto *p = (wifi_promiscuous_pkt_t *)buf;
    auto len = p->rx_ctrl.sig_len;
    auto *wh = (WifiMgmtHdr *)p->payload;
    len -= sizeof(WifiMgmtHdr);

    if (len >= 0 && wh->frame_control.from_ds == 0 && wh->frame_control.to_ds == 0)
        mac_addresses[wh->source_address.toString()] = millis();
    digitalWrite(PIN_BUILTIN_LED, LOW);
}

void purge(unsigned long now)
{
    for (auto it = mac_addresses.begin(); it != mac_addresses.end();)
    {
        if (now - it->second > DEFAULT_TTL * 1000)
        {
            it = mac_addresses.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

Arduino_DataBus *bus = new Arduino_ESP32SPI(PIN_LCD_DC, NOT_DEFINED_PIN, PIN_LCD_SCLK, PIN_LCD_MOSI, NOT_DEFINED_PIN);
Arduino_GFX *gfx = new Arduino_ST7789(bus, NOT_DEFINED_PIN, 0 /* rotation */, false /* IPS */);

void setup()
{
    Serial.begin(115200);
    Serial.println("Starting...");

    gfx->begin();
    gfx->fillScreen(WHITE);
    gfx->setCursor(0, 0);
    gfx->setTextColor(BLACK);
    gfx->setTextSize(1);

    pinMode(PIN_LCD_BL, OUTPUT);
    digitalWrite(PIN_LCD_BL, HIGH);

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    pinMode(PIN_BUILTIN_LED, OUTPUT);
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_promiscuous_filter(&filter);
    esp_wifi_set_promiscuous_rx_cb(&sniffer);
}

void loop()
{
    gfx->fillScreen(WHITE);
    gfx->setCursor(0, 0);
    gfx->println("MAC addresses:");
    gfx->println("Total: " + String(mac_addresses.size()));

    auto now = millis();

    for (auto &mac : mac_addresses)
    {
        auto ttl = now >= mac.second ? (now - mac.second) / 1000 : 0;
        gfx->println(mac.first + " : " + String(ttl));
    }

    purge(now);
    delay(1000);
}