//
// Created by BOURNE on 2021/3/20.
//
#include "x86_desc.h"

/**
 * fill_page
 * Description: Fill the page directory and page table
 * Input: None.
 * Output: None.
 * Side effect: modify x86_desc.S
 */
void fill_page(){
    // --------------------------Fill the Page Directory--------------------
    int i;
    uint32_t temp;
    for(i = 0; i < PAGE_DIRECTORY_SIZE; i++){
        // for the first entry (map to video memory)
        if(i == 0){
            temp = (uint32_t) page_table0;
            page_directory[i].P = 1;
            page_directory[i].RW = 1;
            page_directory[i].US = 0;
            page_directory[i].PWT = 0;
            page_directory[i].PCD = 0;
            page_directory[i].A = 0;
            page_directory[i].D = 0;
            page_directory[i].PS = 0;
            page_directory[i].G = 0;
            page_directory[i].AVAIAL = 0;
            page_directory[i].PAT = (temp >> 12) & 0x1;
            page_directory[i].reserved = (temp >> 13) & 0x1FF;
            page_directory[i].Base_address = temp >> 22;
            continue;
        }
        // for the second entry (kernel page)
        else if(i == 1){
            page_directory[i].P = 1;
            page_directory[i].RW = 1;
            page_directory[i].US = 0;
            page_directory[i].PWT = 0;
            page_directory[i].PCD = 0;
            page_directory[i].A = 0;
            page_directory[i].D = 0;
            page_directory[i].PS = 1;
            page_directory[i].G = 0;
            page_directory[i].AVAIAL = 0;
            page_directory[i].PAT = 0;
            page_directory[i].reserved = 0;
            page_directory[i].Base_address = 1;
            continue;
        }
        // for the audio play support (one-to-one match)
        else if (i == AUDIO_BUF_PD_INDEX) {
            temp = (uint32_t) audio_page_table0;
            page_directory[i].P = 1;
            page_directory[i].RW = 1;
            page_directory[i].US = 0;
            page_directory[i].PWT = 0;
            page_directory[i].PCD = 0;
            page_directory[i].A = 0;
            page_directory[i].D = 0;
            page_directory[i].PS = 0;
            page_directory[i].G = 0;
            page_directory[i].AVAIAL = 0;
            page_directory[i].PAT = (temp >> 12) & 0x1;
            page_directory[i].reserved = (temp >> 13) & 0x1FF;
            page_directory[i].Base_address = temp >> 22;
        }
        // else, just filled with 0
        else{
            page_directory[i].P = 0;
            page_directory[i].RW = 0;
            page_directory[i].US = 0;
            page_directory[i].PWT = 0;
            page_directory[i].PCD = 0;
            page_directory[i].A = 0;
            page_directory[i].D = 0;
            page_directory[i].PS = 0;
            page_directory[i].G = 0;
            page_directory[i].AVAIAL = 0;
            page_directory[i].PAT = 0;
            page_directory[i].reserved = 0;
            page_directory[i].Base_address = 0;
            continue;
        }

    }

    // --------------------------Fill the video Page Table (for kernel)--------------------
    for(i = 0; i < PAGE_TABLE_SIZE; i++){
        // points to the start of vedio memory
        if(i == VIDEO_MEMORY_INDEX){
            page_table0[i].P = 1;
            page_table0[i].RW = 1;
            page_table0[i].US = 0;
            page_table0[i].PWT = 0;
            page_table0[i].PCD = 0;
            page_table0[i].A = 0;
            page_table0[i].D = 0;
            page_table0[i].PAT = 0;
            page_table0[i].G = 0;
            page_table0[i].AVAIAL = 0;
            page_table0[i].Base_address = VIDEO_MEMORY_INDEX;
            continue;
        }

        // else, just filled with 0
        else{
            page_table0[i].P = 0;
            page_table0[i].RW = 0;
            page_table0[i].US = 0;
            page_table0[i].PWT = 0;
            page_table0[i].PCD = 0;
            page_table0[i].A = 0;
            page_table0[i].D = 0;
            page_table0[i].PAT = 0;
            page_table0[i].G = 0;
            page_table0[i].AVAIAL = 0;
            page_table0[i].Base_address = 0;
            continue;
        }
    }

    // --------------------------Fill the video Page Table (for user)--------------------
    for(i = 0; i < PAGE_TABLE_SIZE; i++){
        // points to the start of vedio memory
        if(i == VIDEO_MEMORY_INDEX){
            video_page_table0[i].P = 1;
            video_page_table0[i].RW = 1;
            video_page_table0[i].US = 1;
            video_page_table0[i].PWT = 0;
            video_page_table0[i].PCD = 0;
            video_page_table0[i].A = 0;
            video_page_table0[i].D = 0;
            video_page_table0[i].PAT = 0;
            video_page_table0[i].G = 0;
            video_page_table0[i].AVAIAL = 0;
            video_page_table0[i].Base_address = VIDEO_MEMORY_INDEX;
            continue;
        }

            // else, just filled with 0
        else{
            video_page_table0[i].P = 0;
            video_page_table0[i].RW = 0;
            video_page_table0[i].US = 0;
            video_page_table0[i].PWT = 0;
            video_page_table0[i].PCD = 0;
            video_page_table0[i].A = 0;
            video_page_table0[i].D = 0;
            video_page_table0[i].PAT = 0;
            video_page_table0[i].G = 0;
            video_page_table0[i].AVAIAL = 0;
            video_page_table0[i].Base_address = 0;
            continue;
        }
    }

    // --------------------------Fill the audio Page Table (for kernel)--------------------
    for(i = 0; i < PAGE_TABLE_SIZE; i++){
        // points to the start of vedio memory
        if(i >= AUDIO_BUF_PT_INDEX && i < AUDIO_BUF_PT_INDEX + AUDIO_BUF_PT_INDEX_LEN){
            audio_page_table0[i].P = 1;
            audio_page_table0[i].RW = 1;
            audio_page_table0[i].US = 0;
            audio_page_table0[i].PWT = 0;
            audio_page_table0[i].PCD = 0;
            audio_page_table0[i].A = 0;
            audio_page_table0[i].D = 0;
            audio_page_table0[i].PAT = 0;
            audio_page_table0[i].G = 0;
            audio_page_table0[i].AVAIAL = 0;
            audio_page_table0[i].Base_address = (AUDIO_BUF_ADDR << 12) | i;  // one-to-one map
            continue;
        }

            // else, just filled with 0
        else{
            audio_page_table0[i].P = 0;
            audio_page_table0[i].RW = 0;
            audio_page_table0[i].US = 0;
            audio_page_table0[i].PWT = 0;
            audio_page_table0[i].PCD = 0;
            audio_page_table0[i].A = 0;
            audio_page_table0[i].D = 0;
            audio_page_table0[i].PAT = 0;
            audio_page_table0[i].G = 0;
            audio_page_table0[i].AVAIAL = 0;
            audio_page_table0[i].Base_address = 0;
            continue;
        }
    }
}

