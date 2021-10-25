#pragma once

#include "ModelContainer.h"
#include "SliceProfileForCommon.h"

struct CartridgeData
{
public:
	CartridgeData() {
		material = "No Info.";
		color = "230,240,50";
		length = 100;
		usedLength = 0;
		useStateForModel = false;
		useStateForProfile = false;
		useStateForUIMode = false;
	}
	QString material, color;
	int length, usedLength;
	bool useStateForModel, useStateForProfile, useStateForUIMode;
	bool getUseState() { return useStateForModel || useStateForProfile || useStateForUIMode; }
};

class LayerColor
{
public:
	int layerNr;
	int cartridgeIndex;

	LayerColor(int h, int index)
	{
		layerNr = h;
		cartridgeIndex = index;
	}
};

class CartridgeInfo
{
public:
	static bool usingCustomColor;
	static std::vector<CartridgeData> cartridges;
	static std::vector<QString> customColors;

	static std::vector<LayerColor> layerColorList;
	static std::vector<LayerColor> layerList;//layerColorList에서 pause를 제거한 list
	static std::vector<int> pauseList;
	static void layerColorListInit();
	static std::vector<float> getLayerColorList();


	static QColor getCartColor(int idx_);
	static QColor getCustomColor(int idx_);
	static QString getCartColorString(int idx_);
	static QString getCustomColorString(int idx_);
	static int getCartridgeRemainBar(int idx_);
	static int getCartCount();
	static void cartInit();
	static void cartInit(int);
	static void setCartColor(int idx, QColor cartColor);
	static void setCustomColor(int idx, QColor customColor);
	static void resetUseState();
	static void setUseState(ModelContainer* modelContainer_);
	static void setUseStateForModel(ModelContainer* modelContainer_);
	static std::vector<int> getUsedCartridgeFromProfile(SliceProfileForCommon _commonProfile);
	static void setUseStateForProfile(SliceProfileForCommon _commonProfile);
	static void setUseStateForProfile();
	static std::vector<bool> getUseStateForProfile();
	static void setUseStateForUIMode();
	static void setLayerColorList();
	static bool isPauseFromPauseLayerList(int layerNr_);
	static int getCartIndexFromCartridgeLayerList(int layerNr_);
	static std::vector<int> getUsedCartIdx();



	static bool findCartridgeData();
private:

};