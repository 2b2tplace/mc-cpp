#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <mc_cpp/common/json.hpp>
#include <result.hpp>

namespace mc {

    template<typename Entry>
    struct Registry {
        std::string type;
        int protocolVersion{};
        std::vector<Entry> entries;

        auto save(const std::filesystem::path &filepath) const -> void;

        [[nodiscard]]
        auto load(const std::filesystem::path &filepath) -> bool;

        [[nodiscard]]
        auto trySave(const std::filesystem::path &filepath) const -> Result<std::monostate, std::string> {
            try {
                save(filepath);
            } catch (const std::exception &e) {
                return Err("Failed to save registry to " + filepath.string() + ": " + e.what());
            }
            return {};
        }

        [[nodiscard]]
        auto tryLoad(const std::filesystem::path &filepath) -> Result<std::monostate, std::string> {
            try {
                if (!load(filepath)) return Err("Failed to load registry from " + filepath.string());
            } catch (const std::exception &e) {
                return Err("Failed to load registry from " + filepath.string() + ": " + e.what());
            }
            return {};
        }
    };

    template<typename Entry>
    auto to_json(nlohmann::ordered_json &json, const Registry<Entry> &self) -> void {
        json["type"] = self.type;
        json["protocolVersion"] = self.protocolVersion;
        json["entries"] = self.entries;
    }

    template<typename Entry>
    auto from_json(const nlohmann::json &json, Registry<Entry> &self) -> void {
        json.at("type").get_to(self.type);
        json.at("protocolVersion").get_to(self.protocolVersion);
        json.at("entries").get_to(self.entries);
    }

    template<typename Entry>
    auto Registry<Entry>::save(const std::filesystem::path &filepath) const -> void {
        std::ofstream file(filepath);
        nlohmann::ordered_json json;
        mc::to_json(json, *this);
        file << json.dump(4);
        file.close();
    }

    template<typename Entry>
    auto Registry<Entry>::load(const std::filesystem::path &filepath) -> bool {
        std::ifstream file(filepath);
        if (!file.is_open()) return false;
        nlohmann::json json;
        file >> json;
        file.close();
        mc::from_json(json, *this);
        return true;
    }

}
