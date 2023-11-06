#include <axidma.h>
#include <sys/types.h>

axiDma::axiDma(unsigned int uio_number, unsigned int uio_size) {
  char device_file_name[20];
  sprintf(device_file_name, "/dev/uio%d", uio_number);

  int device_file;

  if ((device_file = open(device_file_name, O_RDWR | O_SYNC)) < 0) {
    std::stringstream ss;
    ss << device_file_name << " could not be opened";
    throw ss.str();
  }

  uio_map_ = (uint32_t *)mmap(NULL, uio_size, PROT_READ | PROT_WRITE,
                              MAP_SHARED, device_file, 0);

  if (uio_map_ == MAP_FAILED) {
    std::stringstream ss;
    ss << device_file_name << " could not be mapped";
    throw ss.str();
  }
}
void axiDma::Reset() {
  uint mm2s_statatus = getMM2SStatus();

  /* Reset
   */
  // if (InstancePtr->HasMm2S) {
  //   RegBase = InstancePtr->RegBase + XAXIDMA_TX_OFFSET;
  // } else {
  //   RegBase = InstancePtr->RegBase + XAXIDMA_RX_OFFSET;
  // }

  writeAXI(MM2S_CONTROL_REGISTER, RESET_DMA);

  /* Set TX/RX Channel state */

  // writeAXI(MM2S_CONTROL_REGISTER, DMA_HALTED);
  // writeAXI(S2MM_CONTROL_REGISTER, DMA_HALTED);
  // if (InstancePtr->HasMm2S) {
  //   TxRingPtr = XAxiDma_GetTxRing(InstancePtr);

  //   TxRingPtr->RunState = AXIDMA_CHANNEL_HALTED;
  // }

  // if (InstancePtr->HasS2Mm) {
  //   for (RingIndex = 0; RingIndex < InstancePtr->RxNumChannels; RingIndex++)
  //   {
  //     RxRingPtr = XAxiDma_GetRxIndexRing(InstancePtr, RingIndex);
  //     if (InstancePtr->HasS2Mm) {
  //       RxRingPtr->RunState = AXIDMA_CHANNEL_HALTED;
  //     }
  //   }
  // }
}
int axiDma::ResetIsDone() {
  u32 RegisterValue;
  uint mm2s_status = readAXI(MM2S_CONTROL_REGISTER);
  uint s2mm_status = readAXI(S2MM_CONTROL_REGISTER);

  /* Reset is done when the reset bit is low
   */
  if (mm2s_status & RESET_DMA) {

    return 0;
  }

  if (s2mm_status & RESET_DMA) {

    return 0;
  }

  return 1;
}
int axiDma::Start() {
  int Status;

  uint mm2s_status = readAXI(MM2S_CONTROL_REGISTER);
  uint s2mm_status = readAXI(S2MM_CONTROL_REGISTER);

  if (mm2s_status & STATUS_HALTED) {

    /* Start the channel
     */
    writeAXI(MM2S_CONTROL_REGISTER, readAXI(MM2S_CONTROL_REGISTER) | RUN_DMA);
  }

  if (!(mm2s_status & STATUS_HALTED)) {
    return 0;
  }

  /* Start the channel
   */

  writeAXI(S2MM_CONTROL_REGISTER, readAXI(S2MM_CONTROL_REGISTER) | RUN_DMA);
  return 0;
}
int axiDma::Pause() {

  /* If channel is halted, then we do not need to do anything
   */
  writeAXI(MM2S_CONTROL_REGISTER, readAXI(MM2S_CONTROL_REGISTER) | ~RUN_DMA);

  writeAXI(S2MM_CONTROL_REGISTER, readAXI(S2MM_CONTROL_REGISTER) | ~RUN_DMA);

  return 0;
}
int axiDma::Resume() {
  int Status;

  /* If the DMA engine is not running, start it. Start may fail.
   */
  if (!isStarted()) {
    Status = Start();

    if (Status != 0) {
      return Status;
    }
  }

  /* Mark the state to be not halted
   */

  return 0;
}
u32 axiDma::Busy(int Direction) {

  if (Direction == DMA_DIR_DMA_TO_DEVICE)
    return ((readAXI(MM2S_STATUS_REGISTER) & STATUS_IDLE) ? 0 : 1);
  else if (Direction == DMA_DIR_DEVICE_TO_DMA) {
    return ((readAXI(S2MM_STATUS_REGISTER) & STATUS_IDLE) ? 0 : 1);
  } else {
    return 1;
  }
}
u32 axiDma::SimpleTransfer(UINTPTR BuffAddr, u32 Length, int Direction) {
  u32 WordBits;
  int RingIndex = 0;

  if (Direction == DMA_DIR_DMA_TO_DEVICE) {
    // if ((Length < 1) || (Length > 64)) {
    //   return 15;
    // }

    /* If the engine is doing transfer, cannot submit
     */

    if (!(readAXI(MM2S_STATUS_REGISTER) & STATUS_HALTED)) {
      if (Busy(Direction)) {
        return 1;
      }
    }

    // WordBits = (u32)((((int)32) >> 3) - 1);

    // if ((BuffAddr & WordBits)) {

    //   return 15;
    // }

    writeAXI(MM2S_SRC_ADDRESS_REGISTER, BuffAddr);
    // writeAXI(MM2S_SRC_ADDRESS_REGISTER_MSB, UPPER_32_BITS(BuffAddr));

    writeAXI(MM2S_CONTROL_REGISTER, readAXI(MM2S_CONTROL_REGISTER) | RUN_DMA);

    /* Writing to the BTT register starts the transfer
     */
    writeAXI(MM2S_TRNSFR_LENGTH_REGISTER, Length);
  } else if (Direction == DMA_DIR_DEVICE_TO_DMA) {
    // if ((Length < 1) || (Length > 64)) {
    //   return 15;
    // }

    if (!(readAXI(S2MM_STATUS_REGISTER) & STATUS_HALTED)) {
      if (Busy(Direction)) {
        return 1;
      }
    }

    WordBits = (u32)((((int)32) >> 3) - 1);

    // if ((BuffAddr & WordBits)) {

    //   return 15;
    // }

    writeAXI(S2MM_DST_ADDRESS_REGISTER, BuffAddr);
    // writeAXI(S2MM_DST_ADDRESS_REGISTER_MSB, UPPER_32_BITS(BuffAddr));

    writeAXI(S2MM_CONTROL_REGISTER, readAXI(MM2S_CONTROL_REGISTER) | RUN_DMA);

    /* Writing to the BTT register starts the transfer
     */
    writeAXI(S2MM_BUFF_LENGTH_REGISTER, Length);

    // XAxiDma_WriteReg(InstancePtr->RxBdRing[RingIndex].ChanBase,
    //                  XAXIDMA_DESTADDR_OFFSET, LOWER_32_BITS(BuffAddr));
    // if (InstancePtr->AddrWidth > 32)
    //   XAxiDma_WriteReg(InstancePtr->RxBdRing[RingIndex].ChanBase,
    //                    XAXIDMA_DESTADDR_MSB_OFFSET, UPPER_32_BITS(BuffAddr));

    // XAxiDma_WriteReg(InstancePtr->RxBdRing[RingIndex].ChanBase,
    //                  XAXIDMA_CR_OFFSET,
    //                  XAxiDma_ReadReg(InstancePtr->RxBdRing[RingIndex].ChanBase,
    //                                  XAXIDMA_CR_OFFSET) |
    //                      XAXIDMA_CR_RUNSTOP_MASK);
    // /* Writing to the BTT register starts the transfer
    //  */
    // XAxiDma_WriteReg(InstancePtr->RxBdRing[RingIndex].ChanBase,
    //                  XAXIDMA_BUFFLEN_OFFSET, Length);
  }

  return 0;
}

int axiDma::SelectKeyHole(int Direction, int Select) {
  u32 Value;

  if (Direction == DMA_DIR_DMA_TO_DEVICE)
    Value = readAXI(MM2S_CONTROL_REGISTER);
  else if (Direction == DMA_DIR_DEVICE_TO_DMA) {
    Value = readAXI(S2MM_CONTROL_REGISTER);
  }

  if (Select) {
    Value |= KEYHOLE_DMA;
  } else {
    Value &= ~KEYHOLE_DMA;
  }

  if (Direction == DMA_DIR_DMA_TO_DEVICE)
    writeAXI(MM2S_CONTROL_REGISTER, Value);
  else if (Direction == DMA_DIR_DEVICE_TO_DMA) {
    writeAXI(S2MM_CONTROL_REGISTER, Value);
  }

  return 0;
}

int axiDma::SelectCyclicMode(int Direction, int Select) {
  u32 Value;

  if (Direction == DMA_DIR_DMA_TO_DEVICE)
    Value = readAXI(MM2S_CONTROL_REGISTER);
  else if (Direction == DMA_DIR_DEVICE_TO_DMA) {
    Value = readAXI(S2MM_CONTROL_REGISTER);
  }

  if (Select) {
    Value |= CYCLIC_DMA;
  } else {
    Value &= ~CYCLIC_DMA;
  }

  if (Direction == DMA_DIR_DMA_TO_DEVICE)
    writeAXI(MM2S_CONTROL_REGISTER, Value);
  else if (Direction == DMA_DIR_DEVICE_TO_DMA) {
    writeAXI(S2MM_CONTROL_REGISTER, Value);
  }

  return 0;
}

void axiDma::IntrDisable(uint32_t Mask, int Direction) {

  if (Direction == DMA_DIR_DMA_TO_DEVICE)
    writeAXI(MM2S_CONTROL_REGISTER, (readAXI(MM2S_CONTROL_REGISTER)& ~(Mask & IRQ_ALL)));
  else if (Direction == DMA_DIR_DEVICE_TO_DMA) {
    writeAXI(S2MM_CONTROL_REGISTER, (readAXI(S2MM_CONTROL_REGISTER)& ~(Mask & IRQ_ALL)));
  }
}