#include <qmainwindow.h>
#include <qpopupmenu.h>
#include <qstring.h>

// Steal htmlview.h from Unizone ;)
#include "../src/htmlview.h"
#include "../src/combo.h"

class UniWindow : public QMainWindow
{
	Q_OBJECT
public:
	UniWindow( QWidget* parent = 0, const char* name = 0, WFlags f = WType_TopLevel );
	virtual ~UniWindow();

	enum
	{
		ID_HOME,
		ID_BACK,
		ID_FORWARD,
	};
protected:
	virtual void customEvent(QCustomEvent * event);

private:
	WHTMLView * fView;
	QPopupMenu * fileMenu;
	QPopupMenu * navMenu;
	WComboBox * cmbURL;

private slots:
	void Exit();
	void Back();
	void Forward();
	void Home();

	void backwardAvailable( bool available );
	void forwardAvailable( bool available );
	void linkClicked(const QString &);

};