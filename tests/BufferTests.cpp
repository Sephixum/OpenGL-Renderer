#include <gtest/gtest.h>

#include "Graphics/Buffer.hpp"
#include "Utils/Log.hpp"
#include <glad.h>
#include <GLFW/glfw3.h>

// Include your buffer headers (adjust paths as needed)

// -----------------------------------------------------------------------------
// Test fixture that creates an OpenGL context for all tests
// -----------------------------------------------------------------------------
class OpenGLBufferTest : public ::testing::Test
{
  protected:
    static void SetUpTestSuite()
    {
      ASSERT_TRUE(::glfwInit()) << "Failed to initialize GLFW";

      ::glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
      window = ::glfwCreateWindow(640, 480, "OpenGL Test", nullptr, nullptr);
      ASSERT_NE(window, nullptr) << "Failed to create GLFW window";

      ::glfwMakeContextCurrent(window);

      ASSERT_TRUE(::gladLoadGL(::glfwGetProcAddress)) << "Failed to load OpenGL functions";
    }

    static void TearDownTestSuite()
    {
      ::glfwDestroyWindow(window);
      ::glfwTerminate();
    }

    static inline ::GLFWwindow* window = nullptr;
};

template<typename T>
std::vector<T> ReadBuffer(GLuint bufferID, std::size_t count)
{
  std::vector<T> data(count);
  glGetNamedBufferSubData(bufferID, 0, count * sizeof(T), data.data());
  return data;
}

// -----------------------------------------------------------------------------
// StaticBuffer tests
// -----------------------------------------------------------------------------


TEST_F(OpenGLBufferTest, StaticBuffer_WriteAndRead)
{
  glr::StaticBuffer<int, 5> buffer("TestStatic");
  ASSERT_NE(buffer.GetID(), 0u);

  std::vector<int> writeData = {1, 2, 3, 4, 5};
  buffer.WriteData(std::as_bytes(std::span(writeData)), 0);

  auto readBack = ReadBuffer<int>(buffer.GetID(), 5);
  glr::log::Info{"{}", readBack};
  EXPECT_EQ(readBack, writeData);
  EXPECT_EQ(buffer.Size(), 5);
  EXPECT_EQ(buffer.SizeBytes(), 5 * sizeof(int));
}

TEST_F(OpenGLBufferTest, StaticBuffer_WriteAtOffset)
{
  glr::StaticBuffer<int, 10> buffer;
  ASSERT_NE(buffer.GetID(), 0u);

  std::vector<int> data = {100, 200, 300};
  buffer.WriteData(std::as_bytes(std::span(data)), 2 * sizeof(int));

  auto readBack = ReadBuffer<int>(buffer.GetID(), 10);
  std::vector<int> expected = {0, 0, 100, 200, 300, 0, 0, 0, 0, 0};
  EXPECT_EQ(readBack, expected);
}

TEST_F(OpenGLBufferTest, StaticBuffer_WriteExceedsSize_ExpectFailure)
{
  glr::StaticBuffer<int, 5> buffer;
  std::vector<int> tooLarge(6, 42);
  auto bytes = std::as_bytes(std::span(tooLarge));
  EXPECT_DEATH({ buffer.WriteData(bytes, 0); }, "");
}

TEST_F(OpenGLBufferTest, StaticBuffer_WritePartialValid)
{
  glr::StaticBuffer<int, 10> buffer;
  std::vector<int> data = {99, 88, 77};
  buffer.WriteData(std::as_bytes(std::span(data)), 7 * sizeof(int));

  auto readBack = ReadBuffer<int>(buffer.GetID(), 10);
  std::vector<int> expected = {0,0,0,0,0,0,0,99,88,77};
  EXPECT_EQ(readBack, expected);
}

// -----------------------------------------------------------------------------
// StaticPersistantBuffer tests
// -----------------------------------------------------------------------------

TEST_F(OpenGLBufferTest, StaticPersistantBuffer_WriteAndView)
{
  glr::StaticPersistantBuffer<int, 8> buffer("TestPersist");
  ASSERT_NE(buffer.GetID(), 0u);

  std::vector<int> writeData = {10, 20, 30};
  buffer.WriteData(std::as_bytes(std::span(writeData)), 0);

  auto view = buffer.View();
  EXPECT_EQ(view[0], 10);
  EXPECT_EQ(view[1], 20);
  EXPECT_EQ(view[2], 30);

  auto readBack = ReadBuffer<int>(buffer.GetID(), 8);
  EXPECT_EQ(readBack[0], 10);
  EXPECT_EQ(readBack[1], 20);
  EXPECT_EQ(readBack[2], 30);
}

TEST_F(OpenGLBufferTest, StaticPersistantBuffer_WriteAtOffset)
{
  glr::StaticPersistantBuffer<int, 12> buffer;
  std::vector<int> first = {1, 1, 1};
  buffer.WriteData(std::as_bytes(std::span(first)), 0);
  std::vector<int> second = {2, 2};
  buffer.WriteData(std::as_bytes(std::span(second)), 5 * sizeof(int));

  auto view = buffer.View();
  EXPECT_EQ(view[0], 1);
  EXPECT_EQ(view[1], 1);
  EXPECT_EQ(view[2], 1);
  EXPECT_EQ(view[5], 2);
  EXPECT_EQ(view[6], 2);
}

TEST_F(OpenGLBufferTest, StaticPersistantBuffer_BoundsCheck)
{
  glr::StaticPersistantBuffer<int, 4> buffer;
  std::vector<int> tooLarge(5, 123);
  auto bytes = std::as_bytes(std::span(tooLarge));
  EXPECT_DEATH({ buffer.WriteData(bytes, 0); }, "");
}

TEST_F(OpenGLBufferTest, StaticPersistantBuffer_IteratorSupport)
{
  glr::StaticPersistantBuffer<int, 5> buffer;
  std::vector<int> data = {5, 4, 3, 2, 1};
  buffer.WriteData(std::as_bytes(std::span(data)), 0);

  std::vector<int> fromIterators(buffer.begin(), buffer.end());
  EXPECT_EQ(fromIterators, data);
}

TEST_F(OpenGLBufferTest, StaticPersistantBuffer_MultipleWritesOverlap)
{
  glr::StaticPersistantBuffer<int, 6> buffer;
  std::vector<int> first = {10, 20};
  buffer.WriteData(std::as_bytes(std::span(first)), 0);
  std::vector<int> second = {30, 40, 50};
  buffer.WriteData(std::as_bytes(std::span(second)), 1 * sizeof(int));

  auto view = buffer.View();
  EXPECT_EQ(view[0], 10);
  EXPECT_EQ(view[1], 30);
  EXPECT_EQ(view[2], 40);
  EXPECT_EQ(view[3], 50);
}

// -----------------------------------------------------------------------------
// DynamicBuffer tests
// -----------------------------------------------------------------------------

TEST_F(OpenGLBufferTest, DynamicBuffer_WriteAndRead)
{
  glr::DynamicBuffer<int> buffer(4);
  std::vector<int> writeData = {42, 43, 44};
  buffer.WriteData(std::as_bytes(std::span(writeData)), 0);

  auto readBack = ReadBuffer<int>(buffer.GetID(), writeData.size());
  EXPECT_EQ(readBack, writeData);
  EXPECT_EQ(buffer.Size(), 3);
  EXPECT_EQ(buffer.Capacity(), 4);
}

TEST_F(OpenGLBufferTest, DynamicBuffer_GrowsWhenNeeded)
{
  glr::DynamicBuffer<int> buffer(2);
  std::vector<int> large(100, 99);
  buffer.WriteData(std::as_bytes(std::span(large)), 0);

  EXPECT_GE(buffer.Capacity(), 100);
  EXPECT_EQ(buffer.Size(), 100);

  auto readBack = ReadBuffer<int>(buffer.GetID(), 100);
  EXPECT_EQ(readBack, large);
}

TEST_F(OpenGLBufferTest, DynamicBuffer_WriteAtOffsetUpdatesUsedSize)
{
  glr::DynamicBuffer<int> buffer(10);
  std::vector<int> first = {1, 2, 3};
  buffer.WriteData(std::as_bytes(std::span(first)), 0);
  EXPECT_EQ(buffer.Size(), 3);

  std::vector<int> second = {10, 11, 12};
  buffer.WriteData(std::as_bytes(std::span(second)), 5 * sizeof(int));
  EXPECT_EQ(buffer.Size(), 8);

  auto readBack = ReadBuffer<int>(buffer.GetID(), 8);
  EXPECT_EQ(readBack[0], 1);
  EXPECT_EQ(readBack[1], 2);
  EXPECT_EQ(readBack[2], 3);
  EXPECT_EQ(readBack[5], 10);
  EXPECT_EQ(readBack[6], 11);
  EXPECT_EQ(readBack[7], 12);
}

// -----------------------------------------------------------------------------
// DynamicPersistantBuffer tests (using std::array for Append)
// -----------------------------------------------------------------------------

TEST_F(OpenGLBufferTest, DynamicPersistantBuffer_AppendAndView)
{
  glr::DynamicPersistantBuffer<int> buffer(4);
  std::array data = {5, 6, 7};
  buffer.Append(data);

  auto view = buffer.View();
  ASSERT_EQ(view.size(), 3);
  EXPECT_EQ(view[0], 5);
  EXPECT_EQ(view[1], 6);
  EXPECT_EQ(view[2], 7);
  EXPECT_EQ(buffer.Size(), 3);
}

TEST_F(OpenGLBufferTest, DynamicPersistantBuffer_GrowAndAppend)
{
  glr::DynamicPersistantBuffer<int> buffer(1);
  std::array data = {1, 2, 3, 4, 5};
  buffer.Append(data);
  EXPECT_GE(buffer.Capacity(), 5);
  EXPECT_EQ(buffer.Size(), 5);
  EXPECT_EQ(buffer.View()[4], 5);
}

TEST_F(OpenGLBufferTest, DynamicPersistantBuffer_WriteDataWithOffset)
{
  glr::DynamicPersistantBuffer<int> buffer(8);
  std::array data1 = {1, 2, 3};
  buffer.WriteData(std::as_bytes(std::span(data1)), 0);
  std::array data2 = {10, 11};
  buffer.WriteData(std::as_bytes(std::span(data2)), 3 * sizeof(int));

  auto view = buffer.View();
  ASSERT_EQ(view.size(), 5);
  EXPECT_EQ(view[0], 1);
  EXPECT_EQ(view[1], 2);
  EXPECT_EQ(view[2], 3);
  EXPECT_EQ(view[3], 10);
  EXPECT_EQ(view[4], 11);
}

TEST_F(OpenGLBufferTest, DynamicPersistantBuffer_AppendAndWriteDataMixed)
{
  glr::DynamicPersistantBuffer<int> buffer(2);
  buffer.Append(std::array{100, 200});
  auto data = std::array{300};
  buffer.WriteData(std::as_bytes(std::span{data}), 2 * sizeof(int));
  EXPECT_EQ(buffer.Size(), 3);
  EXPECT_EQ(buffer.View()[2], 300);
}

TEST_F(OpenGLBufferTest, DynamicPersistantBuffer_PushBackSingleElement)
{
  glr::DynamicPersistantBuffer<int> buffer(2);
  buffer.Append(42);
  buffer.Append(43);
  EXPECT_EQ(buffer.Size(), 2);
  EXPECT_EQ(buffer.View()[0], 42);
  EXPECT_EQ(buffer.View()[1], 43);
}

TEST_F(OpenGLBufferTest, DynamicPersistantBuffer_DataPersistsAfterGrow)
{
  glr::DynamicPersistantBuffer<int> buffer(3);
  buffer.Append(std::array{1, 2, 3});
  buffer.Append(std::array{4, 5, 6, 7, 8});
  auto view = buffer.View();
  ASSERT_EQ(view.size(), 8);
  for (size_t i = 0; i < 8; ++i)
  {
    EXPECT_EQ(view[i], static_cast<int>(i + 1));
  }
}
