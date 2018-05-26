/*
 * File: mbr.h
 * 
 * Copyright (c) 2017-2018 Sydney Erickson, John Davis
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef MBR_H
#define MBR_H

#include <main.h>
#include <kernel/storage/storage.h>

#define MBR_NO_OF_PARTITIONS    4

#define MBR_TYPE_FREE           0x00
#define MBR_TYPE_FAT12_L32MB    0x01
#define MBR_TYPE_FAT16          0x04
#define MBR_TYPE_EXTENDED       0x05
#define MBR_TYPE_FAT16B         0x06
#define MBR_TYPE_HPFS           0x07
#define MBR_TYPE_NTFS           0x07
#define MBR_TYPE_EXFAT          0x07
#define MBR_TYPE_FAT32_CHS      0x0B
#define MBR_TYPE_FAT32_LBA      0x0C
#define MBR_TYPE_FAT16_LBA      0x0E
#define MBR_TYPE_EXTENDED_LBA   0x0F

typedef struct {
    uint8_t Status;
    uint8_t StartHead;
    uint16_t StartCylinderSector;
    uint8_t Type;
    uint8_t EndHead;
    uint16_t EndCylinderSector;

    uint32_t StartLba;
    uint32_t CountLba;
} __attribute__((packed)) mbr_entry_t;

typedef struct {
    uint8_t Bootstrap1[218];

    uint16_t Zero1;
    uint8_t OriginalDrive;
    uint8_t Seconds;
    uint8_t Minutes;
    uint8_t Hours;

    uint8_t Bootstrap2[216];

    // Optional disk signature.
    uint32_t Signature1;
    uint16_t Signature2;

    // Partition table.
    mbr_entry_t Entries[MBR_NO_OF_PARTITIONS];

    // Boot signature.
    uint16_t BootSignature;
} __attribute__((packed)) mbr_t;

extern bool mbr_init(storage_device_t *storageDevice);

#endif