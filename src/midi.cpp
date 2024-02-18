#include "midi.hpp"
#include "stepper.hpp"
#include <algorithm>
#include <cmath>

midiFile songData;

uint8_t midiFile::readByte() {
  uint8_t byte = this->byteArray->front();
  this->byteArray->pop();
  return byte;
}

void midiFile::allocateTrackChunks(void) {
  this->trackChunkArray.resize(this->headerChunk.headerTrackNum);
  return;
}

uint32_t midiFile::readChunkData32(void) {
  uint32_t chunkData = 0;
  for (uint8_t i = 0; i < 4; i++) {
    chunkData |= (readByte() << (24 - (8 * i))); // shifting bits so that we can read one byte at a time and then store those bytes in our variable
  }
  return chunkData;
}

uint16_t midiFile::readChunkData16(void) {
  uint16_t chunkData = 0;
  for (uint8_t i = 0; i < 2; i++) {
    chunkData |= (readByte() << (8 - (8 * i))); // shifting bits so that we can read one byte at a time and then store those bytes in our variable
  }
  return chunkData;
}

uint8_t midiFile::readVariableLength(uint32_t& varLenQuantity) {
  uint8_t bytesRead = 0;
  while (this->byteArray->front() >= 0x80) {
    varLenQuantity = (varLenQuantity << 7) | (readByte() & 0x7F);
    bytesRead++;
  }
  varLenQuantity = (varLenQuantity << 7) | (readByte() & 0x7F);
  bytesRead++;
  return bytesRead;
}

uint8_t midiFile::readMidiEvent(uint8_t& prevEvent, uint8_t& eventType) {
  if (this->byteArray->front() >= 0x80) {
    eventType = prevEvent = readByte();
    return 1;
  }
  else {
    eventType = prevEvent;
  }
  return 0;
}

uint8_t midiFile::readMidiEventData(const uint8_t eventType, uint16_t& eventData) {
  uint32_t eventLen = 0;
  uint8_t bytesRead = 0;
  switch (eventType & 0xF0) {
  case (0x80):
  case (0x90):
  case (0xA0):
  case (0xB0):
  case (0xE0):
    eventLen = 2;
    break;
  case (0xC0):
  case (0xD0):
    eventLen = 1;
    break;
  case (0xF0):
    bytesRead += readVariableLength(eventLen);
    break;
  }
  for (uint32_t i = 0; i < eventLen; i++) {
    if (((eventType & 0xF0) == MIDI_NOTE_ON) || ((eventType & 0xF0) == MIDI_NOTE_OFF)) {
      eventData = (eventData << 8) | readByte();
    }
    else {
      readByte();
    }
    bytesRead++;
  }
  return bytesRead;
}

uint8_t midiFile::readMidiEventData(const uint8_t eventType, uint32_t& tempoData) {
  uint32_t eventLen = 0;
  uint8_t bytesRead = 0;
  if (byteArray->front() == MIDI_META_TEMPO) {
    tempoData = readByte();
  }
  else if (byteArray->front() == MIDI_META_EOT) {
    tempoData = readByte();
    tempoData = tempoData << 24;
  }
  else {
    readByte();
  }
  bytesRead += readVariableLength(eventLen) + 1;
  for (uint32_t i = 0; i < eventLen; i++) {
    if ((tempoData >> (i * 8)) == MIDI_META_TEMPO) {
      tempoData = (tempoData << 8) | readByte();
    }
    else {
      readByte();
    }
    bytesRead++;
  }
  return bytesRead;
}

bool midiFile::populateHeaderChunk(void) {
  if (readChunkData32() != this->headerChunk.headerType || readChunkData32() != this->headerChunk.headerLen) { // the purpose of this conditional is to ensure our midi file contains data that is expected. every midi file should start with a consistent header chunk type and length. if our header does not, our midi file is likely invalid
    return false;
  }
  this->headerChunk.headerFormat = readChunkData16();   // after ensuring our header chunk contains data that is expected, check the format of the midi file
  this->headerChunk.headerTrackNum = readChunkData16(); // once we've checked the midi format, find the number of tracks in the midi file
  this->headerChunk.headerDiv = readChunkData16();      // finally, read in the division information to know what to expect of our midi's delta times
  return true;
}

bool midiFile::populateTrackChunks(const uint32_t index, std::deque<deltaAndEvent>& trackData) {
  uint8_t prevEvent = 0;
  if (readChunkData32() != CHUNKTYPE_TRACK) { // check to ensure the track type matches the expected value. if not, exit early
    return false;
  }
  this->trackChunkArray[index] = readChunkData32();                // read in the length of the track. this will be used to iterate over the remaining bytes without reading any of the bytes belonging to the next track, if there is one
  for (uint32_t i = 0; i < this->trackChunkArray[index];) { // here we will iterate over our bytes which make up our midi events and delta times
    midiFile::deltaAndEvent tempEvent;
    i += readVariableLength(tempEvent.deltaTime);
    if (!trackData.empty()) {
      tempEvent.deltaTime += trackData.back().deltaTime;
    }
    i += readMidiEvent(prevEvent, tempEvent.event);
    if ((tempEvent.event & 0xFF) != 0xFF) {
      i += readMidiEventData(tempEvent.event, tempEvent.eventData);
    }
    else {
      i += readMidiEventData(tempEvent.event, tempEvent.tempoData);
    }
    if ((tempEvent.event & 0xF0) == MIDI_NOTE_ON && (tempEvent.eventData & 0x00FF) == 0) {
      tempEvent.event = MIDI_NOTE_OFF | (tempEvent.event & 0x0F);
    }
    if (((tempEvent.event & 0xF0) == MIDI_NOTE_OFF) || ((tempEvent.event & 0xF0) == MIDI_NOTE_ON) || ((tempEvent.event == MIDI_META_EVENT) && (((tempEvent.tempoData >> 24) == MIDI_META_TEMPO) || ((tempEvent.tempoData >> 24) == MIDI_META_EOT)))) {
      trackData.push_back(tempEvent);
    }
  }
  return true;
}

void midiFile::sortMidiEvents(std::deque<deltaAndEvent>& trackData) {
  std::sort(trackData.begin(), trackData.end(), [](const deltaAndEvent& a, const deltaAndEvent& b) {
    return ((a.deltaTime != b.deltaTime) ? (a.deltaTime < b.deltaTime) : ((a.event == b.event) || (a.event == MIDI_META_EVENT)));
  });
  return;
}

void midiFile::convertDeltaTime(std::deque<deltaAndEvent>& trackData) {
  uint32_t workingTempo = 500000;
  for (uint32_t i = trackData.size(); i > 0; i--) {
    if (trackData[i].deltaTime != 0) {
      trackData[i].deltaTime = trackData[i].deltaTime - trackData[i - 1].deltaTime;
    }
    else {
      break;
    }
  }
  for (uint32_t i = 0; i < trackData.size(); i++) {
    if (trackData[i].event == MIDI_META_EVENT) {
      workingTempo = (trackData[i].tempoData & 0x00FFFFFF);
    }
    trackData[i].deltaTime = (workingTempo * trackData[i].deltaTime) / this->headerChunk.headerDiv;
  }
  return;
}

void midiFile::enqueueEvents(std::deque<deltaAndEvent>& trackData) {
  this->eventQueue = new std::queue<std::tuple<uint32_t, uint8_t, uint8_t>>;
  for (uint32_t i = 0; i < trackData.size(); i++) {
    if (((trackData.front().event & 0xF0) == MIDI_NOTE_ON) || (trackData.front().event & 0xF0) == MIDI_NOTE_OFF) {
      this->eventQueue->emplace(trackData.front().deltaTime, (uint8_t)(trackData.front().eventData >> 8), trackData.front().event);
      trackData.pop_front();
    }
    else {
      trackData.pop_front();
    }
  }
  return;
}

void midiFile::writeByte(const uint8_t byte) {
  if (this->byteArray == NULL) {
    this->byteArray = new std::queue<uint8_t>;
  }
  this->byteArray->push(byte);
  return;
}

void midiFile::parseMidi(void) {
  if (this->byteArray->empty() || !populateHeaderChunk()) {
    delete this->byteArray;
    return;
  }
  this->trackChunkArray.clear();
  allocateTrackChunks();
  std::deque<deltaAndEvent> trackData;
  for (uint32_t i = 0; i < this->headerChunk.headerTrackNum; i++) {
    populateTrackChunks(i, trackData);
  }
  sortMidiEvents(trackData);
  convertDeltaTime(trackData);
  enqueueEvents(trackData);
  delete this->byteArray;
  this->byteArray = NULL;
  return;
}

void midiFile::playMidi(void) {
  for (uint32_t i = 0, prevTime = 0xFF; i < this->eventQueue->size(); i++) {
    uint8_t note = midi::getFreq(std::get<1>(this->eventQueue->front()));
    uint8_t event = std::get<2>(this->eventQueue->front());
    prevTime = std::get<0>(this->eventQueue->front());
    this->eventQueue->pop();
    if (prevTime != std::get<0>(this->eventQueue->front())) {
      playNote(std::get<0>(this->eventQueue->front()), note, event);
    }
  }
  delete this->eventQueue;
  this->eventQueue = NULL;
  return;
}

void midiFile::dumpContents(std::deque<deltaAndEvent>& trackData) {
  Serial.println();
  Serial.print("Midi format: ");
  Serial.println(this->headerChunk.headerFormat);
  Serial.print("Track Chunks: ");
  Serial.println(this->headerChunk.headerTrackNum);
  Serial.print("Div format: ");
  Serial.print(this->headerChunk.headerDiv, HEX);

  for (uint32_t i = 0; i < this->trackChunkArray.size(); i++) {
    Serial.print("\nTrack chunk ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.println();
    Serial.print("-  Track length: ");
    Serial.println(this->trackChunkArray[i]);
    Serial.print("-  Track contains: ");
    while (!trackData.empty()) {
      Serial.print("\n---  Delta time: ");
      Serial.print(trackData.front().deltaTime, HEX);
      Serial.print(" | Midi event: ");
      Serial.print(((trackData.front().event & 0xFF) == 0xFF) ? "Meta Event" : ((trackData.front().event & 0xF0) == 0x80) ? "Note Off Event"
                                                                                                                          : "Note On Event");
      Serial.print(" | Channel: ");
      Serial.print(trackData.front().event & 0x0F);
      if (trackData.front().event < 0xA0) {
        Serial.print(" | Midi event data:");
        for (int j = 0; j < 2; j++) {
          Serial.print(" ");
          Serial.print(((trackData.front().eventData >> (8 - (j * 8))) & 0x00FF), HEX);
        }
      }
      else if ((trackData.front().event == 0xFF) && ((trackData.front().tempoData >> 24) == MIDI_META_TEMPO)) {
        Serial.print(" | Midi tempo data: ");
        Serial.print(((trackData.front().tempoData << 8) >> 8), HEX);
      }
      else if ((trackData.front().event == 0xFF) && ((trackData.front().tempoData >> 24) == MIDI_META_EOT)) {
        Serial.print(" | End of track");
        trackData.erase(trackData.begin());
        break;
      }
      trackData.erase(trackData.begin());
    }
  }
  return;
}

uint32_t midi::getFreq(const uint8_t note) {
  return (noteFreq[note % noteFreq.size()] * pow(2, ((note / noteFreq.size()) <= MIDI_OCTAVE_MAX ? (note / noteFreq.size()) : MIDI_OCTAVE_MAX))) / 1000;
}