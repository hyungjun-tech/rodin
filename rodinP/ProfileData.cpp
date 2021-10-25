#include "stdafx.h"
#include "ProfileData.h"

ProfileDataD::ProfileDataD()
{
	min = 0.0;
	max = 0.0;
	value = 0.0;
}

ProfileDataD::ProfileDataD(std::string _name_key, std::string _unit_key)
{
	min = 0.0;
	max = 0.0;
	value = 0.0;

	name_key = _name_key;
	unit_key = _unit_key;
}

ProfileDataD::~ProfileDataD()
{

}


ProfileDataB::ProfileDataB()
{
	value = false;
}

ProfileDataB::ProfileDataB(std::string _name_key, std::string _unit_key)
{
	value = false;

	name_key = _name_key;
	unit_key = _unit_key;
}

ProfileDataB::~ProfileDataB()
{
	
}


ProfileDataI::ProfileDataI()
{
	value = 0;
	min = 0;
	max = 0;
}

ProfileDataI::ProfileDataI(std::string _name_key, std::string _unit_key)
{
	min = 0;
	max = 0;
	value = 0;

	name_key = _name_key;
	unit_key = _unit_key;
}

ProfileDataI::~ProfileDataI()
{

}

ProfileDataS::ProfileDataS()
{
	value.clear();
}

ProfileDataS::ProfileDataS(std::string _name_key, std::string _unit_key)
{
	value.clear();

	name_key = _name_key;
	unit_key = _unit_key;
}

ProfileDataS::~ProfileDataS()
{
}

std::string ProfileData::outputProfileData(ProfileDataD* _data)
{
	std::string str;

	str.append(_data->name_key.c_str());

	if (_data->unit_key.empty())
	{
		str.append(": ");
	}
	else
	{
		str.append("(");
		str.append(_data->unit_key.c_str());
		str.append("): ");
	}

	str.append(Generals::to_string_with_precision(_data->value, 2));

	if (!(_data->min == 0.0 && _data->max == 0.0))
	{
		str.append(" ");
		str.append(Generals::to_string_with_precision(_data->min, 2));
		str.append(" ");
		str.append(Generals::to_string_with_precision(_data->max, 2));
	}

	str.append("\n");

	return str;
}

std::string ProfileData::outputProfileData(ProfileDataI* _data)
{
	std::string str;

	str.append(_data->name_key.c_str());

	if (_data->unit_key.empty())
	{
		str.append(": ");
	}
	else
	{
		str.append("(");
		str.append(_data->unit_key.c_str());
		str.append("): ");
	}

	str.append(std::to_string(_data->value));

	if (!(_data->min == 0 && _data->max == 0))
	{
		str.append(" ");
		str.append(std::to_string(_data->min));
		str.append(" ");
		str.append(std::to_string(_data->max));
	}

	str.append("\n");

	return str;
}

std::string ProfileData::outputProfileData(ProfileDataB* _data)
{
	std::string str;

	str.append(_data->name_key.c_str());

	if (_data->unit_key.empty())
	{
		str.append(": ");
	}
	else
	{
		str.append("(");
		str.append(_data->unit_key.c_str());
		str.append("): ");
	}

	str.append(std::to_string(_data->value));
	str.append("\n");

	return str;
}

std::string ProfileData::outputProfileData(ProfileDataS* _data)
{
	std::string str;

	str.append(_data->name_key.c_str());

	if (_data->unit_key.empty())
	{
		str.append(": ");
	}
	else
	{
		str.append("(");
		str.append(_data->unit_key.c_str());
		str.append("): ");
	}

	str.append(_data->value.toStdString());
	str.append("\n");

	return str;
}


std::string ProfileData::outputProfileData_value(ProfileDataD* _data)
{
	std::string str;

	str.append(_data->name_key.c_str());

	if (_data->unit_key.empty())
	{
		str.append(": ");
	}
	else
	{
		str.append("(");
		str.append(_data->unit_key.c_str());
		str.append("): ");
	}

	str.append(Generals::to_string_with_precision(_data->value, 2));

	str.append("\n");

	return str;
}

std::string ProfileData::outputProfileData_value(ProfileDataI* _data)
{
	std::string str;

	str.append(_data->name_key.c_str());

	if (_data->unit_key.empty())
	{
		str.append(": ");
	}
	else
	{
		str.append("(");
		str.append(_data->unit_key.c_str());
		str.append("): ");
	}

	str.append(std::to_string(_data->value));

	str.append("\n");

	return str;
}

std::string ProfileData::outputProfileData_value(ProfileDataB* _data)
{
	std::string str;

	str.append(_data->name_key.c_str());

	if (_data->unit_key.empty())
	{
		str.append(": ");
	}
	else
	{
		str.append("(");
		str.append(_data->unit_key.c_str());
		str.append("): ");
	}

	str.append(std::to_string(_data->value));
	str.append("\n");

	return str;
}

std::string ProfileData::outputProfileData_value(ProfileDataS* _data)
{
	std::string str;

	str.append(_data->name_key.c_str());

	if (_data->unit_key.empty())
	{
		str.append(": ");
	}
	else
	{
		str.append("(");
		str.append(_data->unit_key.c_str());
		str.append("): ");
	}

	str.append(_data->value.toStdString());
	str.append("\n");

	return str;
}
