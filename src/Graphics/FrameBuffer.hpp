#pragma once

#include "Core/EventBus.hpp"
#include "IGLResource.hpp"
#include "Texture.hpp"
#include "Utils/Utils.hpp"
#include "Core/Event.hpp"

namespace glr
{

  enum struct FramebufferSlotType
  {
    // G‑Buffer
    Albedo,
    Normal,
    Position,
    Material,

    // Lighting / post‑process
    HDRColor,
    Bloom,
    Luminance,
    SSAO,
    Velocity,

    // Shadow / depth
    ShadowMap,
    DepthOnly,

    // Custom / user‑defined slots – add as many as you like
    Custom0,
    Custom1,
    Custom2,
    Custom3,
    Custom4,
    Custom5,
    Custom6,
    Custom7,
  };

  struct FramebufferAttachmentCreateInfo
  {
    TextureFormatType   format = TextureFormatType::Rgba8unorm;
    u32                 width  = 0;
    u32                 height = 0;
    FramebufferSlotType slot   = FramebufferSlotType::Albedo;
  };

  struct FramebufferCreateInfo 
  {
    u32                                            width  = 0;
    u32                                            height = 0;
    std::array
    <
      std::optional<FramebufferAttachmentCreateInfo>, 8
    >                                              color_attachments = {};
    bool                                           use_depthstencil  = true;   // adds DEPTH24_STENCIL8 renderbuffer
  };

  class FrameBuffer : public IGLResource
  {
    struct AttachmentInfo
    {
      TextureFormatType   format;
      FramebufferSlotType slot;
    };

    EventSink _on_resize_event_sink = {};


    u32                                          _width;
    u32                                          _height;
    std::array<std::optional<AttachmentInfo>, 8> _attachment_infos;
    std::optional<Texture2D>                     _depth_texture;
    bool                                         _use_depthstencil;

    auto OnResize(Event::Resize const& e) -> void;

    public:
      FrameBuffer(FramebufferCreateInfo const& info, std::string_view name = "Unknown FrameBuffer");

      auto Bind()   -> void;
      auto UnBind() -> void;

      [[nodiscard]] auto TryGetAttachment() -> Texture2D const*;
      [[nodiscard]] auto GetAttachment()    -> Texture2D const&;

      [[nodiscard]] auto TryGetDepthStencil() -> Texture2D const*;
      [[nodiscard]] auto GetDepthStencil()    -> Texture2D const&;

  };

}
