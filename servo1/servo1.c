// *************** A forráskód eleje: include fájlok listája ********
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <termios.h>
#include <wiringPi.h>
// ******************** Include fájl lista vége *********************

// ************************ #define sorok ***************************
/* Mindig nevet adunk minden olyan értéknek, ami konstans, kezdeti
 érték stb. Név szerint több esély van rá, hogy egy későbbi módosításkor
 eszünkbe jusson, hogy egy érték pl. miért annyi, amennyi. Ha kiderül, hogy
 módosítani kell, nem kell végig keresnünk a kódot, hogy vajon hol és hányszor
 használtuk ezt az értéket, hanem itt egy helyen minden ilyesmit megtalálunk,
 és ítt módosítjuk, ha kell. */

#define key_kilep 'X'
#define key_kikapcs '0'
#define key_bekapcs '2'
#define key_lowd '1'
#define key_lowi '7'
#define key_highd '3'
#define key_highi '9'
#define key_center '5'
#define key_bal '4'
#define key_jobb '6'
#define key_auto 'A'
#define key_help 'H'

#define default_setup 10 //impulzusok száma egy servo mozgáshoz
#define default_pulse_delay 50 //két imulzus közötti idő [ms]
#define max_pulse 3000 //a szervó impulzus maximális hossza [us]
#define sec1 1000 // delay fv 1 sec késleltetéshez [ms]
#define upper_case_mask 0b11011111
#define msg_help "1:L-\r\n7:L+\r\n3:H-\r\n9:H+\r\n5:közép\r\n4:bal\r\n6:jobb\r\nA:auto\r\nH-help\r\nX:program vége\r\n"
#define msg_verzio "Szervó teszt/ servo test v0.2\r\n"

#define PIROSLED 1 //WiringPi 1. láb, 10-es csati 7. láb
#define SERVO1 0 //WiringPi 0. láb, 10-es csati 6. láb
#define SENSOR1 2 //TCRT5000 szenzor 1
#define SENSOR2 3 //TCRT5000 szenzor 2

struct termios orig_termios;

void reset_terminal_mode()
{	
	/*Az eredeti terminál beállításokat a programból való kilépéskor visszaállítjuk*/
    tcsetattr(0, TCSANOW, &orig_termios);
}

void set_conio_terminal_mode()
{
    struct termios new_termios; /* Helyfoglalás a terminál beállításokat tartalmazó adatoknak*/

    /* Elmentjük a terminál aktuális beállításait */
    tcgetattr(0, &orig_termios);
    memcpy(&new_termios, &orig_termios, sizeof(new_termios));

    /* Meghatározzuk a kilépéskor lefuttatandó funkciót */
    atexit(reset_terminal_mode);
    cfmakeraw(&new_termios);
    tcsetattr(0, TCSANOW, &new_termios);
}

int kbhit() //ha a pufferben nincs karakter, visszatérési értéke 0 (false),
			//egyébként nullától különbözö (true)
{
    struct timeval tv = { 0L, 0L };
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    return select(1, &fds, NULL, NULL, &tv);
}

int getch() // ha az stdin pufferben van karakter, a soron következőt adja vissza
{
    int r;
    int c;
    if ((r = read(0, &c, sizeof(c))) < 0) {
        return r;
    } else {
        return c;
    }
}

void servo_goto(int usec) //szervó forgatás, usec arányában fordul el a
						  //tengely.
{
int setup;
					for(setup=0;setup<default_setup;setup++)
					{
					digitalWrite(SERVO1,HIGH);
					delayMicroseconds(usec);
					digitalWrite(SERVO1,LOW);
					delay(default_pulse_delay);
					}
					printf("Aktuális impulzus hossz [us]=%u\r\n",usec);

}

int main(int argc, char *argv[]) // minden C programba kell egy main()
// ez a program belépési pontja, vagyis a main elejétől kezdődik a program futása
{
// **************** Itt deklaráljuk a változókat ******************************
	unsigned char karcsi=0,sens1=0,sens2=0;
	unsigned int highest=1600,lowest=100,pulse,autoc;
	
// ********************** Deklarációk vége ************************************



// ***************** Inicializáló szakasz kezdete *****************************
// Általában olyasmi, amit a program elején egyszer kell csak elvégezni

    set_conio_terminal_mode(); /*ezzel tesszük lehetővé, hogy enter lenyomása
								nélkül olvassuk a billentyűzetet*/

    if (isatty(fileno(stdout))) setbuf(stdout, NULL);
	/* #include <unistd.h> ! Line buffert ezzel kikapcsoljuk*/
	wiringPiSetup() ; //e fv. lefutás után használhatjuk a wiringPi-t
	pinMode(PIROSLED, OUTPUT) ; //beállítjuk a lábakat kimenetnek/bemenetnek
	pinMode(SERVO1,OUTPUT); 
	pinMode(SENSOR1,INPUT);
	pinMode(SENSOR2,INPUT);
	printf(msg_verzio); //
	printf(__FILE__);
	printf(" lefordítva/compiled at: %s %s\r\n\n",__DATE__,__TIME__);
	printf(msg_help);

// ******************* Inicializáló szakasz vége ******************************	


// ************************ Főciklus kezdete ***********************************

	while(karcsi!=key_kilep) /* a kapcsos zárójelek közötti rész mindaddig végrehajtódik,
	amíg az íves zárójelet közötti kifejezés értéke nullától különbözó */
	{	
		sens1=0;
		sens2=0;
		while ((!kbhit())&&(!(sens1+sens2))) //ha
		{
			if (!digitalRead(SENSOR1)) sens1=key_bal; else sens1=0; 
			if (!digitalRead(SENSOR2)) sens2=key_jobb; else sens2=0;
			

		// Ha több szálon futnak az események, itt lehet beilleszteni
		}
    switch(sens1+sens2)
    {
	case	0:	karcsi=getch();
				break;
	case	key_bal: karcsi=key_bal;
				break;
	case	key_jobb: karcsi=key_jobb;
				break;
	default:	if(kbhit()) karcsi=getch();else karcsi=0xff;
    }
    if(karcsi>'A')karcsi=karcsi&upper_case_mask; /*
    Ha karcsi betű, nagybetűre változtatjuk (ld. ASCII tábla!)*/
    switch (karcsi) //többállású kapcsoló
    {
	case	key_kikapcs: 	digitalWrite(PIROSLED,LOW);
			break;
	case	key_lowd: 	if (lowest>10) lowest=lowest-10;
			break;
	case	key_bekapcs: 	digitalWrite(PIROSLED,HIGH);
			break;
	case	key_highd: 	if((highest>10)&&(highest>(lowest+10))) highest=highest-10;
			break;
	case	key_bal:	pulse=lowest;
						servo_goto(pulse);
			break;
	case	key_center: 	pulse=(highest-lowest)/2+lowest;
					if(pulse>max_pulse) pulse=max_pulse;
						servo_goto(pulse);
			break;
	case	key_jobb: 	pulse=highest;
						servo_goto(pulse);
			break;
	case	key_lowi: 	if(lowest<(highest-10))lowest=lowest+10;
			break;
	case	key_highi: 	highest=highest+10;
			break;
	case	key_auto:	for(autoc=0;autoc<6;autoc++) 
						{
							servo_goto(lowest);
							delay(sec1);
							servo_goto(highest);
							delay(sec1);
						
						}
			break;
	case    key_help:		printf(msg_help);
			break;
	default : ;	
	}
	printf("Also ertek [us]= %u, felso ertek [us]= %u           \n\r",lowest,highest);
	}
    return(0);
}

