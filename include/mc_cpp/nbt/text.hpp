#pragma once

#include <mc_cpp/common/json.hpp>
#include <mc_cpp/nbt/nbt.hpp>
#include <utility>
#include <fmt/format.h>

namespace mc {

    struct Text;

    struct NbtSerializable {
        virtual ~NbtSerializable() = default;

        virtual auto writeToNbt(NbtCompound &compound) const -> void = 0;

        virtual auto writeToJson(nlohmann::json &json) const -> void = 0;

        [[nodiscard]]
        auto createCompound() const -> NbtCompound {
            NbtCompound compound;
            writeToNbt(compound);
            return compound;
        }

        [[nodiscard]]
        auto createJson() const -> nlohmann::json {
            nlohmann::json json;
            writeToJson(json);
            return json;
        }

    };

    struct TextColor {
        std::string color;

        static auto of(const uint32_t argb) -> TextColor {
            return of(argb >> 16 & 0xFF, argb >> 8 & 0xFF, argb & 0xFF);
        }

        static auto of(const uint8_t r, const uint8_t g, const uint8_t b) -> TextColor {
            std::ostringstream oss;
            oss << '#' << std::hex
                << std::setw(2) << std::setfill('0') << static_cast<int>(r)
                << std::setw(2) << std::setfill('0') << static_cast<int>(g)
                << std::setw(2) << std::setfill('0') << static_cast<int>(b);

            return TextColor{oss.str()};
        }
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
        Option<TextColor> color;
        Option<std::string> font;
        Option<bool> bold;
        Option<bool> italic;
        Option<bool> underlined;
        Option<bool> strikethrough;
        Option<bool> obfuscated;

        auto withColor(const TextColor &colorNew) -> TextStyle& {
            color = colorNew;
            return *this;
        }

        auto withFont(const std::string &fontNew) -> TextStyle& {
            font = fontNew;
            return *this;
        }

        auto withBold(const bool boldNew) -> TextStyle& {
            bold = boldNew;
            return *this;
        }

        auto withItalic(const bool italicNew) -> TextStyle& {
            italic = italicNew;
            return *this;
        }

        auto withUnderlined(const bool underlinedNew) -> TextStyle& {
            underlined = underlinedNew;
            return *this;
        }

        auto withStrikethrough(const bool strikethroughNew) -> TextStyle& {
            strikethrough = strikethroughNew;
            return *this;
        }

        auto withObfuscated(const bool obfuscatedNew) -> TextStyle& {
            obfuscated = obfuscatedNew;
            return *this;
        }

        auto writeToNbt(NbtCompound &compound) const -> void override {
            if (color)
                compound.put("color", color->color);
            if (font)
                compound.put("font", font.value());
            if (bold)
                compound.put("bold", bold.value());
            if (italic)
                compound.put("italic", italic.value());
            if (underlined)
                compound.put("underline", underlined.value());
            if (strikethrough)
                compound.put("strikethrough", strikethrough.value());
            if (obfuscated)
                compound.put("obfuscated", obfuscated.value());
        }

        auto writeToJson(nlohmann::json &json) const -> void override {
            if (color)
                json["color"] = color->color;
            if (font)
                json["font"] = font.value();
            if (bold)
                json["bold"] = bold.value();
            if (italic)
                json["italic"] = italic.value();
            if (underlined)
                json["underlined"] = underlined.value();
            if (strikethrough)
                json["strikethrough"] = strikethrough.value();
            if (obfuscated)
                json["obfuscated"] = obfuscated.value();
        }
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

        explicit TextClickEvent(const TextClickAction action, std::string value): action(action), value(std::move(value)) {}

        [[nodiscard]]
        auto actionKey() const -> std::string {
            switch (action) {
                case TextClickAction::OPEN_URL: return "open_url";
                case TextClickAction::OPEN_FILE: return "open_file";
                case TextClickAction::RUN_COMMAND: return "run_command";
                case TextClickAction::SUGGEST_COMMAND: return "suggest_command";
                case TextClickAction::CHANGE_PAGE: return "change_page";
                case TextClickAction::COPY_TO_CLIPBOARD: return "copy_to_clipboard";
            }
            assert(false && "unreachable");
        }

        void writeToNbt(NbtCompound &compound) const override {
            NbtCompound clickEvent;
            clickEvent.put("action", actionKey());
            clickEvent.put("value", value);
            compound.putNbt("clickEvent", clickEvent);
        }

        auto writeToJson(nlohmann::json &json) const -> void override {
            nlohmann::json clickEvent;
            clickEvent["action"] = actionKey();
            clickEvent["value"] = value;
            json["clickEvent"] = clickEvent;
        }
    };

    struct TextHoverEvent : NbtSerializable {};

    struct TextInteractivity final : NbtSerializable {
        Option<std::string> insertion;
        Option<TextClickEvent> clickEvent;
        std::shared_ptr<TextHoverEvent> hoverEvent;

        auto withInsertion(const std::string &insertionNew) -> TextInteractivity& {
            insertion = insertionNew;
            return *this;
        }

        auto withClickEvent(const TextClickEvent &clickEventNew) -> TextInteractivity& {
            clickEvent = clickEventNew;
            return *this;
        }

        auto withClickEvent(const TextClickAction action, const std::string &value) -> TextInteractivity& {
            return withClickEvent(TextClickEvent{action, value});
        }

        auto withHoverText(const Text &hoverEventNew) -> TextInteractivity &;

        auto writeToNbt(NbtCompound &compound) const -> void override {
            if (insertion)
                compound.put("insertion", insertion.value());
            if (clickEvent)
                clickEvent->writeToNbt(compound);
            if (hoverEvent)
                hoverEvent->writeToNbt(compound);
        }

        auto writeToJson(nlohmann::json &json) const -> void override {
            if (insertion)
                json["insertion"] = insertion.value();
            if (clickEvent)
                clickEvent->writeToJson(json);
            if (hoverEvent)
                hoverEvent->writeToJson(json);
        }
    };

    struct Text final : NbtSerializable {
        std::shared_ptr<NbtSerializable> content;
        std::vector<Text> siblings{};
        TextStyle style{};
        TextInteractivity interactivity{};

        explicit Text(const std::shared_ptr<NbtSerializable> &content): content(content) {}

        auto append(const Text &sibling) -> Text& {
            siblings.push_back(sibling);
            return *this;
        }

        auto color(const TextColor &colorNew) -> Text& {
            style.withColor(colorNew);
            return *this;
        }

        auto font(const std::string &fontNew) -> Text& {
            style.withFont(fontNew);
            return *this;
        }

        auto bold(const bool boldNew) -> Text& {
            style.withBold(boldNew);
            return *this;
        }

        auto italic(const bool italicNew) -> Text& {
            style.withItalic(italicNew);
            return *this;
        }

        auto underlined(const bool underlinedNew) -> Text& {
            style.withUnderlined(underlinedNew);
            return *this;
        }

        auto strikethrough(const bool strikethroughNew) -> Text& {
            style.withStrikethrough(strikethroughNew);
            return *this;
        }

        auto obfuscated(const bool obfuscatedNew) -> Text& {
            style.withObfuscated(obfuscatedNew);
            return *this;
        }

        auto insertion(const std::string &insertionNew) -> Text& {
            interactivity.withInsertion(insertionNew);
            return *this;
        }

        auto clickEvent(const TextClickEvent &clickEventNew) -> Text& {
            interactivity.withClickEvent(clickEventNew);
            return *this;
        }

        auto clickEvent(const TextClickAction action, const std::string &value) -> Text& {
            interactivity.withClickEvent(action, value);
            return *this;
        }

        auto hoverText(const Text &hoverEventNew) -> Text& {
            interactivity.withHoverText(hoverEventNew);
            return *this;
        }

        auto writeToNbt(NbtCompound &compound) const -> void override {
            content->writeToNbt(compound);

            NbtList extra;
            for (const auto &sibling : siblings)
                extra.add(sibling.createCompound());

            if (extra.length() != 0)
                compound.putNbt("extra", extra);
            style.writeToNbt(compound);
            interactivity.writeToNbt(compound);
        }

        auto writeToJson(nlohmann::json &json) const -> void override {
            content->writeToJson(json);

            std::vector<nlohmann::json> extra;
            for (const auto &sibling : siblings)
                extra.push_back(sibling.createJson());

            if (!extra.empty())
                json["extra"] = extra;
            style.writeToJson(json);
            interactivity.writeToJson(json);
        }

        [[nodiscard]]
        auto serializeNbt() const -> std::string {
            return stringifyNbt(createCompound(), false);
        }

        [[nodiscard]]
        auto serialize() const -> std::string {
            return createJson().dump();
        }
    };

    struct TextHoverEventShowText final : TextHoverEvent {

        Text contents;

        explicit TextHoverEventShowText(Text contents): contents(std::move(contents)) {}

        auto writeToNbt(NbtCompound &compound) const -> void override {
            NbtCompound hoverEvent;
            hoverEvent.put("action", "show_text");
            hoverEvent.putNbt("contents", contents.createCompound());
            compound.putNbt("hoverEvent", hoverEvent);
        }

        auto writeToJson(nlohmann::json &json) const -> void override {
            nlohmann::json hoverEvent;
            hoverEvent["action"] = "show_text";
            hoverEvent["contents"] = contents.createJson();
            json["hoverEvent"] = hoverEvent;
        }

    };

    struct PlainTextContent final : NbtSerializable {
        std::string text;

        explicit PlainTextContent(std::string text): text(std::move(text)) {}

        auto writeToNbt(NbtCompound &compound) const -> void override {
            compound.put("text", text);
        }

        auto writeToJson(nlohmann::json &json) const -> void override {
            json["text"] = text;
        }
    };

    struct TranslatableTextContent final : NbtSerializable {
        std::string translate;
        Option<std::string> fallback;
        Option<std::vector<Text>> with;

        explicit TranslatableTextContent(std::string translate): translate(std::move(translate)) {}

        auto withFallback(const std::string &fallbackNew) -> TranslatableTextContent& {
            fallback = fallbackNew;
            return *this;
        }

        auto withArgs(const std::vector<Text> &withNew) -> TranslatableTextContent& {
            with = withNew;
            return *this;
        }

        auto writeToNbt(NbtCompound &compound) const -> void override {
            compound.put("translate", translate);
            if (fallback)
                compound.put("fallback", fallback.value());
            if (with) {
                NbtList list;
                for (const auto &text : with.value())
                    list.add(text.createCompound());

                compound.putNbt("with", list);
            }
        }

        auto writeToJson(nlohmann::json &json) const -> void override {
            json["translate"] = translate;
            if (fallback)
                json["fallback"] = fallback.value();
            if (with) {
                std::vector<nlohmann::json> list;
                for (const auto &text : with.value())
                    list.push_back(text.createJson());

                json["with"] = list;
            }
        }
    };

    struct ScoreTextContent final : NbtSerializable {
        std::string name;
        std::string objective;

        explicit ScoreTextContent(std::string name, std::string objective): name(std::move(name)), objective(std::move(objective)) {}

        auto writeToNbt(NbtCompound &compound) const -> void override {
            NbtCompound score;
            score.put("name", name);
            score.put("objective", objective);
            compound.putNbt("score", score);
        }

        auto writeToJson(nlohmann::json &json) const -> void override {
            nlohmann::json score;
            score["name"] = name;
            score["objective"] = objective;
            json["score"] = score;
        }
    };

    struct SelectorTextContent final : NbtSerializable {
        std::string selector;
        Option<Text> separator;

        explicit SelectorTextContent(std::string selector): selector(std::move(selector)) {}

        auto withSeparator(const Text &separatorNew) -> SelectorTextContent& {
            separator = separatorNew;
            return *this;
        }

        auto writeToNbt(NbtCompound &compound) const -> void override {
            compound.put("selector", selector);
            if (separator)
                compound.putNbt("separator", separator->createCompound());
        }

        auto writeToJson(nlohmann::json &json) const -> void override {
            json["selector"] = selector;
            if (separator)
                json["separator"] = separator->createJson();
        }
    };

    struct KeybindTextContent final : NbtSerializable {
        std::string keybind;

        explicit KeybindTextContent(std::string keybind): keybind(std::move(keybind)) {}

        auto writeToNbt(NbtCompound &compound) const -> void override {
            compound.put("keybind", keybind);
        }

        auto writeToJson(nlohmann::json &json) const -> void override {
            json["keybind"] = keybind;
        }
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
        Option<bool> interpret;
        Option<Text> separator;

        explicit NbtTextContent(const NbtSourceType sourceType, std::string nbt, std::string source):
            sourceType(sourceType), nbt(std::move(nbt)), source(std::move(source)) {}

        auto withInterpret(const bool interpretNew) -> NbtTextContent& {
            interpret = interpretNew;
            return *this;
        }

        auto withSeparator(const Text &separatorNew) -> NbtTextContent& {
            separator = separatorNew;
            return *this;
        }

        auto writeToNbt(NbtCompound &compound) const -> void override {
            compound.put("nbt", this->nbt);
            if (interpret)
                compound.put("interpret", interpret.value());

            if (separator)
                compound.putNbt("separator", separator->createCompound());

            switch (sourceType) {
                case NbtSourceType::BLOCK:
                    compound.put("block", source);
                    break;
                case NbtSourceType::ENTITY:
                    compound.put("entity", source);
                    break;
                case NbtSourceType::STORAGE:
                    compound.put("storage", source);
                    break;
            }
        }

        auto writeToJson(nlohmann::json &json) const -> void override {
            json["nbt"] = this->nbt;
            if (interpret)
                json["interpret"] = interpret.value();

            if (separator)
                json["separator"] = separator->createJson();

            switch (sourceType) {
                case NbtSourceType::BLOCK:
                    json["block"] = source;
                    break;
                case NbtSourceType::ENTITY:
                    json["entity"] = source;
                    break;
                case NbtSourceType::STORAGE:
                    json["storage"] = source;
                    break;
            }
        }
    };

    [[nodiscard]]
    inline auto text(const std::string &text) -> Text {
        return Text{std::make_shared<PlainTextContent>(text)};
    }

    template<typename... T>
    [[nodiscard]]
    auto fmt(fmt::format_string<T...> fmt, T &&... args) -> Text {
        return text(fmt::vformat(fmt, fmt::make_format_args(args...)));
    }

    [[nodiscard]]
    inline auto score(const std::string &name, const std::string &objective) -> Text {
        return Text{std::make_shared<ScoreTextContent>(name, objective)};
    }

    [[nodiscard]]
    inline auto selector(const std::string &selector) -> Text {
        return Text{std::make_shared<SelectorTextContent>(selector)};
    }

    [[nodiscard]]
    inline auto keybind(const std::string &keybind) -> Text {
        return Text{std::make_shared<KeybindTextContent>(keybind)};
    }

    [[nodiscard]]
    inline auto translate(const std::string &translate) -> Text {
        return Text{std::make_shared<TranslatableTextContent>(translate)};
    }

    [[nodiscard]]
    inline auto nbtText(const NbtSourceType sourceType, const std::string &nbt, const std::string &source) -> Text {
        return Text{std::make_shared<NbtTextContent>(sourceType, nbt, source)};
    }

}
