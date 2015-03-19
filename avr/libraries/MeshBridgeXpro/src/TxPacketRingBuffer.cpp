/**
* \file TxPacketRingBuffer.cpp
*
* \brief Aquila 802.15.4 Mesh implementation.
*
* Copyright (C) 2014, Rodrigo Méndez. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
*
* 3. The name of Makerlab may not be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
* 4. This software may only be redistributed and used in connection with a
*    Makerlab product.
*
* THIS SOFTWARE IS PROVIDED BY MAKERLAB "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
* EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL MAKERLAB BE LIABLE FOR
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
* OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
* ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
* Modification and other use of this code is subject to Makerlab's Limited
* License Agreement (license.txt).
*
*/


#include "TxPacketRingBuffer.h"

void TxRingBuffer_init(TxRingBuffer* buffer)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		buffer->head = buffer->buffer;
		buffer->tail = buffer->buffer;
		buffer->count = 0;
	}
}

bool TxRingBuffer_isFull(TxRingBuffer* buffer)
{
	uint8_t count;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		count = buffer->count;
	}
	return count == TXPACKETRINGBUFFER_SIZE;
}

bool TxRingBuffer_isEmpty(TxRingBuffer* buffer)
{
	uint8_t count;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		count = buffer->count;
	}
	return count == 0;
}

void TxRingBuffer_insert(TxRingBuffer* buffer, TxBufPacket* data)
{

	memcpy(buffer->head, data, sizeof(TxBufPacket));

	if (++buffer->head == &buffer->buffer[TXPACKETRINGBUFFER_SIZE])
	{
		buffer->head = buffer->buffer;
	}

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		buffer->count++;
	}
}

void TxRingBuffer_remove(TxRingBuffer* buffer, TxBufPacket* data)
{
	memcpy(data, buffer->tail, sizeof(TxBufPacket));

	if (++buffer->tail == &buffer->buffer[TXPACKETRINGBUFFER_SIZE])
	{
		buffer->tail = buffer->buffer;
	}


	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		buffer->count--;
	}
}
