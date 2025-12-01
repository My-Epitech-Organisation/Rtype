#include <iostream>

#include "Graphic/Graphic.hpp"

class ClientApp {
private:
    Graphic _graphic;
public:

    explicit ClientApp(const std::shared_ptr<ECS::Registry>& registry);
    ~ClientApp() = default;
};
