#include <Adafruit_NeoPixel.h>
#include <avr/power.h>

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip_0 = Adafruit_NeoPixel(25, 3, NEO_RGB + NEO_KHZ800);
Adafruit_NeoPixel strip_1 = Adafruit_NeoPixel(25, 4, NEO_RGB + NEO_KHZ800);
Adafruit_NeoPixel strip_2 = Adafruit_NeoPixel(25, 5, NEO_RGB + NEO_KHZ800);
Adafruit_NeoPixel strip_3 = Adafruit_NeoPixel(25, 6, NEO_RGB + NEO_KHZ800);
Adafruit_NeoPixel strip_4 = Adafruit_NeoPixel(25, 7, NEO_RGB + NEO_KHZ800);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.


#define STATE_IDLE 0
#define STATE_INIT 1
#define STATE_PLAY 2
#define STATE_FINISH 3


#define KEY_0 0x70
#define KEY_1 0x69
#define KEY_2 0x72
#define KEY_3 0x7A
#define KEY_4 0x6B
#define KEY_5 0x73
#define KEY_6 0x74
#define KEY_7 0x6C
#define KEY_8 0x75
#define KEY_9 0x7D



int game_state;
bool result; //result of game


uint32_t scene[5][5][5];


class Shuttle {
private:
  int pos[3] = {0, 0, 0};
  int move_d[3] = {0, 0, 0};
 
 
public:
  void setp() {
   pos[0] = 2;
   pos[1]= 0;
   pos[2] = 2;
  }

  int get_x(){
    return pos[0];
  }

  int get_z(){ 
    return pos[2];
  }
  
  void move(int dir) {
    draw(0);
    switch(dir) {
      case 1: 
       move_d[0] = -1;
   move_d[2] = -1;
        break;
      case 2:
       move_d[0] = 0;
   move_d[2] = -1;
        break;
      case 3:
        move_d[0] = 1;
   move_d[2] = -1;
        break;
      case 4: 
         move_d[0] = -1;
   move_d[2] = 0;
        break;
      case 6:
         move_d[0] = 1;
   move_d[2] = 0;
        break;
      case 7:
         move_d[0] = -1;
   move_d[2] = 1;
        break;
        case 8:
         move_d[0] = 0;
   move_d[2] = 1;
        break;
      case 9:
         move_d[0] = 1;
   move_d[2] = 1;
        break;
    }
    int tmp = pos[0] + move_d[0];
    if(tmp<=4 && tmp>=0){
      pos[0] = tmp;
    }
    tmp = pos[2] + move_d[2];
    if(tmp<=4 && tmp>=0){
      pos[2] = tmp;
    }
   move_d[0] = 0;
   move_d[2] = 0;
   draw(strip_0.Color(0,255,0));
  }

  void draw(uint32_t color){
    scene[pos[0]][pos[1]][pos[2]] = color;
  }
};
 
Shuttle shuttle;


class Meteor {  
  private:
int pos[3] = {0, 0, 0};
int form[5][5];
uint32_t color;
public:
 
  void setp(uint32_t color) {    
    pos[0] = random(0,3);
    pos[2] = random(0,3);
    this->color = color;
    for (int i = 0; i < 5; i++) {
      for (int j = 0; j < 5; j++) {
        form[i][j] = random(0,2); 
      }
    }
  }  
  void set_y(int y){
    pos[1] = y;
  }

  void move(){
   draw(0);
   if(pos[1]==0){
        setp(strip_0.Color(random(0,255),0,random(0,255)));
        set_y(16);
   }
    pos[1]--;
    if(pos[1] == 0){
      if(isColision(shuttle.get_x(), shuttle.get_z() )){
        game_state = STATE_FINISH;
        result = false;
      } 
    }
    draw(color);
  }

  bool isColision(int x, int z) {
    for (int i = 0; i < 5; i++) {
      for (int j = 0; j < 5; j++) {
        if (x == i && z == j && form[i][j] > 0) return true;
      }
    }
    return false;
  }
  
  void draw(uint32_t color){
    if(pos[1] < 5){
    for(int i = 0; i < 5; i++){
      for(int j = 0; j < 5; j++){
        if(form[i][j] > 0){
        scene[i][pos[1]][j] = color;
        }
      }
    }
  }
 }
};

Meteor meteor[5];

void setup() {
  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
#if defined (__AVR_ATtiny85__)
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
  // End of trinket special code
  Serial.begin(11000, SERIAL_8O1);
 // Serial.setTimeout(50);
  //Serial.begin(9600);

  strip_0.begin();
  strip_1.begin();
  strip_2.begin();
  strip_3.begin();
  strip_4.begin();
  strip_0.show(); // Initialize all pixels to 'off'
  strip_1.show();
  strip_2.show();
  strip_3.show();
  strip_4.show();

  game_state = STATE_IDLE;  
  
}


uint8_t signalKey;

void serialEvent(){
// statements
  static char event_state = 0;//0 key_down, 1 - key_up
  while (Serial.available()){
    uint8_t rec_byte = Serial.read();
    if (rec_byte == 0xf0){
      event_state = 1;
    } else {
      if (event_state == 1){
        event_state = 0;
      } else {
        signalKey = rec_byte;    
      }
    }
  }
}

int get_signal2() {
  switch (signalKey) {
    case KEY_1:
      signalKey = 0; 
      return 1;
    case KEY_2:
      signalKey = 0;
      return 2;
    case KEY_3:
      signalKey = 0;
      return 3;
    case KEY_4:
      signalKey = 0;
      return 4;
      case KEY_5:
      signalKey = 0; 
      return 5;
    case KEY_6:
      signalKey = 0;
      return 6;
    case KEY_7:
      signalKey = 0;
      return 7;
    case KEY_8:
      signalKey = 0;
      return 8;
      case KEY_9:
      signalKey = 0;
      return 9;
  }
  return 0;
}

int get_signal() { 
  int sensorPin1 = A0; 
  int sensorPin2= A1;
  int axis1 = 0;
  int axis2 = 0;
  int thrHold = 200;
  int normalizer = 524;
  double phi = 0;
  double r_com = 1;
  int command;
  
  axis1 = analogRead(sensorPin1);
  axis1 -= normalizer;
  axis2 = analogRead(sensorPin2);
  axis2 -= normalizer;
  
  r_com = sqrt(pow(axis1,2) + pow(axis2,2));
  if(axis2 > 0) phi = acos(axis1 / r_com);
  else phi = 2*PI - acos(axis1 / r_com);
  
  if (r_com > thrHold){
    if (phi < PI/4)  command = 1;
    else if (phi < 3*PI/4)  command = 2;
    else if (phi < 5*PI/4)  command = 3;
    else if (phi < 7*PI/4)  command = 4;
    else command = 1;
  }
  else command = 0;
  return command;
}


void loop() {
  int sg = get_signal2();  
  //Serial.print("  Signal "); 
  //Serial.print(sg);
  //Serial.print(game_state);
  switch(game_state) {
    case STATE_IDLE : 
      if (sg == 0) {
        idle_state();
      } else {
        game_state = STATE_INIT;
      }
      break;
      
    case STATE_INIT: 
      init_game();
      game_state = STATE_PLAY;
      break;

    case STATE_PLAY:
      //game logic
      shuttle.move(sg);
      for(int i = 0; i < 5; i++){
        meteor[i].move();
      }
      update_scene(); 
      delay(700);  
     //выдавать резльтат
      break;

    case STATE_FINISH: 
      //final screen
      if (result) {
        //rainbow(20);
        splash();
      } else {
        fill_screen(strip_0.Color(255,0,0));  
        delay(600);
        fill_screen(0);
        delay(200);
        fill_screen(strip_0.Color(255,0,0)); 
      }
      fill_screen(0);
      game_state = STATE_IDLE;
      break;
  }   
}


void idle_state() {
  fill_screen(0);
    //for (int i = 0; i < 5; i++) {
    //  for (int j = 0; j < 5; j ++) {
        for (int k = 0; k < 5; k++) {
           set3dpix(k,0,0 , strip_0.Color(255,0,0));
           delay(100);     
        }
      //}
    //} 
}

void init_game() {
  result = false;
  shuttle.setp();
  for(int i=0; i < 5 ; i++){
    meteor[i].setp(strip_0.Color(random(0,255),0,random(0,255)));
    meteor[i].set_y(4 + i*3);
  }
  
}


void fill_screen(uint32_t color) {
  for (int i = 0; i < 5; i++) {
      for (int j = 0; j < 5; j ++) {
        for (int k = 0; k < 5; k++) {
           scene[i][j][k] = color;
           set3dpix(i,j,k, scene[i][j][k]);              
        }
      }
   } 
}

void update_scene() {  
  for (int i = 0; i < 5; i++) {
      for (int j = 0; j < 5; j ++) {
        for (int k = 0; k < 5; k++) {
           set3dpix(i,j,k, scene[i][j][k]);               
        }
      }
   } 
}

void set3dpix(uint16_t x, uint16_t y, uint16_t z, uint32_t color) {  
  switch (z) {
    case 0: 
      if (y % 2 == 0) {
        strip_0.setPixelColor(x + y * 5, color);
      } else {
        strip_0.setPixelColor(y * 5 + 4 - x, color);
      }
      strip_0.show();
      break;
    case 1: 
      if (y % 2 == 0) { 
        strip_1.setPixelColor(x + 5 * y, color);
      } else {
        strip_1.setPixelColor(y * 5 + 4 - x, color);
      }
      strip_1.show();
      break;
    case 2: 
      if (y % 2 == 0) { 
        strip_2.setPixelColor(x + 5 * y, color);
      } else {
        strip_2.setPixelColor(y * 5 + 4 - x, color);
      }
      strip_2.show();
      break;
    case 3: 
      if (y % 2 == 0) { 
        strip_3.setPixelColor(x + 5 * y, color);
      } else {
        strip_3.setPixelColor(y * 5 + 4 - x, color);
      }
      strip_3.show();
      break;
    case 4: 
      if (y % 2 == 0) { 
        strip_4.setPixelColor(x + 5 * y, color);
      } else {
        strip_4.setPixelColor(y * 5 + 4 - x, color);
      }
      strip_4.show();
      break;
  }
}

void splash() {
  // Some example procedures showing how to display to the pixels:

  colorWipe(strip_0.Color(0, 255, 0), 50); // Green
  colorWipe(strip_0.Color(0, 0, 255), 50); // Blue
  // Send a theater pixel chase in...
  theaterChase(strip_0.Color(127, 127, 127), 50); // White
  theaterChase(strip_0.Color(127,   0,   0), 50); // Red
  theaterChase(strip_0.Color(  0,   0, 127), 50); // Blue 
  
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<25; i++) {
      strip_0.setPixelColor(i, c);
      strip_0.show();
      strip_1.setPixelColor(i, c);
      strip_1.show();
      strip_2.setPixelColor(i, c);
      strip_2.show();
      strip_3.setPixelColor(i, c);
      strip_3.show();
      strip_4.setPixelColor(i, c);
      strip_4.show();
     
      delay(wait);
  }
}



//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (int i=0; i < 25; i=i+3) {
        strip_0.setPixelColor(i+q, c); 
        strip_1.setPixelColor(i+q, c);
        strip_2.setPixelColor(i+q, c);
        strip_3.setPixelColor(i+q, c);
        strip_4.setPixelColor(i+q, c);//turn every third pixel on
      }
    strip_0.show();
    strip_1.show();
    strip_2.show();
    strip_3.show();
    strip_4.show();
     
      delay(wait);
     
      for (int i=0; i < 25; i=i+3) {
        strip_0.setPixelColor(i+q, 0); 
        strip_1.setPixelColor(i+q, 0);
        strip_2.setPixelColor(i+q, 0);
        strip_3.setPixelColor(i+q, 0);
        strip_4.setPixelColor(i+q, 0);   
        //turn every third pixel off
      }
    }
  }
}
