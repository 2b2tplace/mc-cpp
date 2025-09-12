#include <mc_cpp/nbt/text.hpp>

namespace mc {

    TextInteractivity &TextInteractivity::withHoverText(const Text &hoverEventNew) {
        hoverEvent = std::make_shared<TextHoverEventShowText>(hoverEventNew);
        return *this;
    }

}