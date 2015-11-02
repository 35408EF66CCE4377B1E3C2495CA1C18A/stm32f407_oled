#include "audiotransmitter.h"
#include "adc.h"
#include "dac.h"
#include "tim.h"
#include "dma.h"
#include "socket.h"
#include "wizchip_conf.h"
#include "stm32f4xx_hal_dac_addtional.h"


#define BUF_SIZE	16
#define SOCK_AUDIO	1

uint16_t adc_buf_1[AUDIO_FRAME_SIZE], adc_buf_2[AUDIO_FRAME_SIZE];
uint16_t dac_buf_1[AUDIO_FRAME_SIZE], dac_buf_2[AUDIO_FRAME_SIZE];

DefAudioFrameStruct audio_send_stream[BUF_SIZE];
DefAudioFrameStruct audio_recv_stream[BUF_SIZE];

DefPtrAudioFrameStruct p_head_send, p_tail_send, p_head_recv, p_tail_recv;

DefSockBufStruct socket_recv_buf;

OS_Q SOCK_CMD_Q;

/*
*********************************************************************************************************
*                                         CpyToSendBuffer()
*
* Description : Copy the ADC sampling data to the send buffer
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : various.
*
* Note(s)     : none
*********************************************************************************************************
*/
static void CpyToSendBuffer(uint16_t* src)
{
	HAL_DMA_Start_IT(&hdma_memtomem_dma2_stream1, (uint32_t)src, (uint32_t)p_tail_send->AudioFrame.AudioData, AUDIO_FRAME_SIZE);
}

/*
*********************************************************************************************************
*                                         CpyToRecvBuffer()
*
* Description : Copy the audio frame to the receive buffer for DAC output
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : various.
*
* Note(s)     : none
*********************************************************************************************************
*/
static void CpyToRecvBuffer(uint16_t* src)
{
	HAL_DMA_Start_IT(&hdma_memtomem_dma2_stream2, (uint32_t)src, (uint32_t)p_tail_recv->AudioFrame.AudioData, AUDIO_FRAME_SIZE);
}

/*
*********************************************************************************************************
*                                         CpyToRecvBuffer()
*
* Description : Copy the audio frame to the receive buffer for DAC output
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : various.
*
* Note(s)     : none
*********************************************************************************************************
*/
static void ObtainFromRecvBuffer(uint16_t* dac_dma_mem)
{
	HAL_DMA_Start_IT(&hdma_memtomem_dma2_stream2, (uint32_t)p_head_send->AudioFrame.AudioData, (uint32_t)dac_dma_mem, AUDIO_FRAME_SIZE);
}

/*
*********************************************************************************************************
*                                         CpyToBufferCpltCallback()
*
* Description : Copy the audio frame to the receive buffer for DAC output
*
* Argument(s) : DMA_HandleTypeDef*
*
* Return(s)   : none.
*
* Caller(s)   : various.
*
* Note(s)     : none
*********************************************************************************************************
*/
static void CpyToBufferCpltCallback(DMA_HandleTypeDef* hdma)
{
	//hdma->State = HAL_DMA_STATE_READY;
	
	if(hdma->Instance == DMA2_Stream1)
	{
		p_tail_send ++;
		if(p_tail_send > &audio_send_stream[BUF_SIZE - 1])	// p_tail_send exceed the last element of array
		{
			p_tail_send = audio_send_stream;
		}

		if(p_tail_send == p_head_send) // p_tail_send catches up the p_head_send, increase the p_head_send
		{
			p_head_send++;
			if(p_head_send > &audio_send_stream[BUF_SIZE - 1])	
			{
				p_head_send = audio_send_stream;
			}
		}
	}
	else if(hdma->Instance == DMA2_Stream2) // Transfer data from send buffer to DAC DMA MEM
	{
		/*
		p_tail_recv ++;
		if(p_tail_recv > &audio_recv_stream[BUF_SIZE - 1])	// p_tail_recv exceed the last element of array
		{
			p_tail_recv = audio_recv_stream;
		}

		if(p_tail_recv == p_head_recv) // p_tail_recv catches up the p_head_recv, increase the p_head_recv
		{
			p_head_recv++;
			if(p_head_recv > &audio_recv_stream[BUF_SIZE - 1])	
			{
				p_head_recv = audio_recv_stream;
			}
		}
		*/
		p_head_send ++;
		
		if(p_head_send == p_tail_send) // the buffer is empty
		{
			// Do nothing
		}
		else if(p_head_send > &audio_send_stream[BUF_SIZE - 1])	// p_head_send exceed the last element of array
		{
			p_head_send = audio_send_stream;
		}

		
		
	}
	
	#ifdef __DEBUG_INFO__
	TM_SWO_Printf("p_head_send: 0x%08X\r\np_tail_send: 0x%08X\r\n", (uint32_t)p_head_send, (uint32_t)p_tail_send);
	#endif
}

/*
*********************************************************************************************************
*                                         Init_AudioTransmitter()
*
* Description : Initial the variables of AudioTransmitter Class
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : various.
*
* Note(s)     : none
*********************************************************************************************************
*/
void Init_AudioTransmitter(void)
{
	int i;
	OS_ERR err;
	
	p_tail_send = p_head_send = audio_send_stream;
	p_tail_recv = p_head_recv = audio_recv_stream;
	
	for(i = 0; i < BUF_SIZE; i++)
	{
		audio_send_stream[i].Header = FRAME_HEADER;
		audio_send_stream[i].Type = TYPE_AUDIO_FRAME;
		audio_send_stream[i].Lenght = sizeof(DefPayloadAudioFrame);	
	}
	
	memcpy(p_head_recv, p_head_send, BUF_SIZE * sizeof(DefAudioFrameStruct));
	
	// init the callback function for the completion of the DMA transfer
	hdma_memtomem_dma2_stream1.XferCpltCallback = CpyToBufferCpltCallback;
	
	OSQCreate(&SOCK_CMD_Q, "SOCK_CMD", 1, &err);
}


/*
*********************************************************************************************************
*                                         StartAudio()
*
* Description : Start the audio collection and transfer
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : various.
*
* Note(s)     : none
*********************************************************************************************************
*/
void StartAudio(void)
{
	int cnt = 0;
	int sub_cnt = 0;
	
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buf_1, AUDIO_FRAME_SIZE);
	HAL_ADC_Stop_DMA(&hadc1);
	HAL_ADC_Start_DMA_MultiBuffer(&hadc1, (uint32_t*)adc_buf_1, (uint32_t*)adc_buf_2, AUDIO_FRAME_SIZE);
	
	for(cnt = 0; cnt < AUDIO_FRAME_SIZE; cnt++)
	{
		sub_cnt ++;
		if(sub_cnt <= 4)
		{
			dac_buf_1[cnt] = 0x26;
			dac_buf_2[cnt] = 0x26;
		}
		else if(sub_cnt <= 8)
		{
			dac_buf_1[cnt] = 0x4c;
			dac_buf_2[cnt] = 0x4c;
		}
		else
		{
			cnt --;
			sub_cnt = 0;
		}
	}
	HAL_DAC_Start_MultiBufferDMA(&hdac, DAC_CHANNEL_1, (uint32_t*)dac_buf_1, (uint32_t*)dac_buf_2, AUDIO_FRAME_SIZE, DAC_ALIGN_8B_R);
	
	HAL_TIM_Base_Start_IT(&htim2);
}


/*
*********************************************************************************************************
*                                         StopAudio()
*
* Description : Stop the audio collection and transfer
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : various.
*
* Note(s)     : none
*********************************************************************************************************
*/
void StopAudio(void)
{
	HAL_TIM_Base_Stop_IT(&htim2);
	HAL_ADC_Stop_DMA(&hadc1);
}


/*
*********************************************************************************************************
*                                         HAL_ADC_ConvCpltCallback()
*
* Description : Callback function of the ADC converting
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : various.
*
* Note(s)     : The function transmit the audio data from ADC buffer to the transmitting buffer
*********************************************************************************************************
*/
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	uint16_t *pBuf;
	if(hadc->Instance == ADC1)
	{
		if(hadc->DMA_Handle->State == HAL_DMA_STATE_READY_HALF_MEM0)
		{
			//HAL_UART_Transmit_DMA(&huart1, (uint8_t*)adc_buf_1, 2000);
			pBuf = adc_buf_1;
		}
		else
		{
			//HAL_UART_Transmit_DMA(&huart1, (uint8_t*)adc_buf_2, 2000);
			pBuf = adc_buf_2;
		}
		
		CpyToSendBuffer(pBuf);
	}
}

/*
*********************************************************************************************************
*                                         HAL_DAC_ConvCpltCallbackCh1()
*
* Description : Callback function of the ADC converting
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : various.
*
* Note(s)     : The function transmit the audio data from ADC buffer to the transmitting buffer
*********************************************************************************************************
*/
void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef* hdac)
{
	uint16_t *pBuf;
	
  if(hdac->DMA_Handle1->State == HAL_DMA_STATE_READY_HALF_MEM0)
	{
		pBuf = dac_buf_1;
	}
	else
	{
		pBuf = dac_buf_2;
	}
	
	// transfer audio data from buffer to DMA memory
	//ObtainFromRecvBuffer(pBuf);
}

/*
*********************************************************************************************************
*                                         GetDataFromW5500()
*
* Description : Callback function of the ADC converting
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : various.
*
* Note(s)     : The function transmit the audio data from ADC buffer to the transmitting buffer
*********************************************************************************************************
*/
uint32_t GetDataFromW5500(void)
{
	OS_ERR err;
	
	int32_t  ret;
	uint16_t size, sentsize;
	uint8_t  destip[4];
	uint16_t destport;
	
	if((size = getSn_RX_RSR(SOCK_AUDIO)) > 0)
	{
		if(size > AUDIO_FRAME_SIZE * 2) 
			size = AUDIO_FRAME_SIZE * 2;
		ret = recvfrom(SOCK_AUDIO, (uint8_t*)&socket_recv_buf, size, destip, (uint16_t*)&destport);
		if(ret <= 0)
		{
	#ifdef __DEBUG_INFO__
			 TM_SWO_Printf("%d: recvfrom error. %ld\r\n", SOCK_AUDIO, ret);
	#endif
			 return ret;
		}
		
		if(socket_recv_buf.Header == 0xAAAA)	// Got a valid package
		{
			switch(socket_recv_buf.Type)
			{
				case TYPE_COMMAND_FRAME:
					// Post the command to the command handle task
					OSQPost(
						&SOCK_CMD_Q, 
						(void*)(&((DefPtrCommandStruct)&socket_recv_buf)->Command),
						socket_recv_buf.Lenght,
						OS_OPT_POST_FIFO,
						&err);
				break;
				
				case TYPE_AUDIO_FRAME:
					
				break;
			}
		}			
	}
}
