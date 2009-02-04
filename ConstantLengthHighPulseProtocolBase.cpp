#include "ConstantLengthHighPulseProtocolBase.h"


#define HIGH 0x1
#define LOW  0x0


ConstantLengthHighPulseProtocolBase::ConstantLengthHighPulseProtocolBase(
	char * id,
	void (*BitsstreamReceivedEvent)(ProtocolBase * protocol , byte* buffer , byte length ),	
	int bitstreamlength,
	int sendrepeats , 
	float timeperiodduration , // The duration of one oscillation
	float syncbitperiods , // The number of timeperiods in the entire syncbit-pulsecycle
	float shortperiods , // The number of timeperiods in a short-pulse
	float longperiods // The number of timeperiods in a long-pulse
	) : ProtocolBase(id , BitsstreamReceivedEvent , bitstreamlength , sendrepeats)
{
	_timeperiodduration = timeperiodduration;
	_syncbitperiods = syncbitperiods;
	_shortperiods = shortperiods;
	_longperiods = longperiods;
	
	_highpulseduration = 0;
}

void ConstantLengthHighPulseProtocolBase::Decode(short state, unsigned int duration)
{
    if (state==LOW)
    {
		unsigned int pulsecycleduration = duration + _highpulseduration ;
		if (WithinExpectedDeviation( pulsecycleduration , (_shortperiods+_longperiods) * _timeperiodduration ,  _timeperiodduration / 8) )
		{
			AddBit( decoder_bitbuffer , decoder_bitbufferlength, decoder_bitpos , false);				

		} else if (WithinExpectedDeviation( pulsecycleduration , ( _shortperiods + _shortperiods ) * _timeperiodduration ,  _timeperiodduration / 8) )
		{
			AddBit( decoder_bitbuffer , decoder_bitbufferlength, decoder_bitpos , true);			
		} else 
		{
			DecodeBitstream(_highpulseduration, duration);
			ResetDecoder();
			pulsecycleduration = 0;
		}
    } else if (state==HIGH)
	{
		_highpulseduration = duration;
	}
}

// This function returns a buffer with the pulse data of a bit for this protocol
void ConstantLengthHighPulseProtocolBase::EncodeBit(unsigned int *& pulsebuffer, byte & pulsebufferlength, bool bitvalue)
{
  pulsebuffer = 0;
  pulsebufferlength = 0;
  
  SetPulse(pulsebuffer, pulsebufferlength, 0 , _shortperiods * _timeperiodduration );
  if (bitvalue) 
  {	
	SetPulse(pulsebuffer, pulsebufferlength, 1 , _longperiods * _timeperiodduration );
  } else
  {
  	SetPulse(pulsebuffer, pulsebufferlength, 1 , _shortperiods * _timeperiodduration );
  } 
}

// This function returns a buffer with the pulse data of the terminator for this protocol
void ConstantLengthHighPulseProtocolBase::EncodeTerminator(unsigned int *& pulsebuffer, byte & pulsebufferlength) 
{
  pulsebuffer = 0;
  pulsebufferlength = 0;
  
  SetPulse(pulsebuffer, pulsebufferlength, 0 , _shortperiods * _timeperiodduration );
  SetPulse(pulsebuffer, pulsebufferlength, 1 , _syncbitperiods * _timeperiodduration );
}

void ConstantLengthHighPulseProtocolBase::DecodeBitstream(unsigned int lasthigh, unsigned int lastlow)
{
	if (_BitsstreamReceivedEvent!=0) _BitsstreamReceivedEvent( this , decoder_bitbuffer , decoder_bitpos);
}