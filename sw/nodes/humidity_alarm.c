#include "hal.h"
#include "../node.h"
#include "../config.h"
#include "../drivers/tone.h"

typedef struct
{
  uint8_t note;
  uint8_t duration;
} note;

note melody[] = {{88, 32}, {83, 16}, {84, 16}, {86, 32}, {84, 16}, {83, 16}, {81, 32}, {81, 16}, {84, 16}, {88, 32}, {86, 16}, {84, 16}, {83, 32}, {83, 16}, {84, 16}, {86, 32}, {88, 32}, {84, 32}, {81, 32}, {81, 32}, {81, 1}};

uint8_t song;

void app_init(void)
{
  tone_init();
}

void app_fast_tick(void)
{
  static uint8_t song_pos;
  static uint8_t note_pos;
  if(song)
  {
    if(note_pos == 0)
    {
      note cur = melody[song_pos];
      note_pos = cur.duration * 4;
      tone_output_note(cur.note, 255);

      if(++song_pos > (sizeof(melody) / sizeof(melody[0])))
      {
        song_pos = 0;
        note_pos = 0;
        song = 0;
        tone_output_note(60, 0);
      }
    } else {
      note_pos--;
    }
  }
}

void app_tick(void)
{
  static uint16_t count = 0;
  if(count == 1)
  {
    song = 1;
  }
  count++;
}
