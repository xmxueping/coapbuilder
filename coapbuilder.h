#ifndef COAP_MESSAGE_BUILDER_H_
#define COAP_MESSAGE_BUILDER_H_

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct coap_message_builder_t
{
    unsigned char *ptr; //current output pointer
    unsigned int   pos; //current output position
    unsigned int   size;//total buffer size
    unsigned int   option_number;//current number of option outputed
}coap_message_builder_t,TCoapMessageBuilder;

//start coap packet builder
int CoapMessageBuilder_Prepare(coap_message_builder_t *builder
                              ,unsigned char          *buffer
                              ,unsigned int            size
                              ,unsigned int            id
                              ,unsigned char           type
                              ,unsigned char           code
                              ,unsigned char           token_length
                              ,unsigned char          *token);

//finish the builder
//<0: failed
//>0: succeeded,return bytes used
int CoapMessageBuilder_Unprepare(coap_message_builder_t *builder
                                ,unsigned int           *messageSize);

//finish the builder
//<0: failed
//>0: succeeded,return bytes used
int CoapMessageBuilder_UnprepareEx(coap_message_builder_t *builder
                                     ,void                *payload
                                     ,unsigned int         payloadSize
                                     ,unsigned int        *messageSize);

int CoapMessageBuilder_UnprepareBegin(coap_message_builder_t *builder
                                     ,void                  **ptr
                                     ,unsigned int           *size);
int CoapMessageBuilder_UnprepareEnd(coap_message_builder_t *builder
                                   ,unsigned int            payloadSize
                                   ,unsigned int           *messageSize);

//when appending options,the number of options should be incremental
//which is required by CoAP

//append a int option
//<0: failed
//>0: succeeded,return bytes used
int CoapMessageBuilder_AppendIntOption(coap_message_builder_t *builder
                                      ,unsigned int  number
                                      ,unsigned long value);

//append a binary option
//<0: failed
//>0: succeeded,return bytes used
int CoapMessageBuilder_AppendBinaryOption(coap_message_builder_t *builder
                                         ,unsigned int number
                                         ,void        *value
                                         ,unsigned int length);

//append a string option
//<0: failed
//>0: succeeded,return bytes used
int CoapMessageBuilder_AppendStringOption(coap_message_builder_t *builder
                                         ,unsigned int number
                                         ,char        *value);

void CoapMessage_ReplaceType(unsigned char *buffer, unsigned char type);
void CoapMessage_ReplaceId(unsigned char *buffer, unsigned int id);
void CoapMessage_ReplaceCode(unsigned char *buffer, unsigned char code);
int  CoapMessage_BuildHeader(unsigned char *buffer
                            ,unsigned int   id
                            ,unsigned char  type
                            ,unsigned char  code
                            ,unsigned char  token_length
                            ,unsigned char *token);

//start CoAP message builder without header & token
void CoapMessageBuilder_PrepareNoHeader(coap_message_builder_t *builder
                                       ,unsigned char *buffer
                                       ,unsigned int   size);

#ifdef __cplusplus
}
#endif

#endif //COAP_MESSAGE_BUILDER_H_
