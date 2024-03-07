#include "midi.hpp"
#include "stepper.hpp"
#include <algorithm>
#include <cmath>
#include <sstream>

midiFile songData;

void midiFile::assignQueue(std::queue<uint8_t>* fileContents) {
  this->byteArray = fileContents;
  return;
}

bool midiFile::parseMidi(void) {
  std::deque<midiEvent> trackData;

  if (this->byteArray->empty() || !populateHeaderChunk()) {
    delete this->byteArray;
    return false;
  }

  for (uint32_t i = 0; i < this->headerChunk.headerTrackNum; i++) {
    if (!populateTrackChunks(i, trackData)) {
      return false;
    }
  }

  sortEvents(trackData);
  convertDeltaTime(trackData);
  enqueueEvents(trackData);
  delete this->byteArray;
  this->byteArray = NULL;
  if (this->eventQueue->front().deltaTime != 0) {
    this->eventQueue->front().deltaTime = 0;
  }
  return true;
}

bool midiFile::populateHeaderChunk(void) {
  // every midi file should start with a consistent
  // header chunk type and length. if our header does not
  // our midi file is likely invalid
  if (readChunkData32() != CHUNKTYPE_HEADER || readChunkData32() != HEADER_LEN) {
    return false;
  }

  // get the format of the midi file
  this->headerChunk.headerFormat = readChunkData16();

  // find the number of tracks in the midi file
  this->headerChunk.headerTrackNum = readChunkData16();

  // finally, read in the division information
  // to know what to expect of our midi's delta times
  this->headerChunk.headerDiv = readChunkData16();
  return true;
}

bool midiFile::populateTrackChunks(const uint32_t index, std::deque<midiEvent>& trackData) {
  uint8_t prevEvent = 0;
  uint32_t trackChunkLen = 0;

  // check to ensure the track type matches the expected value
  // if not, exit early
  if (readChunkData32() != CHUNKTYPE_TRACK) {
    return false;
  }

  // read in the length of the track
  // this will be used to iterate over the remaining bytes
  // without reading any of the bytes belonging to the next track
  // if there is one
  trackChunkLen = readChunkData32();

  // here we will iterate over our bytes
  // which make up our midi events and delta times
  // we know the expected length of the chunk
  // in bytes. we use i to keep track
  // of the number of bytes we've already read in
  // to ensure we read only what's necessary
  for (uint32_t i = 0; i < trackChunkLen;) {
    midiFile::midiEvent tempEvent;

    i += readVariableLen(tempEvent.deltaTime);

    // to allow use to merge all tracks into a single
    // queue of events later, we must convert
    // our deltaTimes from values relative to the 
    // previous event to absolute times from the 
    // start of the track
    if (!trackData.empty()) {
      tempEvent.deltaTime += trackData.back().deltaTime;
    }

    i += readMidiEvent(prevEvent, tempEvent.eventType);
    i += readMidiEventData(tempEvent.eventType, tempEvent.metaType, tempEvent.eventData);

    // to simplify playback, we check the velocity
    // of our note on events. if velocity is 0,
    // volume is 0 and our event is functionally
    // a note off event
    if (((tempEvent.eventType & 0xF0) == MIDI_NOTE_ON) && ((tempEvent.eventData & 0x00FF) == 0)) {
      tempEvent.eventType = (tempEvent.eventType & 0x8F);
    }

    // because our stepper motor does not have
    // volume control, velocity has no value for us
    // shift our event data 8 bits over
    // so that only our note data is stored
    if (((tempEvent.eventType & 0xF0) == MIDI_NOTE_ON) || ((tempEvent.eventType & 0xF0) == MIDI_NOTE_OFF)) {
      tempEvent.eventData = tempEvent.eventData >> 8;
    }

    // we only want to store note on, note off, and tempo events
    // no other events have any use to us for playback
    // via our stepper motors
    if (((tempEvent.eventType & 0xF0) == MIDI_NOTE_OFF) || ((tempEvent.eventType & 0xF0) == MIDI_NOTE_ON) || (tempEvent.metaType == MIDI_META_TEMPO)) {
      trackData.push_back(tempEvent);
    }
  }
  return true;
}

uint8_t midiFile::readMidiEvent(uint8_t& prevEvent, uint8_t& eventType) {
  if (this->byteArray->front() >= 0x80) {
    eventType = prevEvent = readByte();
    return 1;
  }
  else {
    eventType = prevEvent;
    return 0;
  }
}

uint8_t midiFile::readMidiEventData(const uint8_t eventType, uint8_t& metaType, uint32_t& eventData) {
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
    if (eventType == MIDI_META_EVENT) {
      metaType = readByte();
      bytesRead++;
    }
    bytesRead += readVariableLen(eventLen);
    break;
  }
  for (uint32_t i = 0; i < eventLen; i++, bytesRead++) {
    eventData = (eventData << 8) | readByte();
  }
  return bytesRead;
}

uint8_t midiFile::readVariableLen(uint32_t& varLenQuantity) {
  uint8_t bytesRead = 0;
  while (this->byteArray->front() >= 0x80) {
    varLenQuantity = (varLenQuantity << 7) | (readByte() & 0x7F);
    bytesRead++;
  }
  varLenQuantity = (varLenQuantity << 7) | (readByte() & 0x7F);
  bytesRead++;
  return bytesRead;
}

uint32_t midiFile::readChunkData32(void) {
  uint32_t chunkData = 0;

  // shifting bits so that we can read one byte at a time
  // and then store those bytes in our variable
  for (uint8_t i = 0; i < 4; i++) {
    chunkData |= (readByte() << (24 - (8 * i)));
  }
  return chunkData;
}

uint16_t midiFile::readChunkData16(void) {
  uint16_t chunkData = 0;

  // shifting bits so that we can read one byte at a time
  // and then store those bytes in our variable
  for (uint8_t i = 0; i < 2; i++) {
    chunkData |= (readByte() << (8 - (8 * i)));
  }
  return chunkData;
}

uint8_t midiFile::readByte() {
  if (!this->byteArray->empty()) {
    uint8_t byte = this->byteArray->front();
    this->byteArray->pop();
    return byte;
  }
  else {
    return 0;
  }
}

void midiFile::sortEvents(std::deque<midiEvent>& trackData) {
  std::sort(trackData.begin(), trackData.end(), [](const midiEvent& a, const midiEvent& b) {
    return ((a.deltaTime != b.deltaTime) ? (a.deltaTime < b.deltaTime) : ((a.eventType == b.eventType) || (a.eventType == MIDI_META_EVENT)));
  });
  return;
}

void midiFile::convertDeltaTime(std::deque<midiEvent>& trackData) {
  uint32_t workingTempo = 500000;

  // it's important that we first convert our deltaTimes from absolute value
  // since the start of the track back to relative values since the last event
  for (uint32_t i = trackData.size(); i > 0; i--) {
    if (trackData[i].deltaTime != 0) {
      trackData[i].deltaTime = trackData[i].deltaTime - trackData[i - 1].deltaTime;
    }
    else {
      break;
    }
  }

  // now we check the format of our header chunk's tickdivs value
  // this tells us if we're using metrical timing
  if (!(this->headerChunk.headerDiv >> 15)) {
    for (uint32_t i = 0; i < trackData.size(); i++) {
      if ((trackData[i].eventType == MIDI_META_EVENT) && (trackData[i].metaType == MIDI_META_TEMPO)) {
        workingTempo = trackData[i].eventData;
      }
      trackData[i].deltaTime = (workingTempo * trackData[i].deltaTime) / this->headerChunk.headerDiv;
    }
  }

  // or if we're using timecodes
  else {
    for (uint32_t i = 0; i < trackData.size(); i++) {
      trackData[i].deltaTime = pow((~((this->headerChunk.headerDiv >> 8) - 1) * (this->headerChunk.headerDiv & 0x00FF) * 1000), -1) * trackData[i].deltaTime;
    }
  }
  return;
}

void midiFile::enqueueEvents(std::deque<midiEvent>& trackData) {
  this->eventQueue = new std::queue<midiEvent>;
  while (!trackData.empty()) {
    if ((trackData.front().getEventOrChannel(true) == MIDI_NOTE_ON) || (trackData.front().getEventOrChannel(true) == MIDI_NOTE_OFF)) {
      trackData.front().eventData = midi::getFreq(trackData.front().eventData);
      this->eventQueue->push(trackData.front());
    }
    trackData.pop_front();
  }
  return;
}

void midiFile::playMidi(void) {
  std::stringstream debugString;
  uint16_t eventNum = 1;
  while (!this->eventQueue->empty()) {
    debugString << "\n" << eventNum;
    playNote(this->eventQueue->front().deltaTime, this->eventQueue->front().eventData, this->eventQueue->front().eventType, debugString);
    this->eventQueue->pop();
    eventNum++;
  }
  Serial.println(debugString.str().c_str());
  delete this->eventQueue;
  this->eventQueue = NULL;
  return;
}

void midiFile::printQueue(void) {
  uint32_t deltaTime = 0;
  Serial.print("Header Data: ");
  Serial.print("Format: ");
  Serial.print(this->headerChunk.headerFormat);
  Serial.print(" | Tracks: ");
  Serial.print(this->headerChunk.headerTrackNum);
  Serial.print(" | Tickdiv: ");
  Serial.println(this->headerChunk.headerDiv, HEX);
  Serial.print("Queue Size: ");
  Serial.println(this->eventQueue->size());
  for (uint32_t i = 0; i < this->eventQueue->size(); i++) {
    Serial.print(i + 1);
    Serial.print(" | Delta Time in uS: ");
    Serial.print(this->eventQueue->front().deltaTime);
    deltaTime += this->eventQueue->front().deltaTime;
    Serial.print(" | Note in mHz: ");
    Serial.print(this->eventQueue->front().eventData);
    Serial.print(" | Event: ");
    Serial.print((this->eventQueue->front().getEventOrChannel(true) == MIDI_NOTE_OFF) ? "Note Off" : "Note On");
    Serial.print(" | Channel: ");
    Serial.println(this->eventQueue->front().getEventOrChannel(false));
    this->eventQueue->push(this->eventQueue->front());
    this->eventQueue->pop();
  }
  Serial.print("\nDelta Time Total: ");
  Serial.println(deltaTime);
  return;
}

uint32_t midi::getFreq(const uint8_t note) {
  uint8_t index = ((note + 1) & 0x7F) % noteFreq.size();
  int8_t octave = (note / noteFreq.size());
  if (octave > MIDI_OCTAVE_MAX) {
    octave = MIDI_OCTAVE_MAX;
  }
  else if (octave < MIDI_OCTAVE_MIN) {
    octave = MIDI_OCTAVE_MIN;
  }
  return (noteFreq[index] * pow(2, octave));
}

uint8_t midiFile::midiEvent::getEventOrChannel(bool event) {
  if (event) {
    return (this->eventType & 0xF0);
  }
  else {
    return (this->eventType & 0x0F);
  }
}
