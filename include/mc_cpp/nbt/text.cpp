#include <mc_cpp/nbt/text.hpp>

namespace mc {

    auto TextInteractivity::withHoverText(const Text &hoverEventNew) -> TextInteractivity& {
        hoverEvent = std::make_shared<TextHoverEventShowText>(hoverEventNew);
        return *this;
    }

}