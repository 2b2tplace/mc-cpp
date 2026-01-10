#pragma once

#include <mc_cpp/common/json.hpp>
#include <mc_cpp/nbt/nbt.hpp>
#include <fmt/format.h>

namespace mc {

    struct Text;

    struct NbtSerializable {
        virtual ~NbtSerializable() = default;

        virtual auto writeToNbt(NbtCompound &compound) const -> void = 0;

        virtual auto writeToJson(nlohmann::json &json) const -> void = 0;

        [[nodiscard]]
        auto createCompound() const -> NbtCompound;

        [[nodiscard]]
        auto createJson() const -> nlohmann::json;
    };

    struct TextColor {
        std::string color;

        static auto of(uint32_t argb) -> TextColor;

        static auto of(uint8_t r, uint8_t g, uint8_t b) -> TextColor;
    };

    namespace text_colors {
        static const auto black         = TextColor{"black"};
        static const auto dark_blue     = TextColor{"dark_blue"};
        static const auto dark_green    = TextColor{"dark_green"};
        static const auto dark_aqua     = TextColor{"dark_aqua"};
        static const auto dark_red      = TextColor{"dark_red"};
        static const auto dark_purple   = TextColor{"dark_purple"};
        static const auto gold          = TextColor{"gold"};
        static const auto gray          = TextColor{"gray"};
        static const auto dark_gray     = TextColor{"dark_gray"};
        static const auto blue          = TextColor{"blue"};
        static const auto green         = TextColor{"green"};
        static const auto aqua          = TextColor{"aqua"};
        static const auto red           = TextColor{"red"};
        static const auto light_purple  = TextColor{"light_purple"};
        static const auto yellow        = TextColor{"yellow"};
        static const auto white         = TextColor{"white"};
    }

    struct TextStyle final : NbtSerializable {
        result::Option<TextColor> color;
        result::Option<std::string> font;
        result::Option<bool> bold;
        result::Option<bool> italic;
        result::Option<bool> underlined;
        result::Option<bool> strikethrough;
        result::Option<bool> obfuscated;

        auto withColor(const TextColor &colorNew) -> TextStyle&;

        auto withFont(const std::string &fontNew) -> TextStyle&;

        auto withBold(bool boldNew) -> TextStyle&;

        auto withItalic(bool italicNew) -> TextStyle&;

        auto withUnderlined(bool underlinedNew) -> TextStyle&;

        auto withStrikethrough(bool strikethroughNew) -> TextStyle&;

        auto withObfuscated(bool obfuscatedNew) -> TextStyle&;

        auto writeToNbt(NbtCompound &compound) const -> void override;

        auto writeToJson(nlohmann::json &json) const -> void override;
    };

    enum class TextClickAction {
        OPEN_URL,
        OPEN_FILE,
        RUN_COMMAND,
        SUGGEST_COMMAND,
        CHANGE_PAGE,
        COPY_TO_CLIPBOARD
    };

    struct TextClickEvent final : NbtSerializable {
        TextClickAction action;
        std::string value;

        explicit TextClickEvent(TextClickAction action, std::string value);

        [[nodiscard]]
        auto actionKey() const -> std::string;

        void writeToNbt(NbtCompound &compound) const override;

        auto writeToJson(nlohmann::json &json) const -> void override;
    };

    struct TextHoverEvent : NbtSerializable {};

    struct TextInteractivity final : NbtSerializable {
        result::Option<std::string> insertion;
        result::Option<TextClickEvent> clickEvent;
        std::shared_ptr<TextHoverEvent> hoverEvent;

        auto withInsertion(const std::string &insertionNew) -> TextInteractivity&;

        auto withClickEvent(const TextClickEvent &clickEventNew) -> TextInteractivity&;

        auto withClickEvent(TextClickAction action, const std::string &value) -> TextInteractivity&;

        auto withHoverText(const Text &hoverEventNew) -> TextInteractivity&;

        auto writeToNbt(NbtCompound &compound) const -> void override;

        auto writeToJson(nlohmann::json &json) const -> void override;
    };

    struct Text final : NbtSerializable {
        std::shared_ptr<NbtSerializable> content;
        std::vector<Text> siblings{};
        TextStyle style{};
        TextInteractivity interactivity{};

        explicit Text(const std::shared_ptr<NbtSerializable> &content);

        auto append(const Text &sibling) -> Text&;

        auto color(const TextColor &colorNew) -> Text&;

        auto font(const std::string &fontNew) -> Text&;

        auto bold(bool boldNew) -> Text&;

        auto italic(bool italicNew) -> Text&;

        auto underlined(bool underlinedNew) -> Text&;

        auto strikethrough(bool strikethroughNew) -> Text&;

        auto obfuscated(bool obfuscatedNew) -> Text&;

        auto insertion(const std::string &insertionNew) -> Text&;

        auto clickEvent(const TextClickEvent &clickEventNew) -> Text&;

        auto clickEvent(TextClickAction action, const std::string &value) -> Text&;

        auto hoverText(const Text &hoverEventNew) -> Text&;

        auto writeToNbt(NbtCompound &compound) const -> void override;

        auto writeToJson(nlohmann::json &json) const -> void override;

        [[nodiscard]]
        auto serializeNbt() const -> std::string;

        [[nodiscard]]
        auto serialize() const -> std::string;
    };

    struct TextHoverEventShowText final : TextHoverEvent {

        Text contents;

        explicit TextHoverEventShowText(Text contents);

        auto writeToNbt(NbtCompound &compound) const -> void override;

        auto writeToJson(nlohmann::json &json) const -> void override;
    };

    struct PlainTextContent final : NbtSerializable {
        std::string text;

        explicit PlainTextContent(std::string text);

        auto writeToNbt(NbtCompound &compound) const -> void override;

        auto writeToJson(nlohmann::json &json) const -> void override;
    };

    struct TranslatableTextContent final : NbtSerializable {
        std::string translate;
        result::Option<std::string> fallback;
        result::Option<std::vector<Text>> with;

        explicit TranslatableTextContent(std::string translate);

        auto withFallback(const std::string &fallbackNew) -> TranslatableTextContent&;

        auto withArgs(const std::vector<Text> &withNew) -> TranslatableTextContent&;

        auto writeToNbt(NbtCompound &compound) const -> void override;

        auto writeToJson(nlohmann::json &json) const -> void override;
    };

    struct ScoreTextContent final : NbtSerializable {
        std::string name;
        std::string objective;

        explicit ScoreTextContent(std::string name, std::string objective);

        auto writeToNbt(NbtCompound &compound) const -> void override;

        auto writeToJson(nlohmann::json &json) const -> void override;
    };

    struct SelectorTextContent final : NbtSerializable {
        std::string selector;
        result::Option<Text> separator;

        explicit SelectorTextContent(std::string selector);

        auto withSeparator(const Text &separatorNew) -> SelectorTextContent&;

        auto writeToNbt(NbtCompound &compound) const -> void override;

        auto writeToJson(nlohmann::json &json) const -> void override;
    };

    struct KeybindTextContent final : NbtSerializable {
        std::string keybind;

        explicit KeybindTextContent(std::string keybind);

        auto writeToNbt(NbtCompound &compound) const -> void override;

        auto writeToJson(nlohmann::json &json) const -> void override;
    };

    enum class NbtSourceType {
        BLOCK,
        ENTITY,
        STORAGE
    };

    struct NbtTextContent final : NbtSerializable {
        NbtSourceType sourceType;
        std::string nbt;
        std::string source;
        result::Option<bool> interpret;
        result::Option<Text> separator;

        explicit NbtTextContent(NbtSourceType sourceType, std::string nbt, std::string source);

        auto withInterpret(bool interpretNew) -> NbtTextContent&;

        auto withSeparator(const Text &separatorNew) -> NbtTextContent&;

        auto writeToNbt(NbtCompound &compound) const -> void override;

        auto writeToJson(nlohmann::json &json) const -> void override;
    };

    [[nodiscard]]
    auto text(const std::string &text) -> Text;

    template<typename... T>
    [[nodiscard]]
    auto fmt(fmt::format_string<T...> fmt, T &&... args) -> Text {
        return text(fmt::vformat(fmt, fmt::make_format_args(args...)));
    }

    [[nodiscard]]
    auto score(const std::string &name, const std::string &objective) -> Text;

    [[nodiscard]]
    auto selector(const std::string &selector) -> Text;

    [[nodiscard]]
    auto keybind(const std::string &keybind) -> Text;

    [[nodiscard]]
    auto translate(const std::string &translate) -> Text;

    [[nodiscard]]
    auto nbtText(NbtSourceType sourceType, const std::string &nbt, const std::string &source) -> Text;
}
