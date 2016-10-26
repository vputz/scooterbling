#include <SPI.h>

#include <Adafruit_DotStar.h>

const int Num_pixels = 23;
const int Data_pin = 4;
const int Clock_pin = 5;

typedef uint32_t color_t;

int head=0, tail=-5;
color_t color = 0xFF0000;

class Effect {
 public:
  virtual color_t color( int pixel, long t ) = 0;
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

class Solid : public Effect {
 public:
  color_t my_color;
 Solid( Adafruit_DotStar new_strip, color_t new_color ):
  Effect(new_strip),
  my_color(new_color)
  {};

  virtual color_t color( int pixel, long t ) {
    return my_color;
  };
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
  virtual color_t base_color( long t ) {
    return 0xFF0000 >> ((t/color_change_t) % 3)*8;
  };
  int head(long t) {
    return (t % (20 * Num_pixels)) / Num_pixels;
  };
  virtual color_t color( int pixel, long t ) {
    int the_head = head(t);
    int tail = the_head - length;
    if ((the_head >= pixel) && (pixel >= tail)) {
      return base_color(t);
    } else {
      return 0;
    }
  };
};

color_t random_color() {
  color_t c1 = random(255);
  color_t c2 = random(255);
  color_t c3 = random(255);
  return ( (c3 << 16) | (c2 << 8) | (c1) );
}

class Pummer : public Effect {
public:
  int fade_duration;
  int loop;
  color_t my_color;
  Pummer( Adafruit_DotStar new_strip, int new_fade_duration):
    Effect(new_strip),
      fade_duration(new_fade_duration),
      loop(0),
      my_color(random_color())
  {};
    
    virtual color_t base_color(int pixel, long t) {
    // alternating colors)
    //return 0xFF0000 >> ((t / fade_duration)%3)*8;
    int this_loop = t/fade_duration;
    if (this_loop != loop) {
      my_color = random_color();
      loop = this_loop;
    }
    return my_color;
  };
  virtual color_t color(int pixel, long t) {
    color_t c = base_color(pixel, t);
    //now fade each component
    int in_fade = fade_duration - (t % fade_duration)^2;
    color_t c1 = ((long)(c & 0xff) * in_fade) / fade_duration;
    color_t c2 = (((long)(c >> 8) & 0xff) * in_fade) / fade_duration;
    color_t c3 = (((long)(c >> 16) & 0xff) * in_fade) / fade_duration;
    return ( (c3 << 16) | (c2 << 8) | (c1) );
  };
};

class AltPummer : public Pummer {
 public:
  color_t my_other_color;
 AltPummer( Adafruit_DotStar new_strip, int new_fade_duration):
  Pummer(new_strip, new_fade_duration),
    my_other_color(random_color())
    {};
  virtual color_t base_color(int pixel, long t) {
    int this_loop = t/fade_duration;
    if (this_loop != loop) {
      my_color = random_color();
      my_other_color = random_color();
      loop = this_loop;
    }
    switch (pixel % 2) {
    case 0 :
      return my_color;
      break;
    case 1 :
      return my_other_color;
      break;
    default:
      return 0;
    }
  };

};

class Turn_signal : public Effect {
 public:
  boolean is_left;
  long delay;
  int width;
 Turn_signal( Adafruit_DotStar new_strip, boolean new_is_left, long new_delay=800, int new_width=3) :
  Effect(new_strip),
    is_left(new_is_left),
    delay(new_delay),
    width(new_width)
    {};
  boolean is_on( long t ) {
    return ((t/delay) % 2) == 1;
  }
  virtual color_t color( int pixel, long t ) {
    if (is_on( t )) {
      if (is_left && (0 <= pixel) && (pixel <= width)) {
	return 0x00FF00;
      }
      else if ((!is_left) && (pixel <= strip.numPixels()) && ((strip.numPixels()-(width+1)) <= pixel)) {
	return 0x00FF00;
      }
      else {
	return 0;
      }
    }
  }
};

class Reverse : public Effect {
 public:
  int width;
 Reverse(Adafruit_DotStar new_strip, int new_width=3) :
  Effect(new_strip),
    width(new_width)
  {};

  virtual color_t color( int pixel, long t) {
    if (((0 <= pixel) && (pixel <= width))
	|| ((pixel <= strip.numPixels()) && ((strip.numPixels()-(width+1))<= pixel))) {
      return 0xFFFFFF;
    }
    else {
      return 0;
    }
  }
};

class Headlights : public Effect {
 public:
  int width;
  int separation;
 Headlights(Adafruit_DotStar new_strip, int new_width=2, int new_separation=2):
  Effect(new_strip),
    width(new_width),
    separation(new_separation)
    {};
  boolean is_headlight(int pixel) {
    int rmax = (strip.numPixels() + separation)/2 + width;
    int rmin = (strip.numPixels() + separation)/2;
    int lmax = (strip.numPixels() - separation)/2;
    int lmin = (strip.numPixels() - separation)/2 - width;
    return (((rmin <= pixel) && (pixel <= rmax)) ||
	((lmin <= pixel) && (pixel <= lmax)));
  }
  virtual color_t color(int pixel, long t) {
    if (is_headlight(pixel)) {
      return 0xFFFFFF;
    } else {
      return 0;
    }
  }
};

Adafruit_DotStar strip = Adafruit_DotStar(Num_pixels, Data_pin, Clock_pin, DOTSTAR_BRG);

Solid off(strip, 0);
Solid underlight(strip, 0x333333);
Chaser chaser(strip, 5);
Pummer pummer(strip, 500);
AltPummer alt_pummer(strip, 500);
Turn_signal left_turn(strip, true);
Turn_signal right_turn(strip, false);
Reverse reverse(strip);
Headlights headlights(strip);

const int Pin_input_enable = 8;
const int Pin_input = 9;
const int Pin_s0 = 10;
const int Pin_s1 = 11;
const int Pin_s2 = 12;

byte read_mux()
{
  byte result = 0;
  for (int i = 0; i <= 7; ++i) {
    // select the input
    digitalWrite(Pin_s0, i & 1);
    digitalWrite(Pin_s1, (i >> 1) & 1);
    digitalWrite(Pin_s2, (i >> 2) & 1);

    // strobe low to read
    digitalWrite(Pin_input_enable, LOW);
    result |= (digitalRead( Pin_input ) << i);
    digitalWrite(Pin_input_enable, HIGH);
  }
  return result;
}

Effect* effects[] = {&off,
		    &underlight,
		    &left_turn,
		    &right_turn,
		    &reverse,
		    &headlights,
		    &pummer,
		    &alt_pummer,
		    &chaser};

int last_selection = 0;
Effect* chosen_effect = NULL;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  strip.begin();
  strip.show();

  last_selection = 0;
  chosen_effect = effects[last_selection];
  
  pinMode( Pin_input, INPUT );
  pinMode( Pin_input_enable, OUTPUT );
  pinMode( Pin_s0, OUTPUT );
  pinMode( Pin_s1, OUTPUT );
  pinMode( Pin_s2, OUTPUT );
}

int selection_from_mux(byte mux) {
  int result = 0;
  switch(mux) {
  case 0:
    result = 0;
    break;
  case 1:
    result = 1;
    break;
  case 2:
    result = 2;
    break;
  case 4:
    result = 3;
    break;
  case 8:
    result = 4;
    break;
  case 16:
    result = 5;
    break;
  case 32:
    result = 6;
    break;
  case 64:
    result = 7;
    break;
  case 128:
    result = 8;
    break;
  default:
    result = 0;
    break;
  }
  return result;
}

void loop() {
  chosen_effect = effects[selection_from_mux(read_mux())];
  chosen_effect->set();
  chosen_effect->show();
  //Serial.println("--");
  //Serial.println(chaser.head( millis() ));
  //Serial.println(chaser.base_color( millis() ));
  //Serial.println(selection_from_mux(read_mux()));
  //delay(200);
  //chaser.set();
  //chaser.show();
  //headlights.set();
  //headlights.show();
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

