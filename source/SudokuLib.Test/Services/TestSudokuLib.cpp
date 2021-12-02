#include <gtest/gtest.h>

#include <Core/Types.h>

namespace ddahlkvist
{

class SudokuLibFixture : public testing::Test {
public:
    SudokuLibFixture()
        : _maxValue(17)
    {}

protected:
    void SetUp() override { 
        
    }
    void TearDown() override { 
    }

    u32 _maxValue;
};

TEST_F(SudokuLibFixture, isInvoked) {
    EXPECT_EQ(_maxValue, 17u);
}

}

