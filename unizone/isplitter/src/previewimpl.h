#ifndef PREVIEW_H
#define PREVIEW_H

#include <qwidget.h>
#include <qlabel.h>
#include <qlayout.h>

class QImage;
class QString;

class Preview : public QWidget
{
	Q_OBJECT
public:
	Preview( QWidget* parent = 0, const char* name = 0, WFlags fl = 0);
	~Preview();
	void setOwner(QWidget * owner);

    QPushButton* SaveButton;
    QLabel* pxlPreview;
    QPushButton* PreviewButton;

protected slots:
	void PreviewImage();
	void Save();

protected:
	void resizeEvent(QResizeEvent *e);
	friend class ImageSplitter;
	void ClearPreview();
	void ShowImage(QImage *);

    QGridLayout* GridLayout;

private:
	QImage * image;
	QPixmap * pixPreview;
	ImageSplitter * Splitter;
};
#endif
