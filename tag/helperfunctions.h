// Header file that contains all the helper functions for running the experiment

// WARNING: 
// Be very careful when changing any format that would be printed to the serial output.
// They may be written in such way that can be decoded/parsed by our post-processing scripts.
// Any changes to the serial output format may result in a failure in the decoding/parsing stage after the experiment

#define pinnum1 25
#define pinnum2 26
#define pinport PIOD
#define pinmask1 0b0000000001	//(1<<0)
#define pinmask2 0b0000000010	//(1<<1)

// Arduino IDE is lack of inline options to speed up the program
// Therefore, the functions are defined as macros to speed up the bit flips when transmitting
// To enfore an inline option for future development, may refer to
// https://circuitjournal.com/arduino-force-inline#:~:text=C%2B%2B%20has%20an%20%22inline%22%20keyword,code%20size%20and%20not%20speed.
#define set_to_one()  {pinport->PIO_CODR = pinmask1; pinport->PIO_SODR = pinmask2;}
#define set_to_zero() {pinport->PIO_CODR = pinmask2; pinport->PIO_SODR = pinmask1;}

// Pre-defined constants for different MCS rates
//
const double rates[12][2] = {
	{78,  	86.7},		// MCS 12
	{104, 	115.6},  	// MCS 13
	{117, 	130},    	// MCS 14
	{130, 	144.4},  	// MCS 15
	{117, 	130},    	// MCS 20
	{156, 	173.3},   	// MCS 21
	{175.5, 195},   	// MCS 22
	{195, 	217},     	// MCS 23
	{156.0, 173.3},  	// MCS 28
	{208.0, 231.1}, 	// MCS 29
	{234.0, 260.0},  	// MCS 30
	{260.0, 288.9} 		// MCS 31
};

// Pre-defined constants for number of bits that can be encoded in a symbol given the MCS value
//
const double nDBPS[12]= {
	312,	// MCS 12
	416,  	// MCS 13
	468,  	// MCS 14
	520,  	// MCS 15
	468,  	// MCS 20
	624,  	// MCS 21
	702,  	// MCS 22
	780,  	// MCS 23
	624,  	// MCS 28
	832,  	// MCS 29
	936,  	// MCS 30
	1040 	// MCS 31
};

long 	ptotal_messages_per_dump;
double 	zero_bit_duration;
int 	numbits = 12;
double 	mpdu_time = 16;  // 7.9778; //40/3; //11.324099723;

//double bit_duration = mpdu_time * M;
double 	m_bit_duration;
double	p_bit_duration;
int 	t_c = 6; // Variable to set by the sensor

#define BIT(n,i) (n>>i&1)

// This function calculates the MPDU duration with the given parameters
// If the MPDU duration is hand-calculated and computed, 
// in order to speed up the program, you may not need this function 
// 
double mpdu_duration(int payload, int rate, int SGI) {
	double	PLCP_bits = 0;
	int		rate_idx;
	double	duration;
	double	gi_length;
	int		min_dept = 8; 	
	int		npad = 3;
	int		MPDU_delimiter_dur = 0;

	// Accounts for the gap (MCS rate for 16-19 are missing) in our MCS rate array
	// 
	if (rate <= 15){
		rate_idx = rate - 12;
	} else {
		rate_idx = rate - 16;
	}

	if (SGI == 1){  
		// Short Gaurd Interval (SGI) is 3.6 ms
		gi_length = 3.6;
	} else {
		// Long Gaurd Interval is 4.0 ms
		gi_length = 4;
	}

	npad		= payload % 4;				// paddings will be inserted if payload is not divisible by 4
	payload		= (payload + npad) * 8;		// 1 bytes -> 8 bits

	// The bits are packaged into guard intervals (GIs)
	// Even if the last GI is not fully filled, we still need to transmit for an entire GI
	// 
	duration	= ceil(payload / nDBPS[rate_idx]) * gi_length; 

	// The duration cannot be shorter than min_dept
	if (duration < min_dept)
		duration = min_dept;

	return ( round(duration) );
}

// Function that dumps the message to the serial output
//
long messages_per_dump(double pmpdu_time,long pexpr_duration ){
	double  total_message_duration;
	double  ptotal_messages_per_dump;
	// 1 second -> 1000000 micro-seconds
	pexpr_duration				= pexpr_duration *1000000; 

	// Total number of MPDU needed = B(bits/message) * M(number of repetition for each bit) + 2 * P(length of pre-/post-ambles)
	total_message_duration		= pmpdu_time * (B * M + 2 * P) + wait_time + overhead;
	ptotal_messages_per_dump	= pexpr_duration / total_message_duration;

	Serial.print ("mpdu_time = ");
	Serial.println ((pmpdu_time));

	Serial.print ("Total Duration = ");
	Serial.println ((total_message_duration));

	Serial.print ("Total messages = ");
	Serial.println ((ptotal_messages_per_dump));

	return ((long)ptotal_messages_per_dump);
}

void send_data(int data){

	// Sending Preamble
	#ifdef  DELTA_ZERO
	set_to_one();
	delayMicroseconds(deltazero);
	#endif 

	set_to_zero();
	delayMicroseconds(p_bit_duration);   

	#ifdef DELTA_ZERO
	set_to_one();
	delayMicroseconds(deltazero);
	#endif

	// Sending data part of the message
	// for (int i = 0; i < B; i++){
	for (int i = B - 1; i >= 0; i--) {
		if (BIT(data, i) == 0){
			// Sending a Zero
			#ifdef DELTA_ZERO
			set_to_one();
			delayMicroseconds(deltazero);
			#endif

			set_to_zero();
			delayMicroseconds(zero_bit_duration);    

			#ifdef DELTA_ZERO
			set_to_one();
			delayMicroseconds(deltazero);
			#endif  
		} else {
			// Sending a One
			set_to_one();
			delayMicroseconds(m_bit_duration);
		}
	}

	// Sending Postamble 
	#ifdef DELTA_ZERO 
	set_to_one();
	delayMicroseconds(deltazero);
	#endif 

	set_to_zero();
	delayMicroseconds(p_bit_duration);    
	
	#ifdef DELTA_ZERO
	set_to_one();  
	delayMicroseconds(deltazero);    
	#endif

	// Reset at the end
	pinport->PIO_CODR = pinmask1;
	pinport->PIO_SODR = pinmask2;
}

// tagTX is the function we use for testing purpose before running the actual experiment
// 
void tagTX(){
	const int repeatData = 5000;
	int temp[2] = {0b010101010101010,0b010101010101010};
	int *data = temp;
	B = 15;
	
	for (int i = 0; i < repeatData; i++){
		send_data(data[0]);
		delayMicroseconds (wait_time);
		// send_data(data[1]);;
		// delayMicroseconds (200);
	}
}

// This function prints out the data (that we are sending) to the serial output
void printdata(int data){ 
	// Print number in binary format according to B value
	for (int i = B-1; i >=0; i--) {
		if (BIT(data, i) == 0){
			Serial.print ("0");
		} else {
			Serial.print ("1");
		}
	}
	Serial.println ("");
}

// rotate_w_burst is the function that sends out the actual data for experiment purpose
//
void rotate_w_burst(){
	unsigned long startTime;
	unsigned long endTime;
	unsigned long delta;

	// Printing the M,B,P values to the serial output
	Serial.print ("B= ");
	Serial.println ((B));
	Serial.print ("M= ");
	Serial.println ((M));
	Serial.print ("P= ");
	Serial.println ((P));

	// Compute lastnum to avoid large b values being used 
	int lastnum = startnum + (sizeofarray - 1);
	// for (int num = 0; num < pow(2,B); num++) {
	for (unsigned int num = startnum; num <= lastnum ; num++) {
		Serial.print ("start ");
		printdata(num);
		//Serial.println (data[num],BIN);

		// Send a bit pattern and wait for defined wait_time before sending the next one 
		for (long i = 0; i < ptotal_messages_per_dump; i++) {            
			send_data(num);
			delayMicroseconds (wait_time);
		}

		// End of experiment for the current set of M,B,P values
		Serial.print ("end ");
		Serial.println(num);

		delayMicroseconds (5000000);
	}
}

// The setter for P value
// The function also calculates the MPDU time for pre-/post-ambles
//
void setP(int p){
	P = p;
	p_bit_duration = mpdu_time * P;
	p_bit_duration = p_bit_duration - (2 * deltazero);
}

// The setter for M value
// The function also calculates the MPDU time for each bit (after repeating M times) in the messages
//
void setM(int m){
	M = m;
	m_bit_duration = mpdu_time * M;
	zero_bit_duration   = m_bit_duration - (2 * deltazero);
	ptotal_messages_per_dump = messages_per_dump( mpdu_time , exp_duration);
  
}

// The setter for B value
// CURRRENTLY NOT USED, PLEASE IGNORE THIS PART OF THE CODE FOR NOW
//
void setB(int B){   
	numbits = B + 2;
}
