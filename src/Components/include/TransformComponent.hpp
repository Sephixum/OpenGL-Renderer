#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace glr::Component
{

  struct Transform 
  {
    glm::vec3 position = glm::vec3(0.0f);
    glm::quat rotation = glm::quat{1.0f, 0.0f, 0.0f, 0.0f}; // identity
    glm::vec3 scale    = glm::vec3(1.0f);

    [[nodiscard]] auto GetMatrix() const -> glm::mat4
    {
      glm::mat4 T = glm::translate(glm::mat4(1.0f), position);
      glm::mat4 R = glm::mat4_cast(rotation);
      glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);
      return T * R * S;  // common order: scale -> rotate -> translate
    }
  };

}
