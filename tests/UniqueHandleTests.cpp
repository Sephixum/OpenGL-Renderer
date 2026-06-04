#include <gtest/gtest.h>

#include "Utils/UniqueHandle.hpp"

namespace
{

  using AutoIntPtr = glr::UniqueHandle<int*, nullptr>;

  void deleter(int *value)
  {
    --*value;
  }

}

TEST(unique_handle, ctor)
{
  auto x = 1;
  auto v = AutoIntPtr{&x, deleter};

  ASSERT_EQ(v.Get(), &x);
  ASSERT_EQ(static_cast<int *>(v), &x);
  ASSERT_TRUE(v);
}

TEST(unique_handle, function_deleter)
{
  auto x = 1;

  {
    auto v = AutoIntPtr{&x, deleter};
  }

  ASSERT_EQ(x, 0);
}

TEST(unique_handle, lambda_deleter)
{
  auto x = 1;
  auto y = 2;

  {
    auto v = AutoIntPtr{&x, [y](int* p) { *p += y; }};
  }

  ASSERT_EQ(x, 3);
}

TEST(unique_handle, invalid)
{
  auto v = AutoIntPtr{nullptr, deleter};

  ASSERT_EQ(v.Get(), nullptr);
  ASSERT_FALSE(v);
}

TEST(unique_handle, move_ctor)
{
  auto x = 1;

  {
    auto v1 = AutoIntPtr{&x, deleter};
    auto v2 = AutoIntPtr{std::move(v1)};

    ASSERT_FALSE(v1);
    ASSERT_EQ(v1.Get(), nullptr);
    ASSERT_TRUE(v2);
    ASSERT_EQ(v2.Get(), &x);
  }

  ASSERT_EQ(x, 0);
}

TEST(unique_handle, move_assignment)
{
  auto x = 1;
  auto y = 1;

  {
    auto v1 = AutoIntPtr{&x, deleter};
    auto v2 = AutoIntPtr{&y, deleter};
    v2      = std::move(v1);

    ASSERT_FALSE(v1);
    ASSERT_EQ(v1.Get(), nullptr);
    ASSERT_TRUE(v2);
    ASSERT_EQ(v2.Get(), &x);
  }

  ASSERT_EQ(x, 0);
  ASSERT_EQ(y, 0);
}

TEST(unique_handle, address)
{
  auto x = 1;

  {
    auto       v      = AutoIntPtr{nullptr, deleter};
    auto const setter = [&x](int** p) { *p = std::addressof(x); };
    setter(&v);
  }

  ASSERT_EQ(x, 0);
}
