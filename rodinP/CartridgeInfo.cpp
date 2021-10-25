#include "stdafx.h"
#include "CartridgeInfo.h"
#include "ConnectionControl.h"

bool CartridgeInfo::usingCustomColor = false;
std::vector<CartridgeData> CartridgeInfo::cartridges = std::vector<CartridgeData>{};
std::vector<QString> CartridgeInfo::customColors = std::vector<QString>{};
std::vector<LayerColor> CartridgeInfo::layerColorList = std::vector<LayerColor>{};

std::vector<LayerColor> CartridgeInfo::layerList = std::vector<LayerColor>{};
std::vector<int> CartridgeInfo::pauseList = std::vector<int>{};

void CartridgeInfo::layerColorListInit()
{
	// default setting of layer color.///////////////////////////////////////////
	// start ������ �� 0���� �ϸ� rendering �� ������ ���� �� �־� -10���� ������ �ش�.//
	// end ������ �����Ӱ� ��������.//
	layerColorList.clear();
	layerColorList.push_back(LayerColor(-10, 0)); // start		
	layerColorList.push_back(LayerColor(10000, 0)); // end
}
std::vector<float> CartridgeInfo::getLayerColorList()
{
	std::vector<float> rtn;
	float layerHeight = Profile::sliceProfileCommon.layer_height.value;
	int preCartridgeId = 0;
	for (auto&& it : layerColorList)
	{
		if (preCartridgeId != it.cartridgeIndex || it.cartridgeIndex == -1)
		{
			rtn.push_back(it.layerNr*layerHeight);
			preCartridgeId = it.cartridgeIndex;
		}
		else if (&it == &layerColorList.back())
			break;
	}
	if (rtn.size() == 0)
		rtn.push_back(Profile::machineProfile.machine_height_default.value);
	return rtn;
}

QColor CartridgeInfo::getCartColor(int idx)
{
	QColor color = QColor(230, 240, 50);//cartridge�� default color�� �ٸ��� �� �ʿ� ������
	if (idx < 0 || idx >= cartridges.size()) return QColor(255, 148, 53);
	if (cartridges.at(idx).color == "") return color;

	QStringList l_rgb;
	if (usingCustomColor)
	{
		color = getCustomColor(idx);
	}
	else
	{
		l_rgb = cartridges.at(idx).color.split(",");
		if (l_rgb.size() < 3) return color;
		color = QColor(l_rgb.at(0).toInt(), l_rgb.at(1).toInt(), l_rgb.at(2).toInt());
	}

	return color;
}

QColor CartridgeInfo::getCustomColor(int idx)
{
	QColor color = QColor(230, 240, 50);//cartridge�� default color�� �ٸ��� �� �ʿ� ������
	if (idx < 0 || idx >= customColors.size()) return color;
	//if (cartridges.at(idx).customColor == "") return color;

	QStringList l_rgb = customColors.at(idx).split(",");
	//custom color�� ���� ���������� ��� ���� ���� ��� color�� ã��.
	if (l_rgb.size() < 3)
		l_rgb = cartridges.at(idx).color.split(",");

	if (l_rgb.size() < 3) return color;
	color = QColor(l_rgb.at(0).toInt(), l_rgb.at(1).toInt(), l_rgb.at(2).toInt());
	return color;
}

QString CartridgeInfo::getCartColorString(int idx)
{
	QColor l_rgb = getCartColor(idx);
	return (QString::number(l_rgb.red()) + "," + QString::number(l_rgb.green()) + "," + QString::number(l_rgb.blue()));
}

QString CartridgeInfo::getCustomColorString(int idx)
{
	QColor l_rgb = getCustomColor(idx);
	return (QString::number(l_rgb.red()) + "," + QString::number(l_rgb.green()) + "," + QString::number(l_rgb.blue()));
}

int CartridgeInfo::getCartridgeRemainBar(int idx_)
{
	int filaRemainPer; //�ʶ��Ʈ �ܷ� % = (filaTotalLen - filaUsedLen) / filaTotalLen * 100;
	int filaRemainBar;// 10���� ��� ���� ǥ������
	if (cartridges.at(idx_).length == 0)
	{
		filaRemainPer = 0;
		filaRemainBar = 0;
	}
	else
	{
		//�ݿø��� ���� 0.5 ���� �� ����.	
		filaRemainPer = floor(((double(cartridges.at(idx_).length) - double(cartridges.at(idx_).usedLength)) * 100 / double(cartridges.at(idx_).length)) + 0.5);
		filaRemainBar = floor((double(filaRemainPer) / 10) + 0.5);
	}
	return filaRemainBar;
}

void CartridgeInfo::setCartColor(int idx, QColor cartColor)
{
	cartridges.at(idx).color = QString::number(cartColor.red()) + "," + QString::number(cartColor.green()) + "," + QString::number(cartColor.blue());
}

void CartridgeInfo::setCustomColor(int idx, QColor customColor)
{
	customColors.at(idx) = QString::number(customColor.red()) + "," + QString::number(customColor.green()) + "," + QString::number(customColor.blue());
}

int CartridgeInfo::getCartCount()
{
	return cartridges.size();
}

void CartridgeInfo::cartInit()
{
	cartInit(Profile::machineProfile.extruder_count.value);
}

void CartridgeInfo::cartInit(int extruderCnt)
{
	//Machine Profile�� ����� �� extruder ������ ���� cartridge�� ���� ���� �� UI �ʱ�ȭ
	if (customColors.size() != extruderCnt)
		customColors.resize(extruderCnt);

	cartridges.clear();
	cartridges.resize(extruderCnt);
	if (extruderCnt > 1)
		cartridges.at(1).color = "255,148,53";
}

void CartridgeInfo::resetUseState()
{
	for (int i = 0; i < cartridges.size(); i++)
	{
		cartridges.at(i).useStateForModel = false;
		cartridges.at(i).useStateForProfile = false;
		cartridges.at(i).useStateForUIMode = false;
	}
}

void CartridgeInfo::setUseState(ModelContainer* modelContainer_)
{
	setUseStateForModel(modelContainer_);
	setUseStateForProfile();
	setUseStateForUIMode();
}

void CartridgeInfo::setUseStateForModel(ModelContainer* modelContainer_)
{
	for (auto cartridge : cartridges)
		cartridge.useStateForModel = false;
	std::vector<IMeshModel*> models = modelContainer_->models;

	for (auto it = models.begin(); it != models.end(); ++it)
	{
		std::vector<int> indexes = (*it)->getCartridgeIndexes();

		for (int idx = 0; idx < indexes.size(); idx++)
		{
			if (indexes[idx] < cartridges.size())
				cartridges[indexes[idx]].useStateForModel = true;
		}
	}
}

std::vector<int> CartridgeInfo::getUsedCartridgeFromProfile(SliceProfileForCommon _commonProfile)
{
	std::vector<int> rtn;
	if (_commonProfile.layer_height.value != 0)
	{
		//RAFT�� ���� adhesion cartridge index�� �־��ִ� ���� �ǹ̰� ����.
		//BRIM, SKIRT��  slicing�������� �� �� �����Ƿ� �̸� adhesion�� ����ϸ� �ȵ�.
		if (_commonProfile.platform_adhesion.value == Generals::PlatformAdhesion::Raft)
		{
			int raftIdx = _commonProfile.adhesion_cartridge_index.value;
			if (raftIdx < cartridges.size())
				rtn.push_back(raftIdx);
		}

		if (_commonProfile.support_placement.value != Generals::SupportPlacement::SupportNone)
		{
			int supportIdx = _commonProfile.support_main_cartridge_index.value;
			if (supportIdx < cartridges.size())
				rtn.push_back(supportIdx);
		}
	}

	rtn.resize(std::distance(rtn.begin(), std::unique(rtn.begin(), rtn.end())));
	return rtn;
}
void CartridgeInfo::setUseStateForProfile(SliceProfileForCommon _commonProfile)
{
	for (auto cartridge : cartridges)
		cartridge.useStateForProfile = false;
	for (auto cartIdx : getUsedCartridgeFromProfile(_commonProfile))
	{
		cartridges[cartIdx].useStateForProfile = true;
	}
}
void CartridgeInfo::setUseStateForProfile()
{
	setUseStateForProfile(Profile::sliceProfileCommon);
}
std::vector<bool> CartridgeInfo::getUseStateForProfile()
{
	std::vector<bool> rtn;
	for (auto cart : cartridges)
	{
		rtn.push_back(cart.useStateForProfile);
	}
	return rtn;
}

void CartridgeInfo::setUseStateForUIMode()
{
	for (auto cartridge : cartridges)
		cartridge.useStateForUIMode = false;
	//�� ��� ��� īƮ������ ����Ѵٰ� �Ǵ�
	if (Generals::isReplicationUIMode())
	{
		for (int i = 0; i < cartridges.size(); i++)
			cartridges[i].useStateForUIMode = true;
	}
	else if (Generals::isLayerColorUIMode())
	{
		for (int j = 0; j < layerColorList.size(); j++)
		{
			int index = layerColorList.at(j).cartridgeIndex;
			if (index != -1)
				continue;
			else if (index < cartridges.size())
				cartridges[index].useStateForUIMode = true;
		}
	}
}

void CartridgeInfo::setLayerColorList()
{
	layerList.clear();
	pauseList.clear();
	for (int i = 0; i < layerColorList.size(); i++)
	{
		if (layerColorList[i].cartridgeIndex == -1)
			pauseList.push_back(layerColorList[i].layerNr);
		else
			layerList.push_back(layerColorList[i]);
	}
}
bool CartridgeInfo::isPauseFromPauseLayerList(int layerNr_)
{
	for (auto pause : pauseList)
		if (pause == layerNr_)
			return true;
	return false;
}
int CartridgeInfo::getCartIndexFromCartridgeLayerList(int layerNr_)
{
	if (layerList.size() < 2) return 0;

	for (int i = 0; i < layerList.size() - 1; i++)
	{
		if (layerList.at(i).layerNr <= layerNr_ && layerNr_ < layerList.at(i + 1).layerNr)
		{
			return layerList.at(i).cartridgeIndex;
		}
	}
	return 0;
}

std::vector<int> CartridgeInfo::getUsedCartIdx()
{
	std::vector<int> rtn;
	for (int i = 0; i < cartridges.size(); i++)
	{
		if (cartridges[i].getUseState())
			rtn.push_back(i);
	}
	return rtn;
}

bool CartridgeInfo::findCartridgeData()
{
	cartInit();
	QString connectionType = PrinterInfo::currentConnectionType;
	QString ip = PrinterInfo::currentIP;
	if (connectionType == "Network")
	{
		NetworkConnection network;
		if (network.checkConnection(ip, 7000)) {
			NetworkControl a;
			a.m_tmo = 2000;
			/*temp.color = a.getFilaColor(prtData->ip); //�ʶ��Ʈ ����
			temp.length = a.getFilaLength(prtData->ip); //�ʶ��Ʈ �� ����
			temp.usedLength = a.getFilaUsed(prtData->ip);	//�ʶ��Ʈ ������
			temp.material = a.getFilaMaterial(prtData->ip); //�ʶ��Ʈ ����
			cartridges.push_back(temp);*/

			QStringList l_material = a.getFilaMaterialList(ip);
			QStringList l_color = a.getFilaColorList(ip);
			std::vector<int> l_length = a.getFilaLengthList(ip);
			std::vector<int> l_usedLength = a.getFilaUsedList(ip);

			int size;
			if (l_material.size() < cartridges.size())
				size = l_material.size();
			else size = cartridges.size();

			for (int i = 0; i < size; i++)
			{
				cartridges.at(i).color = l_color.at(i);
				cartridges.at(i).length = l_length.at(i);
				cartridges.at(i).usedLength = l_usedLength.at(i);
				if (l_material.at(i) == "None" || l_material.at(i) == "")
					cartridges.at(i).material = "No Info.";
				else
					cartridges.at(i).material = l_material.at(i);
				//cartridges.push_back(temp);
			}
			return true;
		}
	}
	else if (connectionType == "USB")
	{
		PJLControl pjl;
		PrinterInterface selectedPrinter = pjl.getSelectPrinter(ip, false);

		if (selectedPrinter.getRawPointer() != NULL) {
			return pjl.setFilaInfoList(selectedPrinter);
		}
	}
	return false;
}