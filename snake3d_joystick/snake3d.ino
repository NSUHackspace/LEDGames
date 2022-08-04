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

#define MAX_RABBITS 6

int game_state;
bool result; //result of game


uint32_t scene[5][5][5];


class Rabbit {  

public:
  int pos[3] = {0, 0, 0};
 
  void setup() {    
    pos[0] = random(0,5);
    pos[1] = random(0,5);
    pos[2] = random(0,5);
    scene[pos[0]][pos[1]][pos[2]] = strip_0.Color(0,255,0);
  }  
};

class Point {
public:
  void set(int xx, int yy, int zz) {
    x = xx;
    y = yy;
    z = zz;
  }
  int x; 
  int y;
  int z;
};

Rabbit rabbit;

class Snake {
private:
  Point body[MAX_RABBITS + 1]; 
  int start_dir[4] = {0, 0, 1, 1};  
  int pos[3] = {0, 0, 0};
  int init_angle[4][4] = { {1, 0, 0, 0}, 
                      {0, 1, 0, 0},
                      {0, 0, 1, 0},
                      {0, 0, 0, 1} };

  int angle[4][4];


  void xRotPlus() { //Right
    int rotation[4][4] = { { 1, 0, 0, 0},
                           { 0, 0, -1, 0}, 
                           { 0, 1, 0, 0},
                           { 0, 0, 0, 1}} ;
    int result[4][4];
    rotate(angle, rotation, result);
    copy(result, angle);    
  }

  void xRotMinus() { //Left
    int rotation[4][4] = { { 1, 0, 0, 0},
                           { 0, 0, 1, 0}, 
                           { 0, -1, 0, 0},
                           { 0, 0, 0, 1}} ;
    int result[4][4];
    rotate(angle, rotation, result);
    copy(result, angle);    
  }

  void yRotPlus() { //Up
    int rotation[4][4] = { { 0, 0, 1, 0},
                           { 0, 1, 0, 0}, 
                           { -1, 0, 0, 0},
                           { 0, 0, 0, 1}} ;
    int result[4][4];
    rotate(angle, rotation, result);
    copy(result, angle);    
  }

  void yRotMinus() { //Down
    int rotation[4][4] = { { 0, 0, -1, 0},
                           { 0, 1, 0, 0}, 
                           { 1, 0, 0, 0},
                           { 0, 0, 0, 1}} ;
    int result[4][4];
    rotate(angle, rotation, result);
    copy(result, angle);    
  }

  void rotate(int m1[4][4], int m2[4][4], int result[4][4]) {    
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++) {
        result[i][j] = 0;
        for (int k = 0; k < 4; k++) {
          result[i][j] += m1[i][k] * m2[k][j]; 
        }
      }
    }
  }

  void copy(int m1[4][4], int m2[4][4]) {
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++) {
        m2[i][j] = m1[i][j];        
      }
    }
  }

  void shfit() {
    for (int i = rabbits; i > 0; i--) {
      body[i] = body[i-1];      
    }
    body[0].set(pos[0], pos[1], pos[2]);
  }

  void draw_snake(uint32_t body_color, uint32_t head_color) {
    for (int i = 1; i <= rabbits; i++) {
      scene[body[i].x][body[i].y][body[i].z] = body_color;
    }
    scene[body[0].x][body[0].y][body[0].z] = head_color;
  }

  bool isIntersect() {
    for (int i = 1; i <= rabbits; i++) {
      if (body[0].x == body[i].x && body[0].y == body[i].y && body[0].z == body[i].z) return true; 
    }
    return false;
  }
  
public:
  int rabbits;

  void setup() { 
    
    rabbits = 0;
    pos[0] = 0; pos[1]= 0; pos[2] = 0;
    
    body[0].set(pos[0],pos[1],pos[2]);
    for (int i = 1; i < MAX_RABBITS; i++) {
      body[i].set(-1,-1,-1);
    }
    
    copy(init_angle, angle); 
    scene[pos[0]][pos[1]][pos[2]] = strip_0.Color(0,0,255);
  }

  void get_dir(int new_dir[4]) {    
    for (int i = 0; i < 4; i++) {
      new_dir[i] = 0;
      for (int j = 0; j < 4; j++) {
        new_dir[i] += angle[i][j] * start_dir[j];        
      }
    }    
  }
  
  void move() {
    int new_dir[4];    
    get_dir(new_dir);
    //scene[pos[0]][pos[1]][pos[2]] = 0;
    draw_snake(0,0);    
    pos[0] = pos[0] + new_dir[0];
    if (pos[0] >= 5) pos[0] = 0;
    if (pos[0] <= -1) pos[0] = 4;
    
    pos[1] = pos[1] + new_dir[1]; 
    if (pos[1] >= 5) pos[1] = 0;
    if (pos[1] <= -1) pos[1] = 4;
    
    pos[2] = pos[2] + new_dir[2];
    if (pos[2] >= 5) pos[2] = 0;
    if (pos[2] <= -1) pos[2] = 4;
    if (check_rabbit(rabbit)) {
      rabbits++;                  
      rabbit.setup();
    } 
    shfit();
    if (isIntersect()) {
      draw_snake(strip_0.Color(255,0,0), strip_0.Color(255,0,0) );
      result = false;      
      game_state = STATE_FINISH;
    } else {
    //scene[pos[0]][pos[1]][pos[2]] = strip_0.Color(0,0,255);
      draw_snake(strip_0.Color(0,0,255), strip_0.Color(255,0,255) );
    }
  }

  bool check_rabbit(Rabbit rabbit) {
    return (pos[0] == rabbit.pos[0] &&
            pos[1] == rabbit.pos[1] &&
            pos[2] == rabbit.pos[2]);
  }

  void set_dir(int dir) {
    switch(dir) {
      case 1: 
        yRotPlus();
        break;
      case 2:
        xRotMinus();
        break;
      case 3:
        yRotMinus();
        break;
      case 4:
        xRotPlus();
        break;
    }
  }
};



Snake snake;

void setup() {
  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
#if defined (__AVR_ATtiny85__)
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
  // End of trinket special code
  Serial.begin(9600);

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
  int sg = get_signal();
  //Serial.print("  Signal "); 
  Serial.print(sg);
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
      snake.set_dir(sg);
      snake.move();
      update_scene(); 
      delay(1000);  
      if (snake.rabbits >= MAX_RABBITS) {
        game_state = STATE_FINISH;
        result = true;               
      }
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
  snake.setup();
  rabbit.setup();
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

  colorWipe(strip_0.Color(255, 0, 0), 50); // Red
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
