#pragma once

#include "Utils/Error.hpp"
#include "IGLResource.hpp"
#include "Utils/Formatter.hpp"

#include <glad.h>
#include <span>
#include <string_view>
#include <utility>
#include <cstring>

namespace glr
{

  enum struct BufferType
  {
    ShaderStorage     = GL_SHADER_STORAGE_BUFFER,
    Index             = GL_ELEMENT_ARRAY_BUFFER,
    DrawIndirect      = GL_DRAW_INDIRECT_BUFFER,
    Vertex            = GL_ARRAY_BUFFER,
    TransformFeedback = GL_TRANSFORM_FEEDBACK_BUFFER,
    Uniform           = GL_UNIFORM_BUFFER
  };
  
  class IBuffer : public IGLResource
  {
    friend class VertexArray;

    public:
      virtual auto WriteData(std::span<std::byte const> data, std::size_t offset = 0) -> void = 0;
      virtual ~IBuffer() = default;

    private:
      template<BufferType Type>
        requires (Type != BufferType::ShaderStorage)
             and (Type != BufferType::TransformFeedback)
             and (Type != BufferType::Uniform)
      auto BindAs() -> void
      {
        ::glBindBuffer(std::to_underlying(Type), _id);
      }

      template<BufferType Type>
        requires (Type == BufferType::ShaderStorage)
              or (Type == BufferType::TransformFeedback)
              or (Type == BufferType::Uniform)
      auto BindAs(uint32_t binding_index = 0) -> void
      {
        ::glBindBufferBase(std::to_underlying(Type), binding_index, _id);
      }
  };
  
  template <typename T>
  concept IsBuffer = std::derived_from<T, IBuffer>;
  
  template <typename T, std::size_t Count>
  class StaticBuffer : public IBuffer
  {
    std::string _name = {};

    public:
      StaticBuffer(std::string_view name = "Unknown StaticBuffer")
        : _name{name}
      {
        _id = {0u, [](auto e){::glDeleteBuffers(1, &e);}};
        ::glCreateBuffers(1, &_id);
        ::glNamedBufferStorage(_id, Count * sizeof(T), nullptr, GL_DYNAMIC_STORAGE_BIT);
        ::glObjectLabel(GL_BUFFER, _id, name.length(), name.data());
      }

      virtual auto WriteData(std::span<std::byte const> data, std::size_t offset = 0) -> void override final
      {
        Expect(data.size() + offset <= SizeBytes(), "data.size() + offset > SizeBytes()");
        ::glNamedBufferSubData(_id, offset, data.size(), data.data());
      }

      constexpr auto Size()      -> std::size_t { return Count; }
      constexpr auto SizeBytes() -> std::size_t { return sizeof(T) * Count; }
  };

  template <typename T, std::size_t Count>
  class StaticPersistantBuffer : public IBuffer
  {
    T*          _map  = nullptr;
    std::string _name = {};

    public:
      StaticPersistantBuffer(std::string_view name = "Unknown StaticPersistantBuffer")
        : _name{name}
      {
        constexpr auto const flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
        _id = {0u, [](auto e){::glUnmapNamedBuffer(e); ::glDeleteBuffers(1, &e);}};
        ::glCreateBuffers(1, &_id);
        ::glNamedBufferStorage(_id, sizeof(T) * Count, nullptr, GL_DYNAMIC_STORAGE_BIT | flags);
        ::glObjectLabel(GL_BUFFER, _id, name.length(), name.data());
        _map = static_cast<T*>(::glMapNamedBufferRange(_id, static_cast<GLintptr>(0), sizeof(T) * Count, flags));
      }

      virtual auto WriteData(std::span<std::byte const> data, std::size_t offset = 0) -> void override final
      {
        Expect(data.size() + offset <= SizeBytes(), "data.size() + offset > SizeBytes()");
        std::memcpy(reinterpret_cast<std::byte*>(_map) + offset, data.data(), data.size());
        ::glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);
      }

      auto begin() { return _map; }
      auto end()   { return _map + Count; }

      [[nodiscard]] constexpr auto Size()      -> std::size_t { return Count; }
      [[nodiscard]] constexpr auto SizeBytes() -> std::size_t { return sizeof(T) * Count; }

      [[nodiscard]] auto View() -> std::span<T> { return {_map, Count}; }
  };

  template <typename T>
  class DynamicBuffer : public IBuffer
  {
    std::size_t _used     = 0;
    std::size_t _capacity = 0;
    std::string _name     = "Unknown DynamicPersistantBuffer";

    auto Grow(std::size_t new_capacity) -> void
    {
      auto old_id = _id.Release();

      auto new_id = ::GLuint{};
      ::glCreateBuffers(1, &new_id);
      ::glNamedBufferStorage(new_id, sizeof(T) * new_capacity, nullptr, GL_DYNAMIC_STORAGE_BIT);
      ::glObjectLabel(GL_BUFFER, new_id, _name.length(), _name.data());

      if (_used > 0)
      {
        ::glCopyNamedBufferSubData(old_id, new_id, 0, 0, sizeof(T) * _used);
      }
      ::glDeleteBuffers(1, &old_id);

      _id.Reset(new_id);
      _capacity = new_capacity;
    }

    public:
      DynamicBuffer(std::size_t reserve_count, std::string_view name = "Unknown DynamicBuffer")
        : _capacity{std::max(1zu, reserve_count)}
        , _used{0}
        , _name{name}
      {
        _id = {0u, [](auto e){::glDeleteBuffers(1, &e);}};
        ::glCreateBuffers(1, &_id);
        ::glNamedBufferStorage(_id, sizeof(T) * _capacity, nullptr, GL_DYNAMIC_STORAGE_BIT);
        ::glObjectLabel(GL_BUFFER, _id, name.length(), name.data());
      }

      virtual auto WriteData(std::span<std::byte const> data, std::size_t offset = 0) -> void override final
      {
        std::size_t const needed_bytes = offset + data.size();
        if (needed_bytes > CapacityBytes())
        {
          auto new_cap_bytes = CapacityBytes();
          if (new_cap_bytes == 0) new_cap_bytes = 1;

          while (new_cap_bytes < needed_bytes)
          {
            new_cap_bytes *= 2;
          }
          Grow(new_cap_bytes / sizeof(T));
        }
        ::glNamedBufferSubData(_id, offset, data.size(), data.data());

        auto const written_end_elements = (needed_bytes + sizeof(T) - 1) / sizeof(T);
        if (written_end_elements > _used) _used = written_end_elements;
      }

      auto Clear() -> void
      {
        _used = 0;
      }

      auto ClearAndZero() -> void
      {
        if (_used == 0)
        {
          return;
        }
        static auto const zeros = std::vector<std::byte>(_used * sizeof(T), std::byte{0});
        glNamedBufferSubData(_id, 0, zeros.size(), zeros.data());
        _used = 0;
      }

      [[nodiscard]] auto Size()          const -> std::size_t { return _used; }
      [[nodiscard]] auto Capacity()      const -> std::size_t { return _capacity; }       
      [[nodiscard]] auto SizeBytes()     const -> std::size_t { return _used * sizeof(T); }
      [[nodiscard]] auto CapacityBytes() const -> std::size_t { return _capacity * sizeof(T); }
  };

  template <typename T>
  class DynamicPersistantBuffer : public IBuffer
  {
    T*          _map      = nullptr;
    std::size_t _used     = 0;
    std::size_t _capacity = 0;
    std::string _name     = "Unknown DynamicPersistantBuffer";

    auto Grow(std::size_t new_capacity) -> void
    {
      constexpr auto const flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;

      auto old_id = _id.Release();
      ::glUnmapNamedBuffer(old_id);

      auto new_id = ::GLuint{};
      ::glCreateBuffers(1, &new_id);
      ::glNamedBufferStorage(new_id, sizeof(T) * new_capacity, nullptr, GL_DYNAMIC_STORAGE_BIT | flags);
      ::glObjectLabel(GL_BUFFER, new_id, _name.length(), _name.data());

      if (_used > 0)
      {
        ::glCopyNamedBufferSubData(old_id, new_id, 0, 0, sizeof(T) * _used);
      }
      ::glDeleteBuffers(1, &old_id);

      _id.Reset(new_id);
      _capacity = new_capacity;
      _map      = static_cast<T*>(::glMapNamedBufferRange(_id, 0, static_cast<GLsizeiptr>(sizeof(T) * _capacity), flags));
    }

    public:
      DynamicPersistantBuffer(std::size_t reserve_count, std::string_view name = "Unknown DynamicPersistentBuffer") 
        : _used{0}
        , _capacity{std::max(1zu, reserve_count)}
        , _name{name}
      {
        constexpr auto const flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
        _id = {0u, [](auto e){::glUnmapNamedBuffer(e); ::glDeleteBuffers(1, &e);}};
        ::glCreateBuffers(1, &_id);
        ::glNamedBufferStorage(_id, sizeof(T) * _capacity, nullptr, GL_DYNAMIC_STORAGE_BIT | flags);
        ::glObjectLabel(GL_BUFFER, _id, name.length(), name.data());
        _map = static_cast<T*>(::glMapNamedBufferRange(_id, static_cast<GLintptr>(0), sizeof(T) * _capacity, flags));
      }

      auto Append(std::span<T const> data) -> void
      {
        if (_used + data.size() > _capacity)
        {
          auto new_cap = _capacity;
          if (new_cap == 0) new_cap = 1;
          while (new_cap < (_used + data.size()))
          {
            new_cap *= 2;
          }
          Grow(new_cap);
        }
        std::memcpy(_map + _used, data.data(), data.size() * sizeof(T));
        _used += data.size();
        ::glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);
      }

      auto Append(T const& value) -> void
      {
        Append(std::span{&value, 1});
      }

      virtual auto WriteData(std::span<std::byte const> data, std::size_t offset = 0) -> void override final
      {
        std::size_t const needed_bytes = offset + data.size();
        if (needed_bytes > CapacityBytes())
        {
          auto new_cap_bytes = CapacityBytes();
          if (new_cap_bytes == 0) new_cap_bytes = 1;

          while (new_cap_bytes < needed_bytes)
          {
            new_cap_bytes *= 2;
          }
          Grow(new_cap_bytes / sizeof(T));
        }
        std::memcpy(reinterpret_cast<std::byte*>(_map) + offset, data.data(), data.size());
        ::glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);

        auto const written_end_elements = (needed_bytes + sizeof(T) - 1) / sizeof(T);
        if (written_end_elements > _used) _used = written_end_elements;
      }

      auto Clear() -> void
      {
        _used = 0;
      }

      auto ClearAndZero() -> void
      {
        if (_used == 0)
        {
          return;
        }
        std::memset(_map, 0, _used);
        ::glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);
        _used = 0;
      }

      [[nodiscard]] auto Size()          const -> std::size_t { return _used; }
      [[nodiscard]] auto Capacity()      const -> std::size_t { return _capacity; }       
      [[nodiscard]] auto SizeBytes()     const -> std::size_t { return _used * sizeof(T); }
      [[nodiscard]] auto CapacityBytes() const -> std::size_t { return _capacity * sizeof(T); }

      [[nodiscard]] std::span<T>       View()       { return {_map, _used}; }
      [[nodiscard]] std::span<const T> View() const { return {_map, _used}; }

      auto begin() { return _map; }
      auto end()   { return _map + _used; }
  };

  [[nodiscard]] constexpr auto to_string(BufferType t)
  {
    switch (t)
    {
      using enum BufferType;
      case ShaderStorage     : return  "ShaderStorage";
      case Index             : return  "Index";
      case DrawIndirect      : return  "DrawIndirect";
      case Vertex            : return  "Vertex";
      case TransformFeedback : return  "TransformFeedback";
      case Uniform           : return  "Uniform";
    }
    std::unreachable();
  }
  static_assert(CanFormat<BufferType>);
}
