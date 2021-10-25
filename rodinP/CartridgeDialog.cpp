#include "stdafx.h"
#include "CartridgeDialog.h"
#include "CartridgeInfo.h"

CartridgeDialog::CartridgeDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose, true);
	setWindowFlags(windowFlags() & (~Qt::WindowContextHelpButtonHint));
	connect(ui.buttonOK, SIGNAL(clicked()), this, SLOT(clickedOK()));
	connect(ui.checkBox_usingCustomColor, SIGNAL(stateChanged(int)), this, SLOT(stateChanged_usingCustomColor()));

	//connect(this, SIGNAL(customColorChanged()), parent, SLOT(refreshFilaInfo()));

	refreshContents();
	ui.checkBox_usingCustomColor->setChecked(CartridgeInfo::usingCustomColor);
}

CartridgeDialog::~CartridgeDialog()
{

}


void CartridgeDialog::deleteFilaInfo()
{
	qDeleteAll(ui.frame_CartridgeInfo->children());

	//frame_filaInfo 하위 object를 삭제할때 vertical layout이 같이 삭제되므로 추가해줌.
	QVBoxLayout *vertical_CartridgeInfo;
	vertical_CartridgeInfo = new QVBoxLayout(ui.frame_CartridgeInfo);
	vertical_CartridgeInfo->setSpacing(6);
	vertical_CartridgeInfo->setObjectName(QStringLiteral("vertical_CartridgeInfo"));
	vertical_CartridgeInfo->setContentsMargins(2, 6, 2, 6);
}

void CartridgeDialog::refreshContents()
{
	int filaRemainPer; //필라멘트 잔량 % = (filaTotalLen - filaUsedLen) / filaTotalLen * 100;
	int filaRemainBar;// 10개중 몇개에 색을 표현할지
	deleteFilaInfo();
	for (int i = 0; i < CartridgeInfo::cartridges.size(); i++)
	{
		if (CartridgeInfo::cartridges.at(i).length == 0)
		{
			filaRemainPer = 0;
			filaRemainBar = 0;
		}
		else
		{
			//반올림을 위해 0.5 더한 후 버림.	
			filaRemainPer = floor(((double(CartridgeInfo::cartridges.at(i).length) - double(CartridgeInfo::cartridges.at(i).usedLength)) * 100 / double(CartridgeInfo::cartridges.at(i).length)) + 0.5);
			filaRemainBar = floor((double(filaRemainPer) / 10) + 0.5);
		}

		addFilaInfo(i, filaRemainBar, CartridgeInfo::cartridges.at(i).material);
		//selectCart->addItem(tr("Cartridge") + QString::number(i + 1));
	}
}

void CartridgeDialog::addFilaInfo(int idx, int filaRemainCnt, QString material)
{
	QVBoxLayout *vertical_CartridgeInfo = findChild<QVBoxLayout *>("vertical_CartridgeInfo");

	QFont font1;
	font1.setFamily(QStringLiteral("Malgun Gothic"));
	font1.setPointSize(9);
	font1.setBold(true);
	font1.setWeight(75);

	/*QFrame *filaRemains;
	filaRemains = new QFrame(ui.frame_CartridgeInfo);
	filaRemains->setObjectName("filaRemains_" + QString::number(idx));
	filaRemains->setMinimumSize(QSize(0, 20));
	filaRemains->setFrameShape(QFrame::NoFrame);
	filaRemains->setFrameShadow(QFrame::Plain);
	filaRemains->setLineWidth(0);*/

	QHBoxLayout *horizontal_filaRemains;
	/*horizontal_filaRemains = new QHBoxLayout(filaRemains);
	horizontal_filaRemains->setSpacing(6);
	horizontal_filaRemains->setObjectName("horizontal_filaRemains_" + QString::number(idx));
	horizontal_filaRemains->setContentsMargins(0, 0, 0, 0);*/

	horizontal_filaRemains = new QHBoxLayout();
	horizontal_filaRemains->setSpacing(6);
	horizontal_filaRemains->setObjectName(QStringLiteral("horizontal_filaRemains"));
	horizontal_filaRemains->setContentsMargins(-1, -1, -1, 0);
	vertical_CartridgeInfo->addLayout(horizontal_filaRemains);

	QLabel *categoryNumber;
	categoryNumber = new QLabel(ui.frame_CartridgeInfo);
	categoryNumber->setObjectName("categoryNumber_" + QString::number(idx));
	categoryNumber->setFont(font1);
	categoryNumber->setText(CustomTranslate::tr("Cartridge") + QString::number(idx+1));
	categoryNumber->setMinimumSize(QSize(90, 0));
	categoryNumber->setMaximumSize(QSize(90, 16777215));
	horizontal_filaRemains->addWidget(categoryNumber);

	if (material != "No Info.")
	{
		QFrame *filaRemainsSub;
		filaRemainsSub = new QFrame(ui.frame_CartridgeInfo);
		filaRemainsSub->setObjectName("filaRemainsSub_" + QString::number(idx));
		filaRemainsSub->setMinimumSize(QSize(0, 30));
		filaRemainsSub->setFrameShape(QFrame::NoFrame);
		filaRemainsSub->setFrameShadow(QFrame::Plain);
		filaRemainsSub->setLineWidth(0);
		filaRemainsSub->setStyleSheet("#filaRemainsSub_" + QString::number(idx) + "{background-color: rgb(200, 200, 200);border-style: solid; border-color: rgb(150, 150, 150); border-width: 1px; border-radius: 3px;}");

		QHBoxLayout *horizontal_filaRemainsSub;
		horizontal_filaRemainsSub = new QHBoxLayout(filaRemainsSub);
		horizontal_filaRemainsSub->setSpacing(2);
		horizontal_filaRemainsSub->setObjectName("horizontal_filaRemainsSub_" + QString::number(idx));
		horizontal_filaRemainsSub->setContentsMargins(4,4,4,4);

		horizontal_filaRemains->addWidget(filaRemainsSub);

		QString l_rgb;
		if (ui.checkBox_usingCustomColor->isChecked())
			l_rgb = CartridgeInfo::getCustomColorString(idx);
		else
			l_rgb = CartridgeInfo::cartridges.at(idx).color;

		for (int i = 0; i < 10; i++)
		{
			QLabel *fila;
			fila = new QLabel(filaRemainsSub);
			fila->setObjectName("fila_" + QString::number(idx) + QString::number(i));
			fila->setMaximumSize(QSize(6, 16777215));

			if (i < filaRemainCnt)
			{
				fila->setStyleSheet("background-color: rgb(" + l_rgb + "); border-style: solid; border-color: rgb(70,70,70); border-width: 1px;");/* border-style: solid; border-color: rgb(70,70,70); border-width: 1px; */
			}
			else
			{
				fila->setStyleSheet("background-color: rgb(200,200,200); border-style: solid; border-color: rgb(70,70,70); border-width: 1px;");
			}

			horizontal_filaRemainsSub->addWidget(fila);

		}
	}

	QLabel *filaInfo;
	filaInfo = new QLabel(ui.frame_CartridgeInfo);
	filaInfo->setObjectName(QStringLiteral("filaInfo"));
	filaInfo->setFont(font1);
	//filaInfo->setStyleSheet(QLatin1String("background-color: none;\n color: rgb(255, 255, 255);"));
	filaInfo->setText(material);
	filaInfo->setAlignment(Qt::AlignCenter);
	//filaInfo->setWordWrap(true);
	QSizePolicy sizePolicy3(QSizePolicy::Expanding, QSizePolicy::Preferred);
	sizePolicy3.setHorizontalStretch(0);
	sizePolicy3.setVerticalStretch(0);
	sizePolicy3.setHeightForWidth(filaInfo->sizePolicy().hasHeightForWidth());
	filaInfo->setSizePolicy(sizePolicy3);

	horizontal_filaRemains->addWidget(filaInfo);

	QPushButton *pushButton_colorSetting;
	pushButton_colorSetting = new QPushButton(ui.frame_CartridgeInfo);
	pushButton_colorSetting->setObjectName("pushButtonColorSetting_" + QString::number(idx));
	QSizePolicy sizePolicy4(QSizePolicy::Minimum, QSizePolicy::Preferred);
	sizePolicy4.setHorizontalStretch(0);
	sizePolicy4.setVerticalStretch(0);
	sizePolicy4.setHeightForWidth(pushButton_colorSetting->sizePolicy().hasHeightForWidth());
	pushButton_colorSetting->setSizePolicy(sizePolicy4);
	pushButton_colorSetting->setMinimumSize(QSize(20, 20));
	pushButton_colorSetting->setMaximumSize(QSize(20, 20));
	pushButton_colorSetting->setFont(font1);
	pushButton_colorSetting->setText("...");

	QString l_rgb = CartridgeInfo::getCustomColorString(idx);
	pushButton_colorSetting->setStyleSheet("background-color: rgb(" + l_rgb + "); ");
	connect(pushButton_colorSetting, SIGNAL(clicked()), this, SLOT(clickedColorSetting()));

	horizontal_filaRemains->addWidget(pushButton_colorSetting);




	//vertical_CartridgeInfo->addWidget(ui.frame_CartridgeInfo);
}

void CartridgeDialog::clickedColorSetting()
{
	QPushButton* buttonIcon = dynamic_cast<QPushButton*>(sender());
	QStringList tempList = buttonIcon->objectName().split("_");
	int idx = tempList.at(1).toInt();

	QColorDialog colorPicker(CartridgeInfo::getCustomColor(idx), this);
	if (colorPicker.exec() == QDialog::Accepted)
	{
		QColor selectedColor = colorPicker.selectedColor();
		if (!selectedColor.isValid()) return;
		CartridgeInfo::setCustomColor(idx, selectedColor);
		refreshContents();
	}
}

void CartridgeDialog::clickedOK()
{
	CartridgeInfo::usingCustomColor = ui.checkBox_usingCustomColor->isChecked();
	emit signal_customColorChanged();
	close();
}

void CartridgeDialog::stateChanged_usingCustomColor()
{
	refreshContents();
}