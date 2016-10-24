#include <SPI.h>

#include <Adafruit_DotStar.h>

const int Num_pixels = 23;
const int Data_pin = 4;
const int Clock_pin = 5;

int head=0, tail=-5;
uint32_t color = 0xFF0000;

class Effect {
 public:
  virtual uint32_t color( int pixel, long t ) = 0;
  long start_time;
  Adafruit_DotStar strip;
 Effect( Adafruit_DotStar new_strip ):
  start_time(millis()),
    strip(new_strip)
  {
    strip.begin();
  };
  void set() {
    long t = millis() - start_time;
    for (int i = 0; i < Num_pixels; ++i) {
      strip.setPixelColor( i, color( i, t ) );
    }
  };
  void show() {
    strip.show();
  }
};

class Chaser : public Effect {
 public:
  int length;
  long color_change_t;
 
 Chaser(Adafruit_DotStar new_strip, int new_length) :
  length(new_length),
    Effect(new_strip),
    color_change_t( 20*Num_pixels )
      {};
  virtual uint32_t base_color( long t ) {
    return 0xFF0000 >> ((t/color_change_t) % 3);
  };
  int head(long t) {
    return (t % (20 * Num_pixels)) / Num_pixels;
  }
  virtual uint32_t color( int pixel, long t ) {
    int the_head = head(t);
    int tail = the_head - length;
    if ((the_head >= pixel) && (pixel >= tail)) {
      return base_color(t);
    } else {
      return 0;
    }
  };
};
  

Adafruit_DotStar strip = Adafruit_DotStar(Num_pixels, Data_pin, Clock_pin, DOTSTAR_BRG);

Chaser chaser(strip, 5);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  strip.begin();
  strip.show();
}


void loop() {
  //Serial.println("--");
  //Serial.println(chaser.head( millis() ));
  //Serial.println(chaser.base_color( millis() ));
  //delay(200);
  chaser.set();
  chaser.show();
  /* strip.setPixelColor(head, color); */
  /* strip.setPixelColor(tail, 0); */
  /* strip.show(); */
  /* delay(20); */

  /* if (++head >= Num_pixels) { */
  /*   head = 0; */
  /*   if ((color >>= 8) == 0) { */
  /*     color = 0xFF0000; */
  /*   } */
  /* } */

  /* if (++tail >= Num_pixels) { */
  /*   tail = 0; */
  /* } */

}

