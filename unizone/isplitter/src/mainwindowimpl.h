#include "mainwindow.h"

class MenuBar;
class QImage;
class QString;

class ImageSplitter : public ImageSplitterBase
{
	Q_OBJECT
public:
	ImageSplitter( QWidget* parent = 0, const char* name = 0, WFlags fl = 0);
	~ImageSplitter();

protected slots:
	void Preview();
	void Save();
	void Load();
	void ClearImage();
	void ClearPreview();
	void Exit();

protected:
	void resizeEvent(QResizeEvent *e);

private:
	QImage *image;
	QString filename;
	QString lastdir;
	MenuBar * menuBar;

	QPixmap * pixPreview;
};
