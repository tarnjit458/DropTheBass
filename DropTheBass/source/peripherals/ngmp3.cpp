#include "ngmp3.hpp"

void Mp3::FullInit(){
  //Create a new SSP object at runtime
  //TODO: See if we can increase the clockspeed here
  _comm = new SSP(16, SSP::kSPI, 8, 0);
  _comm->Init();

  //GPIO Signals:
  //XCS (Chip Select): P0_10
  _xcs = new GPIO(0, 10);
  _xcs->SetAsOutput();
  _xcs->SetHigh();
  //XRESET (Chip Reset): P0_11
  _xreset = new GPIO(0, 11);
  _xreset->SetAsOutput();
  _xreset->SetHigh();
  //DREQ (Data Request): P0_0
  _dreq = new GPIO(0, 25);
  _dreq->SetAsInput();
  //XDCS (Data Chip Select): P0_6
  _xdcs = new GPIO(0, 6);
  _xdcs->SetAsOutput();
  _xdcs->SetHigh();

  //Initialize the filesystem
  _fs = new FATFS;
  //Mount and check the SD card filesystem
	FRESULT fr = f_mount(_fs, "", 0);
	if(fr){
		LOG_ERROR("Could not mount filesystem, returned with code %i\n", fr);
	}
	else{
		LOG_INFO("Main: Filesystem mounted!\n");
	}

  //Initialize the file object
  _song_file = new FIL;

  //Set the song to not paused
  _paused = 0;

  //After initializing all the private GPIO and SSP objects, hard reset
  //the decoder
  HardReset();
  //Initialize the decoder
  DecoderInit();
}

void Mp3::DecoderInit(){
  //Set up the CLOCKF register
  //Set CLKI to XTALI * 3
  WriteReg(Mp3::SCIReg::kCLOCKF, 0x6000);
  //The chip needs a second to change the frequency
  Delay(1);
  //Start the volume at slightly less than half
  SetVolume(0x70);
}

Mp3::~Mp3(){
  //Make sure we delete the SSP object
  delete _comm;

  //Make sure we delete all the GPIO objects
  delete _xcs;
  delete _xreset;
  delete _dreq;
  delete _xdcs;

  //Make sure we delete the filesystem object
  delete _fs;

  //Make sure we delete the file object`
  delete _song_file;
}

uint16_t Mp3::ReadReg(SCIReg reg){
  uint16_t buf;
  //Select the chip
  _xcs->SetLow();
  //Send the read command and the address of the register
  buf = (SCI_READ << 8) | reg;
  _comm->Send(&buf);
  //Get the value the mp3 decoder sends back
  _comm->Recv(&buf);
  //Deselect the chip
  _xcs->SetHigh();
  //Wait for DREQ
  WaitDreq();
  //Return the value the decoder sent back
  return buf;
}

void Mp3::WriteReg(SCIReg reg, uint16_t data){
  uint16_t buf;
  //Select the chip
  _xcs->SetLow();
  //Send the write command and the address of the register
  buf = (SCI_WRITE << 8) | reg;
  _comm->Send(&buf);
  //Send the actual data
  buf = data;
  _comm->Send(&buf);
  //Deselect the chip
  _xcs->SetHigh();
  //Wait for DREQ
  WaitDreq();
}

void Mp3::HardReset(){
  //XRESET is active low
  _xreset->SetLow();
  //Wait 4 microseconds
  Delay(4);
  //Re-enable the chip
  _xreset->SetHigh();
  //Wait for it to be ready
  Delay(1);
  WaitDreq();
}

void Mp3::SoftReset(){
  //Write to the SM_RESET bit in MODE
  WriteReg(SCIReg::kMODE, (1 << 2));
  //Wait for it to reset
  Delay(1);
  //Wait for the chip to become ready
  WaitDreq();
}

void Mp3::SetVolume(uint8_t vol){
  //Set the left and write channels to be the same value
  WriteReg(SCIReg::kVOL, (vol << 8) | vol);
}

uint8_t Mp3::GetVolume(){
    //Only return the LSB
    return ReadReg(SCIReg::kVOL) & 0xFF;
}

bool Mp3::CheckDreq(){
  return _dreq->ReadBool();
}

void Mp3::WaitDreq(){
  //Wait for DREQ to go back to high, means the chip is ready
  while(!CheckDreq()){
    continue;
  }
}

bool Mp3::PrepareSong(char* filename){
  //Make sure we're in VS10xx native mode
  WriteReg(SCIReg::kMODE, (1 << 11));
  //Clear out the resync variable (0x1e29) to resync the player
  //Just in case we want to play some WMA or M4A files
  WriteReg(SCIReg::kWRAMADDR, 0x1e29);
  WriteReg(SCIReg::kWRAM,     0x00);
  //Clear the decode time register
  //Write to it twice because the datasheet says so
  WriteReg(SCIReg::kDECODE_TIME, 0x00);
  WriteReg(SCIReg::kDECODE_TIME, 0x00);
  //Open the song on the filesystem
  FRESULT fr = f_open(_song_file, filename, FA_READ);
  if(fr){
    LOG_ERROR("Could not open song!");
    return 0;
  }
  return 1;
}

bool Mp3::PlaySong(char* filename){
  FRESULT fr;
  PrepareSong(filename);
  //Buffer to store the data to stream to the decoder
  uint8_t buf[SONG_BUF_SIZE];
  //Store the bytes read
  UINT bytes_read = SONG_BUF_SIZE;
  //Start streaming song data
  //If we ever read less than 32 bytes, we've hit the end of the song
  while(bytes_read >= SONG_BUF_SIZE){
    //Read 32 bytes from the SD card, store in buffer
    fr = f_read(_song_file, buf, SONG_BUF_SIZE, &bytes_read);
    //Send the buffer in 32 byte chunks
    for(uint16_t i = 0; i < SONG_BUF_SIZE/32; i++){
      //Pull XDCS low, tell the chip we have song data for it
      StartSdi();
      //Send our buffer
      for(uint8_t j = 0; j < 32; j+=2){
        //Don't send if the chip is full
        WaitDreq();
        SendSongData((buf[i*32 + j] << 8) | (buf[i*32 + j + 1]));
      }
      //Pull the XDCS back high to sync, like the datasheet says
      EndSdi();
    }
  }
}

uint16_t Mp3::GetPlayTime(){
  return ReadReg(SCIReg::kDECODE_TIME);
}

void Mp3::RegisterDREQInterrupt(IsrPointer isr){
  _dreq->AttachIsrHandle(isr, GPIO::Edge::kRising);
  _dreq->EnableInterrupts();
}

void Mp3::StartSdi(){
  _xdcs->SetLow();
}

void Mp3::EndSdi(){
  _xdcs->SetHigh();
}

bool Mp3::CheckSdi(){
  return _xdcs->ReadBool();
}

void Mp3::SendSongData(uint16_t data){
  _comm->Send(data);
}

FIL* Mp3::GetFileHandle(){
  return _song_file;
}

void Mp3::StopSong(){
  if(_song_file){
    f_close(_song_file);
  }
}

bool Mp3::CheckPaused(){
  return _paused;
}

void Mp3::SetPaused(bool pause){
  _paused = pause;
}

void Mp3::TogglePause(){
  _paused = !_paused;
}

//Set the bass level
void Mp3::SetBass(uint8_t bass){
  //Bass can be set from 0 to 15
  //Mask the bottom 4 bits and shift 4 to the left, as per the datasheet
  bass = ((bass & 0x0F) << 4);
  WriteReg(SCIReg::kBASS, bass);
}

//Get the bass level
uint8_t Mp3::GetBass(){
  //Shift the bits and mask to get just the bass level
  return (0xF0 & ReadReg(SCIReg::kBASS)) >> 4;
}
