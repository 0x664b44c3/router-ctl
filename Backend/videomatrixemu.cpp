#include "videomatrixemu.h"

VideoMatrixEmu::VideoMatrixEmu(QObject *parent) : AbstractVideoMatrix(parent)
{
	for(int i=0;i< MTX_OUTPUTS; ++i)
		mOutputAssignment[i] = -1;

}

int VideoMatrixEmu::sourceCount() const
{
	return MTX_INPUTS;
}

int VideoMatrixEmu::destinationCount() const
{
	return MTX_OUTPUTS;
}

int VideoMatrixEmu::sourceForDestination(int output) const
{
	if ((output < 0) || (output >= MTX_OUTPUTS))
		return -1;
	return mOutputAssignment[output];
}

bool VideoMatrixEmu::setSource(int output, int input)
{
	if ((output < 0) || (output >= MTX_OUTPUTS))
		return false;
	if ((input < -1) || (input >= MTX_INPUTS))
		return false;

	mOutputAssignment[output] = input;
	return true;
}

bool VideoMatrixEmu::isOnline() const
{
	return true;
}

void VideoMatrixEmu::assign(int output, int input)
{
	setSource(output, input);
}
