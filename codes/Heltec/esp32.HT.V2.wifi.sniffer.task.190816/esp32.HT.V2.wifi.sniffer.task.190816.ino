#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_wifi_types.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

#include <U8x8lib.h>  // bibliothèque à charger a partir de 
// the OLED used 
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(15, 4, 16);

#define CARDKB_ADDR 0x5F

#define LED_GPIO_PIN                     25
#define WIFI_CHANNEL_SWITCH_INTERVAL  (500)  //(500)
#define WIFI_CHANNEL_MAX               (13)

uint8_t level = 0, channel = 1;

static wifi_country_t wifi_country =  {.cc="EU", .schan=1, .nchan=13};

typedef struct {
  unsigned frame_ctrl:16;
  unsigned duration_id:16;
  uint8_t addr1[6]; /* receiver address */
  uint8_t addr2[6]; /* sender address */
  uint8_t addr3[6]; /* filtering address */
  unsigned sequence_ctrl:16;
  uint8_t addr4[6]; /* optional */
} wifi_ieee80211_mac_hdr_t;

typedef struct {
  wifi_ieee80211_mac_hdr_t hdr;
  uint8_t payload[0]; /* network data ended with 4 bytes csum (CRC32) */
} wifi_ieee80211_packet_t;

static esp_err_t event_handler(void *ctx, system_event_t *event);
static void wifi_sniffer_init(void);
static void wifi_sniffer_set_channel(uint8_t channel);
static const char *wifi_sniffer_packet_type2str(wifi_promiscuous_pkt_type_t type);
static void wifi_sniffer_packet_handler(void *buff, wifi_promiscuous_pkt_type_t type);

esp_err_t event_handler(void *ctx, system_event_t *event)
{
  return ESP_OK;
}

void wifi_sniffer_init(void)
{
  nvs_flash_init();
  tcpip_adapter_init();
  ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
  ESP_ERROR_CHECK( esp_wifi_set_country(&wifi_country) ); /* set country for channel range [1, 13] */
  ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
  ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_NULL) );
  ESP_ERROR_CHECK( esp_wifi_start() );
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(&wifi_sniffer_packet_handler);
}

void wifi_sniffer_set_channel(uint8_t channel)
{
  esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
}

const char * wifi_sniffer_packet_type2str(wifi_promiscuous_pkt_type_t type)
{
  switch(type) {
  case WIFI_PKT_MGMT: return "MGMT";
  case WIFI_PKT_DATA: return "DATA";
  default:  
  case WIFI_PKT_MISC: return "MISC";
  }
}

uint8_t tmac[400][6]; int ti=0; int match=0;
uint8_t minmac[6],maxmac[6];

bool clearmac()
{
  int i=0;
  for(i=0;i<ti;i++) memset(tmac[i],0x00,6);
  ti=0;
  return 0;
}

bool cmpmac(uint8_t *mac1,uint8_t *mac2)
{
  int i=0;
  for(i=0;i<6;i++) if(mac1[i]!=mac2[i]) return false;
  return true;
}

bool cpymac(uint8_t *mac, int ind)
{
  int i=0;
  for(i=0;i<6;i++) tmac[ind][i]=mac[i];
}

int limRSSI=-100;
char dbuff[32];
int val=99;
int minRSSI=0; int maxRSSI=-100;

void wifi_sniffer_packet_handler(void* buff, wifi_promiscuous_pkt_type_t type)
{
  if (type != WIFI_PKT_MGMT)
    return;

  const wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t *)buff;
  const wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)ppkt->payload;
  const wifi_ieee80211_mac_hdr_t *hdr = &ipkt->hdr;

  if(ppkt->rx_ctrl.rssi < limRSSI) return;
  if(ppkt->rx_ctrl.rssi < minRSSI) { minRSSI=ppkt->rx_ctrl.rssi; memcpy(minmac,hdr->addr2,6); }
  if(ppkt->rx_ctrl.rssi > maxRSSI) { maxRSSI=ppkt->rx_ctrl.rssi; memcpy(maxmac,hdr->addr2,6); }
  

  printf("PACKET TYPE=%s, CHAN=%02d, RSSI=%02d,"
    " ADDR1=%02x:%02x:%02x:%02x:%02x:%02x,"
    " ADDR2=%02x:%02x:%02x:%02x:%02x:%02x,"
    " ADDR3=%02x:%02x:%02x:%02x:%02x:%02x\n",
    wifi_sniffer_packet_type2str(type),
    ppkt->rx_ctrl.channel,
    ppkt->rx_ctrl.rssi,
    /* ADDR1 */
    hdr->addr1[0],hdr->addr1[1],hdr->addr1[2],
    hdr->addr1[3],hdr->addr1[4],hdr->addr1[5],
    /* ADDR2 */
    hdr->addr2[0],hdr->addr2[1],hdr->addr2[2],
    hdr->addr2[3],hdr->addr2[4],hdr->addr2[5],
    /* ADDR3 */
    hdr->addr3[0],hdr->addr3[1],hdr->addr3[2],
    hdr->addr3[3],hdr->addr3[4],hdr->addr3[5]
  );

 match=0;
 for(int j=0;j<ti;j++)
   {
    if(cmpmac((uint8_t *)hdr->addr2,tmac[j])) match=1;
    else continue;
   }
 if(match==0) 
   {
    if(ti==399) ti=0;
    cpymac((uint8_t *)hdr->addr2,ti);
    ti++;
   }
  Serial.println(ti); 
  sprintf(dbuff,"RSSI>%3.3d",limRSSI);
  u8x8.drawString(0,2,dbuff);
  sprintf(dbuff,"number=%3.3d",ti);
  u8x8.drawString(0,3,dbuff);
  sprintf(dbuff,"minRSSI=%3.3d",minRSSI);
  u8x8.drawString(0,4,dbuff);
  sprintf(dbuff,"maxRSSI=%3.3d",maxRSSI);
  u8x8.drawString(0,5,dbuff);
  sprintf(dbuff,"MAC:%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x",maxmac[0],maxmac[1],maxmac[2],maxmac[3],maxmac[4],maxmac[5]);
  u8x8.drawString(0,6,dbuff);
}

const int buttonPin=34; //=0;

int setRSSI()
{
int did=-120;
int  buttonState;
u8x8.clear(); 
char dbuf[32];
u8x8.drawString(0, 0,"Setting RSSI");
while(1)
  {  
  buttonState = digitalRead(buttonPin);
  if(buttonState) { did=did+10; if(did==-30) did=-120;
                    Serial.println(did);
                    sprintf(dbuf,"RSSI=%3.3d",did);
                    u8x8.drawString(0, 2,dbuf);

                    
                  }
  else 
     { 
      Serial.println(did); 
      u8x8.drawString(0, 4,"RSSI");
      sprintf(dbuf,"set to %3.3d",did);
             u8x8.drawString(0, 5,dbuf);
      return did;
      }
  delay(3000);
  }
}

int const transdelay=60000;
int const transtime=5000;
int const cycle=128;

SemaphoreHandle_t xSemaphore;

void taskCycle( void * parameter)
{ 
  const TickType_t xDelay = 20000 / portTICK_PERIOD_MS; // portTICK_PERIOD is 1 ms
while(1)
   {
   if( xSemaphoreTake( xSemaphore, ( TickType_t ) 10 ) == pdTRUE )
        {
        Serial.println("in the task"); 
        clearmac();
        xSemaphoreGive(xSemaphore );
        }

    vTaskDelay(cycle*(WIFI_CHANNEL_SWITCH_INTERVAL/portTICK_PERIOD_MS));  // cycle steps of wifi scan task
   }      
}


void setup() {
  // initialize digital pin 5 as an output.
  Serial.begin(9600);
  u8x8.begin();  // initialize OLED
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.clear();
  u8x8.drawString(0,0,"SMTR sniffer");
  limRSSI=setRSSI();
  delay(1000);
  wifi_sniffer_init();
  pinMode(LED_GPIO_PIN, OUTPUT);
  
  xSemaphore = xSemaphoreCreateMutex();

  xTaskCreate(
                    taskCycle,          /* Task function. */
                    "taskTS",        /* String with name of task. */
                    10000,            /* Stack size in words. */
                    NULL,             /* Parameter passed as input of the task */
                    0,                /* Priority of the task. */
                    NULL); 
}
             

int time_now=0, initmilis=0;

int chann=2;

void loop() {


   if( xSemaphoreTake( xSemaphore, ( TickType_t ) 10 ) == pdTRUE )
     {
     wifi_sniffer_set_channel(channel);
     channel = (channel % WIFI_CHANNEL_MAX) + 1;
     xSemaphoreGive(xSemaphore );
     }

vTaskDelay(WIFI_CHANNEL_SWITCH_INTERVAL/portTICK_PERIOD_MS);  // should be modified if restart !
     
}
