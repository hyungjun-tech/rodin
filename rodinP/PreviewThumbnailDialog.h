#pragma once
#include "ui_PreviewThumbnailDialog.h"

class PreviewThumbnailDialog : public QDialog
{
	Q_OBJECT

public:
	PreviewThumbnailDialog(QWidget* _parent);
	~PreviewThumbnailDialog();

	void setImage(QImage image);

private:
	Ui::PreviewThumbnailDialog ui;
};
