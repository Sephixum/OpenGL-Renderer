#include "FrameBuffer.hpp"
#include "Core/Application.hpp"
#include "Graphics/IGLResource.hpp"
#include "Graphics/Texture.hpp"
#include "Graphics/SamplerLibrary.hpp"
#include "Utils/InRangeOf.hpp"

#include <algorithm>
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
    : _width{info.width}
    , _height{info.height}
    , _use_depthstencil{info.use_depthstencil}
    , _name{name}
  {
    _id = {0u, [](auto e){ ::glDeleteFramebuffers(1, &e);}};
    ::glCreateFramebuffers(1, &_id);
    ::glObjectLabel(GL_FRAMEBUFFER, _id, name.length(), name.data());

    auto color_count = u64{};
    for (auto const [idx, att] : info.color_attachments | vws::enumerate)
    {
      if (not att.has_value()) continue;

      auto const& att_value = att.value();
      auto w = att_value.width  ? att_value.width  : info.width;
      auto h = att_value.height ? att_value.height : info.height;

      auto texture_create_info = Texture2DCreateInfo{
        .width         = w,
        .height        = h,
        .format        = att_value.format,
        .sampler       = SamplerLibrary::FramebufferClamp(),
        .data          = std::nullopt,
        .mipmap_levels = 1
      };

      auto tex = Texture2D(texture_create_info, std::format("{}_ColorAttachment_{}", name, to_string(att_value.slot)));
      ::glNamedFramebufferTexture(_id, GL_COLOR_ATTACHMENT0 + color_count, tex.GetID(), 0);

      _attachments[color_count].emplace(
          std::move(tex),
          ColorAttachmentInfo{.format = att_value.format, .slot = att_value.slot}
      );

      ++color_count;
    }

    if (_use_depthstencil)
    {
      auto tex_stencil_info = Texture2DCreateInfo{
        .width         = info.width,
        .height        = info.height,
        .format        = TextureFormatType::Depth24stencil8,
        .sampler       = SamplerLibrary::ShadowMapDepth(),
        .data          = std::nullopt,
        .mipmap_levels = 1
      };

      auto depth_texture = Texture2D{tex_stencil_info, std::format("{}_DepthStencil", name)};
      ::glNamedFramebufferTexture(_id, GL_DEPTH_STENCIL_ATTACHMENT, depth_texture.GetID(), 0);
      _depth_texture.emplace(std::move(depth_texture));
    }

    auto draw_buffers = std::vector<::GLenum>{};
    for (auto i : InRangeOf(0zu, color_count))
    {
      draw_buffers.push_back(GL_COLOR_ATTACHMENT0 + i);
    }
    ::glNamedFramebufferDrawBuffers(_id, draw_buffers.size(), draw_buffers.data());
    Expect((::glCheckNamedFramebufferStatus(_id, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE), "FrameBuffer {} is incomplete !", name);
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
    if (static_cast<u32>(e.width) == _width and static_cast<u32>(e.height) == _height) return;

    auto old_attachments = std::move(_attachments);
    auto had_depth       = _depth_texture.has_value();

    _depth_texture.reset();

    _width  = e.width;
    _height = e.height;

    {
      auto new_fbo = ::GLuint{};
      ::glCreateFramebuffers(1, &new_fbo);
      _id.Reset(new_fbo);
    }

    auto color_count = u64{};
    for (auto [idx, att] : old_attachments | vws::enumerate)
    {
      if (not att.has_value()) continue;

      auto const& old = att.value();
      auto tex_info = Texture2DCreateInfo{
        .width   = _width,
        .height  = _height,
        .format  = old.info.format,
        .sampler = SamplerLibrary::FramebufferClamp(),
        .data    = std::nullopt,
        .mipmap_levels = 1
      };

      auto new_texture = Texture2D(tex_info, std::format("{}_ColorAttachment_{}", _name, to_string(old.info.slot)));
      ::glNamedFramebufferTexture(_id, GL_COLOR_ATTACHMENT0 + color_count, new_texture.GetID(), 0);
      _attachments[color_count].emplace(ColorAttachment{
          .texture = std::move(new_texture),
          .info    = old.info
      });

      ++color_count;
    }

    if (had_depth)
    {
      auto depth_create_info = Texture2DCreateInfo{
        .width         = _width,
        .height        = _height,
        .format        = TextureFormatType::Depth24stencil8,
        .sampler       = SamplerLibrary::ShadowMapDepth(),
        .data          = std::nullopt,
        .mipmap_levels = 1
      };
      auto new_depth_stencil = Texture2D(depth_create_info, std::format("{}_DepthStencil", _name));
      ::glNamedFramebufferTexture(_id, GL_DEPTH_STENCIL_ATTACHMENT, new_depth_stencil.GetID(), 0);
      _depth_texture.emplace(std::move(new_depth_stencil));
    }

    auto draw_buffers = std::vector<::GLenum>{};
    for (auto i : InRangeOf(0zu, color_count))
    {
      draw_buffers.push_back(GL_COLOR_ATTACHMENT0 + i);
    }
    ::glNamedFramebufferDrawBuffers(_id, draw_buffers.size(), draw_buffers.data());
    Expect((::glCheckNamedFramebufferStatus(_id, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE), "FrameBuffer {} is incomplete !", _name);
  }

}
