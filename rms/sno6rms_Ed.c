#include<18F452.h>
#fuses HS,NOWDT,NOPROTECT,NOLVP
#use delay(clock=20000000)
#use rs232(baud=9600, xmit=PIN_C6, rcv=PIN_C7, stream=com_A)  // Jumpers: 8 to 11, 7 to 12
#use rs232(baud=9600, xmit=PIN_C5, invert, stream=com_B)
#include <stdlib.h>
#include <math.h>
#define NOP #asm NOP #endasm
#define ONE_WIRE_PIN PIN_A0
#define ONE_WIRE_PIN2 PIN_A1

set_tris_e(0x00); //all outputs
set_tris_A(0x02); //tristate data2 input

static int wl;
int1 trigbit, fdelay, datbit, datbit2;
int adnum, x, sec, pattern, vpat, trigdelay, fibredelay, pulseA, pulseB, clear, Mtrig;
int lnum, LEDgroup, widthh, widthl, heighth, heightl, hdel, usdelh, usdell, delayedpulse;//periodmult, periodunit;
int board, board0, board1, board2, board3, board4, board5, board6;
int board7, board8, board9, board10, board11, board12, board13, board14;
int Lgroup0, Lgroup1, Lgroup2, Lgroup3, Lgroup4, Lgroup5, Lgroup6,Lgroup7,Lgroup8,Lgroup9,Lgroup10,Lgroup11,Lgroup12,Lgroup13,Lgroup14;
int periodmult=1;
int periodunit=1;
int tsh,tsl,cmd,addrs1,addrs2;
int command=0b00000011; //write and update DAC
int address1=0b00000000; //select DACA
int address2=0b00000001; //select DACB
int32 j, z, ctr, preset;
long N, num, numhi, numlo, width, height, pheight, dtoabit, pperiod, measuredph, measuredph2, extavedph, count;
float avrge, total, temp;  //temp & value were LONG
float value = 0.0;
float result=0.0;
signed int16 temp3, temp3b;
int temp1, temp1b, temp2, temp2b;
int8 busy=0;
int16 therm2, i, therm;
int32 sum, a;
float total_sq, measuredrms, extrms;

void groupselect(void);
void function1select(void);
void triggerdelay(void);
void fibrelengthdelay(void);
void run(void);
void continuous(void);
void numberhi(void);
void numberlo(void);
void secdelay(void);
void secenable(void);
void secdisable(void);
void singleselect(void);
void multipleselect(void);
void zero(void);
void load(void);
void loaddel(void);
void loadheight(void);
void loadmulti(void);
void heighthi(void);
void heightlo(void);
void widthhi(void);
void widthlo(void);
void loadwidth(void);
void pulseheightout(void);
void avpulseheight(void);
void pulseheightout2(void);
void upperaveph(void);
void averagedph(void);
void upperaverage(void);
void cleardisp(void);
void usdelay(void);
void wl1select(void);
void secpulse(void);
void wl2select(void);
void wl3select(void);
void extrigenable(void);
void extrigdisable(void);
void extrigger(void);
void onewire_reset();
void onewire_write(int8 data);
int onewire_read();
int db18b20select(void);
void ds18b20_read(int);
void onewire_reset2();
void onewire_write2(int8 data2);
int onewire_read2();
int db18b20select2(void);
void ds18b20_read2(int);
void extaverage(void);
void extaverage2(void);

output_high(PIN_D6);   //set clock line high
output_low(PIN_C0);   //Disable external trigger
output_low(PIN_D2);   //Disable internal pulsing


void main(void)
{
output_low(PIN_A4); // Green LED ON
delay_ms(500);
output_high(PIN_A4); // Green LED OFF
delay_ms(500);
output_low(PIN_A4); // Green LED ON
delay_ms(500);
output_high(PIN_A4); // Green LED OFF
delay_ms(500);

delay_ms(900); // setup time for display

sec=0; // disable secondary pulse

fprintf(com_B,"%C",254); //Control code follows:
fprintf(com_B,"%C",1);    //clear display
delay_ms(20);

loop:
x=getch();
if (x=='a') continuous();
if (x=='b') extaverage();
if (x=='p') extaverage2();
if (x=='c') cleardisp();
if (x=='d') triggerdelay();
if (x=='e') fibrelengthdelay();
if (x=='g') run();
if (x=='h') secdelay();
if (x=='i') secenable();
if (x=='j') secdisable();
if (x=='C') zero();
if (x=='H') numberhi();
if (x=='G') numberlo();
if (x=='I') singleselect();
if (x=='J') multipleselect();
if (x=='N') load();
if (x=='D') loaddel();
if (x=='E') loadmulti();
if (x=='L') heighthi();
if (x=='M') heightlo();
if (x=='P') loadheight();
if (x=='Q') widthhi();
if (x=='R') widthlo();
if (x=='S') loadwidth();
if (x=='r') pulseheightout();
if (x=='s') averagedph();
if (x=='m') pulseheightout2();
if (x=='U') upperaverage();
if (x=='u') usdelay();
if (x=='A') extrigenable();
if (x=='B') extrigdisable();
if (x=='F') extrigger();
if (x=='n') therm=db18b20select();
if (x=='T') ds18b20_read(therm);
if (x=='f') therm2=db18b20select2();
if (x=='k') ds18b20_read2(therm2);
goto loop;
}

//Group Select -selects tray
//**************************************************
void groupselect(void)
{
 //pattern=0b00000001;
// pattern=pattern << (LEDgroup-1);

for(i=1;i<=8;i++)
 {
 vpat=bit_test(pattern,0); //look at LSBit
 output_bit(PIN_E0, vpat); //present data to 74LS595

 output_high(PIN_E1);   //clock data
 output_low(PIN_E1);
 pattern=pattern >> 1; //move next into LSBit
 }
 output_high(PIN_E2);  //latch data
 output_low(PIN_E2);
 //return;
}

//*************************************************
void function1select(void)
{
printf("Pattern?\r\n");
gets(pattern);
output_b(pattern); //output pattern to PortB
fprintf(com_A,"k\r\n");
}

//Trigger Delay
//**************************************************
void triggerdelay(void)
{
trigdelay=getch();

output_high(PIN_D1); //enable DS1020-50
//output_low(PIN_D6);  //Set clock low

for(i=1;i<=8;i++)
 {
 output_low(PIN_D6);  //Set clock low

 trigbit=bit_test(trigdelay,7); //look at MSBit
 output_bit(PIN_D5, trigbit); //present data to Delay line
 output_high(PIN_D6);   //clock data
 trigdelay=trigdelay<< 1; //move next into MSBit
}
output_low(PIN_D1);   //activate new delay
fprintf(com_A,"d");
}

// Individual fibre length delays
//**************************************************
void fibrelengthdelay(void)
{
fibredelay=getch();

output_B(0b00000011); //Enable global set delay
//also requires an LED select to produce ENABLE signal

for(i=1;i<=8;i++)
{
output_low(PIN_D6);  //Set clock low
fdelay=bit_test(fibredelay,7); //look at MSBit
output_bit(PIN_D5, fdelay); //present data to Delay line
output_high(PIN_D6);   //clock data
fibredelay=fibredelay<< 1; //move next into MSBit
}
output_B(0b00000000); //Latch data
fprintf(com_A,"e");
}

//***************************************************
void numberhi(void)
{
numhi=getch();
fprintf(com_A,"H");
}
//***************************************************
void numberlo(void)
{
numlo=getch();
fprintf(com_A,"G");
}

//***************************************************
void singleselect(void)
{
lnum=getch();

if (lnum>=1 && lnum<=8)
{
board=1;
LEDgroup=lnum;
fprintf(com_A,"B1");
}
if (lnum>=9 && lnum<=16)
{
board=2;
LEDgroup=lnum-8;
fprintf(com_A,"B2");
}
if (lnum>=17 && lnum<=24)
{
board=3;
LEDgroup=lnum-16;
fprintf(com_A,"B3");
}
if (lnum>=25 && lnum<=32)
{
board=4;
LEDgroup=lnum-24;
fprintf(com_A,"B4");
}
if (lnum>=33 && lnum<=40)
{
board=5;
LEDgroup=lnum-32;
fprintf(com_A,"B5");
}
if (lnum>=41 && lnum<=48)
{
board=6;
LEDgroup=lnum-40;
fprintf(com_A,"B6");
}
if (lnum>=49 && lnum<=56)
{
board=7;
LEDgroup=lnum-48;
fprintf(com_A,"B7");
}
if (lnum>=57 && lnum<=64)
{
board=8;
LEDgroup=lnum-56;
fprintf(com_A,"B8");
}
if (lnum>=65 && lnum<=72)
{
board=9;
LEDgroup=lnum-64;
fprintf(com_A,"B9");
}
if (lnum>=73 && lnum<=80)
{
board=10;
LEDgroup=lnum-72;
fprintf(com_A,"B10");
}
if (lnum>=81 && lnum<=88)
{
board=11;
LEDgroup=lnum-80;
fprintf(com_A,"B11");
}
if (lnum>=89 && lnum<=96)
{
board=12;
LEDgroup=lnum-88;
fprintf(com_A,"B12");
}
if (lnum>=97 && lnum<=104)
{
board=13;
LEDgroup=lnum-96;
fprintf(com_A,"B13");
}
if (lnum>=105 && lnum<=112)
{
board=14;
LEDgroup=lnum-104;
fprintf(com_A,"B14");
}

 pattern=0b00000001;
 pattern=pattern << (LEDgroup-1);
}

//***************************************************
//Multipleselect- All boards initialised to zero after a clear
//Bits representing LEDs are added to each board with each select
//until the next clear.

void multipleselect(void)
{
lnum=getch();

if (lnum>=1 && lnum<=8)
  {
  Lgroup0=1;
  Lgroup0=Lgroup0 << lnum-1;
  Lgroup1=Lgroup1 | Lgroup0;  //add latest LED
  //LEDgroup=0b01010101;
  //output_B(board1);
  fprintf(com_A,"B1");
  }
if (lnum>=9 && lnum<=16)
  {
  Lgroup0=1;
  Lgroup0=Lgroup0 << (lnum-9);
  Lgroup2=Lgroup2 | Lgroup0;
  //output_B(board2);
  fprintf(com_A,"B2");
  }
if (lnum>=17 && lnum<=24)
  {
  Lgroup0=1;
  Lgroup0=Lgroup0 << (lnum-17);
  Lgroup3=Lgroup3 | Lgroup0;
  //output_B(board3);
  fprintf(com_A,"B3");
  }
if (lnum>=25 && lnum<=32)
  {
  Lgroup0=1;
  Lgroup0=Lgroup0 << (lnum-25);
  Lgroup4=Lgroup4 | Lgroup0;
 // output_B(board4);
  fprintf(com_A,"B4");
  }
if (lnum>=33 && lnum<=40)
  {
  Lgroup0=1;
  Lgroup0=Lgroup0 << (lnum-33);
  Lgroup5=Lgroup5 | Lgroup0;
 // output_B(board5);
  fprintf(com_A,"B5");
  }
if (lnum>=41 && lnum<=48)
  {
  Lgroup0=1;
  Lgroup0=Lgroup0 << (lnum-41);
  Lgroup6=Lgroup6 | Lgroup0;
 // output_B(board6);
  fprintf(com_A,"B6");
  }
if (lnum>=49 && lnum<=56)
{
  Lgroup0=1;
  Lgroup0=Lgroup0 << (lnum-49);
  Lgroup7=Lgroup7 | Lgroup0;
  //output_B(board7);
  fprintf(com_A,"B7");
  }
if (lnum>=57 && lnum<=64)
{
  Lgroup0=1;
  Lgroup0=Lgroup0 << (lnum-57);
  Lgroup8=Lgroup8 | Lgroup0;
  //output_B(board7);
  fprintf(com_A,"B8");
  }
if (lnum>=65 && lnum<=72)
{
  Lgroup0=1;
  Lgroup0=Lgroup0 << (lnum-65);
  Lgroup9=Lgroup9 | Lgroup0;
  //output_B(board7);
  fprintf(com_A,"B9");
  }
if (lnum>=73 && lnum<=80)
{
  Lgroup0=1;
  Lgroup0=Lgroup0 << (lnum-73);
  Lgroup10=Lgroup10 | Lgroup0;
  //output_B(board7);
  fprintf(com_A,"B10");
  }
if (lnum>=81 && lnum<=88)
{
  Lgroup0=1;
  Lgroup0=Lgroup0 << (lnum-81);
  Lgroup11=Lgroup11 | Lgroup0;
  //output_B(board7);
  fprintf(com_A,"B11");
  }
if (lnum>=89 && lnum<=96)
{
  Lgroup0=1;
  Lgroup0=Lgroup0 << (lnum-89);
  Lgroup12=Lgroup12 | Lgroup0;
  //output_B(board7);
  fprintf(com_A,"B12");
  }
if (lnum>=97 && lnum<=104)
{
  Lgroup0=1;
  Lgroup0=Lgroup0 << (lnum-97);
  Lgroup13=Lgroup13 | Lgroup0;
  //output_B(board7);
  fprintf(com_A,"B13");
  }
 if (lnum>=105 && lnum<=112)
{
  Lgroup0=1;
  Lgroup0=Lgroup0 << (lnum-105);
  Lgroup14=Lgroup14 | Lgroup0;
  //output_B(board7);
  fprintf(com_A,"B14");
  }
//pattern=Lgroup1;
}

//***************************************************
void zero(void)
{
board1=0; //Clear all selected LEDs from variables
board2=0;
board3=0;
board4=0;
board5=0;
board6=0;
board7=0;
board8=0;
board9=0;
board10=0;
board11=0;
board12=0;
board13=0;
board14=0;
Lgroup1=0;
Lgroup2=0;
Lgroup3=0;
Lgroup4=0;
Lgroup5=0;
Lgroup6=0;
Lgroup7=0;
Lgroup8=0;
Lgroup9=0;
Lgroup10=0;
Lgroup11=0;
Lgroup12=0;
Lgroup13=0;
Lgroup14=0;
output_B(0b00000111); //CLEAR all LED latches
//delay_us(10);
output_B(0b00000000);
fprintf(com_A,"C");
}

//***************************************************
void load(void)
{

//pattern=board1;
//?????pattern=LEDgroup;
groupselect();
if (board==1)
{
output_B(0b00001000); //select board 1
output_B(0b00001001); //Latch with 'NORMAL'
output_B(0b00000000);
}
//Add more boards here!!!
if (board==2)
{
output_B(0b00010000); //select board 2
output_B(0b00010001); //Latch with 'NORMAL'
output_B(0b00000000);
}
if (board==3)
{
output_B(0b00011000); //select board 3
output_B(0b00011001); //Latch with 'NORMAL'
output_B(0b00000000);
}
if (board==4)
{
output_B(0b00100000); //select board 4
output_B(0b00100001); //Latch with 'NORMAL'
output_B(0b00000000);
}
if (board==5)
{
output_B(0b00101000); //select board 5
output_B(0b00101001); //Latch with 'NORMAL'
output_B(0b00000000);
}
if (board==6)
{
output_B(0b00110000); //select board 6
output_B(0b00110001); //Latch with 'NORMAL'
output_B(0b00000000);
}
if (board==7)
{
output_B(0b00111000); //select board 7
output_B(0b00111001); //Latch with 'NORMAL'
output_B(0b00000000);
}
if (board==8)
{
output_high(PIN_C1); //select board 8
output_low(PIN_C2);
output_low(PIN_C3);
output_B(0b00000001); //Latch with 'NORMAL'
output_B(0b00000000);
output_low(PIN_C1);
output_low(PIN_C2);
output_low(PIN_C3);
}
if (board==9)
{
output_low(PIN_C1);
output_high(PIN_C2); //select board 9
output_low(PIN_C3);
output_B(0b00000001); //Latch with 'NORMAL'
output_B(0b00000000);
output_low(PIN_C1);
output_low(PIN_C2);
output_low(PIN_C3);
}
if (board==10)
{
output_high(PIN_C1); //select board 10
output_high(PIN_C2);
output_low(PIN_C3);
output_B(0b00000001); //Latch with 'NORMAL'
output_B(0b00000000);
output_low(PIN_C1);
output_low(PIN_C2);
output_low(PIN_C3);
}
if (board==11)
{
output_low(PIN_C1);
output_low(PIN_C2);
output_high(PIN_C3); //select board 11
output_B(0b00000001); //Latch with 'NORMAL'
output_B(0b00000000);
output_low(PIN_C1);
output_low(PIN_C2);
output_low(PIN_C3);
}
if (board==12)
{
output_high(PIN_C1); //select board 12
output_low(PIN_C2);
output_high(PIN_C3);
output_B(0b00000001); //Latch with 'NORMAL'
output_B(0b00000000);
output_low(PIN_C1);
output_low(PIN_C2);
output_low(PIN_C3);
}
if (board==13)
{
output_low(PIN_C1);
output_high(PIN_C2);//select board 13
output_high(PIN_C3);
output_B(0b00000001); //Latch with 'NORMAL'
output_B(0b00000000);
output_low(PIN_C1);
output_low(PIN_C2);
output_low(PIN_C3);
}
if (board==14)
{
output_high(PIN_C1); //select board 14
output_high(PIN_C2);
output_high(PIN_C3);
output_B(0b00000001); //Latch with 'NORMAL'
output_B(0b00000000);
output_low(PIN_C1);
output_low(PIN_C2);
output_low(PIN_C3);
}

fprintf(com_A,"N");

fprintf(com_B,"%C",254); //Control code follows:
fprintf(com_B,"%C",128);

if (lnum < 10)
fprintf(com_B,"CH0%U",lnum);

if (lnum >= 10)
fprintf(com_B,"CH%U",lnum);
}

//***************************************************
void loaddel(void)
{
pattern=Lgroup1;
groupselect();
output_B(0b00001000); //select board 1
output_B(0b00001010); //Latch with 'DELAYED'
output_B(0b00000000);
pattern=Lgroup2;
groupselect();
output_B(0b00010000); //select board 2
output_B(0b00010010); //Latch with 'DELAYED'
output_B(0b00000000);
pattern=Lgroup3;
groupselect();
output_B(0b00011000); //select board 3
output_B(0b00011010); //Latch with 'DELAYED'
output_B(0b00000000);
pattern=Lgroup4;
groupselect();
output_B(0b00100000); //select board 4
output_B(0b00100010); //Latch with 'DELAYED'
output_B(0b00000000);
pattern=Lgroup5;
groupselect();
output_B(0b00101000); //select board 5
output_B(0b00101010); //Latch with 'DELAYED'
output_B(0b00000000);
pattern=Lgroup6;
groupselect();
output_B(0b00110000); //select board 6
output_B(0b00110010); //Latch with 'DELAYED'
output_B(0b00000000);
pattern=Lgroup7;
groupselect();
output_B(0b00111000); //select board 7
output_B(0b00111010); //Latch with 'DELAYED'
output_B(0b00000000);
pattern=Lgroup8;
groupselect();
output_high(PIN_C1); //select board 8
output_B(0b00000010); //Latch with 'DELAYED'
output_B(0b00000000);
output_low(PIN_C1);
pattern=Lgroup9;
groupselect();
output_high(PIN_C2); //select board 9
output_B(0b00000010); //Latch with 'DELAYED'
output_B(0b00000000);
output_low(PIN_C2);
pattern=Lgroup10;
groupselect();
output_high(PIN_C1); //select board 10
output_high(PIN_C2);
output_B(0b00000010); //Latch with 'DELAYED'
output_B(0b00000000);
output_low(PIN_C1);
output_low(PIN_C2);
pattern=Lgroup11;
groupselect();
output_high(PIN_C3); //select board 11
output_B(0b00000010); //Latch with 'DELAYED'
output_B(0b00000000);
output_low(PIN_C3);
pattern=Lgroup12;
groupselect();
output_high(PIN_C1); //select board 12
output_high(PIN_C3);
output_B(0b00000010); //Latch with 'DELAYED'
output_B(0b00000000);
output_low(PIN_C1);
output_low(PIN_C3);
pattern=Lgroup13;
groupselect();
output_high(PIN_C2); //select board 13
output_high(PIN_C3);
output_B(0b00000010); //Latch with 'DELAYED'
output_B(0b00000000);
output_low(PIN_C2);
output_low(PIN_C3);
pattern=Lgroup14;
groupselect();
output_high(PIN_C1); //select board 14
output_high(PIN_C2);
output_high(PIN_C3);
output_B(0b00000010); //Latch with 'DELAYED'
output_B(0b00000000);
output_low(PIN_C1);
output_low(PIN_C2);
output_low(PIN_C3);
fprintf(com_A,"D");
}

//***************************************************
void loadmulti(void)
{
pattern=Lgroup1;
groupselect();
output_B(0b00001000); //select board 1
output_B(0b00001001); //Latch with 'NORMAL'
output_B(0b00000000);
pattern=Lgroup2;
groupselect();
output_B(0b00010000); //select board 2
output_B(0b00010001); //Latch with 'NORMAL'
output_B(0b00000000);
pattern=Lgroup3;
groupselect();
output_B(0b00011000); //select board 3
output_B(0b00011001); //Latch with 'NORMAL'
output_B(0b00000000);
pattern=Lgroup4;
groupselect();
output_B(0b00100000); //select board 4
output_B(0b00100001); //Latch with 'NORMAL'
output_B(0b00000000);
pattern=Lgroup5;
groupselect();
output_B(0b00101000); //select board 5
output_B(0b00101001); //Latch with 'NORMAL'
output_B(0b00000000);
pattern=Lgroup6;
groupselect();
output_B(0b00110000); //select board 6
output_B(0b00110001); //Latch with 'NORMAL'
output_B(0b00000000);
pattern=Lgroup7;
groupselect();
output_B(0b00111000); //select board 7
output_B(0b00111001); //Latch with 'NORMAL'
output_B(0b00000000);
pattern=Lgroup8;
groupselect();
output_high(PIN_C1); //select board 8
output_B(0b00000001); //Latch with 'NORMAL'
output_B(0b00000000);
output_low(PIN_C1);
pattern=Lgroup9;
groupselect();
output_high(PIN_C2); //select board 9
output_B(0b00000001); //Latch with 'NORMAL'
output_B(0b00000000);
output_low(PIN_C2);
pattern=Lgroup10;
groupselect();
output_high(PIN_C1); //select board 10
output_high(PIN_C2);
output_B(0b00000001); //Latch with 'NORMAL'
output_B(0b00000000);
output_low(PIN_C1);
output_low(PIN_C2);
pattern=Lgroup11;
groupselect();
output_high(PIN_C3); //select board 11
output_B(0b00000001); //Latch with 'NORMAL'
output_B(0b00000000);
output_low(PIN_C3);
pattern=Lgroup12;
groupselect();
output_high(PIN_C1); //select board 12
output_high(PIN_C3);
output_B(0b00000001); //Latch with 'NORMAL'
output_B(0b00000000);
output_low(PIN_C1);
output_low(PIN_C3);
pattern=Lgroup13;
groupselect();
output_high(PIN_C2); //select board 13
output_high(PIN_C3);
output_B(0b00000001); //Latch with 'NORMAL'
output_B(0b00000000);
output_low(PIN_C2);
output_low(PIN_C3);
pattern=Lgroup14;
groupselect();
output_high(PIN_C1); //select board 14
output_high(PIN_C2);
output_high(PIN_C3);
output_B(0b00000001); //Latch with 'NORMAL'
output_B(0b00000000);
output_low(PIN_C1);
output_low(PIN_C2);
output_low(PIN_C3);
fprintf(com_A,"E");
}

//***********************************************
void heighthi(void)
{
heighth=getch();
fprintf(com_A,"L");
}
//***********************************************
void heightlo(void)
{
heightl=getch();
fprintf(com_A,"M");
}

//**********************************************
void loadheight(void)
{
height=(heighth*256)+heightl;
pheight=height;
cmd=command;
addrs1=address1;
addrs2=address2;
//height=0b11111111111111;

output_B(0b00000100); //set CS/LOAD low on D/A
output_low(PIN_D6);  //Set clock low

for(i=1;i<=4;i++)  //send command to write and update
{
 dtoabit=bit_test(cmd,3); //look at Bit3
 output_bit(PIN_D5, dtoabit);   //present data to LTC2612
 output_high(PIN_D6);   //clock data
 output_low(PIN_D6);
 cmd=cmd << 1; //move next into Bit 3
}
for(i=1;i<=4;i++)  //address DAC1
{
 dtoabit=bit_test(addrs1,3); //look at Bit3
 output_bit(PIN_D5, dtoabit);   //present data to LTC2612
 output_high(PIN_D6);   //clock data
 output_low(PIN_D6);
 addrs1=addrs1 << 1; //move next into Bit3
}
for(i=1;i<=14;i++) //send data
{
 dtoabit=bit_test(height,13); //look at Bit13
 output_bit(PIN_D5, dtoabit);   //present data to LTC2612
 output_high(PIN_D6);   //clock data
 output_low(PIN_D6);
 height=height << 1; //move next into Bit13
}
 output_high(PIN_D6);   //clock don't care data
 output_low(PIN_D6);
 output_high(PIN_D6);   //clock don't care data
 output_low(PIN_D6);    // giving a total of 24 clock cycles

output_B(0b00000000); //set CS/LOAD high to disable D/A(s)

fprintf(com_A,"P");

fprintf(com_B,"%C",254); //Control code follows:
fprintf(com_B,"%C",138);
delay_ms(10);
fprintf(com_B,"H     ");
delay_ms(10);
fprintf(com_B,"%C",254); //Control code follows:
fprintf(com_B,"%C",138);
delay_ms(10);
fprintf(com_B,"H%lu",pheight);
delay_ms(10);
}

//***********************************************
void widthhi(void)
{
widthh=getch();
fprintf(com_A,"Q");
}
//***********************************************
void widthlo(void)
{
widthl=getch();
fprintf(com_A,"R");
}

//**********************************************
void loadwidth(void)
{
width=(widthh*256)+widthl;
pheight=height;
cmd=command;
addrs1=address1;
addrs2=address2;
//height=0b11111111111111;

output_B(0b00000100); //set CS/LOAD low on D/A
output_low(PIN_D6);  //Set clock low

for(i=1;i<=4;i++)  //send command to write and update
{
 dtoabit=bit_test(cmd,3); //look at Bit3
 output_bit(PIN_D5, dtoabit);   //present data to LTC2612
 output_high(PIN_D6);   //clock data
 output_low(PIN_D6);
 cmd=cmd << 1; //move next into Bit 3
}
for(i=1;i<=4;i++)  //address DAC1
{
 dtoabit=bit_test(addrs2,3); //look at Bit3
 output_bit(PIN_D5, dtoabit);   //present data to LTC2612
 output_high(PIN_D6);   //clock data
 output_low(PIN_D6);
 addrs2=addrs2 << 1; //move next into Bit3
}
for(i=1;i<=14;i++) //send data
{
 dtoabit=bit_test(width,13); //look at Bit13
 output_bit(PIN_D5, dtoabit);   //present data to LTC2612
 output_high(PIN_D6);   //clock data
 output_low(PIN_D6);
 width=width << 1; //move next into Bit13
}
 output_high(PIN_D6);   //clock don't care data
 output_low(PIN_D6);
 output_high(PIN_D6);   //clock don't care data
 output_low(PIN_D6);    // giving a total of 24 clock cycles

output_B(0b00000000); //set CS/LOAD high to disable D/A(s)

fprintf(com_A,"S");

fprintf(com_B,"%C",254); //Control code follows:
fprintf(com_B,"%C",138);
delay_ms(10);
fprintf(com_B,"H     ");
delay_ms(10);
fprintf(com_B,"%C",254); //Control code follows:
fprintf(com_B,"%C",138);
delay_ms(10);
fprintf(com_B,"H%lu",pheight);
delay_ms(10);
}

//***********************************************
void pulseheightout(void)
{
output_B(0b00000110); //Enable selected A/D for read
output_B(0b00000000);
for(i=1;i<=16;i++)
{
 output_low(PIN_D6); //clock LOW
 output_high(PIN_D6);   //clock data
 datbit=input(PIN_D7); //read data
 measuredph= measuredph << 1; // move pulse height data left
 measuredph=measuredph + datbit;
}
 output_low(PIN_D6); //Extra clock pulse to return DATA OUT line to zero ***
 output_high(PIN_D6);   //***
fprintf(com_A,"%lu",measuredph);
}

//***********************************************

void avpulseheight(void)
{
output_B(0b00000110); //Enable selected A/D for read
output_B(0b00000000);

for(i=1;i<=16;i++)
{
 output_low(PIN_D6); //clock LOW
 output_high(PIN_D6);   //clock data
 datbit=input(PIN_D7); //read data
 measuredph= measuredph << 1; // move pulse height data left
 measuredph=measuredph + datbit;
}
 output_low(PIN_D6); //Extra clock pulse to return DATA OUT line to zero ***
 output_high(PIN_D6);   //***
}

//***********************************************
void pulseheightout2(void)
{
output_B(0b00000110); //Enable selected A/D for read
output_B(0b00000000);
for(i=1;i<=16;i++)
{
 output_low(PIN_D6); //clock LOW
 output_high(PIN_D6);   //clock data
 datbit=input(PIN_A2); //read data
 measuredph= measuredph << 1; // move pulse height data left
 measuredph=measuredph + datbit;
}
 output_low(PIN_D6); //Extra clock pulse to return DATA OUT line to zero ***
 output_high(PIN_D6);   //***
fprintf(com_A,"%lu",measuredph);
}

//***********************************************
void upperaveph(void)
{
output_B(0b00000110); //Enable selected A/D for read
output_B(0b00000000);

for(i=1;i<=16;i++)
{
 output_low(PIN_D6); //clock LOW
 output_high(PIN_D6);   //clock data
 datbit=input(PIN_A2); //read data
 measuredph= measuredph << 1; // move pulse height data left
 measuredph=measuredph + datbit;
}
 output_low(PIN_D6); //Extra clock pulse to return DATA OUT line to zero ***
 output_high(PIN_D6);   //***
}

//**********************************************
void cleardisp(void)
{
//fprintf(com_B,"%C",254);
//fprintf(com_B,"%C",0); //cancel scrolling
//delay_ms(10);
fprintf(com_B,"%C",254);
fprintf(com_B,"%C",1); //clear display
delay_ms(20);
}

//**********************************************
void usdelay(void)
{
usdelh=getch();
usdell=getch();
fprintf(com_A,"u");
}

//**********************************************
void run(void)
{
num=numhi*numlo;

pulseA=0b00000100; //Normal pulse
pulseB=0b00001000; //Delayed pulse
Mtrig=0b00000001;
clear=0;

hdel=usdelh;  //save copy of delay loop counter
++hdel;
output_low(PIN_A4); // Pulsing LED ON
fprintf(com_A,"g");
do
{
output_B(0b00000110); //Enable A/D ready to sample
output_B(0b00000000);

//output_d(pulseA);
output_d(Mtrig);//output NIM or TTL main trigger
delay_us(1);
output_d(pulseA);// output main pulse
output_d(clear);

if (sec==1) secpulse();

delay_us(usdell);
delay_us(usdell);
delay_us(usdell);
delay_us(usdell);
delay_ms(usdelh);

if(kbhit()) goto Jump;
num=--num;
} while(num>=1);

jump:
fprintf(com_A,"K");

output_high(PIN_A4); // Pulsing LED OFF
}

//********************************************************
void secdelay(void) //read time delay to secondary pulse(s)
{
delayedpulse=getch();
if (delayedpulse<6) // correction to delay
delayedpulse=6;
//else
//delayedpulse=delayedpulse;
fprintf(com_A,"h");
}

//********************************************************
void secenable(void) //enable secondary pulse(s)
{
sec=1;
fprintf(com_A,"i");
}

//********************************************************
void secdisable(void) //disable secondary pulse(s)
{
sec=0;
fprintf(com_A,"j");
}

//********************************************************
void secpulse(void) //do secondary pulse(s)
{
delay_us(delayedpulse-5);
output_d(pulseB);  // output delayed pulse(s)
output_d(clear);
}

//Enable external trigger
//*************************************************
void extrigenable(void)
{
output_low(PIN_D2);   //Disable internal pulsing
output_high(PIN_C0); //
output_low(PIN_D0);   //Enable external trigger to use OR gate
fprintf(com_A,"A");
}

//Disable external trigger
//*************************************************
void extrigdisable(void)
{
output_low(PIN_D2);   //Disable internal pulsing
output_low(PIN_C0); //
fprintf(com_A,"B");
}

//external trigger sequence - preset number of pulses before calling
//*************************************************
void extrigger(void)
{
count=0;
num=numhi*numlo;
output_low(PIN_A4);
fprintf(com_A,"F");
output_low(PIN_D0);  //Enable external trigger to use OR gate
output_high(PIN_C0); //Enable external trigger

do
{
  do
  {
   if(kbhit()) goto distrig; //***NOP;
  }while (input(PIN_D4)==0); //wait for trigger pulse start
  do
  {
   NOP;
  }while (input(PIN_D4)==1); //wait for trigger pulse end
  count++;
}while (count<num);

distrig: //***
output_low(PIN_C0); //Disable external trigger
output_high(PIN_A4);
fprintf(com_A,"K");
}

//**********************************************
void continuous(void)
{
pulseA=0b00000100; //Normal pulse
pulseB=0b00001000; //Delayed pulse
Mtrig=0b00000001;
clear=0;

output_low(PIN_A4); // Pulsing LED ON
fprintf(com_A,"a");
do
{
output_B(0b00000110); //Enable A/D ready to sample
output_B(0b00000000);

output_d(Mtrig);//output NIM or TTL main trigger
delay_us(1);
output_d(pulseA);// output main pulse
output_d(clear);

if (sec==1) secpulse();

delay_us(usdell);
delay_us(usdell);
delay_us(usdell);
delay_us(usdell);
delay_ms(usdelh);

if(kbhit()) goto escape;

} while(TRUE);

escape:
fprintf(com_A,"K");

output_high(PIN_A4); // Pulsing LED OFF
}

//**********************************************

void averagedph(void)
{
num=numhi*numlo;

pulseA=0b00000100;
Mtrig=0b00000001;
clear=0;

N=0; //N=number of readings
total=0; //summed pulse heights
total_sq=0; //sum of the squares
output_low(PIN_A4); // Green LED ON
do
{
output_B(0b00000110); //Enable A/D ready to sample
output_B(0b00000000);

output_d(Mtrig);//output NIM or TTL main trigger
delay_us(1);
output_d(pulseA);
output_d(clear);
delay_us(1);
delay_us(9); //wait for delayed convert signal***NEW
avpulseheight(); //get pulse height reading

total=total+measuredph;
total_sq += measuredph*measuredph;

delay_us(usdell);  //4 times usdell @ 250 gives 1mS delay
delay_us(usdell);
delay_us(usdell);
delay_us(usdell);
delay_ms(usdelh);

N=N+1; //inc divisor for average
if(kbhit()) goto Jmp2;
num=--num;
} while(num>=1);

Jmp2:
measuredph=total/N;   //calc average pulse height
measuredrms=sqrt(total_sq/N);

fprintf(com_A,"%lu\t%f",measuredph,measuredrms);

output_high(PIN_A4); // Green LED OFF
}

//**********************************************
void upperaverage(void)
{
num=numhi*numlo;

pulseA=0b00000100;
Mtrig=0b00000001;
clear=0;

N=0; //N=number of readings
total=0; //summed pulse heights
total_sq=0; //sum of the squares
output_low(PIN_A4); // Green LED ON
do
{
output_B(0b00000110); //Enable A/D ready to sample
output_B(0b00000000);

output_d(Mtrig);//output NIM or TTL main trigger
delay_us(1);
output_d(pulseA);
output_d(clear);
delay_us(1);
delay_us(9); //wait for delayed convert signal***NEW
upperaveph(); //get pulse height reading

total=total+measuredph;
total_sq+=measuredph*measuredph;

delay_us(usdell);  //4 times usdell @ 250 gives 1mS delay
delay_us(usdell);
delay_us(usdell);
delay_us(usdell);
delay_ms(usdelh);

N=N+1; //inc divisor for average
if(kbhit()) goto Jmp5;
num=--num;
} while(num>=1);

Jmp5:
measuredph=total/N;   //calc average pulse height
measuredrms=sqrt(total_sq/N);

fprintf(com_A,"%lu\t%f",measuredph,measuredrms);

output_high(PIN_A4); // Green LED OFF
}
//**********************************************

void extaverage(void)
{
num=numhi*numlo;
clear=0;

//count=0;
N=0; //N=number of readings
total=0; //summed pulse heights
total_sq=0; //sum of the squares
output_low(PIN_A4); // Green LED ON
//fprintf(com_A,"F");
output_low(PIN_D0);  //Enable external trigger to use OR gate
output_high(PIN_C0); //Enable external trigger
do
{
output_B(0b00000110); //Enable A/D ready to sample
output_B(0b00000000);
   do
   {
   if(kbhit()) goto distrig2;
   } while(input(PIN_D4)==0);  //wait for trigger pulse start

   delay_us(1);
   delay_us(1);
   delay_us(9);
   avpulseheight(); //get pulse height reading
   total=total+measuredph;
   total_sq+=measuredph*measuredph;
   N=N+1; //inc divisor for average

   do
   {
   if(kbhit()) goto distrig2;
   } while(input(PIN_D4)==1);  //wait for trigger pulse end
   //count++;
   num=--num;
//}while(count<num);
}while(num>=1);

distrig2: //***
output_low(PIN_C0); //Disable external trigger
output_high(PIN_A4); // Green LED OFF
//fprintf(com_A,"K");
extavedph=total/N;   //calc average pulse height
extrms=sqrt(total_sq/N);

fprintf(com_A,"%lu\t%f",extavedph,extrms);
}


//**********************************************
void extaverage2(void)
{
num=numhi*numlo;
clear=0;

//count=0;
N=0; //N=number of readings
total=0; //summed pulse heights
total_sq=0;
output_low(PIN_A4); // Green LED ON
//fprintf(com_A,"F");
output_low(PIN_D0);  //Enable external trigger to use OR gate
output_high(PIN_C0); //Enable external trigger
do
{
output_B(0b00000110); //Enable A/D ready to sample
output_B(0b00000000);
   do
   {
   if(kbhit()) goto distrig3;
   } while(input(PIN_D4)==0);  //wait for trigger pulse start

   delay_us(1);
   delay_us(1);
   delay_us(9);
   upperaveph(); //get pulse height reading
   total=total+measuredph;
   total_sq=measuredph*measuredph;
   N=N+1; //inc divisor for average

   do
   {
   if(kbhit()) goto distrig3;
   } while(input(PIN_D4)==1);  //wait for trigger pulse end
   //count++;
   num=--num;
//}while(count<num);
}while(num>=1);


distrig3: //***
output_low(PIN_C0); //Disable external trigger
output_high(PIN_A4); // Green LED OFF
//fprintf(com_A,"K");
extavedph=total/N;   //calc average pulse height
extrms=sqrt(total_sq/N);

fprintf(com_A,"%lu\t%f",extavedph,extrms);
}

/*-----------------------------------------------------------------------------
/*****************************************************************************
 * onewire_reset()
 * Description: Initiates the one wire bus.
 */

void onewire_reset() {
    output_low(ONE_WIRE_PIN);       // pull the bus low for reset
    delay_us(500);
    output_float(ONE_WIRE_PIN);     // float the bus high
    delay_us(500);                  // wait-out remaining initialisation window
    output_float(ONE_WIRE_PIN);
}

/**************************************************************************
 * onewire_write(int8 data)
 * Arguments: a byte of data.
 * Description: writes a byte of data to the device.
 */

void onewire_write(int8 data) {
    int8 count;

    for(count = 0; count < 8; ++count) {
        output_low(ONE_WIRE_PIN);
        delay_us(2);                // pull 1-wire low to initiate write time-slot.
        output_bit(ONE_WIRE_PIN, shift_right(&data, 1, 0)); // set output bit on 1-wire
        delay_us(60);               // wait until end of write slot.
        output_float(ONE_WIRE_PIN); // set 1-wire high again,
        delay_us(2);                // for more than 1us minimum.
    }
}

/************************************************************************
 * onewire_read()
 * Description: reads and returns a byte of data from the device.
 */

int onewire_read() {
    int count, data;

    for(count = 0; count < 8; ++count) {
       output_low(ONE_WIRE_PIN);
       delay_us(2);                // pull 1-wire low to initiate read time-slot.
        output_float(ONE_WIRE_PIN); // now let 1-wire float high,
       delay_us(8);                // let device state stabilise,
        shift_right(&data, 1, input(ONE_WIRE_PIN)); // and load result.
        delay_us(120);              // wait until end of read slot.
    }
    return data;
}
//********************************************************
int db18b20select(void) //select sensor
{
int16 sens, therm;
//n=getch();
tsh=getch();
tsl=getch();
n=(tsh*10)+tsl;

if (n==1) sens=0;
if (n==2) sens=8;
if (n==3) sens=16;
if (n==4) sens=24;
if (n==5) sens=32;
if (n==6) sens=40;
if (n==7) sens=48;
if (n==8) sens=56;
if (n==9) sens=64;
if (n==10) sens=72;
if (n==11) sens=80;
if (n==12) sens=88;
if (n==13) sens=96;
if (n==14) sens=104;
if (n==15) sens=112;
if (n==16) sens=120;
if (n==17) sens=128;
if (n==18) sens=136;
if (n==19) sens=144;
if (n==20) sens=152;
if (n==21) sens=160;
if (n==22) sens=168;
if (n==23) sens=176;
if (n==24) sens=184;
if (n==25) sens=192;
if (n==26) sens=200;
if (n==27) sens=208;
if (n==28) sens=216;
if (n==29) sens=224;
if (n==30) sens=232;
if (n==31) sens=240;
if (n==32) sens=248;


//if (n=='1') fprintf(com_A,"@");
fprintf(com_A,"%lu",sens);
fprintf(com_B,"%lu", sens);
therm=sens;
//return (sens);
return (therm);
}
/***************************************************************************
 * ds18b20_read()
 * reads the ds18x20 device on the 1-wire bus and returns the temperature*/

//float ds18b20_read(int therm)

void ds18b20_read(therm)
{
/******LOOKUP TABLE- DS18B20 unique codes*******/
 //remember to change romcode[n] = number of sensors x 8

// char romcode[256]={0x28,0x52,0x34,0x62,0x04,0x00,0x00,0xCC,//B1.1 Lab test
//char romcode[256]={0x28,0x9E,0xCC,0x62,0x04,0x00,0x00,0xB6,//B1.1 Eds test
//char romcode[256]={0x28,0xBE,0xC9,0x8F,0x03,0x00,0x00,0x7C,//B1.1 True no 1
char romcode[256]={0x28,0xBE,0xC9,0x8F,0x03,0x00,0x00,0x7C,//B1.1
0x28,0xF3,0x86,0x20,0x04,0x00,0x00,0xFC,//B1.2
0x28,0x46,0xDB,0x8F,0x03,0x00,0x00,0x50,//B1.3
0x28,0x43,0x3F,0x62,0x04,0x00,0x00,0xD0,//B1.4
0x28,0x98,0x8B,0x8F,0x03,0x00,0x00,0x12,//B1.5
0x28,0xF4,0x89,0x8F,0x03,0x00,0x00,0x2F,//B2.6
0x28,0xCA,0x91,0x62,0x04,0x00,0x00,0x3D,//B2.7
0x28,0x96,0xDB,0x8F,0x03,0x00,0x00,0x94,//B2.8
0x28,0x16,0xBF,0x62,0x04,0x00,0x00,0xDE,//B2.9
0x28,0xC6,0xDA,0x8F,0x03,0x00,0x00,0x77,//B2.10
0x28,0x1B,0xCF,0x8F,0x03,0x00,0x00,0x57,//B3.11
0x28,0x52,0x6F,0x62,0x04,0x00,0x00,0x29,//B3.12
0x28,0x4C,0x8F,0x8F,0x03,0x00,0x00,0x15,//B3.13
0x28,0xD8,0xFF,0x62,0x04,0x00,0x00,0xBB,//B3.14
0x28,0x00,0xAC,0x8F,0x03,0x00,0x00,0xAB,//B3.15
0x28,0xED,0xD0,0x8F,0x03,0x00,0x00,0x84,//B4.16
0x28,0x7F,0xF6,0x62,0x04,0x00,0x00,0x91,//B4.17
0x28,0x06,0xB0,0x8F,0x03,0x00,0x00,0x44,//B4.18
0x28,0x20,0x1C,0x62,0x04,0x00,0x00,0xFC,//B4.19
0x28,0xF6,0xA6,0x8F,0x03,0x00,0x00,0xD6,//B4.20
0x28,0x44,0xA6,0x8F,0x03,0x00,0x00,0xBF,//B5.21
0x28,0xBB,0x72,0x62,0x04,0x00,0x00,0x06,//B5.22
0x28,0x17,0xA9,0x8F,0x03,0x00,0x00,0xA7,//B5.23
0x28,0x42,0x48,0x62,0x04,0x00,0x00,0xDB,//B5.24
0x28,0xB0,0x93,0x8F,0x03,0x00,0x00,0x47,//B5.25
0x28,0xBA,0xB7,0x8F,0x03,0x00,0x00,0x6F,//B6.26
0x28,0x11,0xE8,0x62,0x04,0x00,0x00,0x9F,//B6.27
0x28,0xA5,0xD5,0x8F,0x03,0x00,0x00,0x82,//B6.28
0x28,0xCA,0x41,0x62,0x04,0x00,0x00,0x63,//B6.29
0x28,0xDA,0xCA,0x8F,0x03,0x00,0x00,0x2D,//B6.30
0x28,0x77,0xD4,0x8F,0x03,0x00,0x00,0xE5,//B13.31a
0x28,0x87,0xF8,0x62,0x04,0x00,0x00,0xE0};//B13.32a


//char romcode[16]={0x28,0xFF,0xD9,0x8F,0x03,0x00,0x00,0x42,
//0x28,0xBF,0xAB,0x8F,0x03,0x00,0x00,0xD9};

     onewire_reset();
     onewire_write(0x55);  //Match to lookup romcode
   for (i=0;i<8;i++)
     {
      onewire_write(romcode[i+ therm]);
     }

    onewire_write(0x44);            //Start temperature conversion  ****
     while(busy == 0)                //Wait while busy (bus is low)
        busy = onewire_read();

   onewire_reset();
    onewire_write(0x55);  //Match to lookup romcode
   for (i=0;i<8;i++)
      {
       onewire_write(romcode[i+ therm]);
      }

    onewire_write(0xBE);            //Read scratchpad
    temp1 = onewire_read();
    temp2 = onewire_read();
    temp3 = make16(temp2, temp1);

    result = (float) temp3 / 16.0;    //Calculation for DS18B20

    delay_ms(200);

    fprintf(com_A,"%f",result);   //temperature reading out to RS232 or USB

    fprintf(com_B,"%C",254);
    fprintf(com_B,"%C",1);   //Clear display
    delay_ms(200);  // extra delay gives time for CLEAR
    fprintf(com_B,"%f",result);

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/*****************************************************************************
 * onewire_reset()
 * Description: Initiates the one wire bus.
 */

void onewire_reset2() {
    output_low(ONE_WIRE_PIN2);       // pull the bus low for reset
    delay_us(500);
    output_float(ONE_WIRE_PIN2);     // float the bus high
    delay_us(500);                  // wait-out remaining initialisation window
    output_float(ONE_WIRE_PIN2);
}

/**************************************************************************
 * onewire_write(int8 data)
 * Arguments: a byte of data.
 * Description: writes a byte of data to the device.
 */

void onewire_write2(int8 data2) {
    int8 count;

    for(count = 0; count < 8; ++count) {
        output_low(ONE_WIRE_PIN2);
        delay_us(2);                // pull 1-wire low to initiate write time-slot.
        output_bit(ONE_WIRE_PIN2, shift_right(&data2, 1, 0)); // set output bit on 1-wire
        delay_us(60);               // wait until end of write slot.
        output_float(ONE_WIRE_PIN2); // set 1-wire high again,
        delay_us(2);                // for more than 1us minimum.
    }
}

/************************************************************************
 * onewire_read()
 * Description: reads and returns a byte of data from the device.
 */

int onewire_read2() {
    int count, data2;

    for(count = 0; count < 8; ++count) {
       output_low(ONE_WIRE_PIN2);
       delay_us(2);                // pull 1-wire low to initiate read time-slot.
        output_float(ONE_WIRE_PIN2); // now let 1-wire float high,
       delay_us(8);                // let device state stabilise,
        shift_right(&data2, 1, input(ONE_WIRE_PIN2)); // and load result.
        delay_us(120);              // wait until end of read slot.
    }
    return data2;
}
//********************************************************
int db18b20select2(void) //select sensor
{
int16 sens2, therm2;
//n=getch();
tsh=getch();
tsl=getch();
n=(tsh*10)+tsl;

if (n==1) sens2=0;
if (n==2) sens2=8;
if (n==3) sens2=16;
if (n==4) sens2=24;
if (n==5) sens2=32;
if (n==6) sens2=40;
if (n==7) sens2=48;
if (n==8) sens2=56;
if (n==9) sens2=64;
if (n==10) sens2=72;
if (n==11) sens2=80;
if (n==12) sens2=88;
if (n==13) sens2=96;
if (n==14) sens2=104;
if (n==15) sens2=112;
if (n==16) sens2=120;
if (n==17) sens2=128;
if (n==18) sens2=136;
if (n==19) sens2=144;
if (n==20) sens2=152;
if (n==21) sens2=160;
if (n==22) sens2=168;
if (n==23) sens2=176;
if (n==24) sens2=184;
if (n==25) sens2=192;
if (n==26) sens2=200;
if (n==27) sens2=208;
if (n==28) sens2=216;
if (n==29) sens2=224;
if (n==30) sens2=232;
if (n==31) sens2=240;
if (n==32) sens2=248;

//if (n=='1') fprintf(com_A,"@");
fprintf(com_A,"%lu",sens2);
fprintf(com_B,"%lu", sens2);
therm2=sens2;
//return (sens);
return (therm2);
}
/***************************************************************************
 * ds18b20_read()
 * reads the ds18x20 device on the 1-wire bus and returns the temperature*/

//float ds18b20_read(int therm)

void ds18b20_read2(therm2)
{
/******LOOKUP TABLE- DS18B20 unique codes*******/
 //remember to change romcode[n] = number of sensors x 8
//0x28,0xF4,0xA0,0x8F,0x03,0x00,0x00,0x24,//B7.1
char romcode2[256]={0x28,0xF4,0xA0,0x8F,0x03,0x00,0x00,0x24,//B7.1
0x28,0x1D,0x95,0x62,0x04,0x00,0x00,0x63,//B7.2
0x28,0xC8,0x97,0x8F,0x03,0x00,0x00,0x61,//B7.3
0x28,0x01,0xC1,0x62,0x04,0x00,0x00,0xCF,//B7.4
0x28,0x40,0x8D,0x8F,0x03,0x00,0x00,0xEB,//B7.5
0x28,0xC0,0xD6,0x8F,0x03,0x00,0x00,0xE4,//B8.6
0x28,0x4B,0xD0,0x62,0x04,0x00,0x00,0xC4,//B8.7
0x28,0x66,0xAA,0x8F,0x03,0x00,0x00,0x46,//B8.8
0x28,0x22,0xE6,0x61,0x04,0x00,0x00,0xBC,//B8.9
0x28,0x97,0x9A,0x8F,0x03,0x00,0x00,0x87,//B8.10
0x28,0x99,0x9A,0x8F,0x03,0x00,0x00,0x94,//B9.11
0x28,0x4B,0xC6,0x62,0x04,0x00,0x00,0x24,//B9.12
0x28,0xC2,0xD0,0x8F,0x03,0x00,0x00,0x16,//B9.13
0x28,0x3E,0xD9,0x62,0x04,0x00,0x00,0x44,//B9.14
0x28,0x87,0xD7,0x8F,0x03,0x00,0x00,0xD9,//B9.15
0x28,0xC2,0x99,0x8F,0x03,0x00,0x00,0x0C,//B10.16
0x28,0x7A,0xDD,0x62,0x04,0x00,0x00,0xF2,//B10.17
0x28,0xB0,0x9A,0x8F,0x03,0x00,0x00,0xB4,//B10.18
0x28,0x8D,0xF6,0x62,0x04,0x00,0x00,0x8D,//B10.19
0x28,0xB0,0x9A,0x8F,0x03,0x00,0x00,0xB4,//B10.20
0x28,0xA5,0x95,0x8F,0x03,0x00,0x00,0x6B,//B11.21
0x28,0xF2,0xB8,0x62,0x04,0x00,0x00,0x7A,//B11.22
0x28,0x8E,0xA7,0x8F,0x03,0x00,0x00,0x22,//B11.23
0x28,0xC1,0x2A,0x62,0x04,0x00,0x00,0xFA,//B11.24
0x28,0x75,0x96,0x8F,0x03,0x00,0x00,0xE1,//B11.25
0x28,0x8E,0xBE,0x8F,0x03,0x00,0x00,0xAD,//B12.26
0x28,0xAD,0x85,0x62,0x04,0x00,0x00,0x18,//B12.27
0x28,0x79,0xD4,0x8F,0x03,0x00,0x00,0xF6,//B12.28
0x28,0x18,0x68,0x62,0x04,0x00,0x00,0xC2,//B12.29
0x28,0x00,0xD3,0x8F,0x03,0x00,0x00,0xA9,//B12.30
0x28,0xF5,0x01,0x63,0x04,0x00,0x00,0xCC,//B13.31b
0x28,0xA1,0x93,0x8F,0x03,0x00,0x00,0x2B};//B13.32b
//0x28,0xB7,0xA4,0x8F,0x03,0x00,0x00,0x17, //box14 not used
//0x28,0xE1,0xCA,0x62,0x04,0x00,0x00,0x96,
//0x28,0xFF,0xD9,0x8F,0x03,0x00,0x00,0x42,
//0x28,0x3B,0xC4,0x62,0x04,0x00,0x00,0x3F,
//0x28,0xBF,0xAB,0x8F,0x03,0x00,0x00,0xD9};

//char romcode[16]={0x28,0xFF,0xD9,0x8F,0x03,0x00,0x00,0x42,
//0x28,0xBF,0xAB,0x8F,0x03,0x00,0x00,0xD9};

     onewire_reset2();
     onewire_write2(0x55);  //Match to lookup romcode
   for (i=0;i<8;i++)
     {
      onewire_write2(romcode2[i+ therm2]);
     }

    onewire_write2(0x44);            //Start temperature conversion  ****
     while(busy == 0)                //Wait while busy (bus is low)
        busy = onewire_read2();

   onewire_reset2();
    onewire_write2(0x55);  //Match to lookup romcode
   for (i=0;i<8;i++)
      {
       onewire_write2(romcode2[i+ therm2]);
      }

    onewire_write2(0xBE);            //Read scratchpad
    temp1b = onewire_read2();
    temp2b = onewire_read2();
    temp3b = make16(temp2b, temp1b);

    result = (float) temp3b / 16.0;    //Calculation for DS18B20

    delay_ms(200);

    fprintf(com_A,"%f",result);   //temperature reading out to RS232 or USB

    fprintf(com_B,"%C",254);
    fprintf(com_B,"%C",1);   //Clear display
    delay_ms(200);  // extra delay gives time for CLEAR
    fprintf(com_B,"%f",result);

}
