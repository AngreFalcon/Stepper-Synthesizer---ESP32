#ifndef MIDI_HPP
#define MIDI_HPP
#include "globals.hpp"
#include <cmath>
#include <unordered_map>

// for reference, the frequencies of notes at given octaves follow:
// octave 1: c = 32,  c# = 34,  d = 36,  d# = 38,  e = 41,  f = 43,  f# = 46,  g = 49,  g# = 52,  a = 55,  a# = 58,  b = 61
// octave 2: c = 65,  c# = 69,  d = 73,  d# = 77,  e = 82,  f = 87,  f# = 92,  g = 98,  g# = 104, a = 110, a# = 116, b = 123
// octave 3: c = 130, c# = 138, d = 146, d# = 155, e = 164, f = 174, f# = 185, g = 196, g# = 208, a = 220, a# = 233, b = 246
// octave 4: c = 261, c# = 277, d = 293, d# = 311, e = 329, f = 349, f# = 369, g = 392, g# = 415, a = 440, a# = 466, b = 493
// octave 5: c = 523, c# = 554, d = 587, d# = 622, e = 659, f = 698, f# = 739, g = 784, g# = 830, a = 880, a# = 932, b = 987
// using our getFreq function, we can generate a frequency within <1hz of expected for each note in each octave

namespace midi {
  const std::unordered_map<std::string, uint16_t> noteFreq = { { "C", 1635 }, { "C#", 1732 }, { "D", 1835 }, { "D#", 1945 }, { "E", 2060 }, { "F", 2183 }, { "F#", 2312 }, { "G", 2450 }, { "G#", 2596 }, { "A", 2750 }, { "A#", 2914 }, { "B", 3087 } };

  uint16_t getFreq(std::string note, int8_t octave); // takes a note frequency and octave and returns the new frequency of the note at the given octave. returns 0 if note or octave are invalid
};
 
#endif