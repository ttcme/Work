//define included libraries
#include <sd.h>
#include <SPI.h>

//slave select pins
#define ADC_SS 10
#define MUX_SS 9
#define SD_SS 11
//define input pins
#define N_DRDY 8
#define TEMP_PIN A0

void muxselect(int channel){
  const unsigned char lookup[] = {                   //table of all reversed numbers, correctly
    0x80, 0x10, 0x08, 0x18, 0x04, 0x14, 0x0c, 0x1c,   //alligned for the MSB out order the MUX expects
    0x02, 0x12, 0x0a, 0x1a, 0x06, 0x16, 0x0e, 0x1e,
    0x01, 0x11, 0x09, 0x19, 0x05, 0x15, 0x0d, 0x1d,
    0x03, 0x13, 0x0b, 0x1b, 0x07, 0x17, 0x0f, 0x1f}; 
  char dataout = lookup[channel];
  SPI.setDataMode(SPI_MODE2); //mux works on mode 2 
  digitalWrite(MUX_SS,LOW);
  SPI.transfer(dataout);
  digitalWrite(MUX_SS,HIGH);
  SPI.setDataMode(SPI_MODE3);
}
void set_current(int current){ //current in mA
  current = current/4;          // 0-1000 mA is approx scaled to 0-255, actually to 250
  analogWrite(current);
}
int read_temp(){    //reads temperature and maps to degrees C
  int return_temp = 0;
  return_temp = map (analogRead(TEMP_PIN),0,1023,-50,150); //maps from input scale (0-1023) to device output scale
                                                           //needs to be changed if device changes
  return return_temp;                                      
}

  

void setup() {
pinMode(ADC_SS,OUTPUT); 
pinMode(MUX_SS,OUTPUT);
pinMode(SD_SS,OUTPUT);
pinMode(N_DRDY,INPUT);

digitalWrite(ADC_SS,HIGH);
digitalWrite(MUX_SS,HIGH);
digitalWrite(SD_SS,HIGH);
set_current(0);
//SPI setup
SPI.begin();
SPI.setBitOrder(MSBFIRST); //correct
SPI.setDataMode(SPI_MODE3); //i think correct, unsure

//adc setup
digitalWrite(ADC_SS,LOW);
SPI.transfer(0x10);
SPI.transfer(0x68);
digitalWrite(ADC_SS,HIGH);
//Deselect a mux channel
muxselect(0);

}

void loop() {
  
  
}

