// 85 is the decimal value for b=7 pattern (1010101) and 21 for (10101) b=5

int isdemo			= 0;		// set 1 if running for demo
int first			= 0;
int overhead		= 9;
//#define DELTA_ZERO 1
int deltazero		= 0;		// How many duration to skip in beginning and end of M mpdus corruption

//----------------------- Valiables to change for different settings

int startnum		= 12;		// set number for the start then it will iterate frm startnum to startnum + (sizeofarray - 1)
int sizeofarray		= 1;		// Optional, we use this to reduce number of loops for big b values in  function "rotate_w_burst"
int exp_duration	= 30;		// EXPERIMENT DURATION
int B				= 5;		// B: Number of bits encoded in one message
int M				= 3;		// M: Number of repetitions per bit
int P				= 3;		// P: Length of Pre- and Post- amlbes
int wait_time		= 1815;		// TAG WAIT TIME  2715, 4145, and 5574

//----------------------- Valiables to change for different settings

#include "helperfunctions.h"


void setup() {
	//Initiate Serial communication.
	Serial.begin(9600);
	pinMode (pinnum1, OUTPUT);
	pinMode (pinnum2, OUTPUT);

	// Compute MDPU time
	mpdu_time = mpdu_duration(313, 15, 1);	// MPDU time calculated by function
	Serial.print(" mpdu_duration ");
	Serial.println(mpdu_time);	
	mpdu_time = 18;							// Hardcoded MPDU time 
  // FIGUREOUT HOW MANY TIMES THE TAG SHOULD CORRUPT
	// Set M, B, P values
	setB(B);	
	setM(M);
	setP(P);

}

void loop() {

	int sensorVal = analogRead(A0);
	if(isdemo == 1){ 
		t_c =(int) round(((float)sensorVal) * 31.0 / ((float)1023));
		Serial.print(" sensorVal ");
		Serial.println(sensorVal);

		Serial.print(" t_c ");  
		Serial.println( t_c);
		//t_c = 6;
		// Set strings to send for new M
		// setMandB(B,M,P);
		for (long i = 0; i < 100; i++){
			send_data(t_c);
			delayMicroseconds (wait_time);
		}
   printdata(t_c);
	} else {

		// Serial.println("numbits" + numbits);
		// Serial.println( bit_duration);
		// Set strings to send for new M
		// setMandB(B,M,P);
		// for (long i = 0; i < 500000; i++){
		// 		send_data0101(0b010101010101010);
		// 		delayMicroseconds (wait_time);
		//}

		// Running experiments with M, B, P values
		delayMicroseconds (500000);
		rotate_w_burst();
		//tagTX();
		delayMicroseconds (500000);
    //int m = M + 4;
		//int p = P + 1;
		//`setM(m);
		// setP(p);  
	}

}
