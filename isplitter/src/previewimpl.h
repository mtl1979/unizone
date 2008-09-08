#ifndef PREVIEW_H
#define PREVIEW_H

#include <qwidget.h>
#include <qlabel.h>
#include <qlayout.h>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QEvent>
#include <Q3GridLayout>
#include <QPixmap>

class QImage;
class QString;
class QPoint;
class QPushButton;
class ImageSplitter;

class Preview : public QWidget
{
	Q_OBJECT
public:
	Preview( QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = 0);
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
	bool eventFilter( QObject *o, QEvent *e );
	void mousePressEvent(QMouseEvent *e);
	void mouseMoveEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent *e);
	void startDrag();

	friend class ImageSplitter;
	void ClearPreview();
	void ShowImage(QImage *);

	bool dragging;
	QPoint startPos;

    Q3GridLayout* GridLayout;
	QWidget* PreviewWidget;

private:
	QImage * image;
	QPixmap * pixPreview;
	ImageSplitter * Splitter;

	void scalePixmap(double oldw, double oldh, int &width, int &height);
};
#endif
