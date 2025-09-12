#pragma once

#include <mc_cpp/common/json.hpp>
#include <mc_cpp/nbt/nbt.hpp>
#include <utility>

namespace mc {

    struct Text;

    struct NbtSerializable {
        virtual ~NbtSerializable() = default;

        virtual void writeToNbt(NbtCompound &compound) const = 0;

        virtual void writeToJson(nlohmann::json &json) const = 0;

        [[nodiscard]]
        NbtCompound createCompound() const {
            NbtCompound compound;
            writeToNbt(compound);
            return compound;
        }

        [[nodiscard]]
        nlohmann::json createJson() const {
            nlohmann::json json;
            writeToJson(json);
            return json;
        }

    };

    struct TextColor {
        std::string color;

        static TextColor of(const uint32_t argb) {
            return of(argb >> 16 & 0xFF, argb >> 8 & 0xFF, argb & 0xFF);
        }

        static TextColor of(const uint8_t r, const uint8_t g, const uint8_t b) {
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

        TextStyle &withColor(const TextColor &colorNew) {
            color = colorNew;
            return *this;
        }

        TextStyle &withFont(const std::string &fontNew) {
            font = fontNew;
            return *this;
        }

        TextStyle &withBold(const bool boldNew) {
            bold = boldNew;
            return *this;
        }

        TextStyle &withItalic(const bool italicNew) {
            italic = italicNew;
            return *this;
        }

        TextStyle &withUnderlined(const bool underlinedNew) {
            underlined = underlinedNew;
            return *this;
        }

        TextStyle &withStrikethrough(const bool strikethroughNew) {
            strikethrough = strikethroughNew;
            return *this;
        }

        TextStyle &withObfuscated(const bool obfuscatedNew) {
            obfuscated = obfuscatedNew;
            return *this;
        }

        void writeToNbt(NbtCompound &compound) const override {
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

        void writeToJson(nlohmann::json &json) const override {
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
        std::string actionKey() const {
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

        void writeToJson(nlohmann::json &json) const override {
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

        TextInteractivity &withInsertion(const std::string &insertionNew) {
            insertion = insertionNew;
            return *this;
        }

        TextInteractivity &withClickEvent(const TextClickEvent &clickEventNew) {
            clickEvent = clickEventNew;
            return *this;
        }

        TextInteractivity &withClickEvent(const TextClickAction action, const std::string &value) {
            return withClickEvent(TextClickEvent{action, value});
        }

        TextInteractivity &withHoverText(const Text &hoverEventNew);

        void writeToNbt(NbtCompound &compound) const override {
            if (insertion)
                compound.put("insertion", insertion.value());
            if (clickEvent)
                clickEvent->writeToNbt(compound);
            if (hoverEvent)
                hoverEvent->writeToNbt(compound);
        }

        void writeToJson(nlohmann::json &json) const override {
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

        Text &append(const Text &sibling) {
            siblings.push_back(sibling);
            return *this;
        }

        Text &color(const TextColor &colorNew) {
            style.withColor(colorNew);
            return *this;
        }

        Text &font(const std::string &fontNew) {
            style.withFont(fontNew);
            return *this;
        }

        Text &bold(const bool boldNew) {
            style.withBold(boldNew);
            return *this;
        }

        Text &italic(const bool italicNew) {
            style.withItalic(italicNew);
            return *this;
        }

        Text &underlined(const bool underlinedNew) {
            style.withUnderlined(underlinedNew);
            return *this;
        }

        Text &strikethrough(const bool strikethroughNew) {
            style.withStrikethrough(strikethroughNew);
            return *this;
        }

        Text &obfuscated(const bool obfuscatedNew) {
            style.withObfuscated(obfuscatedNew);
            return *this;
        }

        Text &insertion(const std::string &insertionNew) {
            interactivity.withInsertion(insertionNew);
            return *this;
        }

        Text &clickEvent(const TextClickEvent &clickEventNew) {
            interactivity.withClickEvent(clickEventNew);
            return *this;
        }

        Text &clickEvent(const TextClickAction action, const std::string &value) {
            interactivity.withClickEvent(action, value);
            return *this;
        }

        Text &hoverText(const Text &hoverEventNew) {
            interactivity.withHoverText(hoverEventNew);
            return *this;
        }

        void writeToNbt(NbtCompound &compound) const override {
            content->writeToNbt(compound);

            NbtList extra;
            for (const auto &sibling : siblings)
                extra.add(sibling.createCompound());

            if (extra.length() != 0)
                compound.putNbt("extra", extra);
            style.writeToNbt(compound);
            interactivity.writeToNbt(compound);
        }

        void writeToJson(nlohmann::json &json) const override {
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
        std::string serializeNbt() const {
            return stringifyNbt(createCompound(), false);
        }

        [[nodiscard]]
        std::string serialize() const {
            return createJson().dump();
        }
    };

    struct TextHoverEventShowText final : TextHoverEvent {

        Text contents;

        explicit TextHoverEventShowText(Text contents): contents(std::move(contents)) {}

        void writeToNbt(NbtCompound &compound) const override {
            NbtCompound hoverEvent;
            hoverEvent.put("action", "show_text");
            hoverEvent.putNbt("contents", contents.createCompound());
            compound.putNbt("hoverEvent", hoverEvent);
        }

        void writeToJson(nlohmann::json &json) const override {
            nlohmann::json hoverEvent;
            hoverEvent["action"] = "show_text";
            hoverEvent["contents"] = contents.createJson();
            json["hoverEvent"] = hoverEvent;
        }

    };

    struct PlainTextContent final : NbtSerializable {
        std::string text;

        explicit PlainTextContent(std::string text): text(std::move(text)) {}

        void writeToNbt(NbtCompound &compound) const override {
            compound.put("text", text);
        }

        void writeToJson(nlohmann::json &json) const override {
            json["text"] = text;
        }
    };

    struct TranslatableTextContent final : NbtSerializable {
        std::string translate;
        Option<std::string> fallback;
        Option<std::vector<Text>> with;

        explicit TranslatableTextContent(std::string translate): translate(std::move(translate)) {}

        TranslatableTextContent &withFallback(const std::string &fallbackNew) {
            fallback = fallbackNew;
            return *this;
        }

        TranslatableTextContent &withArgs(const std::vector<Text> &withNew) {
            with = withNew;
            return *this;
        }

        void writeToNbt(NbtCompound &compound) const override {
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

        void writeToJson(nlohmann::json &json) const override {
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

        void writeToNbt(NbtCompound &compound) const override {
            NbtCompound score;
            score.put("name", name);
            score.put("objective", objective);
            compound.putNbt("score", score);
        }

        void writeToJson(nlohmann::json &json) const override {
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

        SelectorTextContent &withSeparator(const Text &separatorNew) {
            separator = separatorNew;
            return *this;
        }

        void writeToNbt(NbtCompound &compound) const override {
            compound.put("selector", selector);
            if (separator)
                compound.putNbt("separator", separator->createCompound());
        }

        void writeToJson(nlohmann::json &json) const override {
            json["selector"] = selector;
            if (separator)
                json["separator"] = separator->createJson();
        }
    };

    struct KeybindTextContent final : NbtSerializable {
        std::string keybind;

        explicit KeybindTextContent(std::string keybind): keybind(std::move(keybind)) {}

        void writeToNbt(NbtCompound &compound) const override {
            compound.put("keybind", keybind);
        }

        void writeToJson(nlohmann::json &json) const override {
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

        NbtTextContent &withInterpret(const bool interpretNew) {
            interpret = interpretNew;
            return *this;
        }

        NbtTextContent &withSeparator(const Text &separatorNew) {
            separator = separatorNew;
            return *this;
        }

        void writeToNbt(NbtCompound &compound) const override {
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

        void writeToJson(nlohmann::json &json) const override {
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
    inline Text text(const std::string &text) {
        return Text{std::make_shared<PlainTextContent>(text)};
    }

    [[nodiscard]]
    inline Text score(const std::string &name, const std::string &objective) {
        return Text{std::make_shared<ScoreTextContent>(name, objective)};
    }

    [[nodiscard]]
    inline Text selector(const std::string &selector) {
        return Text{std::make_shared<SelectorTextContent>(selector)};
    }

    [[nodiscard]]
    inline Text keybind(const std::string &keybind) {
        return Text{std::make_shared<KeybindTextContent>(keybind)};
    }

    [[nodiscard]]
    inline Text translate(const std::string &translate) {
        return Text{std::make_shared<TranslatableTextContent>(translate)};
    }

    [[nodiscard]]
    inline Text nbtText(const NbtSourceType sourceType, const std::string &nbt, const std::string &source) {
        return Text{std::make_shared<NbtTextContent>(sourceType, nbt, source)};
    }

}
