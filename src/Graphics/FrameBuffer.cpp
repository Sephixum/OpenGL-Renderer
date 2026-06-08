#include "FrameBuffer.hpp"
#include "Graphics/IGLResource.hpp"
#include "Graphics/Texture.hpp"
#include "Graphics/SamplerLibrary.hpp"

namespace glr
{

  FrameBuffer::FrameBuffer(FramebufferCreateInfo const& info, std::string_view name)
    : _width{info.width}
    , _height{info.height}
    , _use_depthstencil{info.use_depthstencil}
  {
    _id = {0u, [](auto e){ ::glDeleteFramebuffers(1, &e);}};
    ::glCreateFramebuffers(1, &_id);
    ::glObjectLabel(GL_FRAMEBUFFER, _id, name.length(), name.data());

    auto color_count = u64{};
    for (auto const& att : info.color_attachments)
    {
      if (not att.has_value()) continue;

      auto const& att_value = att.value();
      auto w = att_value.width  ? att_value.width  : info.width;
      auto h = att_value.height ? att_value.height : info.height;

      auto texture_create_info = Texture2DCreateInfo{
        .width   = w,
        .height  = h,
        .format  = att_value.format,
        .sampler = SamplerLibrary::
      };
    }

  }

  auto Bind()   -> void;
  auto UnBind() -> void;

  [[nodiscard]] auto TryGetAttachment() -> Texture2D const*;
  [[nodiscard]] auto GetAttachment()    -> Texture2D const&;

  [[nodiscard]] auto TryGetDepthStencil() -> Texture2D const*;
  [[nodiscard]] auto GetDepthStencil()    -> Texture2D const&;

}
