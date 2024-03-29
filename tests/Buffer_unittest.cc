#ifndef MUDUO_USE_MEMPOOL

#include <muduo/Buffer.h>
#include <gtest/gtest.h>
#include <string>

using std::string;
using namespace muduo;

TEST(TestBuffer, TestBufferAppendRetrieve) {
    Buffer buf;
    GTEST_ASSERT_EQ(buf.ReadableBytes(), 0);
    GTEST_ASSERT_EQ(buf.WriteableBytes(), Buffer::kInitialSize);
    GTEST_ASSERT_EQ(buf.PrependableBytes(), Buffer::kCheapPrepend);

    const string str(200, 'x');
    buf.Append(str);
    GTEST_ASSERT_EQ(buf.ReadableBytes(), str.size());
    GTEST_ASSERT_EQ(buf.WriteableBytes(), Buffer::kInitialSize - str.size());
    GTEST_ASSERT_EQ(buf.PrependableBytes(), Buffer::kCheapPrepend);

    const string str2 = buf.RetrieveAsString(50);
    GTEST_ASSERT_EQ(str2.size(), 50);
    GTEST_ASSERT_EQ(buf.ReadableBytes(), str.size() - str2.size());
    GTEST_ASSERT_EQ(buf.WriteableBytes(), Buffer::kInitialSize - str.size());
    GTEST_ASSERT_EQ(buf.PrependableBytes(), Buffer::kCheapPrepend + str2.size());
    GTEST_ASSERT_EQ(str2, string(50, 'x'));

    buf.Append(str);
    GTEST_ASSERT_EQ(buf.ReadableBytes(), 2 * str.size() - str2.size());
    GTEST_ASSERT_EQ(buf.WriteableBytes(), Buffer::kInitialSize - 2 * str.size());
    GTEST_ASSERT_EQ(buf.PrependableBytes(), Buffer::kCheapPrepend + str2.size());

    const string str3 = buf.RetrieveAllAsString();
    GTEST_ASSERT_EQ(str3.size(), 350);
    GTEST_ASSERT_EQ(buf.ReadableBytes(), 0);
    GTEST_ASSERT_EQ(buf.WriteableBytes(), Buffer::kInitialSize);
    GTEST_ASSERT_EQ(buf.PrependableBytes(), Buffer::kCheapPrepend);
    GTEST_ASSERT_EQ(str3, string(350, 'x'));
}

TEST(TestBuffer, testBufferGrow) {
    Buffer buf;
    buf.Append(string(400, 'y'));
    GTEST_ASSERT_EQ(buf.ReadableBytes(), 400);
    GTEST_ASSERT_EQ(buf.WriteableBytes(), Buffer::kInitialSize - 400);

    buf.Retrieve(50);
    GTEST_ASSERT_EQ(buf.ReadableBytes(), 350);
    GTEST_ASSERT_EQ(buf.WriteableBytes(), Buffer::kInitialSize - 400);
    GTEST_ASSERT_EQ(buf.PrependableBytes(), Buffer::kCheapPrepend + 50);

    buf.Append(string(1000, 'z'));
    GTEST_ASSERT_EQ(buf.ReadableBytes(), 1350);
    GTEST_ASSERT_EQ(buf.WriteableBytes(), 0);
    GTEST_ASSERT_EQ(buf.PrependableBytes(), Buffer::kCheapPrepend + 50);

    buf.RetrieveAll();
    GTEST_ASSERT_EQ(buf.ReadableBytes(), 0);
    GTEST_ASSERT_EQ(buf.WriteableBytes(), 1400);
    GTEST_ASSERT_EQ(buf.PrependableBytes(), Buffer::kCheapPrepend);
}

TEST(TestBuffer, testBufferInsideGrow) {
    Buffer buf;
    buf.Append(string(800, 'y'));
    GTEST_ASSERT_EQ(buf.ReadableBytes(), 800);
    GTEST_ASSERT_EQ(buf.WriteableBytes(), Buffer::kInitialSize - 800);

    buf.Retrieve(500);
    GTEST_ASSERT_EQ(buf.ReadableBytes(), 300);
    GTEST_ASSERT_EQ(buf.WriteableBytes(), Buffer::kInitialSize - 800);
    GTEST_ASSERT_EQ(buf.PrependableBytes(), Buffer::kCheapPrepend + 500);

    buf.Append(string(300, 'z'));
    GTEST_ASSERT_EQ(buf.ReadableBytes(), 600);
    GTEST_ASSERT_EQ(buf.WriteableBytes(), Buffer::kInitialSize - 600);
    GTEST_ASSERT_EQ(buf.PrependableBytes(), Buffer::kCheapPrepend);
}

TEST(TestBuffer, testBufferShrink) {
    Buffer buf;
    buf.Append(string(2000, 'y'));
    GTEST_ASSERT_EQ(buf.ReadableBytes(), 2000);
    GTEST_ASSERT_EQ(buf.WriteableBytes(), 0);
    GTEST_ASSERT_EQ(buf.PrependableBytes(), Buffer::kCheapPrepend);

    buf.Retrieve(1500);
    GTEST_ASSERT_EQ(buf.ReadableBytes(), 500);
    GTEST_ASSERT_EQ(buf.WriteableBytes(), 0);
    GTEST_ASSERT_EQ(buf.PrependableBytes(), Buffer::kCheapPrepend + 1500);

    buf.Shrink();
    GTEST_ASSERT_EQ(buf.ReadableBytes(), 500);
    GTEST_ASSERT_EQ(buf.WriteableBytes(), 0);
    GTEST_ASSERT_EQ(buf.RetrieveAllAsString(), string(500, 'y'));
    GTEST_ASSERT_EQ(buf.PrependableBytes(), Buffer::kCheapPrepend);
}

TEST(TestBuffer, testBufferPrepend) {
    Buffer buf;
    buf.Append(string(200, 'y'));
    GTEST_ASSERT_EQ(buf.ReadableBytes(), 200);
    GTEST_ASSERT_EQ(buf.WriteableBytes(), Buffer::kInitialSize - 200);
    GTEST_ASSERT_EQ(buf.PrependableBytes(), Buffer::kCheapPrepend);

    int x = 0;
    buf.Prepend(&x, sizeof x);
    GTEST_ASSERT_EQ(buf.ReadableBytes(), 204);
    GTEST_ASSERT_EQ(buf.WriteableBytes(), Buffer::kInitialSize - 200);
    GTEST_ASSERT_EQ(buf.PrependableBytes(), Buffer::kCheapPrepend - 4);
}

TEST(TestBuffer, testReadInts) {
    Buffer buf;
    buf.Append("HTTP");

    EXPECT_EQ(buf.ReadableBytes(), 4);
    
    EXPECT_EQ(buf.PeekInt8(), 'H');
    int top16 = buf.PeekInt16();
    EXPECT_EQ(top16, 'H' * 256 + 'T');
    EXPECT_EQ(buf.PeekInt32(), top16 * 65536 + 'T' * 256 + 'P');

    EXPECT_EQ(buf.ReadInt8(), 'H');
    EXPECT_EQ(buf.ReadInt16(), 'T' * 256 + 'T');
    EXPECT_EQ(buf.ReadInt8(), 'P');
    EXPECT_EQ(buf.ReadableBytes(), 0);
    EXPECT_EQ(buf.WriteableBytes(), Buffer::kInitialSize);

    buf.AppendInt8(-1);
    buf.AppendInt16(-2);
    buf.AppendInt32(-3);
    EXPECT_EQ(buf.ReadableBytes(), 7);
    EXPECT_EQ(buf.ReadInt8(), -1);
    EXPECT_EQ(buf.ReadInt16(), -2);
    EXPECT_EQ(buf.ReadInt32(), -3);
}

#endif