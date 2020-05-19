#pragma once

#include "../nxp/ngssp.hpp"
#include "../nxp/nggpio.hpp"

#include "utility/log.hpp"
#include "utility/time.hpp"

#include "third_party/fatfs/source/ff.h"

//#include "L0_LowLevel/interrupt.hpp"
#define SCI_WRITE     0b00000010
#define SCI_READ      0b00000011
#define SONG_BUF_SIZE 32

class Mp3{
public:
  enum SCIReg : uint8_t
  {
    kMODE         = 0x00,
    kSTATUS       = 0x01,
    kBASS         = 0x02,
    kCLOCKF       = 0x03,
    kDECODE_TIME  = 0x04,
    kAUDATA       = 0x05,
    kWRAM         = 0x06,
    kWRAMADDR     = 0x07,
    kHDAT0        = 0x08,
    kHDAT1        = 0x09,
    kAIADDR       = 0x0a,
    kVOL          = 0x0b,
    kAICTRL0      = 0x0c,
    kAICTRL1      = 0x0d,
    kAICTRL2      = 0x0e,
    kAICTRL3      = 0x0f
  };
  //Destructor
  ~Mp3();

  //Fully Initialize the MP3 Decoder. This creates the SSP and GPIO objects AND
  //actually initializes the MP3 decoder with the proper register values
  void FullInit();

  //Initialize just the MP3 Decoder. This doesn't touch the SSP objects.
  //Use this if you've reset the MP3 Decoder and not the SJ2.
  void DecoderInit();

  //Read an SCI Register
  uint16_t ReadReg(SCIReg reg);

  //Write an SCI Register
  void WriteReg(SCIReg reg, uint16_t data);

  //Hard reset the MP3 Decoder by toggling the XRESET line
  void HardReset();

  //Soft reset the MP3 Decoder by writing to the MODE register, SM_RESET bit
  void SoftReset();

  //Set the volume digitally (sets left and right channel at the same time)
  void SetVolume(uint8_t vol);

  //Get the volume value. Only gets the right channel volume, but L/R should
  //always be the same
  uint8_t GetVolume();

  //Wait for DREQ to to go high
  void WaitDreq();

  //Check DREQ
  bool CheckDreq();

  //Play a song
  bool PlaySong(char* filename);

  //Get ready to play a new song
  bool PrepareSong(char* filename);

  //Get the seconds the song has been playing
  uint16_t GetPlayTime();

  //Register an interrupt to trigger when the chip needs more data
  //This also enables the interrupt
  void RegisterDREQInterrupt(IsrPointer isr);

  //Start an SDI transaction
  void StartSdi();

  //End an SDI transaction
  void EndSdi();

  //Check to see if we are in the middle of an SDI transaction
  bool CheckSdi();

  //Send 16 bits of song data
  void SendSongData(uint16_t data);

  //Get a pointer to the file object
  FIL* GetFileHandle();

  //End a song gracefully, close the file
  void StopSong();

  //Check to see if the song is paused
  bool CheckPaused();

  //Set the song's pause state
  void SetPaused(bool pause);

  //Toggle pause
  void TogglePause();

  //Set the bass level
  void SetBass(uint8_t bass);

  //Get the bass level
  uint8_t GetBass();

private:
  SSP *_comm;
  FATFS *_fs;
  FIL *_song_file;
  GPIO *_xcs;
  GPIO *_xreset;
  GPIO *_dreq;
  GPIO *_xdcs;
  bool _paused;
};
