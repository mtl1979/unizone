#ifndef COMBO_H
#define COMBO_H

#include <q3combobox.h>

/** This combo box will send an event to its owner signifying
  * that the text has been changed by an enter keypress or loss
  * of focus
  */
class WComboBox : public Q3ComboBox
{
	Q_OBJECT
public:
	WComboBox(QObject * owner, QWidget * parent = NULL, const char * name = NULL);
	virtual ~WComboBox();

private slots:
	void SendEvent(const QString &);
	
private:
	QObject * fOwner;

protected:
	virtual void focusInEvent(QFocusEvent * e);
};

#endif	// COMBO_H


