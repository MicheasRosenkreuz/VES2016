#ifndef DBGUART_H_INCLUDED
#define DBGUART_H_INCLUDED

#define DBGSTRNL    "\r\n"

void DbgUartInit();
char *DbgPeriodicSvc();
uint8_t DbgCanGetPDO(char *buf);
void DbgCanSetPDO(char *buf);

void DbgFlush();
void DbgSendStr(const char *str);
void DbgSendStrNl(const char *str);
void DbgFormat ( const char * format, ... );


#endif /* DBGUART_H_INCLUDED */
