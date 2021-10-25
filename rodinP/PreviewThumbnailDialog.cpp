#include "stdafx.h"
#include "PreviewThumbnailDialog.h"

PreviewThumbnailDialog::PreviewThumbnailDialog(QWidget* _parent)
	: QDialog(_parent)
{
	ui.setupUi(this);
	setWindowFlags(windowFlags() & (~Qt::WindowContextHelpButtonHint));
	this->setAttribute(Qt::WA_DeleteOnClose);
}

PreviewThumbnailDialog::~PreviewThumbnailDialog()
{

}

void PreviewThumbnailDialog::setImage(QImage image_)
{
	ui.label_image->setPixmap(QPixmap::fromImage(image_.scaled(300, 250, Qt::KeepAspectRatio)));
	ui.label_image->setAlignment(Qt::AlignCenter);
}