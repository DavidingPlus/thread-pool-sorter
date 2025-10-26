#include <gtest/gtest.h>

#include <fstream>
#include <unordered_set>

#include "lrandom.h"


TEST(LRandomTest, GenRandomNumberTest)
{
    int minVal = 10;
    int maxVal = 20;

    for (int i = 0; i < 100; ++i)
    {
        int val = LRandom::genRandomNumber(minVal, maxVal);

        // 检查不小于下界。
        EXPECT_GE(val, minVal);
        // 检查不大于上界。
        EXPECT_LE(val, maxVal);
    }
}

TEST(LRandomTest, GenRandomVectorTest)
{
    int minVal = 0;
    int maxVal = 100;
    int size = 50;

    std::vector<int> vec = LRandom::genRandomVector(minVal, maxVal, size);

    // 检查向量长度。
    EXPECT_EQ(vec.size(), size);
    // 检查数值范围。
    for (int val : vec)
    {
        EXPECT_GE(val, minVal);
        EXPECT_LE(val, maxVal);
    }

    // 简单检查随机性：向量中应该存在多个不同值。
    std::unordered_set<int> uniqueValues(vec.begin(), vec.end());
    EXPECT_GT(uniqueValues.size(), 1);
}

TEST(LRandomTest, GenRandomFileTest)
{
    const std::string testFile = "test.bin";
    int minVal = 1;
    int maxVal = 10;
    int count = 100;

    // 生成文件
    LRandom::genRandomFile(testFile, minVal, maxVal, count);

    // 读取文件并检查内容
    std::ifstream ifs(testFile, std::ios::binary);
    ASSERT_TRUE(ifs.is_open());

    int val;
    int readCount = 0;
    std::unordered_set<int> values;
    while (ifs.read(reinterpret_cast<char *>(&val), sizeof(val)))
    {
        EXPECT_GE(val, minVal);
        EXPECT_LE(val, maxVal);
        values.insert(val);
        ++readCount;
    }

    // 文件中数目是否正确。
    EXPECT_EQ(readCount, count);
    // 简单检查随机性。
    EXPECT_GT(values.size(), 1);

    ifs.close();
    std::remove(testFile.c_str());
}
