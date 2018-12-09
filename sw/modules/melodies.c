/*
 * melodies.c
 *
 *  Created on: 01.12.2018
 *      Author: matthias
 */

#include "melodies.h"
#include "drivers/tone.h"

note melody_tetris[] = {{88, 32}, {83, 16}, {84, 16}, {86, 32}, {84, 16}, {83, 16}, {81, 32}, {81, 16}, {84, 16}, {88, 32}, {86, 16}, {84, 16}, {83, 32}, {83, 16}, {84, 16}, {86, 32}, {88, 32}, {84, 32}, {81, 32}, {81, 32}, {81, 1}, {0, 0}};
note melody_alarm1[] = {{74, 16}, {71, 16}, {67, 16}, {79, 16}, {83, 16}, {74, 16}, {0, 0}};
note melody_alarm2[] = {{84, 32}, {88, 32}, {84, 16}, {88, 16}, {84, 16}, {88, 16}, {84, 16}, {88, 16}, {84, 16}, {88, 16}, {84, 16}, {88, 16}, {0, 0}};
note melody_alarm3[] = {{67, 32}, {20, 8}, {67, 32}, {20, 8}, {67, 96}, {0, 0}};

static uint8_t song_pos;
static uint8_t note_pos;
static melody active_melody;
static uint8_t active_volume;

void melody_play(uint8_t melody, uint8_t volume)
{
  active_volume = volume;
  switch(melody) {
  case 0:
    active_melody = melody_tetris;
    break;
  case 1:
    active_melody = melody_alarm1;
    break;
  case 2:
      active_melody = melody_alarm2;
      break;
  case 3:
      active_melody = melody_alarm3;
      break;
  default:
      active_melody = melody_alarm1;
      break;
  }
}

void melody_tick(void) {
  if(active_melody)
  {
    if(note_pos == 0)
    {
      note cur = active_melody[song_pos++];
      note_pos = cur.duration * 4;
      if(cur.note != 0) {
        tone_output_note(cur.note, active_volume);
      } else {
        song_pos = 0;
        note_pos = 0;
        active_melody = 0;
        tone_output_note(60, 0);
      }
    } else {
      note_pos--;
    }
  }
}
