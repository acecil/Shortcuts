
#include "gtest/gtest.h"

#include "Config.h"

class testConfig : public ::testing::Test
{
protected:

	Config _config;
};

TEST_F(testConfig, GetParamStringEmpty)
{
	EXPECT_EQ(std::wstring(), _config.GetParam<std::wstring>(L"None"));
}
