/**
  minimalistic test implementation of an AbstractVideoMatrix object
*/
#ifndef VIDEOMATRIXEMU_H
#define VIDEOMATRIXEMU_H

#include <abstractvideomatrix.h>

#include <QObject>
const int MTX_INPUTS = 32;
const int MTX_OUTPUTS = 16;


class VideoMatrixEmu : public AbstractVideoMatrix
{
	Q_OBJECT
private:

public:
	explicit VideoMatrixEmu(QObject *parent = nullptr);

	virtual int sourceCount() const;
	virtual int destinationCount() const;
	virtual int sourceForDestination(int output) const;
	virtual bool setSource(int output, int input);
	virtual bool isOnline() const;

private:
	int mOutputAssignment[MTX_OUTPUTS];

    signals:

public slots:
	virtual void assign(int output, int input);
};

#endif // VIDEOMATRIXEMU_H
