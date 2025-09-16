#pragma once

#include <string>
#include <mc_cpp/common/macro_magic.hpp>
#include <mc_cpp/common/json.hpp>

namespace mc {

    struct TileEntityEntry {
        NAMED_FIELD(uint16_t, id);
        NAMED_FIELD(std::string, name);

        DECLARE_ENTRY_BACKEND;
    };

    inline DEFINE_ENTRY_FROM_JSON(TileEntityEntry);
    inline DEFINE_ENTRY_TO_JSON(TileEntityEntry);



}