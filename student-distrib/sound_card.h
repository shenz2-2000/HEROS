#ifndef _SOUND_CARD_H
#define _SOUND_CARD_H

#include "lib.h"

/* Digital Signal Processor */
// irq line number
#define DSP_IRQ_NUM             0x5

// Port
#define DSP_MIX_PORT            0x224
#define DSP_MIX_DATA_PORT       0x225
#define DSP_RESET_PORT          0x226
#define DSP_READ_PORT           0x22A
#define DSP_WRITE_PORT          0x22C
#define DSP_READ_STATUS_PORT    0x22E
#define DSP_16B_INT_ACK_PORT    0x22F

// Command for DSP Write
#define DSP_TIME_CNST_CMD           0x40
#define DSP_SAMPLE_RATE_CMD         0x41
#define DSP_SPEAKER_ON_CMD          0xD1
#define DSP_SPEAKER_OFF_CMD         0xD3
#define DSP_STOP_PLAY_8B_CMD        0xD0
#define DSP_RESUME_PLAY_8B_CMD      0xD4
#define DSP_STOP_PLAY_16B_CMD       0xD5
#define DSP_RESUME_PLAY_16B_CMD     0xD6
#define DSP_EXIT_16B_AUTO_BLOCK_CMD 0xD9
#define DSP_EXIT_8B_AUTO_BLOCK_CMD  0xDA
#define DSP_GET_DSP_VER_CMD         0xE1
#define DSP_PLAY_CMD                0xC6

// Command for Mixer Port
#define DSP_MASTER_VOL_CMD      0x22
#define DSP_SET_IRQ_CMD         0x80

// Signals returned by DSP
#define DSP_STATUS_READY        0xAA

// DSP Mode
#define DSP_MONO_MODE           0x00
#define DSP_STEREO_MODE         0x20
#define DSP_SIGNED_MODE         0x10

// channel number (total 2 channels, 1 (8 bit), 2 (16 bit)) and corresponding ports
#define DMA_CH_1                1
#define DMA_CH_2                2
#define DMA_CH_DISABLE          0x04    // disable + channel_num
#define DMA_CH_ENABLE           0x00    // enable + channel_num
#define DMA_CH_1_PAGE_PORT      0x83
#define DMA_CH_2_PAGE_PORT      0x81
#define DMA_CH_1_ADDR_PORT      0x02
#define DMA_CH_1_CNT_PORT       0x03
#define DMA_CH_2_ADDR_PORT      0x04
#define DMA_CH_2_CNT_PORT       0x05

// IO port addresses for the control registers
#define DMA_REG_MASK            0x0A
#define DMA_REG_MODE            0x0B
#define DMA_REG_CLEAR_FF        0x0C

// DMA mode
#define DMA_MODE_SINGLE_PB      0x48    // single-cycle playback
#define DMA_MODE_AUTO_PB        0x58

// the chunk size for DMA
#define DMA_CHUNK_SIZE_BYTES    0x8000

// the address of sound data
// TODO: to be determined!!!
#define DSP_BUF_ADDR            0xA000000    // pos at 160 MB
#define DSP_BUF_LEN             0x10000     // number of bytes. 128 kB = 0x100000-0x120000

/* DSP status */
#define DSP_ON  1
#define DSP_OFF 0

int32_t audio_open();
int32_t audio_close();
int32_t audio_read();
int32_t audio_write(uint32_t *pos, const void *buf, int32_t bufsize);
int32_t audio_ioctl(uint8_t cmd);
void dsp_interrupt_handler();
void copy_chunk(uint8_t* dest, uint8_t* src);

#endif
