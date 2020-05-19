#include "third_party/FreeRTOS/Source/include/FreeRTOS.h"
#include "third_party/FreeRTOS/Source/include/task.h"
#include "third_party/FreeRTOS/Source/include/queue.h"
#include "third_party/FreeRTOS/Source/include/event_groups.h"
#include "third_party/FreeRTOS/Source/include/semphr.h"
#include "third_party/FreeRTOS/Source/include/timers.h"

#include "utility/log.hpp"
#include "L3_Application/oled_terminal.hpp"
#include "L3_Application/commandline.hpp"
#include "L3_Application/commands/rtos_command.hpp"

#include <cstdint>
#include <iterator>
#include <stdio.h>
#include <stdlib.h>

#include "peripherals/ngmp3.hpp"
#include "nxp/nggpio.hpp"
#include "peripherals/nghbrtos.hpp"

//The amount of RAM for each task
#define SONG_TASK_RAM 		512
#define SCAN_TASK_RAM			512
#define DISPLAY_TASK_RAM 	512
#define EVENT_TASK_RAM		512

//Button listener ISRs
void PauseButtonISR();
void NextButtonISR();
void SelButtonISR();
void PrevButtonISR();
//Play the song
void xPlaySong(void* p);
//Menu to choose a song
void xSongMenu(void* p);
//Scan the directory
void xScanDir(void* p);
//Listen for button presses during song playback
void xEventListener(void *p);
//Flop the fish!
void xFishFlop(void *p);

//The handles to various tasks. Some tasks don't need handlesprev_sem
TaskHandle_t xPlaySongHandle;
TaskHandle_t xEventListenerHandle;
TaskHandle_t xFishFlopHandle;

//The OLED terminal object, so we can print stuff on the screen
OledTerminal oled_terminal;

//The definitions for all the buttons
GPIO prev(2, 5);
GPIO sel(0, 30);
GPIO pause(0, 29);
GPIO next(2, 7);
GPIO func_led(1, 24);
Motor body;
Motor mouth;

//Semaphores to monitor the status of the buttons
//When a semaphore is given by a button ISR, the button has been pressed
SemaphoreHandle_t prev_sem;
SemaphoreHandle_t next_sem;
SemaphoreHandle_t pause_sem;
SemaphoreHandle_t sel_sem;

//Mutex to make sure we don't interrupt SD Card Transfers
SemaphoreHandle_t sd_mutex;

//Mutex to make sure we don't interrupt an MP3 data transfer
SemaphoreHandle_t mp3_mutex;

//The MP3 object for the VS1053 chip
Mp3 mp3;

//A list of all the songs. It's a list of char pointers, each pointer points
//to a null terminated string containing the song name
char** song_list = NULL;

//The total number of songs found on the SD card
uint8_t num_songs = 0;
//The ID of the song being played. This corresponds to the index of the song
//in song_list. Ex: The string for song_id = 3 is song_list[3]
uint8_t song_id = 0;
//Bool that holds state of the function button
bool func_key = 1;

namespace{
	CommandList_t<32> command_list;
	RtosCommand rtos_command;
	CommandLine<command_list> ci;

	void TerminalTask([[maybe_unused]] void * ptr){
	  ci.WaitForInput();

	  vTaskDelete(nullptr);
	}
}


int main()
{
	//Whole bunch of setup shit
	//Attach the interrupts for all the buttons
	prev.AttachIsrHandle(PrevButtonISR, GPIO::Edge::kRising);
	next.AttachIsrHandle(NextButtonISR, GPIO::Edge::kRising);
	sel.AttachIsrHandle(SelButtonISR, GPIO::Edge::kFalling);
	pause.AttachIsrHandle(PauseButtonISR, GPIO::Edge::kFalling);
	//Fully initialize the MP3 chip.
	mp3.FullInit();
	//Max the volume of the MP3 chipkIntPorts
	mp3.SetVolume(0x00);
	//Init the OLED screen
	oled_terminal.Initialize();
	//Set all the buttons as inputs
	next.SetAsInput();
	prev.SetAsInput();
	sel.SetAsInput();
	pause.SetAsInput();
	func_led.SetAsOutput();
	//Turn on pulldowns as needed
	sel.SetPulldown();
	pause.SetPulldown();
	prev.SetPulldown();
	next.SetPulldown();
	func_led.SetHigh();
	func_led.Set(func_key);
	body.Init(1, 29, 1, 14);
	mouth.Init(1, 20, 1, 31);
	//Setup all the semaphores for the buttons. When a semaphore can be taken,
	//that means a button has been pressed.
	prev_sem = xSemaphoreCreateBinary();
	next_sem = xSemaphoreCreateBinary();
	pause_sem = xSemaphoreCreateBinary();
	sel_sem = xSemaphoreCreateBinary();
	sd_mutex = xSemaphoreCreateMutex();
	mp3_mutex = xSemaphoreCreateMutex();
	//Actually turn on the interrupts. We only enable once, but because of the way
	//the GPIO library is written this enables interrupts on all the buttons
	prev.EnableInterrupts();
	pause.EnableInterrupts();
	//Set up the commandline stuff
	ci.AddCommand(&rtos_command);
	ci.Initialize();

	//On bootup, start scanning the SD card for songs.
	xTaskCreate(xScanDir, "scan_task", SCAN_TASK_RAM, NULL, tskIDLE_PRIORITY + 2, NULL);
	//Set up the commandline stuff so we can monitor CPU usage
	//xTaskCreate(TerminalTask, "Terminal", 1024, nullptr, tskIDLE_PRIORITY + 1, nullptr);
	vTaskStartScheduler();
}

//Show a menu and have a user choose a song on the OLED display
void xSongMenu(void* p){
	//Clear out the oled terminal and prompt the user to choose a song
	oled_terminal.Clear();
	oled_terminal.printf("Choose a song\n");
	//Loop forever until the user chooses a song
	for(;;){
		//Scan the buttons once every 10th of a second
		vTaskDelay(100);
		//Reset the cursor at the top of the loop and print the song
		oled_terminal.SetCursor(0, 1);
		oled_terminal.printf("%s               \n", song_list[song_id]);
		//Check to see if the next button is pressed
		if(xSemaphoreTake(next_sem, 0)){
			//When the next button is pressed, select the next song. Make sure to loop
			//back to the beginning of the song list
			if(song_id >= num_songs - 1){
				song_id = 0;
			}
			else{
				song_id++;
			}
		}
		//When the previous button is pressed, select to the previous song. Make
		//sure to loop back to the end of the list
		if(xSemaphoreTake(prev_sem, 0)){
			if(song_id <= 0){
				song_id = num_songs - 1;
			}
			else{
				song_id--;
			}
		}
		//When the select button is pressed, the user has made their choice. Exit
		//the loop
		if(xSemaphoreTake(sel_sem, 0)){
			break;
		}
		//Yield the task after a poll
		taskYIELD();
	}
	//Play the song at the song_id the user selected
	xTaskCreate(xPlaySong, "playsong_task", SONG_TASK_RAM, NULL, tskIDLE_PRIORITY + 1, &xPlaySongHandle);
	//Delete this task
	vTaskDelete(NULL);
}

//Scan the root of the SD card and enumerate all the files to song_list
void xScanDir(void* p){
	//Initialize all the special FATFS variables
	FILINFO fno;
	FRESULT res;
	DIR dir;
	UINT i;
	//Check the root directory
  char path[] = {"/"};
	//Open the directory
	res = f_opendir(&dir, path);
	if (res == FR_OK) {
		uint8_t i = 0;
		num_songs = 0;
		//Loop over all the directory items
		for (;;) {
				//Read a directory item
				res = f_readdir(&dir, &fno);
				//Break if there's an error or no more files
				if (res != FR_OK || fno.fname[0] == 0) break;
				//Increase the number of songs found
				num_songs++;
		}
		//Allocate the song list in memory
		song_list = (char**)malloc(num_songs*sizeof(char*)); //TODO: Free song_list later
		//Open the directory again
		f_opendir(&dir, path);
			for (;;) {
					//Read a song
					res = f_readdir(&dir, &fno);
					//Break on an error or if there are no more songs
					if (res != FR_OK || fno.fname[0] == 0) break;
					//Allocate space for new song names
					song_list[i] = new char[strlen(fno.fname)]; //TODO: free this char later
					//Copy the name of the song into the song list
					strncpy(song_list[i], fno.fname, 50);
					i++;
			}
			//Close the directory cleanly
			f_closedir(&dir);
	}
	else{
		//If the SD card can't be opened, show an error and halt
		oled_terminal.printf("Cannot read SD card!\n");
		vTaskDelete(NULL);
	}
	//Prompt the user to choose a song that this task has found
	xTaskCreate(xSongMenu, "song_menu", DISPLAY_TASK_RAM, NULL, tskIDLE_PRIORITY + 1, NULL);
	//Delete this task
	vTaskDelete(NULL);
}

void xPlaySong(void* p){
	FRESULT fr;
	//Clear the OLED screen
	oled_terminal.Clear();
	//Prepare a song for play
	if(!mp3.PrepareSong(song_list[song_id])){
		//If the song can't be played, notify the user with a message for 2 seconds
		//and then prompt them to choose a new song instead
		oled_terminal.printf("Unable to play %s\n", song_list[song_id]);
		vTaskDelay(2000);
		xTaskCreate(xScanDir, "scan_task", SCAN_TASK_RAM, NULL, tskIDLE_PRIORITY + 1, NULL);
		vTaskDelete(NULL);
	}
	//Print the name of the song being played and the Play icon
	oled_terminal.printf("Playing\n%s\n", song_list[song_id]);
	oled_terminal.SetCursor(0, 7);
	oled_terminal.printf("> " );
	oled_terminal.SetCursor(0, 0);
	//Start the event listener so we can listen for the buttons
	xTaskCreate(xEventListener, "event_listener", EVENT_TASK_RAM, NULL, tskIDLE_PRIORITY + 1, &xEventListenerHandle);
	//Start floppin the fish
	xTaskCreate(xFishFlop, "xFishFlop", 512, NULL, tskIDLE_PRIORITY + 1, &xFishFlopHandle);
	//Buffer to store the data to stream to the decoder
  uint8_t buf[SONG_BUF_SIZE];
  //Store the bytes read
  UINT bytes_read = SONG_BUF_SIZE;
  //Start streaming song data
  //If we ever read less than 32 bytes, we've hit the end of the song
  while(bytes_read >= SONG_BUF_SIZE){
		//Take the SD card mutex
		if(xSemaphoreTake(sd_mutex, 0)){
			//Read 32 bytes from the SD card, store in buffer
			fr = f_read(mp3.GetFileHandle(), buf, SONG_BUF_SIZE, &bytes_read);
			xSemaphoreGive(sd_mutex);
			//Send the buffer in 32 byte chunks
			for(uint16_t i = 0; i < SONG_BUF_SIZE/32; i++){
				//Pull XDCS low, tell the chip we have song data for it
				xSemaphoreTake(mp3_mutex, 0);
				//Start the data transaction
				mp3.StartSdi();
				//Send our buffer
				for(uint8_t j = 0; j < 32; j+=2){
					//Don't send if the chip is full or the song is paused
					while(!mp3.CheckDreq() || mp3.CheckPaused()){
						mp3.EndSdi();
						//Check once, then yield
						taskYIELD();
						mp3.StartSdi();
					}
					//Send the two bytes
					mp3.SendSongData((buf[i*32 + j] << 8) | (buf[i*32 + j + 1]));
				}
				//Pull the XDCS back high to sync and end the data transaction
				mp3.EndSdi();
				//Give back the MP3 mutex when it's done
				xSemaphoreGive(mp3_mutex);
			}
		}
  }
	//Gracefully end the song and close the file when it's done
	mp3.StopSong();
	//Stop the button state machine
	vTaskDelete(xEventListenerHandle);
	//Stop flopping the fish
	vTaskDelete(xFishFlopHandle);
	//Scan the SD card again, prompt the user to choose another song
	xTaskCreate(xScanDir, "scan_task", SCAN_TASK_RAM, NULL, tskIDLE_PRIORITY + 2, NULL);
	//Delete this task
	vTaskDelete(NULL);
}

//Pause Song
void xPauseSong(void *p){
	//Toggle the pause variable
	mp3.TogglePause();
	//If the song is paused, show the pause icon
	if(mp3.CheckPaused()){
		oled_terminal.SetCursor(0, 7);
		oled_terminal.printf("||" );
	}
	//If the song is playing, show the play icon
	else{
		oled_terminal.SetCursor(0, 7);
		oled_terminal.printf("> " );
	}
	//Delete this task
	vTaskDelete(NULL);
}

//Massive state machine for the buttons during song playback.
//The function key doubles the amount of functions that the other buttons
//can do. For example:
//Pause = Pause Song
//Prev = Previous Song
//Next = Next Song
//Pause + Func = Exit to Menu
//Prev + Func = Decrease Bass
//Next + Func = Increase Bass
//Sel = Function Toggle
void xEventListener(void *p){
	//Quick delay to wait for switch to debounce
	vTaskDelay(400);
	while(1){
			//Poll the buttons once every tenth of a second
			vTaskDelay(100);
			//Try to take the next button's semaphore
			if(xSemaphoreTake(next_sem, 0)){
				if(func_key){
					//Increase the song id (or loop it back to zero)
					if(song_id >= num_songs - 1){
						song_id = 0;
					}
					else{
						song_id++;
					}
					//Wait for the SD card to be free
					if(xSemaphoreTake(sd_mutex, 1000)){
						//Clear the OLED screen
						oled_terminal.Clear();
						//Kill the xPlaySong task
						vTaskDelete(xPlaySongHandle);
						//Close the mp3 file
						mp3.StopSong();
						//Release the SD card mutex
						xSemaphoreGive(sd_mutex);
						//Start a new xPlaySong, this time with the new song_id
						xTaskCreate(xPlaySong, "playsong_task", SONG_TASK_RAM, NULL, tskIDLE_PRIORITY + 1, &xPlaySongHandle);
						//Delete the xEventListener task, xPlaySong will spawn a new one
						vTaskDelete(NULL);
					}
				}
				else{
					//Don't interrupt an MP3 transfer
					xSemaphoreTake(mp3_mutex, 1000);
					//Increase the bass by one
					mp3.SetBass(mp3.GetBass() + 1);
					//Give back the mutex
					xSemaphoreGive(mp3_mutex);
				}
			}
			//Try to take the prev button's semaphore
			if(xSemaphoreTake(prev_sem, 0)){
				if(func_key){
					//Decrease the song ID (or loop it back to the end)
					if(song_id <= 0){
						song_id = num_songs - 1;
					}
					else{
						song_id--;
					}
					//Wait for the SD card to be free
					if(xSemaphoreTake(sd_mutex, 1000)){
						//Clear the OLED screen
						oled_terminal.Clear();
						//Kill the xPlaySong task
						vTaskDelete(xPlaySongHandle);
						//Close the mp3 file
						mp3.StopSong();
						//Release the SD card mutex
						xSemaphoreGive(sd_mutex);
						//Start a new xPlaySong, this time with the new song_id
						xTaskCreate(xPlaySong, "playsong_task", SONG_TASK_RAM, NULL, tskIDLE_PRIORITY + 1, &xPlaySongHandle);
						//Delete the xEventListener task, xPlaySong will spawn a new one
						vTaskDelete(NULL);
					}
				}
				else{
					//Don't interrupt an MP3 Transfer
					xSemaphoreTake(mp3_mutex, 1000);
					//Decrease the bass by one
					mp3.SetBass(mp3.GetBass() - 1);
					//Give back the mutex
					xSemaphoreGive(mp3_mutex);
				}
			}

			if(xSemaphoreTake(sel_sem, 0)){
				func_key = !func_key;
				func_led.Set(func_key);
			}
			if(xSemaphoreTake(pause_sem, 0)){
				if(func_key){
					xTaskCreate(xPauseSong, "pausesong_task", SONG_TASK_RAM, NULL, tskIDLE_PRIORITY + 3, NULL);
				}
				else{
					//Toggle function and shut off the LED
					func_key = !func_key;
					func_led.Set(func_key);
					//Stop the song machine
					vTaskDelete(xPlaySongHandle);
					//Stop floppin the fish
					vTaskDelete(xFishFlopHandle);
					//Scan the SD card again, prompt the user to choose another song
					xTaskCreate(xScanDir, "scan_task", SCAN_TASK_RAM, NULL, tskIDLE_PRIORITY + 2, NULL);
					//Delete this task
					vTaskDelete(NULL);
				}
			}
			//Yield the task once all the buttons have been polled
			taskYIELD();
	}
}

void xFishFlop(void* p){
	while(true){
		body.Forward(300);
		body.Backward(150);
	}
}
//These are the button ISR's. They're tied to the falling edges of buttons so
//they trigger on button release.
//When the button is pressed, signal to the rest of the program by giving that
//button's semaphore
void NextButtonISR(){xSemaphoreGiveFromISR(next_sem, NULL);}
void PrevButtonISR(){xSemaphoreGiveFromISR(prev_sem, NULL);}
void SelButtonISR(){xSemaphoreGiveFromISR(sel_sem, NULL);}
void PauseButtonISR(){xSemaphoreGiveFromISR(pause_sem, NULL);}
