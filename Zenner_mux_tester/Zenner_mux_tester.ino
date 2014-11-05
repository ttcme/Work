//define included libraries
#include <SD.h>
#include <SPI.h>
//SPI Pins
#define SCK 13
#define MISO 12
#define MOSI 11
#define SS 10
//slave select pins
#define ADC_SS 0
#define MUX_SS 1
#define SD_SS 2
//define input pins
#define STOP_PIN 7
#define N_DRDY 8
#define TEMP_PIN A0
// define output pins
#define C_CONTROL1 9
#define C_CONTROL2 6
#define C_CONTROL3 5
#define C_CONTROL4 3
//SPI MODES:          (must be set correctly before the SPI bus is used)
// MUX : SPI_MODE2
// ADC : SPI_MODE3
// SDCARD : SPI_MODE0

//misc defines
#define NONE 0
#define ADC 1
#define MUX 2
#define SDCARD 3
#define NOOFZENER 8
File datalog;
long sample_period = 10000; //complete sample set 10 seconds after last sample completion

void Set_Current(int lookupselect){
  const unsigned int CURRENTLOOKUP[] = {0 , 1000, 4, 200, 8, 20, 500, 100, 48}; //current in mA
  int current[] = {0,0,0,0};                                                    // alternation high and low current to prevent extended periods of heating
  for (int x=0; x<4 ; x++){
    int lookupuse = lookupselect + x;
    if (lookupuse > 8){ //allow lookupuse to wrap around CURRENTLOOKUP
      lookupuse = lookupuse - 9;
    }
    current[x] = CURRENTLOOKUP[lookupuse] / 4;    // 0-1000 mA is approx scaled to 0-255, actually to 250
  }
  
  analogWrite(C_CONTROL1, current[0]);
  analogWrite(C_CONTROL2, current[1]);
  analogWrite(C_CONTROL3, current[2]);
  analogWrite(C_CONTROL4, current[3]);
}
void Select(int sel){
  if (sel == NONE){
    digitalWrite(MUX_SS, HIGH);
    digitalWrite(ADC_SS, HIGH);
    digitalWrite(SD_SS, HIGH);
  }
  else if (sel == MUX){
    SPI.setDataMode(SPI_MODE2);
    digitalWrite(MUX_SS, LOW);
    digitalWrite(ADC_SS, HIGH);
    digitalWrite(SD_SS, HIGH);
  }
  else if (sel == ADC){
    SPI.setDataMode(SPI_MODE3);
    digitalWrite(MUX_SS, HIGH);
    digitalWrite(ADC_SS, LOW);
    digitalWrite(SD_SS, HIGH);
  }
  else if (sel == SDCARD){
    SPI.setDataMode(SPI_MODE0);
    digitalWrite(MUX_SS, HIGH);
    digitalWrite(ADC_SS, HIGH);
    digitalWrite(SD_SS, LOW);
  }
}
void Mux_Channel(int channel){ //selects mux channel 0-32 from an intager input of 0-32
  const unsigned char lookup[] = {                   //table of all reversed numbers, correctly
    0x80, 0x10, 0x08, 0x18, 0x04, 0x14, 0x0c, 0x1c,   //alligned for the MSB out order the MUX expects
    0x02, 0x12, 0x0a, 0x1a, 0x06, 0x16, 0x0e, 0x1e,
    0x01, 0x11, 0x09, 0x19, 0x05, 0x15, 0x0d, 0x1d,
    0x03, 0x13, 0x0b, 0x1b, 0x07, 0x17, 0x0f, 0x1f}; 
  char dataout = lookup[channel];
  Select(MUX);
  SPI.transfer(dataout);
  Select(NONE);
}
int Read_Temp(){    //reads temperature and maps to degrees C
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
  pinMode(SS, OUTPUT); //MUST BE THIS WAT TO ALLOW FOR SPI, EVEN IF NOT USED
  digitalWrite(ADC_SS,HIGH);
  digitalWrite(MUX_SS,HIGH);
  digitalWrite(SD_SS,HIGH);
  Select(NONE);
  Set_Current(0);
  //SPI setup
  SPI.begin();
  SPI.setBitOrder(MSBFIRST); //correct
  SPI.setDataMode(SPI_MODE3); //i think correct, unsure
  
  SD.begin(SD_SS); //select not needed, all is set up inside begin. Is needed in future
  datalog = SD.open("Datalog1.csv", FILE_WRITE);
  //File header Writing, CSV format
  datalog.print(" ,");
  for (int x = 1; x <= NOOFZENER; x++){
    datalog.print("Zener");
    datalog.print(x);
    if(x == NOOFZENER){
      datalog.println(", ");
    }
    else{
      datalog.print(", ,");
    }
  }
  datalog.print("Temperature,");
  for (int x = 1; x<= NOOFZENER; x++){
    if (x < NOOFZENER){
      datalog.print("Current,Voltage,");
    }
    else{
      datalog.println("Current,Voltage");
    }
  }
  //adc setup
  Select(ADC);
  SPI.transfer(0x10);
  SPI.transfer(0x68);
  Select(NONE);
  //Deselect a mux channel
  Mux_Channel(0);
}

void loop() {
  
  // loop variables
  long current = 0;
  int holder = 0; //used for transient storage of higher or lower byte from adc
  long voltage = 0;
  delay(sample_period);
  
  datalog.print(Read_Temp());
  datalog.print(",");
  //asumed order of mux connections. 1:zener1 current
                            //      2:       voltage
                           //       3:zener2 current
                            //      4:       voltage etc..
  for (int incursel = 0 ; incursel <=8;incursel++){
    Set_Current(incursel);
    delay(1);
    for (int x=1; x<=8; x++){
      int y =2*x+1;
      Mux_Channel(y);
      Select(ADC);
      while (N_DRDY){
        delay(1);}
        
      SPI.transfer(0x38); //read request
      holder = SPI.transfer(0xFF); //upper adc byte to lower byte inholder
      current = holder << 8; //lower byte to upper byte in current
      holder = SPI.transfer(0xFF); //lower adc byte to lower holder
      current =+ holder; //add lower to upper current
      
      y++;
      Mux_Channel(y);
      Select(ADC);
      while (N_DRDY){
        delay(1);}
      
      SPI.transfer(0x38); //read request
      holder = SPI.transfer(0xFF); //upper adc byte to lower byte inholder
      voltage = holder << 8; //lower byte to upper byte in voltage
      holder = SPI.transfer(0xFF); //lower adc byte to lower holder
      voltage =+ holder; //add lower to upper voltage
      
      Select(SDCARD); // select sdcard for datawriting
      datalog.print(current);
      datalog.print(",");
      datalog.print(voltage);
      if (x == 8){ // if last in loop start new line
        datalog.println();
        if (digitalRead(STOP_PIN) == HIGH){ //exit clause at the end of a cycle if pin high
          Select(NONE);
          while(true){//double blink to indicate stopped, SD card safe to remove
            digitalWrite(SCK, HIGH);
            delay(500);
            digitalWrite(SCK, LOW);
            delay(500);
            digitalWrite(SCK, HIGH);
            delay(500);
            digitalWrite(SCK, LOW);
            delay(1000);
          }
        }
      }
      else {
        datalog.print(",");
      }
    }
  }
}

