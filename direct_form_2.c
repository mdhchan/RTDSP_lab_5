/*************************************************************************************
                  DEPARTMENT OF ELECTRICAL AND ELECTRONIC ENGINEERING
                                  IMPERIAL COLLEGE LONDON 

                     EE 3.19: Real Time Digital Signal Processing
                          Dr Paul Mitcheson and Daniel Harvey

                       LAB 4: Real-time Implementation of FIR Filters

                           ********* I N T I O. C **********

    Implement the FIR filter using the C6713 DSK system in real-time in C and assembly

*************************************************************************************
                Updated for use on 6713 DSK by Danny Harvey: May-Aug 2006
                Updated for CCS V4 Sept 10
************************************************************************************/
/*
*    You should modify the code so that interrupts are used to service the 
*  audio port.
*/
/**************************** Pre-processor statements ******************************/

#include <stdlib.h>
//  Included so program can make use of DSP/BIOS configuration tool.  
#include "dsp_bios_cfg.h"

/* The file dsk6713.h must be included in every program that uses the BSL.  This 
  example also includes dsk6713_aic23.h because it uses the 
  AIC23 codec module (audio interface). */
#include "dsk6713.h"
#include "dsk6713_aic23.h"

// math library (trig functions)
#include <math.h>
// Some functions to help with writing/reading the audio ports when using interrupts.
#include <helper_functions_ISR.h>
// include the FIR coefficients from MATLAB, N is defined in the text file
#include "IIR_coeff.txt"

// declare pointer to array of stored values and N-1 past inputs
double *w;
w = (double *)calloc(N,sizeof(double));
/* Declare size of coefficients array */
unsigned const int N = sizeof(b)/sizeof(b[0]); //Find order of coefficients

/******************************* Global declarations ********************************/

/* Audio port configuration settings: these values set registers in the AIC23 audio 
  interface to configure it. See TI doc SLWS106D 3-3 to 3-10 for more info. */
DSK6713_AIC23_Config Config = { \
            /**********************************************************************/
            /*   REGISTER               FUNCTION                 SETTINGS         */ 
            /**********************************************************************/\
   0x0017,  /* 0 LEFTINVOL  Left line input channel volume  0dB                   */\
   0x0017,  /* 1 RIGHTINVOL Right line input channel volume 0dB                   */\
   0x01f9,  /* 2 LEFTHPVOL  Left channel headphone volume   0dB                   */\
   0x01f9,  /* 3 RIGHTHPVOL Right channel headphone volume  0dB                   */\
   0x0011,  /* 4 ANAPATH    Analog audio path control       DAC on, Mic boost 20dB*/\
   0x0000,  /* 5 DIGPATH    Digital audio path control      All Filters off       */\
   0x0000,  /* 6 DPOWERDOWN Power down control              All Hardware on       */\
   0x0043,  /* 7 DIGIF      Digital audio interface format  16 bit                */\
   0x008d,  /* 8 SAMPLERATE Sample rate control             8 KHZ                 */\
   0x0001   /* 9 DIGACT     Digital interface activation    On                    */\
            /**********************************************************************/
};


// Codec handle:- a variable used to identify audio interface  
DSK6713_AIC23_CodecHandle H_Codec;

/******************************* Function prototypes ********************************/
void init_hardware(void);     
void init_HWI(void);  
void ISR_AIC (void);    
void shift_sample(double sample_in);
void direct_1_iir(void);  
double direct_2_iir(double sampleIn);
/********************************** Main routine ************************************/
void main(){
    // initialize board and the audio port
    init_hardware();
    /* initialize hardware interrupts */
    init_HWI();
    /* loop indefinitely, waiting for interrupts */             
    while(1) 
    {};  
}
       
/********************************** init_hardware() **********************************/  
void init_hardware()
{
    // Initialize the board support library, must be called first 
    DSK6713_init();
    // Start the AIC23 codec using the settings defined above in config 
    H_Codec = DSK6713_AIC23_openCodec(0, &Config);

    /* Function below sets the number of bits in word used by MSBSP (serial port) for 
    receives from AIC23 (audio port). We are using a 32 bit packet containing two 
    16 bit numbers hence 32BIT is set for  receive */
    MCBSP_FSETS(RCR1, RWDLEN1, 32BIT);    

    /* Configures interrupt to activate on each consecutive available 32 bits 
    from Audio port hence an interrupt is generated for each L & R sample pair */    
    MCBSP_FSETS(SPCR1, RINTM, FRM);

    /* These commands do the same thing as above but applied to data transfers to  
    the audio port */
    MCBSP_FSETS(XCR1, XWDLEN1, 32BIT);    
    MCBSP_FSETS(SPCR1, XINTM, FRM);    
}

/********************************** init_HWI() **************************************/  
void init_HWI(void)
{
    IRQ_globalDisable();            // Globally disables interrupts
    IRQ_nmiEnable();                // Enables the NMI interrupt (used by the debugger)
    IRQ_map(IRQ_EVT_RINT1,4);        // Maps an event to a physical interrupt
    IRQ_enable(IRQ_EVT_RINT1);        // Enables the event
    IRQ_globalEnable();                // Globally enables interrupts
} 


/******************** WRITE YOUR INTERRUPT SERVICE ROUTINE HERE***********************/  
void ISR_AIC (void)
{
	//Reads in a value from the codec
	double sampleIn = (double)mono_read_16Bit();	
	//Shift buffered values
	shift_sample();
	//Perform direct II processing and write the output sample of the filtered signal to the DAC
	mono_write_16Bit((Int16)direct_2_iir(sampleIn););
	
}

//Shift samples
void shift_sample(void){
	//Shifts each value in the w buffer back by 1
	int i;
	for (i = M-1;i>0;i--)
	{
		w[i]=w[i-1]; 
	}
}

//Perform direct II processing
double direct_2_iir(double sampleIn){
	//Set y_0 to 0 first
	double y = sampleIn;
	int i,j;
	//implement MAC with 'a' coefficients
	for(i = 1;i<N;i++){
		y -= w[i]*a[i];
	}
	//Multiply output with b[0] 
	y = y*b[0]; //necessary?
	//implement MAC with 'b' coefficients
	for(i = j;j<N;j++){
		y += w[j]*b[j];  //can merge with above loop?
	}
	return y;
}

void direct_1_iir(void){
	//implements convolution of the input signal with the filter's coefficients
	//Set y_0 to 0 first
	double y_0 = 0;
	//implement x difference equation
	int i, j;
	for(i = 0;i<M;i++){
		y_0 += x[i]*b[i];
	}
	//implement y difference equation
	
	for(j = 1;j<N;j++){
		y_0 -= y[j]*a[j];
	}
	y[0] = y_0;
}
