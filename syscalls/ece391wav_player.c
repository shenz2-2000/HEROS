/*
 * Reference
 * http://homepages.cae.wisc.edu/~brodskye/sb16doc/sb16doc.html
 * https://wiki.osdev.org/Sound_Blaster_16
 * https://oemdrivers.com/sound-sb16
 */
/*
 * DEMANDS
 * 1. [x] initialize the mem area of DSP page (in paging.c) (also support user space visit!)
 * 2. [x] analyze the wav file
 * 3. [x] modify the file image
 */

#include <stdint.h>
#include "ece391syscall.h"
#include "ece391support.h"

/* Digital Signal Processor */

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

// the chunk size for DMA
#define DMA_CHUNK_SIZE_BYTES    0x8000

typedef struct wave_file_struct {
    /* The "RIFF" chunk descriptor */
    uint32_t chunkID;       // Contains the letters "RIFF" in ASCII form (0x52494646 big-endian form).
    uint32_t chunkSize;     // 36 + SubChunk2Size, or more precisely: 4 + (8 + SubChunk1Size) + (8 + SubChunk2Size)
    uint32_t format;        // Contains the letters "WAVE" (0x57415645 big-endian form).

    /* The "fmt" sub-chunk */
    uint32_t subchunk1ID;   // Contains the letters "fmt " (0x666d7420 big-endian form).
    uint32_t subchunk1Size; // 16 for PCM.  This is the size of the rest of the Subchunk which follows this number.
    uint16_t audioFormat;   // PCM = 1 (i.e. Linear quantization) Values other than 1 indicate some 
    uint16_t numChannels;   // Mono = 1, Stereo = 2, etc.
    uint32_t sampleRate;    // 8000, 44100, etc.
    uint32_t byteRate;      // SampleRate * NumChannels * BitsPerSample/8
    uint16_t blockAlign;    // NumChannels * BitsPerSample/8
    uint16_t bitsPerSample; // 8 bits = 8, 16 bits = 16, etc.

    /* The "data" sub-chunk */
    uint32_t subchunk2ID;   // Contains the letters "data" (0x64617461 big-endian form).
    uint32_t subchunk2Size; // NumSamples * NumChannels * BitsPerSample/8

    /* Data */
} wave_file_struct;

int32_t wave_file_parse(int32_t wav_fd, wave_file_struct *wav_file);

int32_t sound_card_init(int32_t dsp_fd, wave_file_struct *wav_file);

const char *wav_file_list[] = {"macOS_startup.wav"};
uint8_t sound_card_buf[DMA_CHUNK_SIZE_BYTES];

static int32_t wav_player_fail(int32_t wav_fd, int32_t dsp_fd);

/**
 * wave_file_parse
 * Description: parse the wave file
 * Input: wav_fd - the fd for wav file
 *        wav_file - the allocated wave file struct
 * Output: 0 if success, -1 if not
 */
int32_t wave_file_parse(int32_t wav_fd, wave_file_struct *wav_file) {
    // if (wav_file == NULL) {
    //     ece391_fdputs(1, (uint8_t*) "ERROR in wave_file_parse: NULL input.\n");
    //     return -1;
    // }
    // read the file header
    ece391_read(wav_fd, wav_file, sizeof(*wav_file));
    // check the validity of the file
    if (wav_file->audioFormat != 1) {
        ece391_fdputs(1, (uint8_t*) "ERROR in wave_file_parse(): the wav file is not valid. %d\n");
        return -1;
    }
    if (wav_file->numChannels < 1) {
        ece391_fdputs(1, (uint8_t*) "ERROR in wave_file_parse(): cannot support channel number < 1\n");
    }
    if (wav_file->numChannels > 2) {
        ece391_fdputs(1, (uint8_t*) "ERROR in wave_file_parse: cannot support channel number > 2\n");
        return -1;
    }
    if (wav_file->sampleRate > 44100) {
        ece391_fdputs(1, (uint8_t*) "ERROR in wave_file_parse: cannot support sample rate > 44100\n");
        return -1;
    }
    if (wav_file->bitsPerSample > 8) {
        ece391_fdputs(1, (uint8_t*) "ERROR in wave_file_parse: cannot support bit width > 8\n");
        return -1;
    }

    return 0;
}

// reference: http://homepages.cae.wisc.edu/~brodskye/sb16doc/sb16doc.html
/**
 * sound_card_init
 * Description: init and start the sound card
 * Input: dsp_fd - the fd for the cound card device
 *        wav_fd - the fd for wav file
 * Output: 0 if success, -1 if not
 */
int32_t sound_card_init(int32_t dsp_fd, wave_file_struct *wav_file) {
    uint32_t sampleRate = wav_file->sampleRate,
             numChannels = wav_file->numChannels;
    
    /* init sample rate */
    // Write the command (41h for output rate, 42h for input rate)
    if (ece391_ioctl(dsp_fd, DSP_SAMPLE_RATE_CMD) != 0) return -1;
    // Write the high byte of the sampling rate (56h for 22050 hz)
    if (ece391_ioctl(dsp_fd, (sampleRate >> 8) & 0xFF) != 0) return -1;
    // Write the low byte of the sampling rate (22h for 22050 hz)
    if (ece391_ioctl(dsp_fd, (sampleRate & 0xFF)) != 0) return -1;

    /* Writing transfer mode to DSP */
    if (ece391_ioctl(dsp_fd, DSP_PLAY_CMD) != 0) return -1;
    // Usually values are 0xB0 for 16 bit playing sound or 0xC0 for 8 bit playing sound.
    if (ece391_ioctl(dsp_fd, numChannels == 1 ? DSP_MONO_MODE : DSP_STEREO_MODE) != 0) return -1;

    /* Write data length to DSP (You must calculate LENGTH-1) */
    // low byte first
    if (ece391_ioctl(dsp_fd, (DMA_CHUNK_SIZE_BYTES - 1) & 0xFF) != 0) return -1;
    // high byte
    if (ece391_ioctl(dsp_fd, ((DMA_CHUNK_SIZE_BYTES - 1) >> 8) & 0xFF) != 0) return -1;

    return 0;
}

/**
 * play_wav
 * Description: play the wav file
 * Input: file_num - the file descriptor number for each wav file
 * Output: 0 if success, -1 if not
 */
int32_t main() {
    char f_name[32];
    int32_t dsp_fd, wav_fd;
    int32_t length, i, ret, temp;
    wave_file_struct wav_file;

    // get argument
    ret = ece391_getargs((uint8_t*) f_name, 32);
    if (ret == -1) {
        ece391_fdputs(1, (uint8_t*) "ERROR in play_wav: read file name fail\n");
        return -1;
    }
    
    // open the wav file
    wav_fd = ece391_open((uint8_t*) f_name);
    if (wav_fd == -1) return -1;

    if (wave_file_parse(wav_fd, &wav_file) != 0) {
        ece391_fdputs(1, (uint8_t*) "ERROR in play_wav: parse wav file fail\n");
        ece391_close(wav_fd);
        return -1;
    }

    // open digital signal processor (sound card)
    dsp_fd = ece391_open((uint8_t*) "audio");
    if (dsp_fd == -1) return -1;

    // Start play the music
    i = 0;
    while (1) {
        // write a chunk of data
        // memset(sound_card_buf, 0, DMA_CHUNK_SIZE_BYTES);
        length = ece391_read(wav_fd, sound_card_buf, DMA_CHUNK_SIZE_BYTES);
        if (length < 0) {
            ece391_fdputs(1, (uint8_t*) "ERROR in play_wav: read wav data (1) failure\n");
            return wav_player_fail(wav_fd, dsp_fd);
        } else if (length > 0) {
            ret = ece391_write(dsp_fd, sound_card_buf, length);
            if (ret != length) {
                ece391_fdputs(1, (uint8_t*) "ERROR in play_wav: write wav data (1) to sound card buffer failure\n");
                return wav_player_fail(wav_fd, dsp_fd);
            }
        } else if (length == 0 && (i == 0 || i > 1)) {
            break;  // reach the end
        }

        if (i > 0) {    // it's necessary to preload some data before play (mem loading is slow)

            if (i == 1) {
                // init and start the sound card
                if (sound_card_init(dsp_fd, &wav_file) != 0) {
                    ece391_fdputs(1, (uint8_t*) "ERROR in play_wav: init sound card failure\n");
                    return wav_player_fail(wav_fd, dsp_fd);
                }
            } else if (i > 1) {
                // send command to sound card to continue
                if (ece391_ioctl(dsp_fd, DSP_RESUME_PLAY_8B_CMD) == -1) {
                    ece391_fdputs(1, (uint8_t*) "ERROR in play_wav: resume failure\n");
                    return wav_player_fail(wav_fd, dsp_fd);
                }
            }

            // wait until one block finished
            if (ece391_read(dsp_fd, &temp, 1) == -1) return wav_player_fail(wav_fd, dsp_fd);
            // exit after the end of the current block (stop for a while)
            if (ece391_ioctl(dsp_fd, DSP_EXIT_8B_AUTO_BLOCK_CMD) == -1)  return wav_player_fail(wav_fd, dsp_fd);

            // handle early stop
            if (length < DMA_CHUNK_SIZE_BYTES) {
                break;
            }

        }

        i++;
    }

    ece391_close(wav_fd);
    ece391_close(dsp_fd);

    return 0;
}

/**
 * wav_player_fail
 * Description: close the file and return
 * Input: dsp_fd - the fd for the cound card device
 *        wav_fd - the fd for wav file
 * Output: -1
 */
static int32_t wav_player_fail(int32_t wav_fd, int32_t dsp_fd) {
    ece391_close(wav_fd);
    ece391_close(dsp_fd);
    return -1;
}
