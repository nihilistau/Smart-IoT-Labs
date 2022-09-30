#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_wifi_types.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include <SoftwareSerial.h>
SoftwareSerial uart;

#include <time.h>


#define CARDKB_ADDR 0x5F

#define LED_GPIO_PIN                     25
#define WIFI_CHANNEL_SWITCH_INTERVAL  (800)  //(500)
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
  esp_event_loop_init(event_handler, NULL);
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&cfg);
  esp_wifi_set_country(&wifi_country); /* set country for channel range [1, 13] */
  esp_wifi_set_storage(WIFI_STORAGE_RAM);
  esp_wifi_set_mode(WIFI_MODE_NULL);
  esp_wifi_start();
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
  case WIFI_PKT_MISC: return "MISC";
  default:  return "MISC";
  }
}


int match_a1=0, match_a2=0;
 
union 
  {
  uint8_t frame[4120]; // 512*(8) + 2*6 + 4*4 = 
  struct
    {
    int ti; int minRSSI; int maxRSSI;
    uint8_t minmac[6];
    uint8_t maxmac[6];
    uint8_t tmac[512][8];  // -RSSI , channel
    } pack;
  } udp;     // source address : addr2

  union 
  {
  uint8_t frame[2072]; // 32*(8) + 2*6 + 4*4 = 
  struct
    {
    int ti; int minRSSI; int maxRSSI;
    uint8_t minmac[6];
    uint8_t maxmac[6];
    uint8_t tmac[256][8];  // -RSSI , channel, 32 destination
    } pack;
  } dest;     // destination addresses  addr1

  


bool cmpmac(uint8_t *mac1,uint8_t *mac2)
{
  int i=0;
  for(i=0;i<6;i++) if(mac1[i]!=mac2[i]) return false;
  return true;
}

int limRSSI=-100;
char dbuff[1024];char rep[512];
int val=99;

long dflat=0,dflong=0;

void wifi_sniffer_packet_handler(void* buff, wifi_promiscuous_pkt_type_t type)
{
  const wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t *)buff;
  const wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)ppkt->payload;
  const wifi_ieee80211_mac_hdr_t *hdr = &ipkt->hdr;

  if((-ppkt->rx_ctrl.rssi) <32) return;
  if(ppkt->rx_ctrl.rssi < limRSSI) return;
  if(ppkt->rx_ctrl.rssi < udp.pack.minRSSI) { udp.pack.minRSSI=(int)ppkt->rx_ctrl.rssi; memcpy(udp.pack.minmac,hdr->addr2,6); }
  if(ppkt->rx_ctrl.rssi > udp.pack.maxRSSI) { udp.pack.maxRSSI=(int)ppkt->rx_ctrl.rssi; memcpy(udp.pack.maxmac,hdr->addr2,6); }

  if(type== WIFI_PKT_DATA)
    {
    if(ppkt->rx_ctrl.rssi < dest.pack.minRSSI) { dest.pack.minRSSI=(int)ppkt->rx_ctrl.rssi; memcpy(dest.pack.minmac,hdr->addr1,6); }
    if(ppkt->rx_ctrl.rssi > dest.pack.maxRSSI) { dest.pack.maxRSSI=(int)ppkt->rx_ctrl.rssi; memcpy(dest.pack.maxmac,hdr->addr1,6); }   
    }
  
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

 match_a2=0; match_a1=0;
 
 for(int j=0;j<udp.pack.ti;j++)
   {
    if(cmpmac((uint8_t *)hdr->addr2,udp.pack.tmac[j])) { match_a2=1;}  // cmpmac - true if equal
    else continue;
   }
 for(int j=0;j<dest.pack.ti;j++)
   {
    if(cmpmac((uint8_t *)hdr->addr1,dest.pack.tmac[j])) { match_a1=1;}  // cmpmac - true if equal
    else continue;
   }
   
 if(match_a2==0) 
   {
    if(udp.pack.ti==511) { udp.pack.ti=0;}
    memcpy(udp.pack.tmac[udp.pack.ti],hdr->addr2,6); 
    udp.pack.tmac[udp.pack.ti][6] = (int8_t)-ppkt->rx_ctrl.rssi;
    udp.pack.tmac[udp.pack.ti][7] = (int8_t)ppkt->rx_ctrl.channel;  
    udp.pack.ti++;
   }
     
 if(match_a1==0) 
   {
    if(dest.pack.ti==31) { dest.pack.ti=0;}
    memcpy(dest.pack.tmac[dest.pack.ti],hdr->addr1,6); 
    dest.pack.tmac[dest.pack.ti][6] = (int8_t)-ppkt->rx_ctrl.rssi;
    dest.pack.tmac[dest.pack.ti][7] = (int8_t)ppkt->rx_ctrl.channel;  
    dest.pack.ti++;
   }
   
 if(match_a1==0 || match_a1==0) 
   {
    Serial.println(udp.pack.ti); 
    Serial.println(dest.pack.ti-1); 
   }  
  delay(40);
}

const int buttonPin=0;

int setRSSI()
{
int did=-100;
int  buttonState;
char dbuf[32];
return did;
}

union 
{
  uint8_t frame[28];
  struct
    {
      uint8_t head[4];
      int nmac; int dmac;
      int minrssi;
      int maxrssi;
      long longitude;
      long latitude;
    } pack;
} sdf;


void UART_Task( void * parameter ){
while(1)
  {
  sdf.pack.nmac=(int)udp.pack.ti; 
  sdf.pack.dmac=(int)dest.pack.ti;
  sdf.pack.minrssi= udp.pack.minRSSI; 
  sdf.pack.maxrssi=udp.pack.maxRSSI;           
  sdf.pack.head[0]=0xFF;  
  sdf.pack.head[1]=0x00; 
  sdf.pack.head[2]=0x00; 
  sdf.pack.head[3]=0x00; 
  uart.write(sdf.frame,28); 
  //uart.write("hello");
  Serial.println("In the UART Task");
  Serial.printf("\nnmac=%d,dmac=%d,minrssi=%d,maxrssi=%d\n",sdf.pack.nmac,sdf.pack.dmac,sdf.pack.minrssi,sdf.pack.maxrssi);
  delay(10000);
  }
}

int cycle=0;

void setup() {
  // initialize digital pin 5 as an output.
  Serial.begin(9600);
  uart.begin(9600, SWSERIAL_8N1, 16, 17); // RxD, TxD

  limRSSI=setRSSI(); 
  //memcpy(udp.frame,0x00,4124);
  udp.pack.minRSSI=0; udp.pack.maxRSSI=-100; 
  delay(1000);
  wifi_sniffer_init();
  TaskHandle_t myTask;
  xTaskCreate(
                    UART_Task,        /* Task function. */
                    "UART_Task",      /* String with name of task. */
                    10000,              /* Stack size in words. */
                    NULL,               /* Parameter passed as input of the task */
                    2,                 /* Priority of the task. */
                    &myTask);           /* Task handle. */
 
  Serial.print("Setup: created Task priority = ");
  Serial.println(uxTaskPriorityGet(myTask));
}

void loop() {   
if(channel==0x01) 
  { 
  for(int j=0; j<udp.pack.ti;j++) 
    {
    sprintf(dbuff,"MAC:%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x  RSSI=-%d  CHAN=%d",
    udp.pack.tmac[j][0],udp.pack.tmac[j][1], udp.pack.tmac[j][2],udp.pack.tmac[j][3],
    udp.pack.tmac[j][4],udp.pack.tmac[j][5],udp.pack.tmac[j][6],udp.pack.tmac[j][7]);
    Serial.println(dbuff);  
   }
  for(int j=0; j<dest.pack.ti;j++) 
    {
    sprintf(dbuff,"MAC:%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x  RSSI=-%d  CHAN=%d",
    dest.pack.tmac[j][0],dest.pack.tmac[j][1], dest.pack.tmac[j][2],dest.pack.tmac[j][3],
    dest.pack.tmac[j][4],dest.pack.tmac[j][5],dest.pack.tmac[j][6],dest.pack.tmac[j][7]);
    Serial.println(dbuff);  
   }
  cycle++;
  if(cycle>6) 
   {
   udp.pack.ti=0;    dest.pack.ti=0; 
   cycle=0;
   }
 }
  delay(1000); // wait for a second
  vTaskDelay(WIFI_CHANNEL_SWITCH_INTERVAL/portTICK_PERIOD_MS);
  wifi_sniffer_set_channel(channel);
  channel = (channel % WIFI_CHANNEL_MAX) + 1;
}
