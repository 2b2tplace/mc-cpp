#include <mc_cpp/nbt/text.hpp>

namespace mc {
    auto NbtSerializable::createCompound() const -> NbtCompound {
        NbtCompound compound;
        writeToNbt(compound);
        return compound;
    }

    auto NbtSerializable::createJson() const -> nlohmann::json {
        nlohmann::json json;
        writeToJson(json);
        return json;
    }

    auto TextColor::of(const uint32_t argb) -> TextColor {
        return of(argb >> 16 & 0xFF, argb >> 8 & 0xFF, argb & 0xFF);
    }

    auto TextColor::of(const uint8_t r, const uint8_t g, const uint8_t b) -> TextColor {
        std::ostringstream oss;
        oss << '#' << std::hex
                << std::setw(2) << std::setfill('0') << static_cast<int>(r)
                << std::setw(2) << std::setfill('0') << static_cast<int>(g)
                << std::setw(2) << std::setfill('0') << static_cast<int>(b);

        return TextColor{oss.str()};
    }

    auto TextStyle::withColor(const TextColor &colorNew) -> TextStyle & {
        color = colorNew;
        return *this;
    }

    auto TextStyle::withFont(const std::string &fontNew) -> TextStyle & {
        font = fontNew;
        return *this;
    }

    auto TextStyle::withBold(const bool boldNew) -> TextStyle & {
        bold = boldNew;
        return *this;
    }

    auto TextStyle::withItalic(const bool italicNew) -> TextStyle & {
        italic = italicNew;
        return *this;
    }

    auto TextStyle::withUnderlined(const bool underlinedNew) -> TextStyle & {
        underlined = underlinedNew;
        return *this;
    }

    auto TextStyle::withStrikethrough(const bool strikethroughNew) -> TextStyle & {
        strikethrough = strikethroughNew;
        return *this;
    }

    auto TextStyle::withObfuscated(const bool obfuscatedNew) -> TextStyle & {
        obfuscated = obfuscatedNew;
        return *this;
    }

    auto TextStyle::writeToNbt(NbtCompound &compound) const -> void {
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

    auto TextStyle::writeToJson(nlohmann::json &json) const -> void {
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

    TextClickEvent::TextClickEvent(const TextClickAction action, std::string value): action(action), value(std::move(value)) {}

    auto TextClickEvent::actionKey() const -> std::string {
        switch (action) {
            case TextClickAction::OPEN_URL: return "open_url";
            case TextClickAction::OPEN_FILE: return "open_file";
            case TextClickAction::RUN_COMMAND: return "run_command";
            case TextClickAction::SUGGEST_COMMAND: return "suggest_command";
            case TextClickAction::CHANGE_PAGE: return "change_page";
            case TextClickAction::COPY_TO_CLIPBOARD: return "copy_to_clipboard";
            default: std::unreachable();
        }
    }

    void TextClickEvent::writeToNbt(NbtCompound &compound) const {
        NbtCompound clickEvent;
        clickEvent.put("action", actionKey());
        clickEvent.put("value", value);
        compound.putNbt("clickEvent", clickEvent);
    }

    auto TextClickEvent::writeToJson(nlohmann::json &json) const -> void {
        nlohmann::json clickEvent;
        clickEvent["action"] = actionKey();
        clickEvent["value"] = value;
        json["clickEvent"] = clickEvent;
    }

    auto TextInteractivity::withInsertion(const std::string &insertionNew) -> TextInteractivity & {
        insertion = insertionNew;
        return *this;
    }

    auto TextInteractivity::withClickEvent(const TextClickEvent &clickEventNew) -> TextInteractivity & {
        clickEvent = clickEventNew;
        return *this;
    }

    auto TextInteractivity::withClickEvent(const TextClickAction action,
        const std::string &value) -> TextInteractivity & {
        return withClickEvent(TextClickEvent{action, value});
    }

    auto TextInteractivity::withHoverText(const Text &hoverEventNew) -> TextInteractivity& {
        hoverEvent = std::make_shared<TextHoverEventShowText>(hoverEventNew);
        return *this;
    }

    auto TextInteractivity::writeToNbt(NbtCompound &compound) const -> void {
        if (insertion)
            compound.put("insertion", insertion.value());
        if (clickEvent)
            clickEvent->writeToNbt(compound);
        if (hoverEvent)
            hoverEvent->writeToNbt(compound);
    }

    auto TextInteractivity::writeToJson(nlohmann::json &json) const -> void {
        if (insertion)
            json["insertion"] = insertion.value();
        if (clickEvent)
            clickEvent->writeToJson(json);
        if (hoverEvent)
            hoverEvent->writeToJson(json);
    }

    Text::Text(const std::shared_ptr<NbtSerializable> &content): content(content) {}

    auto Text::append(const Text &sibling) -> Text & {
        siblings.push_back(sibling);
        return *this;
    }

    auto Text::color(const TextColor &colorNew) -> Text & {
        style.withColor(colorNew);
        return *this;
    }

    auto Text::font(const std::string &fontNew) -> Text & {
        style.withFont(fontNew);
        return *this;
    }

    auto Text::bold(const bool boldNew) -> Text & {
        style.withBold(boldNew);
        return *this;
    }

    auto Text::italic(const bool italicNew) -> Text & {
        style.withItalic(italicNew);
        return *this;
    }

    auto Text::underlined(const bool underlinedNew) -> Text & {
        style.withUnderlined(underlinedNew);
        return *this;
    }

    auto Text::strikethrough(const bool strikethroughNew) -> Text & {
        style.withStrikethrough(strikethroughNew);
        return *this;
    }

    auto Text::obfuscated(const bool obfuscatedNew) -> Text & {
        style.withObfuscated(obfuscatedNew);
        return *this;
    }

    auto Text::insertion(const std::string &insertionNew) -> Text & {
        interactivity.withInsertion(insertionNew);
        return *this;
    }

    auto Text::clickEvent(const TextClickEvent &clickEventNew) -> Text & {
        interactivity.withClickEvent(clickEventNew);
        return *this;
    }

    auto Text::clickEvent(const TextClickAction action, const std::string &value) -> Text & {
        interactivity.withClickEvent(action, value);
        return *this;
    }

    auto Text::hoverText(const Text &hoverEventNew) -> Text & {
        interactivity.withHoverText(hoverEventNew);
        return *this;
    }

    auto Text::writeToNbt(NbtCompound &compound) const -> void {
        content->writeToNbt(compound);

        NbtList extra;
        for (const auto &sibling : siblings)
            extra.add(sibling.createCompound());

        if (extra.length() != 0)
            compound.putNbt("extra", extra);
        style.writeToNbt(compound);
        interactivity.writeToNbt(compound);
    }

    auto Text::writeToJson(nlohmann::json &json) const -> void {
        content->writeToJson(json);

        std::vector<nlohmann::json> extra;
        for (const auto &sibling : siblings)
            extra.push_back(sibling.createJson());

        if (!extra.empty())
            json["extra"] = extra;
        style.writeToJson(json);
        interactivity.writeToJson(json);
    }

    auto Text::serializeNbt() const -> std::string {
        return stringifyNbt(createCompound(), false);
    }

    auto Text::serialize() const -> std::string {
        return createJson().dump();
    }

    TextHoverEventShowText::TextHoverEventShowText(Text contents): contents(std::move(contents)) {}

    auto TextHoverEventShowText::writeToNbt(NbtCompound &compound) const -> void {
        NbtCompound hoverEvent;
        hoverEvent.put("action", "show_text");
        hoverEvent.putNbt("contents", contents.createCompound());
        compound.putNbt("hoverEvent", hoverEvent);
    }

    auto TextHoverEventShowText::writeToJson(nlohmann::json &json) const -> void {
        nlohmann::json hoverEvent;
        hoverEvent["action"] = "show_text";
        hoverEvent["contents"] = contents.createJson();
        json["hoverEvent"] = hoverEvent;
    }

    PlainTextContent::PlainTextContent(std::string text): text(std::move(text)) {}

    auto PlainTextContent::writeToNbt(NbtCompound &compound) const -> void {
        compound.put("text", text);
    }

    auto PlainTextContent::writeToJson(nlohmann::json &json) const -> void {
        json["text"] = text;
    }

    TranslatableTextContent::TranslatableTextContent(std::string translate): translate(std::move(translate)) {}

    auto TranslatableTextContent::withFallback(const std::string &fallbackNew) -> TranslatableTextContent & {
        fallback = fallbackNew;
        return *this;
    }

    auto TranslatableTextContent::withArgs(const std::vector<Text> &withNew) -> TranslatableTextContent & {
        with = withNew;
        return *this;
    }

    auto TranslatableTextContent::writeToNbt(NbtCompound &compound) const -> void {
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

    auto TranslatableTextContent::writeToJson(nlohmann::json &json) const -> void {
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

    ScoreTextContent::ScoreTextContent(std::string name, std::string objective): name(std::move(name)), objective(std::move(objective)) {}

    auto ScoreTextContent::writeToNbt(NbtCompound &compound) const -> void {
        NbtCompound score;
        score.put("name", name);
        score.put("objective", objective);
        compound.putNbt("score", score);
    }

    auto ScoreTextContent::writeToJson(nlohmann::json &json) const -> void {
        nlohmann::json score;
        score["name"] = name;
        score["objective"] = objective;
        json["score"] = score;
    }

    SelectorTextContent::SelectorTextContent(std::string selector): selector(std::move(selector)) {}

    auto SelectorTextContent::withSeparator(const Text &separatorNew) -> SelectorTextContent & {
        separator = separatorNew;
        return *this;
    }

    auto SelectorTextContent::writeToNbt(NbtCompound &compound) const -> void {
        compound.put("selector", selector);
        if (separator)
            compound.putNbt("separator", separator->createCompound());
    }

    auto SelectorTextContent::writeToJson(nlohmann::json &json) const -> void {
        json["selector"] = selector;
        if (separator)
            json["separator"] = separator->createJson();
    }

    KeybindTextContent::KeybindTextContent(std::string keybind): keybind(std::move(keybind)) {}

    auto KeybindTextContent::writeToNbt(NbtCompound &compound) const -> void {
        compound.put("keybind", keybind);
    }

    auto KeybindTextContent::writeToJson(nlohmann::json &json) const -> void {
        json["keybind"] = keybind;
    }

    NbtTextContent::NbtTextContent(const NbtSourceType sourceType, std::string nbt, std::string source):
        sourceType(sourceType), nbt(std::move(nbt)), source(std::move(source)) {}

    auto NbtTextContent::withInterpret(const bool interpretNew) -> NbtTextContent & {
        interpret = interpretNew;
        return *this;
    }

    auto NbtTextContent::withSeparator(const Text &separatorNew) -> NbtTextContent & {
        separator = separatorNew;
        return *this;
    }

    auto NbtTextContent::writeToNbt(NbtCompound &compound) const -> void {
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

    auto NbtTextContent::writeToJson(nlohmann::json &json) const -> void {
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

    auto text(const std::string &text) -> Text {
        return Text{std::make_shared<PlainTextContent>(text)};
    }

    auto score(const std::string &name, const std::string &objective) -> Text {
        return Text{std::make_shared<ScoreTextContent>(name, objective)};
    }

    auto selector(const std::string &selector) -> Text {
        return Text{std::make_shared<SelectorTextContent>(selector)};
    }

    auto keybind(const std::string &keybind) -> Text {
        return Text{std::make_shared<KeybindTextContent>(keybind)};
    }

    auto translate(const std::string &translate) -> Text {
        return Text{std::make_shared<TranslatableTextContent>(translate)};
    }

    auto nbtText(const NbtSourceType sourceType, const std::string &nbt, const std::string &source) -> Text {
        return Text{std::make_shared<NbtTextContent>(sourceType, nbt, source)};
    }
}
