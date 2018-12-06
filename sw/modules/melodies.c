/*
 * melodies.c
 *
 *  Created on: 01.12.2018
 *      Author: matthias
 */

#include "melodies.h"
#include "drivers/tone.h"

note melody_tetris[] = {{88, 32}, {83, 16}, {84, 16}, {86, 32}, {84, 16}, {83, 16}, {81, 32}, {81, 16}, {84, 16}, {88, 32}, {86, 16}, {84, 16}, {83, 32}, {83, 16}, {84, 16}, {86, 32}, {88, 32}, {84, 32}, {81, 32}, {81, 32}, {81, 1}, {0, 0}};

static uint8_t song_pos;
static uint8_t note_pos;
static melody active_melody;

void melody_play(melody m)
{

}

void tick(void) {
  if(active_melody)
  {
    if(note_pos == 0)
    {
      note cur = active_melody[song_pos];
      note_pos = cur.duration * 4;
      if(cur.note != 0) {
        tone_output_note(cur.note, 255);
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
