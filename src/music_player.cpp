#include <Arduino.h>
#include "musics.h"
#include "music_player.h"

volatile int currentMusic = 0;
volatile bool playing = false;

void start_playing(int musicIdx) {
  if (playing) stop_playing();
  currentMusic = musicIdx;
  playing = true;
}

void stop_playing() {
  playing = false;
  reset_music();
};

void update_music(int buzzer) {
  if (playing)
    play_note(buzzer, currentMusic);
  else
    delay(100);
}