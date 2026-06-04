#pragma once

namespace glr
{

  template <typename... Ts>
  struct Overload : public Ts...
  {
    using Ts::operator()...;
  };

}
