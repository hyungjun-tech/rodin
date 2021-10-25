#pragma once
#include "ui_ShortcutInfoDialog.h"

class ShortcutInfoDialog : public QDialog
{
	Q_OBJECT

public:
	ShortcutInfoDialog(QWidget *parent);
	~ShortcutInfoDialog();

private:
	Ui::ShortcutInfoDialog ui;

	public slots:
	void dialogClose();
};

