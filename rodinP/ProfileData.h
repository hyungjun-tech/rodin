#pragma once
#include <vector>

enum ProfileDataType { _double, _int, _bool, _QString };

class SliceProfile;
class SliceProfileForCommon;

class ProfileDataD;
class ProfileDataI;
class ProfileDataB;
class ProfileDataS;
class ProfileData
{
public:
	ProfileData() { };
	~ProfileData() { };

public:
	std::string name_key;
	std::string unit_key;

	static std::string outputProfileData(ProfileDataD* _data);
	static std::string outputProfileData(ProfileDataI* _data);
	static std::string outputProfileData(ProfileDataB* _data);
	static std::string outputProfileData(ProfileDataS* _data);

	static std::string outputProfileData_value(ProfileDataD* _data);
	static std::string outputProfileData_value(ProfileDataI* _data);
	static std::string outputProfileData_value(ProfileDataB* _data);
	static std::string outputProfileData_value(ProfileDataS* _data);

};

class ProfileDataD : public ProfileData
{
public:
	ProfileDataD();
	ProfileDataD(std::string _name_key, std::string _unit_key);
	~ProfileDataD();

	double min;
	double max;
	double value;
	std::vector<double> valueList;

};

class ProfileDataI : public ProfileData
{
public:
	ProfileDataI();
	ProfileDataI(std::string _name_key, std::string _unit_key);
	~ProfileDataI();

	int min;
	int max;
	int value;
	std::vector<int> valueList;

};

class ProfileDataB : public ProfileData
{
public:
	ProfileDataB();
	ProfileDataB(std::string _name_key, std::string _unit_key);
	~ProfileDataB();

	bool value;
};

class ProfileDataS : public ProfileData
{
public:
	ProfileDataS();
	ProfileDataS(std::string _name_key, std::string _unit_key);
	~ProfileDataS();

	QString value;
	std::vector<QString> valueList;
};




//template <typename T>
//class ProfileData
//{
//public:
//	ProfileData(T _value);
//	ProfileData(std::string _name_key, std::string _unit_key);
//
//public:
//	T min;
//	T max;
//	T value;
//
//	std::string name_key;
//	std::string unit_key;
//	                         
//	std::vector<T> value_list;
//
//};
//
//template <typename T>
//ProfileData<T>::ProfileData(T _value)
//{
//	value = _value;
//}
//
//template <typename T>
//ProfileData<T>::ProfileData(std::string _name_key, std::string _unit_key)
//{
//	name_key = _name_key;
//	unit_key = _unit_key;
//}



