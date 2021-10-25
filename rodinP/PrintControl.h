#pragma once

class ViewerModule;
class ModelContainer;
class PrintingInfo;
class FileTransfer;
class ModelDataStorage;
class PrintControl : public QThread
{
	Q_OBJECT
public:
	PrintControl(ViewerModule* viewerModule_, bool isSSDSave_ = false);
	~PrintControl();
	void run();

	QString m_pw;
	bool m_isSSDSave;

	bool m_cancelFlag;
	bool m_yesFlag;
	bool m_noFlag;
private slots:
	void threadCancel();
	void dialog_yesClicked() { m_yesFlag = true; }
	void dialog_noClicked() { m_noFlag = true; }
signals:
	void signal_close_dialog();
	void signal_progressValue(int);
	void signal_cancel();
	void signal_dialogContents(QString p_message, int p_type, bool p_OkVisible = true, bool p_cancelVisible = false, bool p_yesVisible = false, bool p_noVisible = false, bool p_delay = false);
	void signal_setPreviewUIMode(ModelDataStorage*);
	void signal_setLabelText(QString p_message);
private:
	ModelContainer* modelContainer;
	PrintingInfo* printingInfo;
	FileTransfer *fileTransfer;
	ModelDataStorage* dataStorage;

	void init();
	bool slice();
	bool createConnection();
	bool checkBeforeFileTransferToSSD();
	bool checkBeforeFileTransfer();
};

