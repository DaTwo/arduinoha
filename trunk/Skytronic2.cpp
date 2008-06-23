#include "Skytronic2.h"
#include "WConstants.h"


//[  28]   37 [  27]   38 [  27]   38 [  26]   39 [  26]   69 
//[  26]   38 [  27]   37 [  27]   69 [  26]   68 [  27]   38 
//[  27]   68 [  26]   39 [  26]   68 [  27]   39 [  26]   38 
//[  27]   68 [  27]   68 [  26]   39 [  26]   39 [  26]   69 
//[  26]   68 [  26]   39 [  27]   38 [  26]   69 [  26]   68 
//[  26]   40 [  25]   39 [  26]   69 [  26]   68 [  26]   40 
//[  26]   38 [  26]   69 [  25]   71 [  24]   41 [  25]   39 
//[  25]   71 [  24]   69 [  26]   39 [  26]   69 [  25]   40 
//[  26]   68 [  26]   40 [  25]   41 [  23]   72 [  24]   40 
//[  24]   71 [  24]   70 [  25]   41 [  24]   41 [  23]   71 
//[  25]   40 [  24]   71 [  24]   40 [  25]   70 [  25]   39 
//[  25]   70 [  26]   39 [  25]   70 [  25]   40 [  24]   71 
//[  25]   39 [  25]   70 [  25]   40 [  25]   70 [  25]   40 
//[  24]   71 [  25]   41 [  22]   71 			    
//[  24]  652 

// [  32]   32 [  31]   32 [  31]   33 [  31]   33 [  31]   62 
// [  31]   33 [  31]   32 [  31]   63 [  31]   62 [  31]   33 
// [  31]   62 [  31]   33 [  31]   62 [  31]   34 [  30]   33 
// [  31]   63 [  30]   63 [  30]   34 [  30]   33 [  31]   63 
// [  30]   63 [  30]   34 [  31]   32 [  31]   63 [  30]   63 
// [  30]   35 [  30]   33 [  30]   64 [  30]   63 [  30]   34 
// [  30]   33 [  30]   64 [  30]   63 [  30]   34 [  30]   33 
// [  30]   64 [  30]   63 [  30]   34 [  30]   63 [  30]   34 
// [  30]   64 [  29]   35 [  29]   34 [  30]   64 [  30]   33 
// [  30]   64 [  30]   63 [  30]   34 [  30]   34 [  29]   64 
// [  30]   34 [  29]   65 [  29]   34 [  29]   65 [  29]   34 
// [  29]   65 [  29]   34 [  29]   65 [  29]   34 [  30]   64 
// [  29]   34 [  30]   64 [  30]   34 [  29]   64 [  30]   34 
// [  29]   65 [  29]   34 [  29]   64 
// [  29]  635 


enum PulseDuration 
{
	DURATION_UNKNOWN = 0 , 
	DURATION_SHORT = 1 , 
	DURATION_LONG = 2, 
	DURATION_TERMINATOR = 3
};

#define	ShortHighPulseDuration_Min 22
#define	ShortHighPulseDuration_Max 36

#define	ShortLowPulseDuration_Min 29
#define	ShortLowPulseDuration_Max 42

#define LongLowPulseDuration_Min 59
#define	LongLowPulseDuration_Max 72

#define	TerminatorDuration_Min 630
#define	TerminatorDuration_Max 720



Skytronic2::Skytronic2(
	void (*Bitstream)(volatile short int[]), 
	void (*debug)(const char *) )
{
	_ProtocolBitstream = Bitstream;
	_debug = debug;

	DecodedBitsBufferSize = 34;
}

void Skytronic2::DecodeBitstream()
{
//	if [6]==1 Command = On else Command = off

//	if ([10]==0 && [11]==1) device +=1
//	if ([10]==1 && [11]==1) device +=2
//	if ([10]==1 && [11]==0) device +=3
//	if ([10]==0 && [11]==0) device +=4

void Skytronic2::DecodePulse(short int pulse, unsigned int duration)
{
    if (HIGH==pulse)
    { // een hoog signaal
       int durationresult = quantizeduration( duration, DURATION_UNKNOWN, 
					DURATION_SHORT , ShortHighPulseDuration_Min, ShortHighPulseDuration_Max);

       switch (durationresult)
       {
         case DURATION_SHORT:
           if (0==BitDecodeState) {BitDecodeState=1;}
           else if (2==BitDecodeState) {BitDecodeState=3;}
	   else if (4==BitDecodeState) {BitDecodeState=5;}
	   else
	   {
		ResetBitDecodeState();
	   }
           break;
         default:
	   ResetBitDecodeState();
	   ResetDecodedBitsBuffer();
           break;
       }
    }
    else
    { // curstate==LOW
	int durationresult = quantizeduration( duration, DURATION_UNKNOWN, 
					DURATION_SHORT , ShortLowPulseDuration_Min, ShortLowPulseDuration_Max, 
					DURATION_LONG , LongLowPulseDuration_Min, LongLowPulseDuration_Max, 
					DURATION_TERMINATOR, TerminatorDuration_Min, TerminatorDuration_Max );

        switch (durationresult)
        {
          case DURATION_SHORT:
            if (1==BitDecodeState) {BitDecodeState=2;}
	    else if (3==BitDecodeState)
            { // SSSS
	      StoreDecodedBit(2);
              ResetBitDecodeState();
            } else if (5==BitDecodeState)
	    { // SLSS
	      StoreDecodedBit(1);
              ResetBitDecodeState();
 	    } else
            {
	      ResetDecodedBitsBuffer();
              ResetBitDecodeState();
            }
            break;
          case DURATION_LONG:
            if (1==BitDecodeState) {BitDecodeState=4; }
	    else if (3==BitDecodeState)
            { // SSSL
	      StoreDecodedBit(0);
              ResetBitDecodeState();
            } else 
	    {
		ResetBitDecodeState();
		ResetDecodedBitsBuffer();
	    }
            break;  
          case DURATION_TERMINATOR :
            if (1==BitDecodeState && DecodedBitsBufferPosIdx+1==DecodedBitsBufferSize)
            {
		if (_ProtocolBitstream!=0) _ProtocolBitstream(DecodedBitsBuffer);
		DecodeBitstream();			
            } 
            ResetBitDecodeState();
	    ResetDecodedBitsBuffer();
            break;
          default: 
            ResetBitDecodeState();
	    ResetDecodedBitsBuffer();
            break;  
        }
    }
}