
#include "gtest/gtest.h"

#include "Config.h"

using std::wstring;

class testConfig : public ::testing::Test
{
protected:

	Config _config;
};

TEST_F(testConfig, GetParamStringEmpty)
{
	EXPECT_EQ(wstring(), _config.GetParam<wstring>(L"None"));
}

TEST_F(testConfig, GetParamStringEmptyDefault)
{
	wstring def(L"Default Value");
	EXPECT_EQ(def, _config.GetParam<wstring>(L"None", def));
}

TEST_F(testConfig, SetGetParamString)
{
	wstring name(L"TestParam");
	wstring value(L"Test Value");
	_config.SetParam<wstring>(name, value);
	EXPECT_EQ(value, _config.GetParam<wstring>(name));
}
