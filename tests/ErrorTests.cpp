#include <gtest/gtest.h>

#include "Utils/Error.hpp"
#include "Utils/UniqueHandle.hpp"

TEST(error, expect_true)
{
  EXPECT_NO_THROW(glr::Expect(true, "This should not throw"));
}

TEST(error, expect_false)
{
  EXPECT_DEATH(glr::Expect(false, "This should terminate"), "");
}

TEST(error, ensure_true)
{
  EXPECT_NO_THROW(glr::Ensure(true, "This should not throw"));
}

TEST(error, ensure_false)
{
  EXPECT_THROW(glr::Ensure(false, "This should throw"), glr::Exception);
}

TEST(error, ensure_auto_release)
{
  auto ptr = glr::UniqueHandle<int *, nullptr>{new int{}, [](int *p) { delete p; }};

    EXPECT_NO_THROW(glr::Ensure(ptr, "This should not throw"));

    auto invalid_ptr = glr::UniqueHandle<int *, nullptr>{nullptr, nullptr};

    EXPECT_THROW(glr::Ensure(invalid_ptr, "This should throw"), glr::Exception);
}

TEST(error, ensure_unique_ptr)
{
  std::unique_ptr<int> ptr = std::make_unique<int>(42);

  EXPECT_NO_THROW(glr::Ensure(ptr, "This should not throw"));

  std::unique_ptr<int> invalid_ptr;

  EXPECT_THROW(glr::Ensure(invalid_ptr, "This should throw"), glr::Exception);
}
