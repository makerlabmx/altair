/**
* \file RingBuffer.cpp
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


#include "RingBuffer.h"

void RingBuffer_init(RingBuffer* buffer)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		buffer->head = buffer->buffer;
		buffer->tail = buffer->buffer;
		buffer->count = 0;
	}
}

bool RingBuffer_isFull(RingBuffer* buffer)
{
	uint8_t count;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		count = buffer->count;
	}
	return count == RINGBUFFER_SIZE;
}

bool RingBuffer_isEmpty(RingBuffer* buffer)
{
	uint8_t count;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		count = buffer->count;
	}
	return count == 0;
}

void RingBuffer_insert(RingBuffer* buffer, RingBufferData* data)
{

	buffer->head->size = data->size;
	memcpy(buffer->head->data, data->data, data->size);

	if (++buffer->head == &buffer->buffer[RINGBUFFER_SIZE])
	{
		buffer->head = buffer->buffer;
	}

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		buffer->count++;
	}
}

void RingBuffer_remove(RingBuffer* buffer, RingBufferData* data)
{
	data->size = buffer->tail->size;
	memcpy(data->data, buffer->tail->data, buffer->tail->size);

	if (++buffer->tail == &buffer->buffer[RINGBUFFER_SIZE])
	{
		buffer->tail = buffer->buffer;
	}


	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		buffer->count--;
	}
}
