#include <stdio.h>
#include <gtest/gtest.h>

//-------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
	testing::InitGoogleTest(&argc, argv);
	uint32_t seed = (uint32_t)::time(0);
	//seed = 1499855826;
	srand(seed);
	printf("seed=%d\n", seed);

	return RUN_ALL_TESTS();
}
