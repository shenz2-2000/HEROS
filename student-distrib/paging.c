//
// Created by BOURNE on 2021/3/20.
//
#include "x86_desc.h"

void fill_page(){
    // --------------------------Fill the Page Directory--------------------
    int i;
    for(i = 0; i < PAGE_DIRECTORY_SIZE; i++){
        // for the first entry (map to video memory)
        if(i == 0){
            uint32_t temp = (uint32_t) page_table0;
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
}

