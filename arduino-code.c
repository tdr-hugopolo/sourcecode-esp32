#include "WiFi.h"
#include <FastLED.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>
#include "ABlocks_LiquidCrystal_I2C.h"
#include <WebServer.h>
#include "ABlocks_DHT.h"
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

#include <ESP32Servo.h>

double Temperatura;
double mode;
double Humitat;
double i;
double Llum;
double Potenciometre;
String s_Mando;
boolean b_Alarma;
boolean b_Persona_dentro;
boolean b_Coche_dentro;
boolean b_Alarma_sonando;
const char espwifi_ssid[]="MiFibra-DBB9";
const char espwifi_pass[]="WEUZW6iY";
CRGB neopixelFL_leds[128];
Adafruit_8x8matrix ledmatrix_1 =Adafruit_8x8matrix();
LiquidCrystal_I2C lcd_1(0x27,16,2);
WebServer http_server;
static const uint8_t PROGMEM ledmatrix_data_0[] PROGMEM = {B11111111,B11111111,B11111111,B11111111,B11111111,B11111111,B11111111,B11111111};
static const uint8_t PROGMEM ledmatrix_data_1[] PROGMEM = {B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000};
DHT dht17(17,DHT11);
IRrecv ir_rx(23);
decode_results ir_rx_results;
unsigned long ir_rx_results_value=0;
String ir_rx_results_protocol="";
int ir_rx_results_bits=0;
static const uint8_t PROGMEM ledmatrix_data_2[] PROGMEM = {B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000,B00000000};
Servo servo_27;
Servo servo_14;
Servo servo_12;
Servo servo_13;

void http_server_on_auto(){
	lcd_1.clear();
	http_server.send(201,String("text/html"),String("S\'ha configurat el mode automàtic."));
	for (i = 0; i <= 150; i=i+1) {
		neopixelFL_leds[(int)(i)].r = 204;
		neopixelFL_leds[(int)(i)].g = 51;
		neopixelFL_leds[(int)(i)].b = 204;
		FastLED.show();
	}

}

void http_server_on_manual(){
	lcd_1.clear();
	http_server.send(201,String("text/html"),String("S\'ha configurat el mode manual."));
	for (i = 0; i <= 150; i=i+1) {
		neopixelFL_leds[(int)(i)].r = 51;
		neopixelFL_leds[(int)(i)].g = 255;
		neopixelFL_leds[(int)(i)].b = 51;
		FastLED.show();
	}

}

void http_server_on_values(){
	http_server.send(201,String("text/html"),String("{")+String((String("\"")+String("light")+String("\"")+String(":")+String(Llum)))+String(",")+String((String("\"")+String("temp")+String("\"")+String(":")+String(Temperatura)))+String(",")+String((String("\"")+String("humidity")+String("\"")+String(":")+String(Humitat)))+String(",")+String((String("\"")+String("ip")+String("\"")+String(":")+String("\"")+String(WiFi.localIP().toString())+String("\"")))+String(",")+String((String("\"")+String("histname")+String("\"")+String(":")+String("\"")+String(WiFi.getHostname())+String("\"")))+String("}"));

}

void espwifi_setup(){
	WiFi.mode(WIFI_STA);
	WiFi.begin(espwifi_ssid,espwifi_pass);
	while (WiFi.status() != WL_CONNECTED) delay(500);
}

void http_server_on_llumon(){
	Encendre_llum();
	http_server.send(201,String("text/html"),String("El llum s\'ha encès!"));

}

void Alarma2() {
	ledmatrix_1.clear();
	ledmatrix_1.drawBitmap(0,0,ledmatrix_data_0,8,8,LED_ON);
	ledmatrix_1.writeDisplay();
	digitalWrite(26, HIGH);
	delay(100);
	ledmatrix_1.clear();
	ledmatrix_1.drawBitmap(0,0,ledmatrix_data_1,8,8,LED_ON);
	ledmatrix_1.writeDisplay();
	digitalWrite(26, LOW);
	delay(100);
}
String fnc_ir_rx_decode_txt()
{
	char buff[16];
	buff[0]=0;
	ir_rx_results_value=0;
	ir_rx_results_bits=0;
	ir_rx_results_protocol="";
	if(ir_rx.decode(&ir_rx_results))
	{
		sprintf(buff,"%08lX",(unsigned long)ir_rx_results.value);
		ir_rx_results_value=(unsigned long)ir_rx_results.value;
		ir_rx_results_bits=ir_rx_results.bits;
		if(ir_rx_results.decode_type==RC5)ir_rx_results_protocol="RC5";
		else if(ir_rx_results.decode_type==RC6)ir_rx_results_protocol="RC6";
		else if(ir_rx_results.decode_type==NEC)ir_rx_results_protocol="NEC";
		else if(ir_rx_results.decode_type==SONY)ir_rx_results_protocol="SONY";
		else if(ir_rx_results.decode_type==PANASONIC)ir_rx_results_protocol="PANASONIC";
		else if(ir_rx_results.decode_type==JVC)ir_rx_results_protocol="JVC";
		else if(ir_rx_results.decode_type==SAMSUNG)ir_rx_results_protocol="SAMSUNG";
		else if(ir_rx_results.decode_type==WHYNTER)ir_rx_results_protocol="WHYNTER";
		else if(ir_rx_results.decode_type==AIWA_RC_T501)ir_rx_results_protocol="AIWA";
		else if(ir_rx_results.decode_type==LG)ir_rx_results_protocol="LG";
		else if(ir_rx_results.decode_type==SANYO)ir_rx_results_protocol="SANYO";
		else if(ir_rx_results.decode_type==MITSUBISHI)ir_rx_results_protocol="MITSUBISHI";
		else if(ir_rx_results.decode_type==DENON)ir_rx_results_protocol="DENON";
		ir_rx.resume();
	}
	return String(buff);
}

double fnc_ultrasonic_distance(int _t, int _e){
	unsigned long dur=0;
	digitalWrite(_t, LOW);
	delayMicroseconds(5);
	digitalWrite(_t, HIGH);
	delayMicroseconds(10);
	digitalWrite(_t, LOW);
	dur = pulseIn(_e, HIGH, 18000);
	if(dur==0)return 999.0;
	return (dur/57);
}

void _http_server_on__NOTFOUND__(){
	http_server.send(404,String("text/html"),String("No s\'ha trobat la sol·licitut que voleu fer."));

}

void Tancar_finestres() {
	servo_27.write(180);
	servo_14.write(180);
	servo_12.write(180);
	servo_13.write(180);
}
void Obrir_finestres() {
	servo_27.write(45);
	servo_14.write(45);
	servo_12.write(45);
	servo_13.write(45);
}
void http_server_on_llumoff(){
	Apagar_llum();
	http_server.send(201,String("text/html"),String("El llum s\'ha apagat!"));

}

void Encendre_llum() {
	for (i = 0; i <= 150; i=i+1) {
		neopixelFL_leds[(int)(i)].r = 255;
		neopixelFL_leds[(int)(i)].g = 255;
		neopixelFL_leds[(int)(i)].b = 255;
		FastLED.show();
	}
}
void Apagar_llum() {
	FastLED.clear();
	FastLED.show();
}
void http_server_on_finestresup(){
	Obrir_finestres();
	http_server.send(201,String("text/html"),String("S\'han aixecat les finestres."));

}

void http_server_on_finestresdown(){
	Tancar_finestres();
	http_server.send(201,String("text/html"),String("S\'han baixat les finestres."));

}

void setup()
{
  	pinMode(26, OUTPUT);
	pinMode(17, INPUT);
	pinMode(19, OUTPUT);
	pinMode(18, INPUT);
	servo_27.attach(27);
	servo_14.attach(14);
	servo_12.attach(12);
	servo_13.attach(13);

	pinMode(35, INPUT);
	dht17.begin();
	pinMode(2, INPUT);
	ir_rx.enableIRIn();

	espwifi_setup();
	FastLED.addLeds<WS2812B, 16,GRB>(neopixelFL_leds,128);
	b_Alarma = true;
	b_Persona_dentro = false;
	mode = 0;
	b_Coche_dentro = false;
	ledmatrix_1.begin(0x70);
	ledmatrix_1.setType(0);
	ledmatrix_1.setTextWrap(false);
	ledmatrix_1.setTextSize(1);
	lcd_1.begin();
	lcd_1.noCursor();
	lcd_1.backlight();
	http_server.begin(80);

	http_server.on("/auto",HTTP_GET,http_server_on_auto);
	http_server.on("/manual",HTTP_GET,http_server_on_manual);
	http_server.on("/values",HTTP_GET,http_server_on_values);
	http_server.on("/llumon",HTTP_GET,http_server_on_llumon);
	http_server.onNotFound(_http_server_on__NOTFOUND__);
	http_server.on("/llumoff",HTTP_GET,http_server_on_llumoff);
	http_server.on("/finestresup",HTTP_GET,http_server_on_finestresup);
	http_server.on("/finestresdown",HTTP_GET,http_server_on_finestresdown);
}


void loop()
{
	yield();

	http_server.handleClient();
  	lcd_1.setCursor(1, 0);
  	lcd_1.print(WiFi.localIP().toString());
  	lcd_1.setCursor(1, 1);
  	lcd_1.print(WiFi.getHostname());
  	Potenciometre = analogRead(35);
  	Temperatura = dht17.readTemperature();
  	Humitat = dht17.readHumidity();
  	Llum = analogRead(2);
  	s_Mando = fnc_ir_rx_decode_txt();
  	if ((mode == 0)) {
  		for (int count = 0; count < 1; count++) {
  			lcd_1.setCursor(15, 1);
  			lcd_1.print(String("A"));
  		}
  		if ((b_Alarma == true)) {
  			digitalWrite(26, HIGH);
  			if ((fnc_ultrasonic_distance(19,18) < 10)) {
  				b_Alarma_sonando = true;
  				while (!((b_Alarma == false))) {
  					Alarma2();
  				}
  			}
  			else if ((fnc_ultrasonic_distance(19,18) > 10)) {
  				ledmatrix_1.clear();
  				ledmatrix_1.writeDisplay();
  			}

  		}

  		if ((b_Alarma_sonando == true)) {
  			lcd_1.setCursor(0, 0);
  			lcd_1.print(String(" APAGAR  ALARMA "));
  			lcd_1.setCursor(0, 1);
  			lcd_1.print(String("Prem OK al mando"));
  			if (String(s_Mando).equals(String("00FF02FD"))) {
  				b_Alarma = false;
  				ledmatrix_1.clear();
  				ledmatrix_1.drawBitmap(0,0,ledmatrix_data_2,8,8,LED_ON);
  				ledmatrix_1.writeDisplay();
  				digitalWrite(26, LOW);
  			}

  		}

  	}

  	if ((mode == 1)) {
  		for (int count = 0; count < 1; count++) {
  			lcd_1.setCursor(15, 1);
  			lcd_1.print(String("M"));
  		}
  	}

}