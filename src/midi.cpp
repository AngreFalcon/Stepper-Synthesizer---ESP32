#include "midi.hpp"

uint16_t midi::getFreq(std::string note, int8_t octave) {
  if (noteFreq.find(note) == noteFreq.cend())
    return 0;
  else
    return (noteFreq.at(note) * pow(2, octave)) / 100;
}