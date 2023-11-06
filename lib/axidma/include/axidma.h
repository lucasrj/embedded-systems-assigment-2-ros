#pragma once

#include <bits/stdint-uintn.h>
#include <stdint.h>
#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stddef.h>

#include <string>
#include <sstream>
#include <vector>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef uint64_t UINTPTR;


#if defined (__aarch64__) || defined (__arch64__)
#define UPPER_32_BITS(n) ((u32)(((n) >> 16) >> 16))
#else
#define UPPER_32_BITS(n) 0U
#endif
#define LOWER_32_BITS(n) ((u32)(n))

#define MM2S_CONTROL_REGISTER       0x00
#define MM2S_STATUS_REGISTER        0x04
#define MM2S_SRC_ADDRESS_REGISTER   0x18 
#define MM2S_SRC_ADDRESS_REGISTER_MSB 0X1C
#define MM2S_TRNSFR_LENGTH_REGISTER 0x28

#define S2MM_CONTROL_REGISTER       0x30
#define S2MM_STATUS_REGISTER        0x34
#define S2MM_DST_ADDRESS_REGISTER   0x48
#define S2MM_DST_ADDRESS_REGISTER_MSB 0X4C
#define S2MM_BUFF_LENGTH_REGISTER   0x58

#define IOC_IRQ_FLAG                1<<12
#define IDLE_FLAG                   1<<1

#define IRQ_ALL                     0x00007000

#define STATUS_HALTED               0x00000001
#define STATUS_IDLE                 0x00000002
#define STATUS_SG_INCLDED           0x00000008
#define STATUS_DMA_INTERNAL_ERR     0x00000010
#define STATUS_DMA_SLAVE_ERR        0x00000020
#define STATUS_DMA_DECODE_ERR       0x00000040
#define STATUS_SG_INTERNAL_ERR      0x00000100
#define STATUS_SG_SLAVE_ERR         0x00000200
#define STATUS_SG_DECODE_ERR        0x00000400
#define STATUS_IOC_IRQ              0x00001000
#define STATUS_DELAY_IRQ            0x00002000
#define STATUS_ERR_IRQ              0x00004000

#define HALT_DMA                    0x00000000
#define RUN_DMA                     0x00000001
#define RESET_DMA                   0x00000004
#define KEYHOLE_DMA                 0x00000008
#define CYCLIC_DMA                  0x00000010
#define ENABLE_IOC_IRQ              0x00001000
#define ENABLE_DELAY_IRQ            0x00002000
#define ENABLE_ERR_IRQ              0x00004000
#define ENABLE_ALL_IRQ              0x00007000

#define DMA_DIR_DMA_TO_DEVICE		0x00
#define DMA_DIR_DEVICE_TO_DMA		0x01

enum dma_status {
    DMA_HALTED = 0x00000001,
    DMA_IDLE = 0x00000002,
    DMA_SG_INCLDED = 0x00000008,
    DMA_INTERNAL_ERR = 0x00000010,
    DMA_SLAVE_ERR = 0x00000020,
    DMA_DECODE_ERR = 0x00000040,
    DMA_SG_INTERNAL_ERR = 0x00000100,
    DMA_SG_SLAVE_ERR = 0x00000200,
    DMA_SG_DECODE_ERR = 0x00000400,
    DMA_IOC_IRQ = 0x00001000,
    DMA_DELAY_IRQ = 0x00002000,
    DMA_ERR_IRQ = 0x00004000
};

class DMAStatus : public std::vector<dma_status> {
public:
    DMAStatus(uint32_t status) {
        if (status & DMA_HALTED) {
            push_back(DMA_HALTED);
        }
        if (status & DMA_IDLE) {
            push_back(DMA_IDLE);
        }
        if (status & DMA_SG_INCLDED) {
            push_back(DMA_SG_INCLDED);
        }
        if (status & DMA_INTERNAL_ERR) {
            push_back(DMA_INTERNAL_ERR);
        }
        if (status & DMA_SLAVE_ERR) {
            push_back(DMA_SLAVE_ERR);
        }
        if (status & DMA_DECODE_ERR) {
            push_back(DMA_DECODE_ERR);
        }
        if (status & DMA_SG_INTERNAL_ERR) {
            push_back(DMA_SG_INTERNAL_ERR);
        }
        if (status & DMA_SG_SLAVE_ERR) {
            push_back(DMA_SG_SLAVE_ERR);
        }
        if (status & DMA_SG_DECODE_ERR) {
            push_back(DMA_SG_DECODE_ERR);
        }
        if (status & DMA_IOC_IRQ) {
            push_back(DMA_IOC_IRQ);
        }
        if (status & DMA_DELAY_IRQ) {
            push_back(DMA_DELAY_IRQ);
        }
        if (status & DMA_ERR_IRQ) {
            push_back(DMA_ERR_IRQ);
        }
    }

    std::string to_string() {
        std::stringstream ss;
        for (auto status : *this) {
            switch (status) {
                case DMA_HALTED:
                    ss << "DMA_HALTED ";
                    break;
                case DMA_IDLE:
                    ss << "DMA_IDLE ";
                    break;
                case DMA_SG_INCLDED:
                    ss << "DMA_SG_INCLDED ";
                    break;
                case DMA_INTERNAL_ERR:
                    ss << "DMA_INTERNAL_ERR ";
                    break;
                case DMA_SLAVE_ERR:
                    ss << "DMA_SLAVE_ERR ";
                    break;
                case DMA_DECODE_ERR:
                    ss << "DMA_DECODE_ERR ";
                    break;
                case DMA_SG_INTERNAL_ERR:
                    ss << "DMA_SG_INTERNAL_ERR ";
                    break;
                case DMA_SG_SLAVE_ERR:
                    ss << "DMA_SG_SLAVE_ERR ";
                    break;
                case DMA_SG_DECODE_ERR:
                    ss << "DMA_SG_DECODE_ERR ";
                    break;
                case DMA_IOC_IRQ:
                    ss << "DMA_IOC_IRQ ";
                    break;
                case DMA_DELAY_IRQ:
                    ss << "DMA_DELAY_IRQ ";
                    break;
                case DMA_ERR_IRQ:
                    ss << "DMA_ERR_IRQ ";
                    break;
            }
        }

        return ss.str();
    }
};


class axiDma
{
public:
    axiDma(unsigned int uio_number, unsigned int uio_size);
    void Reset();
    int ResetIsDone();
    int Start();
    int Pause();
    int Resume();
    u32 Busy(int Direction);
    u32 SimpleTransfer(UINTPTR BuffAddr, u32 Length,
    			   int Direction);
    int SelectKeyHole(int Direction, int Select);
    int SelectCyclicMode(int Direction, int Select);
    void IntrDisable(uint32_t Mask, int Direction);
private:

    bool isStarted()
    {
      return ((readAXI(MM2S_STATUS_REGISTER)& STATUS_HALTED) ? 0:1) & ((readAXI(S2MM_STATUS_REGISTER)& STATUS_HALTED)? 0:1);
    }

    unsigned int writeAXI(uint32_t offset, uint32_t value) {
        uio_map_[offset>>2] = value;
        return 0;
    }

    unsigned int readAXI(uint32_t offset) {
        return uio_map_[offset>>2];
    }

    unsigned int getMM2SStatus() {

        return readAXI(MM2S_STATUS_REGISTER);

    }

    unsigned int getS2MMStatus() {

        return readAXI(S2MM_STATUS_REGISTER);
    }

    u32 *uio_map_;

}; 
