#ifndef NEVIONPROTOCOL_H
#define NEVIONPROTOCOL_H


class NevionProtocol
{
public:
	enum CrossPointType
	{
		xpAnalogAudio=1,
		xpAnalogVideo=2,
		xpDigitalAudio=3,
		xpDigitalVideo=4,
		xpControl422=5
	};

	NevionProtocol();

	bool handleRx();

	bool setCrosspoint(int level,  int out, int in);
};

#endif // NEVIONPROTOCOL_H
