#ifndef MIDI_HPP
#define MIDI_HPP
#include "globals.hpp"
#include <array>
#include <memory>
#include <queue>
#include <vector>

#define MIDI_META_EVENT 0xFF // this HEX value represents meta events as used by midi files
#define MIDI_NOTE_ON 0x90    // this HEX value represents note on events as used by midi files. the lower nibble of this value varies and denotes its channel
#define MIDI_NOTE_OFF 0x80   // this HEX value represents note off events as used by midi files the lower nibble of this value varies and denotes its channel
#define MIDI_META_TEMPO 0x51 // this HEX value accompanies meta events within midi files and is used to denote a tempo change
#define MIDI_META_EOT 0x2F   // this HEX values accompanies meta events within midi files and is used to denote the end of a track
#define MIDI_OCTAVE_MAX 5    // this arbitrary value is used to cap our upper octave. this is used to ensure we don't send a note to our stepper motor whose frequency is too great for us to play

// for reference, the frequencies of notes at given octaves follow:
// octave -1: c = 8.175, c# = 8.660,  d = 9.175, d# = 9.725, e = 10.300,  f = 10.915, f# = 11.560, g = 12.250, g# = 12.980, a = 13.750, a# = 14.570, b = 15.435
// octave 0:  c = 16.35, c# = 17.32,  d = 18.35, d# = 19.45, e = 20.60,   f = 21.83,  f# = 23.120, g = 24.500, g# = 25.960, a = 27.500, a# = 29.140, b = 30.870
// octave 1:  c = 32,    c# = 34,     d = 36,    d# = 38,    e = 41,      f = 43,     f# = 46,     g = 49,     g# = 52,     a = 55,     a# = 58,     b = 61
// octave 2:  c = 65,    c# = 69,     d = 73,    d# = 77,    e = 82,      f = 87,     f# = 92,     g = 98,     g# = 104,    a = 110,    a# = 116,    b = 123
// octave 3:  c = 130,   c# = 138,    d = 146,   d# = 155,   e = 164,     f = 174,    f# = 185,    g = 196,    g# = 208,    a = 220,    a# = 233,    b = 246
// octave 4:  c = 261,   c# = 277,    d = 293,   d# = 311,   e = 329,     f = 349,    f# = 369,    g = 392,    g# = 415,    a = 440,    a# = 466,    b = 493
// octave 5:  c = 523,   c# = 554,    d = 587,   d# = 622,   e = 659,     f = 698,    f# = 739,    g = 784,    g# = 830,    a = 880,    a# = 932,    b = 987
// using our getFreq function, we can generate a frequency within <1hz of expected for each note in each octave

// midi uses a single 8 bit integer to reference various notes across a range of octaves
//             C  |  C#  |  D   |  D#  |  E   |  F   |  F#  |  G   |  G#  |  A   |  A#  |  B
// octave -1  0   |  1   |  2   |  3   |  4   |  5   |  6   |  7   |  8   |  9   |  10  |  11
// octave 0   12  |  13  |  14  |  15  |  16  |  17  |  18  |  19  |  20  |  21  |  22  |  23
// octave 1   24  |  25  |  26  |  27  |  28  |  29  |  30  |  31  |  32  |  33  |  34  |  35
// octave 2   36  |  37  |  38  |  39  |  40  |  41  |  42  |  43  |  44  |  45  |  46  |  47
// octave 3   48  |  49  |  50  |  51  |  52  |  53  |  54  |  55  |  56  |  57  |  58  |  59
// octave 4   60  |  61  |  62  |  63  |  64  |  65  |  66  |  67  |  68  |  69  |  70  |  71
// octave 5   72  |  73  |  74  |  75  |  76  |  77  |  78  |  79  |  80  |  81  |  82  |  83
// octave 6   84  |  85  |  86  |  87  |  88  |  89  |  90  |  91  |  92  |  93  |  94  |  95
// octave 7   96  |  97  |  98  |  99  |  100 |  101 |  102 |  103 |  104 |  105 |  106 |  107
// octave 8   108 |  109 |  110 |  111 |  112 |  113 |  114 |  115 |  116 |  117 |  118 |  119
// octave 9   120 |  121 |  122 |  123 |  124 |  125 |  126 |  127 |

// for reference, midi meta events will always be identified by the 0xFF status, followed by a type byte and then length byte (as a variable length quantity). length indicates the number of data bytes that come next
// escape sequences are preceded by 0xF7, followed by a length byte (as a variable length quantity) and then a series of data bytes
// system exclusive messages begin with an 0xF0, then a length byte (as a variable length quantity), then the message bytes and always ends with an 0xF7. the length indicates the number of message bytes plus the closing 0xF7

// this struct will be used to store the contents of our MIDI file once loaded. much of the information this is based off of is thanks to the documentation at https://www.music.mcgill.ca/~ich/classes/mumt306/StandardMIDIfileformat.html
// credit goes to David Back of mcgill.ca for the descriptions of some of the member variables below
class midiFile {
  struct {
    const uint32_t headerType = CHUNKTYPE_HEADER; // this value represents the type of chunk. for a header chunk, this value should always equate to the ASCII value of "MThd"
    const uint32_t headerLen = 6;                 // this value indicates how long in bytes the body of the header chunk is. this value should always evaluate to 6
    uint16_t headerFormat = 0;                    // if this evaluates to 0, MIDI contains a single multi-channel track. 1 indicates one or more simultaneous ttracks or MIDI outputs of a sequence. 2 indicates one or more sequentially independent single-track patterns
    uint16_t headerTrackNum = 0;                  // number of tracks in the MIDI file. will always be 1 for a format 0 file
    uint16_t headerDiv = 0;                       // specifies meaning of delta-times. has two formats, one for metrical time and one for time-code-based time
  } headerChunk;

  struct deltaAndEvent {    // we make use of a struct here to contain our midi event data for our track
    uint32_t deltaTime = 0; // this is the delta time value read in from our midi file as a variable length quality, converted to a standard 32 bit integer
    uint8_t event = 0;      // this element tracks the type of event that is being called, whether it is a midi event, meta event, or sysex event
    uint16_t eventData = 0; // this is the event that occurs at the given delta time, such as a note on event with its associated note
    uint32_t tempoData = 0; // this variable is only used for meta tempo events. the upper byte of this variable contains our meta event identifier, 0x51 in the case of a tempo event, and the lower three bytes comprise the 24 bit value that is our tempo
  };

  std::vector<uint32_t> trackChunkArray = {};                            // this vector will keep track of the various track chunks and their data in our midi file
  uint32_t tempo = 0;                                                    // our current tempo is stored here
  std::queue<uint8_t>* byteArray = NULL;                                 // the purpose of this queue is to hold the contents of our midi file for parsing
  std::queue<std::tuple<uint32_t, uint8_t, uint8_t>>* eventQueue = NULL; // this queue will contain our parsed midi events, ready for playback. these events will exclusively consist of note on events; note off events will be represented with a velocity of zero

  uint8_t readByte(void);                                                               // this function returns the first byte in our byteArray, consuming it in the process
  void allocateTrackChunks(void);                                                       // this function is called while reading our midi file and is used to allocate an appropriate number of elements in our trackChunkArray corresponding to the number of track chunks in our midi file
  uint32_t readChunkData32(void);                                                       // this function reads the next four bytes in our file, then stores and returns the 4 byte value
  uint16_t readChunkData16(void);                                                       // this function reads the next two bytes in our file, then stores and returns the 2 byte value
  uint8_t readVariableLength(uint32_t& varLenQuantity);                                 // this function is used to parse variable length quantities, converting them to a standard unsigned 32 bit integer
  uint8_t readMidiEvent(uint8_t& prevEvent, uint8_t& eventType);                        // with this function we check the type of midi event that we're reading
  uint8_t readMidiEventData(const uint8_t eventType, uint16_t& eventData);              // this function will read in byte by byte our midi event data and prepare them for parsing
  uint8_t readMidiEventData(const uint8_t eventType, uint32_t& tempoData);              // this function will read in byte by byte our midi event data and prepare them for parsing
  bool populateHeaderChunk(void);                                                       // this function checks our header metadata and reads our midi file's format, track numbers, and delta time format
  bool populateTrackChunks(const uint32_t index, std::deque<deltaAndEvent>& trackData); // this function takes as a parameter an index to our track chunk vector. it will then read in from our midi file until the current track chunk is fully parsed, then exit. returns false if an error occurs
  void sortMidiEvents(std::deque<deltaAndEvent>& trackData);                            // we'll be making use of this function to sort our event data by deltaTime. in the event of two elements having the same deltaTime (i.e., they occur at the same time), any events that change tempo will take priority and be placed first in our queue
  void convertDeltaTime(std::deque<deltaAndEvent>& trackData);                          // we'll use this function to convert our delta time to a value usable by our stepper motor library
  void enqueueEvents(std::deque<deltaAndEvent>& trackData);                             // this function will be used to push all of our midi events into a queue where they will be ready for playback

  public:
  void writeByte(const uint8_t byte);                      // setter function for our byteArray. takes in a byte and pushes it onto the queue
  void parseMidi(void);                                    // wrapper function that calls various functions to parse the header chunk of our midi file. following this, our function parses through the track chunks in our midi file byte by byte. returns false if we encounter an error
  void playMidi(void);                                     // finally, this function will call our various stepper motor function to actually play our music
  void dumpContents(std::deque<deltaAndEvent>& trackData); // this function is used to print our midi file data to our serial monitor and/or a file
};

namespace midi {
  const std::array<uint16_t, 12> noteFreq = { 8175, 8660, 9175, 9725, 10300, 10915, 11560, 12250, 12980, 13750, 14570, 15435 }; // this array is used to define the frequencies of note within octave -1

  uint32_t getFreq(const uint8_t note); // takes a note frequency and octave and returns the new frequency of the note at the given octave in Hz. returns 0 if note or octave are invalid
};

extern midiFile songData; // our object here will be used to store the contents of our midi file once it is selected by the user

#endif