
#include "gtest/gtest.h"

/* Header files containing tests. */
#include "testConfig.h"

int wmain(int argc, wchar_t **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
