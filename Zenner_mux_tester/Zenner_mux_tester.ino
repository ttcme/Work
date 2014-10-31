//define included libraries
#include <sd.h>
#include <SPI.h>

//slave select pins
#define ADC_ss 10
#define MUX_ss 9
#define SD_ss 11

void setup() {
pinMode(ADC_ss,OUTPUT); 
pinMode(MUX_ss,OUTPUT);
pinMode(SD_ss,OUTPUT);
digitalWrite(ADC_ss,HIGH);
digitalWrite(MUX_ss,HIGH);
digitalWrite(SD_ss,HIGH);
//SPI setup
SPI.begin();
SPI.setBitOrder(MSBFIRST); //correct
SPI.setDataMode(SPI_MODE3); //i think correct, unsure

//adc setup
digitalWrite(ADC_ss,LOW);
SPI.transfer(0x10);
SPI.transfer(0x68);


}

void loop() {
  
  
}
void muxselect(int channel){
  const unsigned char lookup[] = {                   //table of all reversed numbers, correctly
    0x80, 0x10, 0x08, 0x18, 0x04, 0x14, 0x0c, 0x1c,   //alligned for the MSB out order the MUX expects
    0x02, 0x12, 0x0a, 0x1a, 0x06, 0x16, 0x0e, 0x1e,
    0x01, 0x11, 0x09, 0x19, 0x05, 0x15, 0x0d, 0x1d,
    0x03, 0x13, 0x0b, 0x1b, 0x07, 0x17, 0x0f, 0x1f}; 
  char dataout = lookup[channel];
  SPI.setDataMode(SPI_MODE2); //mux works on mode 2 
  digitalWrite(MUX_ss,LOW);
  SPI.transfer(dataout);
  digitalWrite(MUX_ss,HIGH);
  SPI.setDataMode(SPI_MODE3);
}

  
