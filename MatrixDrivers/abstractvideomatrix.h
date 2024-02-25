#ifndef VIDEOMATRIX_H
#define VIDEOMATRIX_H

#include <QObject>


/** stores static info on router ports */
struct portInfo
{
	QString name; ///< name for this port (gets listed in GUI)
	QString description; ///< description for this port (think of a tooltip)
	QString mnemonic; ///< mnemonic, typically max 8 chars (for UMD)
	QString mnemonic4; ///< short mnemonic, typically max 4 chars (for key labelling)
	/** @brief unique ID of this port across the network
	 * will be pre-populated to <routerUID>-<dir>-<number>
	 * where <routerUID> is the UID for this router, <dir> is either src or <dest>
	 * and <number> is an integer ranging from 0 to n
	**/
	QString uniqueID;
};

class AbstractVideoMatrix : public QObject
{
	Q_OBJECT
public:

	explicit AbstractVideoMatrix(QObject *parent = 0);

	/**
	 * @brief uid
	 * @return id string for this matrix. uniqueness is application's responsibility
	 */
	virtual QString uid() const;

	virtual void setUid(QString id);

	/**
	 * @brief inputs - get number of inputs
	 * @return number of inputs
	 */
	virtual int sourceCount() const = 0;

	/**
	 * @brief outputs - get number of outputs
	 * @return number of outputs
	 */
	virtual int destinationCount() const = 0;

	/**
	 * @brief inputOnOutput - get inpput currently assigned to output
	 * @param output - output to query
	 * @return input number
	 */
	virtual int sourceForDestination(int output) const = 0;

	/**
	 * @brief setOutput - set input for output
	 * @param output output number
	 * @param input input to assign
	 * @return true if valid
	 */
	virtual bool setSource(int destination, int source) = 0;

	/** mnemonics are supported by some matrices
	 * on a matrix not suporting mnemonics these functions will do nothing
	 * no-op standard implementations are provided in base class
	 */

	/**
	 * @brief mnemonic - get mnemonic for input
	 * @param input - input number
	 */
	virtual QString sourceMnemonic(int source) const {(void) source; return QString();}

   /** @brief mnemonic - get mnemonic for destination
	* @param input - input number
	*/
   virtual QString destinationMnemonic(int dest) const {(void) dest; return QString();}

	/**
	 * @brief setMnemonic - set the mnemonic for an input
	 * @param input input number
	 * @param mnemonic input mnemonic
	 * @return true if valid
	 */
	virtual bool setMnemonic(int input, QString output) const { (void) input; (void) output; return false; }

	/**
	 * @brief isOnline
	 * @return true if matrix is connected and responding
	 */
	virtual bool isOnline() const = 0;

//	bool canLockOutput(int output);
//	bool lockOutput(int output, bool lock);

protected:
	QString mUID;
signals:
	void outputChanged(int output, int input);
	void mnemonicChanged(int input);
	void connectionLost();
	void connectioEstablished();
	void uidChanged(QString);
public slots:
	virtual void assign(int output, int input) = 0;

};

#endif // VIDEOMATRIX_H
