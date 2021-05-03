/*
 * Reference
 * http://www.topherlee.com/software/pcm-tut-wavformat.html
 * http://soundfile.sapp.org/doc/WaveFormat/
 * http://www.topherlee.com/software/pcm-tut-wavformat.html
 * 
 * Converse the wav file to the desired format
 * https://jingyan.baidu.com/article/c74d6000dcadee0f6a595dd2.html
 */

#include "sound_card.h"
#include "wav_player.h"
#include "lib.h"
#include "sys_call.h"

const uint8_t *wav_file_list[] = {"macOS_startup.wav"};
uint8_t sound_card_buf[DMA_CHUNK_SIZE_BYTES];

int32_t wave_file_parse(int32_t wav_fd, wave_file_struct *wav_file) {
    if (wav_file == NULL) {
        printf("ERROR in wave_file_parse: NULL input.\n");
        return -1;
    }
    sys_read(wav_fd, wav_file, sizeof(wav_file));
    // check the validity of the file
    if (wav_file->chunkID != 0x46464952 ||
        wav_file->format != 0x45564157 ||
        wav_file->subchunk1ID != 0x20746d66 ||
        wav_file->subchunk2ID != 0x61746164 ||
        wav_file->audioFormat != 1 ||
        wav_file->numChannels < 1) {
        printf("ERROR in wave_file_parse(): the wav file is not valid. fd: %d\n", wav_fd);
        return -1;
    }

    // Check if the sound card support the wav
    if (wav_file->numChannels > 2) {
        printf("ERROR in wave_file_parse: cannot support channel number > 2\n");
        return -1;
    }
    if (wav_file->sampleRate > 44100) {
        printf("ERROR in wave_file_parse: cannot support sample rate > 44100\n");
        return -1;
    }
    if (wav_file->bitsPerSample > 8) {
        printf("ERROR in wave_file_parse: cannot support bit width > 8\n");
        return -1;
    }

    return 0;
}

// http://homepages.cae.wisc.edu/~brodskye/sb16doc/sb16doc.html
int32_t sound_card_init(int32_t dsp_fd, wave_file_struct *wav_file) {
    uint32_t sampleRate = wav_file->sampleRate,
             numChannels = wav_file->numChannels;
    
    /* init sample rate */
    // Write the command (41h for output rate, 42h for input rate)
    if (sys_ioctl(dsp_fd, DSP_SAMPLE_RATE_CMD) != 0) return -1;
    // Write the high byte of the sampling rate (56h for 22050 hz)
    if (sys_ioctl(dsp_fd, (sampleRate >> 8) & 0xFF) != 0) return -1;
    // Write the low byte of the sampling rate (22h for 22050 hz)
    if (sys_ioctl(dsp_fd, (sampleRate & 0xFF)) != 0) return -1;

    /* Writing transfer mode to DSP */
    if (sys_ioctl(dsp_fd, DSP_PLAY_CMD) != 0) return -1;
    // Usually values are 0xB0 for 16 bit playing sound or 0xC0 for 8 bit playing sound.
    if (sys_ioctl(dsp_fd, numChannels == 1 ? DSP_MONO_MODE : DSP_STEREO_MODE) != 0) return -1;

    /* Write data length to DSP (You must calculate LENGTH-1) */
    // low byte first
    if (sys_ioctl(dsp_fd, (DMA_CHUNK_SIZE_BYTES - 1) & 0xFF) != 0) return -1;
    // high byte
    if (sys_ioctl(dsp_fd, ((DMA_CHUNK_SIZE_BYTES - 1) >> 8) & 0xFF) != 0) return -1;

    return 0;
}

int32_t play_wav(int32_t file_num) {
    // open the wav file
    int32_t dsp_fd, wav_fd;
    int32_t length, i, ret, temp;
    wave_file_struct wav_file;

    wav_fd = sys_open(wav_file_list[file_num]);
    if (wav_fd == -1) return -1;

    if (wave_file_parse(wav_fd, &wav_file) != 0) {
        printf("ERROR in play_wav: parse wav file fail\n");
        return -1;
    }

    // open digital signal processor (sound card)
    dsp_fd = sys_open((uint8_t*) "audio");
    if (dsp_fd == -1) return -1;

    /* preloading some data into the buffer */
    memset(sound_card_buf, 0, DMA_CHUNK_SIZE_BYTES);
    length = sys_read(wav_fd, sound_card_buf, DMA_CHUNK_SIZE_BYTES);
    if (length <= 0) {
        printf("ERROR in play_wav: read wav data (0) failure\n");
        return -1;
    }
    ret = sys_write(dsp_fd, sound_card_buf, length);
    if (ret != length) {
        printf("ERROR in play_wav: write wav data (0) to sound card buffer failure\n");
        return -1;
    }

    memset(sound_card_buf, 0, DMA_CHUNK_SIZE_BYTES);
    length = sys_read(wav_fd, sound_card_buf, DMA_CHUNK_SIZE_BYTES);
    if (length < 0) {
        printf("ERROR in play_wav: read wav data (1) failure\n");
        return -1;
    } else if (length > 0) {
        ret = sys_write(dsp_fd, sound_card_buf, length);
        if (ret != length) {
            printf("ERROR in play_wav: write wav data (1) to sound card buffer failure\n");
            return -1;
        }
    }

    // init and start the sound card
    if (sound_card_init(dsp_fd, &wav_file) != 0) {
        printf("ERROR in play_wav: init sound card failure\n");
        return -1;
    }

    i = 1;
    while (1) {
        i++;

        // wait until one block finished
        if (sys_read(dsp_fd, temp, 0) == -1) return -1;
        // exit after the end of the current block (stop for a while)
        if (sys_ioctl(dsp_fd, DSP_EXIT_8B_AUTO_BLOCK_CMD) == -1)  return -1;

        // write the next chunk of data
        memset(sound_card_buf, 0, DMA_CHUNK_SIZE_BYTES);
        length = sys_read(wav_fd, sound_card_buf, DMA_CHUNK_SIZE_BYTES);
        if (length < 0) {
            printf("ERROR in play_wav: read wav data (%d) failure\n", i);
            return -1;
        } else if (length == 0) {
            break;  // reach the end
        } else {
            // Continue
            ret = sys_write(dsp_fd, sound_card_buf, length);
            if (ret != length) {
                printf("ERROR in play_wav: write wav data (%d) to sound card buffer failure\n", i);
                return -1;
            }
            // send command to sound card to continue
            if (sys_ioctl(dsp_fd, DSP_RESUME_PLAY_8B_CMD) == -1) {
                printf("ERROR in play_wav: resume failure (%d)\n", i);
                return -1;
            }
            // handle early stop
            if (length < DMA_CHUNK_SIZE_BYTES) {
                if (sys_read(dsp_fd, temp, 1) == -1) return -1;
                if (sys_ioctl(dsp_fd, DSP_EXIT_8B_AUTO_BLOCK_CMD) == -1) return -1;
                break;
            }
        }
    }
    sys_close(wav_fd);
    sys_close(dsp_fd);

    return 0;
}

