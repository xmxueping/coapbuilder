/*
 *
 * Description:CoAP message builder
 *
 * Author: Liu Xueping <xmxueping@gmail.com>
 *
**/

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include "coap.h"
#include "coapbuilder.h"

#define E_INVALID_ARGUMENT -1
#define E_NO_ENOUGH_BUFFER -2

void CoapMessage_ReplaceType(unsigned char *buffer, unsigned char type)
{
    buffer[0] &= ~COAP_HEADER_TYPE_MASK;
    buffer[0] |= COAP_HEADER_TYPE_MASK & ((type)<<COAP_HEADER_TYPE_POSITION);
}

void CoapMessage_ReplaceId(unsigned char *buffer, unsigned int id)
{
    buffer[2] = (id >> 8) & 0xff;
    buffer[3] = (id     ) & 0xff;
}

void CoapMessage_ReplaceCode(unsigned char *buffer, unsigned char code)
{
    buffer[1] = code;
}

int CoapMessage_BuildHeader(unsigned char *buffer
                           ,unsigned int   message_id
                           ,unsigned char  type
                           ,unsigned char  code
                           ,unsigned char  token_length
                           ,unsigned char *token)
{
    if (token_length > 8)
    {
        return E_INVALID_ARGUMENT;
    }

    if (buffer != NULL)
    {
        buffer[0]  = COAP_HEADER_VERSION_MASK & (COAP_HEADER_VERSION)<<COAP_HEADER_VERSION_POSITION;
        buffer[0] |= COAP_HEADER_TYPE_MASK & (type)<<COAP_HEADER_TYPE_POSITION;
        buffer[0] |= COAP_HEADER_TOKEN_LEN_MASK & (token_length)<<COAP_HEADER_TOKEN_LEN_POSITION;
        buffer[1] = code;
        buffer[2] = (unsigned char) ((message_id)>>8);
        buffer[3] = (unsigned char) (message_id);

        if (token_length!=0)
        {
            memcpy(&buffer[4],token,token_length);
        }
    }

    return 4+token_length;
}

void CoapMessageBuilder_PrepareNoHeader(coap_message_builder_t *builder
                                       ,unsigned char *buffer
                                       ,unsigned int   size)
{
    builder->ptr  = buffer;
    builder->pos  = 0;
    builder->size = size;
    builder->option_number = 0;
}

int CoapMessageBuilder_Prepare(coap_message_builder_t *builder
                              ,unsigned char *buffer
                              ,unsigned int   size
                              ,unsigned int   message_id
                              ,unsigned char  type
                              ,unsigned char  code
                              ,unsigned char  token_length
                              ,unsigned char *token)
{
    if (token_length > 8)
    {
        return E_INVALID_ARGUMENT;
    }

    if (size < 4+token_length)
    {
        return E_NO_ENOUGH_BUFFER;
    }

    builder->ptr  = buffer;
    builder->pos  = 0;
    builder->size = size;
    builder->option_number = 0;

    buffer[0]  = COAP_HEADER_VERSION_MASK & (COAP_HEADER_VERSION)<<COAP_HEADER_VERSION_POSITION;
    buffer[0] |= COAP_HEADER_TYPE_MASK & (type)<<COAP_HEADER_TYPE_POSITION;
    buffer[0] |= COAP_HEADER_TOKEN_LEN_MASK & (token_length)<<COAP_HEADER_TOKEN_LEN_POSITION;
    buffer[1] = code;
    buffer[2] = (unsigned char) ((message_id)>>8);
    buffer[3] = (unsigned char) (message_id);

    builder->ptr += 4;
    builder->pos += 4;

    if (token_length!=0)
    {
        memcpy(&buffer[4],token,token_length);
        builder->ptr += token_length;
        builder->pos += token_length;
    }

    return (4+token_length);
}

static
int CoapMessageBuilder_PutChar(coap_message_builder_t *builder,int chr)
{
    //count the output length
    if (builder->ptr == NULL)
    {
        return 1;
    }

    //if (builder->pos+1 <= builder->size)
    if (builder->pos < builder->size)
    {
        *builder->ptr = (unsigned char)chr;
        builder->ptr += 1;
        builder->pos += 1;

        return 1;
    }

    return -1;//ENOMEM
}

static
int CoapMessageBuilder_PutArray(coap_message_builder_t *builder, void *ptr, unsigned int size)
{
    if (builder->ptr == NULL)
    {
        return (int)size;
    }

    if (builder->pos+size <= builder->size)
    {
        memcpy(builder->ptr, ptr, size);
        builder->ptr += size;
        builder->pos += size;

        return (int)size;
    }

    return -1;//ENOMEM
}

static
int CoapMessageBuilder_Advance(coap_message_builder_t *builder, unsigned int size)
{
    if (builder->ptr == NULL)
    {
        return (int)size;
    }

    assert(builder->pos+size>=size);
    assert(builder->pos+size>=builder->pos);
    if (builder->pos+size <= builder->size)
    {
        builder->ptr += size;
        builder->pos += size;

        return (int)size;
    }

    return -1;//ENOMEM
}

static
uint8_t coap_option_nibble(unsigned int value)
{
    if (value<13)
    {
        return value;
    }
    else if (value<=0xFF+13)
    {
        return 13;
    }
    else
    {
        return 14;
    }
}

#if 0
//from project libcoap
static
int coap_opt_setheader(unsigned char *opt
                      ,unsigned int maxlen
                      ,unsigned int delta
                      ,unsigned int length)
{
    size_t skip = 0;
    if (maxlen == 0)		/* need at least one byte */
    {
        return 0;
    }

    if (delta < 13)
    {
        opt[0] = delta << 4;
    } else
    if (delta < 270)
    {
        if (maxlen < 2)
        {
            //debug("insufficient space to encode option delta %d", delta);
            return 0;
        }

        opt[0] = 0xd0;
        opt[++skip] = delta - 13;
    } else
    {
        if (maxlen < 3)
        {
            //debug("insufficient space to encode option delta %d", delta);
            return 0;
        }

        opt[0] = 0xe0;
        opt[++skip] = ((delta - 269) >> 8) & 0xff;
        opt[++skip] = (delta - 269) & 0xff;
    }

    if (length < 13)
    {
        opt[0] |= length & 0x0f;
    } else
    if (length < 270)
    {
        if (maxlen < skip + 1)
        {
            //debug("insufficient space to encode option length %d", length);
            return 0;
        }

        opt[0] |= 0x0d;
        opt[++skip] = length - 13;
    } else
    {
        if (maxlen < skip + 2)
        {
            //debug("insufficient space to encode option delta %d", delta);
            return 0;
        }

        opt[0] |= 0x0e;
        opt[++skip] = ((length - 269) >> 8) & 0xff;
        opt[++skip] = (length - 269) & 0xff;
    }

    return skip + 1;
}
#endif

//port from contiki/apps/er-coap-13.c
static
int CoapMessageBuilder_SetOptionHeader(coap_message_builder_t *builder
                                      ,unsigned int delta
                                      ,unsigned int length)
{
    int written = 0;
    unsigned int *x;

    if (CoapMessageBuilder_PutChar(builder, coap_option_nibble(delta)<<4 | coap_option_nibble(length))<0)
    {
        return E_NO_ENOUGH_BUFFER;
    }

    /* avoids code duplication without function overhead */
    x = &delta;
    do
    {
        if (*x>268)
        {
            if (CoapMessageBuilder_PutChar(builder, (*x-269)>>8)<0)
            {
                return E_NO_ENOUGH_BUFFER;
            }
            ++written;

            if (CoapMessageBuilder_PutChar(builder, (*x-269))<0)
            {
                return E_NO_ENOUGH_BUFFER;
            }
            ++written;
        }
        else if (*x>12)
        {
            if (CoapMessageBuilder_PutChar(builder, (*x-13))<0)
            {
                return E_NO_ENOUGH_BUFFER;
            }
            ++written;
        }
    }
    while (x!=&length && (x=&length));

    //PRINTF("WRITTEN %u B opt header\n", written);

    //two nibbles
    return ++written;
}

int CoapMessageBuilder_AppendIntOption(coap_message_builder_t *builder
                                      ,unsigned int  number
                                      ,unsigned long value)
{
    int count;

    if (number < builder->option_number)
    {
      return E_INVALID_ARGUMENT;
    }

    count = 0;
    if (0xFF000000 & value) ++count;
    if (0xFFFF0000 & value) ++count;
    if (0xFFFFFF00 & value) ++count;
    if (0xFFFFFFFF & value) ++count;

    count = CoapMessageBuilder_SetOptionHeader(builder, number - builder->option_number, count);
    if (count<0)
    {
        return count;
    }

    if (0xFF000000 & value)
    {
        if (CoapMessageBuilder_PutChar(builder, (uint8_t) (value>>24))<0)
        {
            return E_NO_ENOUGH_BUFFER;
        }
        count++;
    }

    if (0xFFFF0000 & value)
    {
        if (CoapMessageBuilder_PutChar(builder, (uint8_t) (value>>16))<0)
        {
            return E_NO_ENOUGH_BUFFER;
        }
        count++;
    }

    if (0xFFFFFF00 & value)
    {
        if (CoapMessageBuilder_PutChar(builder, (uint8_t) (value>>8))<0)
        {
            return E_NO_ENOUGH_BUFFER;
        }
        count++;
    }

    if (0xFFFFFFFF & value)
    {
        if (CoapMessageBuilder_PutChar(builder, (uint8_t) (value))<0)
        {
            return E_NO_ENOUGH_BUFFER;
        }
        count++;
    }

    builder->option_number=number;
    return count;
}

int CoapMessageBuilder_AppendBinaryOption(coap_message_builder_t *builder
                                         ,unsigned int number
                                         ,void        *value
                                         ,unsigned int length)
{
    int ir;
    int count;

    if (number < builder->option_number)
    {
      return E_INVALID_ARGUMENT;
    }

    ir=0;
    count = CoapMessageBuilder_SetOptionHeader(builder, number - builder->option_number, length);
    if (count<0)
    {
        return E_NO_ENOUGH_BUFFER;
    }
    ir += count;

    count = CoapMessageBuilder_PutArray(builder, value, length);
    if (count<0)
    {
        return E_NO_ENOUGH_BUFFER;
    }
    ir += count;

    builder->option_number=number;
    return ir;
}

int CoapMessageBuilder_AppendStringOption(coap_message_builder_t *builder
                                         ,unsigned int number
                                         ,char        *value)
{
    int ir;
    int count;
    unsigned int length;

    if (number < builder->option_number)
    {
      return E_INVALID_ARGUMENT;
    }

    ir=0;
    length=strlen(value)+1;
    count = CoapMessageBuilder_SetOptionHeader(builder, number - builder->option_number, length);
    if (count<0)
    {
        return E_NO_ENOUGH_BUFFER;
    }
    ir += count;

    count = CoapMessageBuilder_PutArray(builder, value, length);
    if (count<0)
    {
        return E_NO_ENOUGH_BUFFER;
    }
    ir += count;

    builder->option_number=number;
    return ir;
}

int CoapMessageBuilder_Unprepare(coap_message_builder_t *builder
                                ,unsigned int *messageSize)
{
    int written = 0;

    if (0)
    {
        //payload marker or option header
        if (CoapMessageBuilder_PutChar(builder, 0xff)<0)
        {
            return E_NO_ENOUGH_BUFFER;
        }
        written++;
    }

    if (messageSize!=NULL)
    {
        *messageSize=builder->pos;
    }

    return written;
}

int CoapMessageBuilder_UnprepareEx(coap_message_builder_t *builder
                                  ,void                   *payload
                                  ,unsigned int            payloadSize
                                  ,unsigned int           *messageSize)
{
    int written = 0;

    if (payload!=NULL && payloadSize!=0)
    {
        //payload marker or options finished
        if (CoapMessageBuilder_PutChar(builder, 0xff)<0)
        {
            return E_NO_ENOUGH_BUFFER;
        }
        written++;

        if (CoapMessageBuilder_PutArray(builder, payload, payloadSize)<0)
        {
            return E_NO_ENOUGH_BUFFER;
        }
        written+=payloadSize;
    }

    if (messageSize!=NULL)
    {
        *messageSize=builder->pos;
    }

    return written;
}

int CoapMessageBuilder_UnprepareBegin(coap_message_builder_t *builder
                                     ,void                  **ptr
                                     ,unsigned int           *size)
{
    int written = 0;

    //payload marker or options finished
    if (CoapMessageBuilder_PutChar(builder, 0xff)<0)
    {
        return E_NO_ENOUGH_BUFFER;
    }
    written++;

    if (ptr!=NULL)
    {
      *ptr=builder->ptr;
    }
    if (size!=NULL)
    {
        *size=builder->size-builder->pos;
    }

    return written;
}

int CoapMessageBuilder_UnprepareEnd(coap_message_builder_t *builder
                                   ,unsigned int            payloadSize
                                   ,unsigned int           *messageSize)
{
    int written = 0;

    if (payloadSize!=0)
    {
        if (CoapMessageBuilder_Advance(builder, payloadSize)<0)
        {
            return E_NO_ENOUGH_BUFFER;
        }
        written+=payloadSize;
    }

    if (messageSize!=NULL)
    {
        *messageSize=builder->pos;
    }

    return written;
}
