coapbuilder
===========

a simple CoAP message builder

example:
void test()
{
    int ir;
    char *payload;
    unsigned int size;
    char temp[256];
    char token[8]={1,2,3,4,5,6,7,8};
    coap_message_builder_t builder;
    if ((ir=CoapMessageBuilder_Prepare(&builder
                                      ,temp
                                      ,sizeof(temp)
                                      ,Coap_GetNextMessageId()
                                      ,COAP_MESSAGE_NON
                                      ,COAP_REQUEST_GET
                                      ,0
                                      ,token))>0)
    {
        {
            unsigned char temp[5];

            temp[0]=0x01;
            temp[1]=0x00;
            temp[2]=0x01;
            temp[3]=0x00;
            temp[4]=0x02;
            CoapMessageBuilder_AppendBinaryOption(&builder,COAP_OPTION_IF_MATCH,temp,5);
        }

        ir=CoapMessageBuilder_AppendStringOption(&builder,COAP_OPTION_URI_HOST,"github.com");
        ir=CoapMessageBuilder_AppendBinaryOption(&builder,COAP_OPTION_ETAG,token,sizeof(token));
        ir=CoapMessageBuilder_AppendIntOption(&builder,COAP_OPTION_URI_PORT,1234);
        ir=CoapMessageBuilder_AppendStringOption(&builder,COAP_OPTION_URI_PATH,"/xmxueping/contiki");
        ir=CoapMessageBuilder_AppendStringOption(&builder,COAP_OPTION_URI_QUERY,"name=xmxueping;password=1234567");

        ir=CoapMessageBuilder_UnprepareBegin(&builder, (void**)&payload, &size);
        if (ir>0)
        {
            if (size>=1)  payload[0]='h';
            if (size>=2)  payload[1]='e';
            if (size>=3)  payload[2]='l';
            if (size>=4)  payload[3]='l';
            if (size>=5)  payload[4]='o';
            if (size>=6)  payload[5]=' ';
            if (size>=7)  payload[6]='w';
            if (size>=8)  payload[7]='o';
            if (size>=9)  payload[8]='r';
            if (size>=10) payload[9]='l';
            if (size>=11) payload[10]='d';
            if (size>=12) payload[11]='\0';

            ir=CoapMessageBuilder_UnprepareEnd(&builder, size-1, &size);
          }
        }
    }
}
