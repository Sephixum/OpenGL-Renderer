#include "FrameBuffer.hpp"
#include "Core/Application.hpp"
#include "Graphics/IGLResource.hpp"
#include "Graphics/Texture.hpp"
#include "Graphics/SamplerLibrary.hpp"
#include "Utils/InRangeOf.hpp"

#include <algorithm>
#include <numeric>
#include <ranges>
#include <utility>

namespace vws = std::views;
namespace rng = std::ranges;

namespace
{

  constexpr auto to_string(glr::FramebufferSlotType t)
  {
    switch (t)
    {
      using enum glr::FramebufferSlotType;
      case  Albedo   : return "Albedo";
      case  Normal   : return "Normal";
      case  Position : return "Position";
      case  Material : return "Material";

      // Lighting / post‑process
      case HDRColor  : return "HDRColor";
      case Bloom     : return "Bloom";
      case Luminance : return "Luminance";
      case SSAO      : return "SSAO";
      case Velocity  : return "Velocity";

      // Shadow / depth
      case ShadowMap: return "ShadowMap";
      case DepthOnly: return "DepthOnly";

      // Custom / user‑defined slots – add as many as you like
      case Custom0: return "Custom0";
      case Custom1: return "Custom1";
      case Custom2: return "Custom2";
      case Custom3: return "Custom3";
      case Custom4: return "Custom4";
      case Custom5: return "Custom5";
      case Custom6: return "Custom6";
      case Custom7: return "Custom7";
    }
    std::unreachable();
  }

}

namespace glr
{

  FrameBuffer::FrameBuffer(FramebufferCreateInfo const& info, std::string_view name)
    : _width  { info.width  }
    , _height { info.height }
    , _name   { name        }
  {
    _id = {0u, [](auto e){ ::glDeleteFramebuffers(1, &e);}};
    ::glCreateFramebuffers(1, &_id);
    ::glObjectLabel(GL_FRAMEBUFFER, _id, name.length(), name.data());

    // Build color attachments
    auto color_count = u64{};

    for (auto const& att : info.color_attachments)
    {
      if (not att.has_value()) continue;

      auto const& src = att.value();
      auto const  w   = src.width  ? src.width  : info.width;
      auto const  h   = src.height ? src.height : info.height;

      auto texture = Texture2D(
        Texture2DCreateInfo{
          .width         = w,
          .height        = h,
          .format        = src.format,
          .sampler       = SamplerLibrary::FramebufferClamp(),
          .data          = std::nullopt,
          .mipmap_levels = 1
        },
        std::format("{}_ColorAttachment_{}", name, to_string(src.slot))
      );

      ::glNamedFramebufferTexture(_id, GL_COLOR_ATTACHMENT0 + color_count, texture.GetID(), 0);
      _attachments[color_count].emplace(ColorAttachment{
        .texture = std::move(texture),
        .info    = { .format = src.format, .slot = src.slot, .width = src.width, .height = src.height }
      });

      ++color_count;
    }

    // Build depth/stencil attachment
    if (info.use_depthstencil)
    {
      auto depth = Texture2D(
        Texture2DCreateInfo{
          .width         = info.width,
          .height        = info.height,
          .format        = TextureFormatType::Depth24stencil8,
          .sampler       = SamplerLibrary::ShadowMapDepth(),
          .data          = std::nullopt,
          .mipmap_levels = 1
        },
        std::format("{}_DepthStencil", name)
      );

      ::glNamedFramebufferTexture(_id, GL_DEPTH_STENCIL_ATTACHMENT, depth.GetID(), 0);
      _depth_texture.emplace(std::move(depth));
    }

    // Specify draw buffers
    auto draw_buffers = std::vector<::GLenum>(color_count);
    std::iota(draw_buffers.begin(), draw_buffers.end(), GL_COLOR_ATTACHMENT0);
    ::glNamedFramebufferDrawBuffers(_id, static_cast<GLsizei>(draw_buffers.size()), draw_buffers.data());

    Expect(
      ::glCheckNamedFramebufferStatus(_id, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE,
      "FrameBuffer {} is incomplete!", name
    );

    _on_resize_event_sink = Application::GetInstance().GetEventBus().sink<Event::Resize>().connect<&FrameBuffer::OnResize>(this);
  }

  auto FrameBuffer::Bind()   -> void
  {
    ::glBindFramebuffer(GL_FRAMEBUFFER, _id);
  }

  auto FrameBuffer::UnBind() -> void
  {
    ::glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  auto FrameBuffer::TryGetAttachment(FramebufferSlotType t) const -> Texture2D const*
  {
    for (auto const& att : _attachments)
    {
      if (att.has_value() and att->info.slot == t )
      {
        return &(att->texture);
      }
    }

    return nullptr;
  }

  auto FrameBuffer::GetAttachment(FramebufferSlotType t) const -> Texture2D const&
  {
    auto const* tex = TryGetAttachment(t);
    if (!tex)
    {
      throw Exception{"Framebuffer slot not found"};
    }
    return *tex;
  }

  auto FrameBuffer::TryGetDepthStencil() const -> Texture2D const*
  {
    return _depth_texture.has_value() ? &(_depth_texture.value()) : nullptr;
  }

  auto FrameBuffer::GetDepthStencil() const -> Texture2D const&
  {
    if (not _depth_texture.has_value())
    {
      throw Exception{"Framebuffer has no depth/stencil attachment"};
    }
    return _depth_texture.value();
  }

  auto FrameBuffer::OnResize(Event::Resize const& e) -> void
  {
    auto const new_width  = static_cast<u32>(e.width);
    auto const new_height = static_cast<u32>(e.height);

    if (new_width == _width and new_height == _height) return;

    ::glFinish();

    _width  = new_width;
    _height = new_height;

    auto color_count = u64{};
    for (auto& att : _attachments)
    {
      if (not att.has_value()) continue;

      auto const w = att->info.width  ? att->info.width  : _width;
      auto const h = att->info.height ? att->info.height : _height;

      att->texture.Resize(w, h);
      ::glNamedFramebufferTexture(_id, GL_COLOR_ATTACHMENT0 + color_count, att->texture.GetID(), 0);

      ++color_count;
    }

    if (_depth_texture.has_value())
    {
      _depth_texture->Resize(_width, _height);
      ::glNamedFramebufferTexture(_id, GL_DEPTH_STENCIL_ATTACHMENT, _depth_texture->GetID(), 0);
    }

    Expect(
      ::glCheckNamedFramebufferStatus(_id, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE,
      "FrameBuffer {} incomplete after resize!", _name
    );
  }
  
}
