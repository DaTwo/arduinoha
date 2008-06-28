#include <TerminatedProtocolBase.h>
#ifndef McVoiceProtocol_h
#define McVoiceProtocol_h

class McVoiceProtocol : public TerminatedProtocolBase {
	public:
		virtual void DecodePulse(short int pulse , unsigned int duration ) ;
		McVoiceProtocol(
			char * id, 
			void (*Bitstream)(const char * , unsigned short, volatile short int[]), 
			void (*DeviceTripped)(char * , unsigned short int &),
			void (*DeviceBatteryEmpty)(char * , unsigned short int &),
			void (*debug)(const char *) );
	private:
		void (*_DeviceTripped)(char * , unsigned short int &);
		void (*_DeviceBatteryEmpty)(char * , unsigned short int &);
	protected:
		virtual void DecodeBitstream();
};

#endif
