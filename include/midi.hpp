#ifndef MIDI_HPP
#define MIDI_HPP
#include "globals.hpp"
#include <array>
#include <memory>
#include <queue>
#include <vector>

// this macro is used to keep track of the midi chunk type used by headers
// and is the hexadecimal representation of MThd
#define CHUNKTYPE_HEADER 0x4d546864

// this is the expected length of midi header files
// and is used to read in the correct number of bytes
#define HEADER_LEN 6

// this macro is used to keep track of the midi chunk type used by tracks,
// and is the hexadecimal representation of MTrk
#define CHUNKTYPE_TRACK 0x4d54726b

// this HEX value represents meta events as used by midi files
#define MIDI_META_EVENT 0xFF

// this HEX value represents note on events as used by midi files
// the lower nibble of this value varies and denotes its channel
#define MIDI_NOTE_ON 0x90

// this HEX value represents note off events as used by midi files
// the lower nibble of this value varies and denotes its channel
#define MIDI_NOTE_OFF 0x80

// this HEX values accompanies meta events within midi files
// and is used to denote the end of a track
#define MIDI_META_EOT 0x2F

// this HEX value accompanies meta events within midi files
// and is used to denote a tempo change
#define MIDI_META_TEMPO 0x51

// this arbitrary value is used to cap our upper octave
// this is used to ensure we don't send a note to our stepper motor
// whose frequency is too great for us to play
#define MIDI_OCTAVE_MAX 6

#define MIDI_OCTAVE_MIN -2

// one of four SMPTE standard values denoting 24 frames per second
#define SMPTE_24 0xE8

// one of four SMPTE standard values denoting 25 frames per second
#define SMPTE_25 0xE7

// one of four SMPTE standard values denoting 29 frames per second
#define SMPTE_29 0xE3

// one of four SMPTE standard values denoting 30 frames per second
#define SMPTE_30 0xE2

// for reference, the frequencies of notes at given octaves follow:
// octave -1: c = 8.175, c# = 8.660,  d = 9.175, d# = 9.725, e = 10.300,  f = 10.915, f# = 11.560, g = 12.250, g# = 12.980, a = 13.750, a# = 14.570, b = 15.435
// octave 0:  c = 16.35, c# = 17.32,  d = 18.35, d# = 19.45, e = 20.60,   f = 21.83,  f# = 23.120, g = 24.500, g# = 25.960, a = 27.500, a# = 29.140, b = 30.870
// octave 1:  c = 32,    c# = 34,     d = 36,    d# = 38,    e = 41,      f = 43,     f# = 46,     g = 49,     g# = 52,     a = 55,     a# = 58,     b = 61
// octave 2:  c = 65,    c# = 69,     d = 73,    d# = 77,    e = 82,      f = 87,     f# = 92,     g = 98,     g# = 104,    a = 110,    a# = 116,    b = 123
// octave 3:  c = 130,   c# = 138,    d = 146,   d# = 155,   e = 164,     f = 174,    f# = 185,    g = 196,    g# = 208,    a = 220,    a# = 233,    b = 246
// octave 4:  c = 261,   c# = 277,    d = 293,   d# = 311,   e = 329,     f = 349,    f# = 369,    g = 392,    g# = 415,    a = 440,    a# = 466,    b = 493
// octave 5:  c = 523,   c# = 554,    d = 587,   d# = 622,   e = 659,     f = 698,    f# = 739,    g = 784,    g# = 830,    a = 880,    a# = 932,    b = 987
// octave 6:  c = 1046,  c# = 1108,   d = 1174,  d# = 1244,  e = 1318,    f = 1396,   f# = 1479,   g = 1567,   g# = 1161,   a = 1760,   a# = 1864,   b = 1975
// octave 7:  c = 2093,  c# = 2217,   d = 2349,  d# = 2489,  e = 2637,    f = 2793,   f# = 2959,   g = 3135,   g# = 3324,   a = 3520,   a# = 3729,   b = 3951
// octave 8:  c = 4186,  c# = 4434,   d = 4698,  d# = 4978,  e = 5274,    f = 5587,   f# = 5919,   g = 6271,   g# = 6644,   a = 7040,   a# = 7458,   b = 7902
// using our getFreq function, we can generate a frequency within <1hz of expected for each note in each octave

// midi uses a single 7 bit integer to reference various notes across a range of octaves
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

// for reference, midi meta events will always be identified by the 0xFF status,
// followed by a type byte and then length byte (as a variable length quantity)
// length indicates the number of data bytes that come next
// escape sequences are preceded by 0xF7
// followed by a length byte (as a variable length quantity)
// and then a series of data bytes
// system exclusive messages begin with an 0xF0
// then a length byte (as a variable length quantity),
// then the message bytes and always ends with an 0xF7
// the length indicates the number of message bytes plus the closing 0xF7

// this struct will be used to store the contents of our MIDI file once loaded
// much of the information this is based off of is thanks to the documentation
// at https://www.music.mcgill.ca/~ich/classes/mumt306/StandardMIDIfileformat.html
// credit goes to David Back of mcgill.ca for the
// descriptions of some of the member variables below
class midiFile {
  struct {
    // if this evaluates to 0, MIDI contains a single multi-channel track
    // 1 indicates one or more simultaneous tracks or MIDI outputs of a sequence
    // 2 indicates one or more sequentially independent single-track patterns
    uint16_t headerFormat = 0;

    // number of tracks in the MIDI file. will always be 1 for a format 0 file
    uint16_t headerTrackNum = 0;

    // specifies meaning of delta-times. has two formats
    // one for metrical time and one for time-code-based time
    uint16_t headerDiv = 0;
  } headerChunk;

  struct midiEvent {
    // this is the delta time value read in from our midi file as
    // a variable length quality, converted to a standard 32 bit integer
    uint32_t deltaTime = 0;

    // this element tracks the type of event that is being called
    // whether it is a midi event, meta event, or sysex event
    // in the case of a note on event, our upper nibble will be a 9
    // while our lower nibble is the channel the event occurs on
    // sysex or meta events always have an upper nibble of F
    uint8_t eventType = 0;

    // this variable is only made use of for meta events and is used to
    // differentiate between our various meta events due to the fact
    // that their eventType will always be 0xFF
    uint8_t metaType = 0;

    // here we store the data of our event, whether it be a note on event
    // or note off event or meta event
    // as this is a 32 bit integer, we can only store up to 4 bytes of data
    // which for our purposes is acceptable
    uint32_t eventData = 0;

    uint8_t track = 0;

    // ## true to isolate our event type, false to isolate our channel
    // this is a simple function that performs common bit manipulation
    // on our 8 bit integers to separate values out of a single byte
    uint8_t getEventOrChannel(bool event);
  };

  /* member variable definitions below */

  // the purpose of this queue is to hold the contents
  // of our midi file for parsing
  std::queue<uint8_t>* byteArray = NULL;

  // this queue will contain our parsed midi events, ready for playback
  // these events will exclusively consist of note on events;
  // note off events will be represented with a velocity of zero
  std::queue<midiEvent>* eventQueue = NULL;

  /* member function prototypes below */

  // this function checks our header metadata and reads our
  // midi file's format, track numbers, and delta time format
  bool populateHeaderChunk(void);

  // this function takes as a parameter an index to our track chunk vector
  // it will then read in from our midi file until the current track chunk
  // is fully parsed, then exit. returns false if an error occurs
  bool populateTrackChunks(const uint8_t trackNum, std::deque<midiEvent>& trackData);

  // with this function we check the type of midi event that we're reading
  uint8_t readMidiEvent(uint8_t& prevEvent, uint8_t& eventType);

  // this function will read in byte by byte our midi event data
  // and prepare them for parsing
  uint8_t readMidiEventData(const uint8_t eventType, uint8_t& metaType, uint32_t& eventData);

  // this function is used to parse variable length quantities
  // converting them to a standard unsigned 32 bit integer
  // this function will read in 4 bytes at most
  // and consumes them in the process
  uint8_t readVariableLen(uint32_t& varLenQuantity);

  // this function reads the next four bytes in our file
  // consuming them as a result
  // then stores and returns the 4 byte value
  uint32_t readChunkData32(void);

  // this function reads the next two bytes in our file
  // consuming them as a result
  // then stores and returns the 2 byte value
  uint16_t readChunkData16(void);

  // this function returns the first byte in our byteArray
  // consuming it in the process
  uint8_t readByte(void);

  // we'll be making use of this function to sort our event data by deltaTime
  // in the event of two elements having the same deltaTime
  // (i.e., they occur at the same time)
  // any events that change tempo will take priority
  // and be placed first in our queue
  void sortEvents(std::deque<midiEvent>& trackData);

  // we'll use this function to convert our delta time to
  // a value usable by our stepper motor library
  void convertDeltaTime(std::deque<midiEvent>& trackData);

  // this function will be used to push all of our midi events
  // into a queue where they will be ready for playback
  void enqueueEvents(std::deque<midiEvent>& trackData);

  void analyzeOverlaps(const std::deque<midiEvent>& trackData);

  public:
  std::vector<bool> trackPolyphony;

  // a pointer to an array of unsigned 8 bit integers
  // containing the entirety of our midi file is passed in to this function
  // which attached that array to our byteArray
  void assignQueue(std::queue<uint8_t>* fileContents);

  // wrapper function that calls various functions to parse
  // the header chunk of our midi file
  // following this, our function parses through the track chunks in
  // our midi file byte by byte. returns false if we encounter an error
  bool parseMidi(void);

  // finally, this function will call our various stepper motor function to actually play our music
  void playMidi(void);

  // this is a debug function used to print the contents of our event queue
  // to our serial monitor so that we can manually inspect our event data
  void printQueue(void);
};

namespace midi {
  // this array is used to define the frequencies of each note within octave 0, represented in mHz
  const std::array<uint16_t, 12> noteFreq = { 16350, 17320, 18350, 19450, 20600, 21830, 23120, 24500, 25960, 27500, 29140, 30870 };

  // takes a note frequency and octave and returns the new frequency of the note
  // at the given octave in Hz. returns 0 if note or octave are invalid
  uint32_t getFreq(const uint8_t note);
};

extern midiFile songData;

#endif