



#include <gtest/gtest.h>

namespace dd
{

class TestRunner
{
public:
	int run(int argc, char** argv)
	{
		::testing::InitGoogleTest(&argc, argv);
		return RUN_ALL_TESTS();
	}
};

}

int main(int argc, char** argv)
{
	dd::TestRunner testRunner;
	return testRunner.run(argc, argv);
}