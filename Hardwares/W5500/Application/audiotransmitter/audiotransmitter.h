#ifndef  _AUDIO_TRANSMITTING_H_
#define  _AUDIO_TRANSMITTING_H_

#include "includes.h"

extern OS_Q SOCK_CMD_Q;


#define FRAME_HEADER				0xAAAA
#define TYPE_AUDIO_FRAME		0x0010
#define TYPE_COMMAND_FRAME	0x0020
#define AUDIO_FRAME_SIZE		500


extern uint16_t adc_buf_1[AUDIO_FRAME_SIZE], adc_buf_2[AUDIO_FRAME_SIZE];
extern uint16_t dac_buf_1[AUDIO_FRAME_SIZE], dac_buf_2[AUDIO_FRAME_SIZE];

// Audio Payload Struct
typedef struct 
{
	uint16_t 	FrameCounter;
	uint16_t	AudioData[AUDIO_FRAME_SIZE];
}DefPayloadAudioFrame;

// Command Payload Struct
typedef struct
{
	uint32_t	Command;
}DefPayloadCommand;

// Audio Frame Struct
typedef struct
{
	uint16_t								Header;
	uint16_t								Type;
	uint16_t								Lenght;
	uint16_t								Crc32;
	DefPayloadAudioFrame		AudioFrame;
}DefAudioFrameStruct, *DefPtrAudioFrameStruct;

// Command Frame Struct
typedef struct
{
	uint16_t						Header;
	uint16_t						Type;
	uint16_t						Lenght;
	uint16_t						Crc32;
	DefPayloadCommand		Command;
	
}DefCommandStruct, *DefPtrCommandStruct;

// Socket buffer struct
typedef struct
{
	uint16_t						Header;
	uint16_t						Type;
	uint16_t						Lenght;
	uint16_t						Crc32;
	uint16_t						Payload[AUDIO_FRAME_SIZE];
}DefSockBufStruct, *DefPtrSockBufStruct;


extern void Init_AudioTransmitter(void);
extern void StartAudio(void);
extern void StopAudio(void);
extern uint32_t GetDataFromW5500(void);

#endif
