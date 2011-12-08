#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <string.h>
#include "config.h"
#include "lcd.h"
#include <avr/interrupt.h>

//////////////////SERVICE////BOOTH/////////////////////////////

char amIFree;
char selfID[10];

//01 23 4 56789 012345 6

void printLCD(char f[],char s[])
{
    LCDClear();
    LCDWriteString(f);
    if (strlen(s))
    {
        LCDWriteStringXY(0,1,s);
    }
}


void printLCDFree() { printLCD("Free",selfID); }

void genPacket(char *c, char commandType) {
    int i;
    c[0] = STB_ALIAS_SERVER;
    c[1] = STB_SERVER_ID;
    c[2] = STB_ALIAS_SERVICEBOOTH;
    c[3] = SELF_ID;
    c[4] = commandType;
    for(i=5; i < 17; i++) c[i] = '-';
    c[17] = 0;
}

//01 23 45678 90123456
////////////////////process//////////////////////
void processData(char data[])
{
    //NOISE//
    //printf(data);
    if (data[0]==STB_ALIAS_SERVICEBOOTH && data[1]==SELF_ID)
    {
        //printf("YES i am that guy %c",SELF_ID);
        //printLCD("YES","WOW");
        //printData(data);
    }
    else return ;
    char s[20];
    switch(data[4])
    {
    case STB_SERVER_PING_SERVICE:
        //printf("in ping %c",SELF_ID);
        genPacket(s, amIFree);
        printf("%s", s);
        //printLCD(s, "line 2 :P");
        break;
    case STB_SERVER_NEW_USER:
        amIFree=STB_SERVICE_BUSY;
        genPacket(s, STB_SERVICE_TAKEN_USER);
        printf("%s", s);
        //printLCD("Serving", "");
        char type[10];
        switch(data[5])
        {
        case STB_CATEGORY_WITHDRAW:
            strcpy(type,"Withdraw");
            break;
        case STB_CATEGORY_DEPOSIT:
            strcpy(type,"Deposit");
            break;
        case STB_CATEGORY_HELP:
            strcpy(type,"help");
            break;
        default:
            strcpy(type,"Unknown");
        }
        char no[]={data[6],data[7],data[8],data[9],0};
        //printf(" i got %s and %s",type,no);
        printLCD(type,no);
        break;
    }


}

//========================================================
int SendCmd(char cmd,FILE *stream)
{
    UCSRB |= (1<<TXEN);
    while ((UCSRA & (1 << UDRE)) == 0x00) ;
    UCSRB &= ~(1<<TXB8);
    if ( cmd & 0x0100 ) UCSRB |= (1<<TXB8);
    UDR = cmd;
    UCSRB &= ~(1<<TXEN);
    return 0;
}

int RecieveCmd(FILE *stream)
{
    unsigned char ReceivedByte;
    while ((UCSRA & (1 << RXC)) == 0x00) {} // Do nothing until data have been recieved and is ready to be read from UDR
    ReceivedByte = UDR; // Fetch the recieved byte value into the variable "ByteReceived"
    return ReceivedByte;
}
//*******************************************/

ISR(USART_RXC_vect){ // Handler for RXD interrupt
    uchar in = UDR;
    char m[20];
    scanf("%s",m+1);
    m[0]=in;
    if (strlen(m)<17) return ;
    processData(m);
}

/////////////////BUTTON/////INT////////////////////

ISR(INT0_vect)
{
    amIFree=STB_SERVICE_FREE;
    printLCDFree();
}

///////////////////////////////////////////////////


int main(){

    DDRB = 0xFF;
    DDRC = 0xFF;
    DDRD &= ~1;
    DDRD |= 2;

    LCDInit(LS_NONE);
    LCDClear();
    GICR 	|= 	(1<<INT0);
    MCUCR 	|= 	(1<<ISC00) | (1<<ISC01);

    //SERIAL PORT
    UCSRB |= (1 << RXEN) | (1 << TXEN) | (1<<RXCIE);   // Turn on the transmission and reception circuitry
    UCSRC |= (1 << URSEL) | (1 << UCSZ0) | (1 << UCSZ1)  ; 		 // Use 8-bit character sizes
    UBRRH = (BAUD_PRESCALE >> 8); 	// Load upper 8-bits of the baud rate value into the high byte of the UBRR register
    UBRRL = BAUD_PRESCALE; 			// Load lower 8-bits of the baud rate value into the low byte of the UBRR register

    stdout = fdevopen(SendCmd,NULL);
    stdin =  fdevopen(NULL,RecieveCmd);

    //printf("Welcome!\n\rThis is develop by  Nayeem, Enzam, Shafiul ,Hira& Dipal.\n\rMachineStarted\n\r 0-INIT\n\r 1-SETvalue\n\r 2-STAT\n\r");
    //printf("Welcome! This transmitter will transmit nothing\n\rZZZZZ\n\r");

    LCDClear();
    sei();

    UCSRB &= ~(1<<TXEN);


    SELF_ID	= ((PINC&0b00111000)>>3) + '0';
    amIFree=STB_SERVICE_FREE;


    sprintf(selfID, "S000%c", SELF_ID);
    printLCDFree();

    while(1){}

    return 0;
}

