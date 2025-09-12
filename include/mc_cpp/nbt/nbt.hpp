#pragma once

#include <memory>
#include <ranges>
#include <result.hpp>
#include <string>
#include <utility>
#include <vector>
#include <absl/container/flat_hash_map.h>

namespace mc {

    enum class NbtType {
        END = 0,
        BYTE = 1,
        SHORT = 2,
        INT = 3,
        LONG = 4,
        FLOAT = 5,
        DOUBLE = 6,
        BYTE_ARRAY = 7,
        STRING = 8,
        LIST = 9,
        COMPOUND = 10,
        INT_ARRAY = 11,
        LONG_ARRAY = 12
    };

    static constexpr size_t END_SIZE = 0;
    static constexpr size_t BYTE_SIZE = 1;
    static constexpr size_t SHORT_SIZE = 2;
    static constexpr size_t INT_SIZE = 4;
    static constexpr size_t LONG_SIZE = 8;
    static constexpr size_t FLOAT_SIZE = 4;
    static constexpr size_t DOUBLE_SIZE = 8;

    class NbtElement {
    public:
        virtual ~NbtElement() = default;

        [[nodiscard]]
        virtual NbtType getType() const = 0;

        [[nodiscard]]
        virtual size_t size() const = 0;
    };

    class AbstractNbtList : public NbtElement {

        [[nodiscard]]
        virtual NbtType getHeldType() const = 0;

    };

    template<typename T, NbtType TYPE, size_t SIZE>
    class NbtPrimitive final : public NbtElement {
        static constexpr auto FULL_SIZE = 8 + SIZE;
    public:
        T value;

        NbtPrimitive() = default;
        explicit NbtPrimitive(const T value): value(value) {}

        [[nodiscard]]
        NbtType getType() const override {
            return TYPE;
        }

        [[nodiscard]]
        size_t size() const override {
            return FULL_SIZE;
        }

    };

    class NbtString final : public NbtElement {
        static constexpr size_t SIZE = 36;

    public:
        std::string value;

        NbtString() = default;
        explicit NbtString(std::string value): value(std::move(value)) {}

        [[nodiscard]]
        NbtType getType() const override {
            return NbtType::STRING;
        }

        [[nodiscard]]
        size_t size() const override {
            return SIZE + 2 * value.length();
        }

    };

    class NbtEnd final : public NbtElement {
        static constexpr size_t SIZE = 8;

    public:
        NbtEnd() = default;

        [[nodiscard]]
        NbtType getType() const override {
            return NbtType::END;
        }

        [[nodiscard]]
        size_t size() const override {
            return SIZE;
        }

    };

    template<typename T, NbtType TYPE, NbtType HELD_TYPE, size_t HELD_SIZE>
    class NbtPrimitiveArray final : public AbstractNbtList {
        static constexpr auto SIZE = 24;

    public:
        std::vector<T> value;

        NbtPrimitiveArray() = default;
        explicit NbtPrimitiveArray(const std::vector<T> &value): value(value) {}

        [[nodiscard]]
        NbtType getType() const override {
            return TYPE;
        }

        [[nodiscard]]
        size_t size() const override {
            return SIZE + HELD_SIZE * value.size();
        }

        [[nodiscard]]
        NbtType getHeldType() const override {
            return HELD_TYPE;
        }

    };

    using NbtByte = NbtPrimitive<int8_t, NbtType::BYTE, BYTE_SIZE>;
    using NbtDouble = NbtPrimitive<double, NbtType::DOUBLE, DOUBLE_SIZE>;
    using NbtFloat = NbtPrimitive<float, NbtType::FLOAT, FLOAT_SIZE>;
    using NbtInt = NbtPrimitive<int32_t, NbtType::INT, INT_SIZE>;
    using NbtLong = NbtPrimitive<int64_t, NbtType::LONG, LONG_SIZE>;
    using NbtShort = NbtPrimitive<int16_t, NbtType::SHORT, SHORT_SIZE>;
    using NbtByteArray = NbtPrimitiveArray<int8_t, NbtType::BYTE_ARRAY, NbtType::BYTE, BYTE_SIZE>;
    using NbtIntArray = NbtPrimitiveArray<int32_t, NbtType::INT_ARRAY, NbtType::INT, INT_SIZE>;
    using NbtLongArray = NbtPrimitiveArray<int64_t, NbtType::LONG_ARRAY, NbtType::LONG, LONG_SIZE>;

    template<typename>
    struct UnderlyingType {};

    template<>
    struct UnderlyingType<int8_t> {
        static constexpr auto TypeEnum = NbtType::BYTE;
        using TypeNbt = NbtByte;
        using Type = int8_t;
    };

    template<>
    struct UnderlyingType<int16_t> {
        static constexpr auto TypeEnum = NbtType::SHORT;
        using TypeNbt = NbtShort;
        using Type = int16_t;
    };

    template<>
    struct UnderlyingType<int32_t> {
        static constexpr auto TypeEnum = NbtType::INT;
        using TypeNbt = NbtInt;
        using Type = int32_t;
    };

    template<>
    struct UnderlyingType<int64_t> {
        static constexpr auto TypeEnum = NbtType::LONG;
        using TypeNbt = NbtLong;
        using Type = int64_t;
    };

    template<>
    struct UnderlyingType<float> {
        static constexpr auto TypeEnum = NbtType::FLOAT;
        using TypeNbt = NbtFloat;
        using Type = float;
    };

    template<>
    struct UnderlyingType<double> {
        static constexpr auto TypeEnum = NbtType::DOUBLE;
        using TypeNbt = NbtDouble;
        using Type = double;
    };

    template<>
    struct UnderlyingType<std::string> {
        static constexpr auto TypeEnum = NbtType::STRING;
        using TypeNbt = NbtString;
        using Type = std::string;
    };

    template<>
    struct UnderlyingType<std::string_view> {
        static constexpr auto TypeEnum = NbtType::STRING;
        using TypeNbt = NbtString;
        using Type = std::string;
    };

    template<>
    struct UnderlyingType<char*> {
        static constexpr auto TypeEnum = NbtType::STRING;
        using TypeNbt = NbtString;
        using Type = std::string;
    };

    template<>
    struct UnderlyingType<const char*> {
        static constexpr auto TypeEnum = NbtType::STRING;
        using TypeNbt = NbtString;
        using Type = std::string;
    };

    template<size_t N>
    struct UnderlyingType<char[N]> {
        static constexpr auto TypeEnum = NbtType::STRING;
        using TypeNbt = NbtString;
        using Type = std::string;
    };

    template<size_t N>
    struct UnderlyingType<const char[N]> {
        static constexpr auto TypeEnum = NbtType::STRING;
        using TypeNbt = NbtString;
        using Type = std::string;
    };

    template<>
    struct UnderlyingType<std::vector<int8_t>> {
        static constexpr auto TypeEnum = NbtType::BYTE_ARRAY;
        using TypeNbt = NbtByteArray;
        using Type = std::vector<int8_t>;
    };

    template<>
    struct UnderlyingType<std::vector<int32_t>> {
        static constexpr auto TypeEnum = NbtType::INT_ARRAY;
        using TypeNbt = NbtIntArray;
        using Type = std::vector<int32_t>;
    };

    template<>
    struct UnderlyingType<std::vector<int64_t>> {
        static constexpr auto TypeEnum = NbtType::LONG_ARRAY;
        using TypeNbt = NbtLongArray;
        using Type = std::vector<int64_t>;
    };

    template<>
    struct UnderlyingType<bool> {
        static constexpr auto TypeEnum = NbtType::BYTE;
        using TypeNbt = NbtByte;
        using Type = bool;
    };

    class NbtList final : public AbstractNbtList {
        static constexpr auto SIZE = 37;

        std::vector<std::shared_ptr<NbtElement>> values{};
    public:
        NbtType type{NbtType::END};

        NbtList() = default;
        explicit NbtList(const NbtType type): type(type) {}

        [[nodiscard]]
        NbtType getType() const override {
            return NbtType::LIST;
        }

        [[nodiscard]]
        size_t size() const override {
            auto size = SIZE + 4 * values.size();
            for (const auto &element : values)
                size += element->size();
            return size;
        }

        template<typename NbtTag>
        bool add(const NbtTag &tag) {
            const auto heldType = tag.getType();
            if (heldType == NbtType::END || (type != NbtType::END && type != heldType)) return false;

            type = heldType;
            values.push_back(std::make_shared<NbtTag>(tag));
            return true;
        }

        [[nodiscard]]
        const NbtElement &get(const size_t index) const {
            return *values[index];
        }

        [[nodiscard]]
        size_t length() const {
            return values.size();
        }

        [[nodiscard]]
        NbtType getHeldType() const override {
            return type;
        }

    };

    template<typename Tag>
    [[nodiscard]]
    const Tag &getAsTag(const NbtElement &element) {
        return *dynamic_cast<const Tag*>(&element);
    }

    template<typename T>
    [[nodiscard]]
    const T &get(const NbtElement &element) {
        return getAsTag<typename UnderlyingType<T>::TypeNbt>(element).value;
    }

    class NbtCompound final : public NbtElement {
        static constexpr size_t SIZE = 48;

        absl::flat_hash_map<std::string, std::shared_ptr<NbtElement>> entries;
    public:
        NbtCompound() = default;

        [[nodiscard]]
        NbtType getType() const override {
            return NbtType::COMPOUND;
        }

        [[nodiscard]]
        size_t size() const override {
            auto size = SIZE + 36 * entries.size();
            for (const auto &[key, value] : entries)
                size += 28 + 2 * key.length() + value->size();
            return size;
        }

        [[nodiscard]]
        std::vector<std::string> getKeys() const {
            std::vector<std::string> keys;
            keys.reserve(entries.size());
            for (const auto &key : entries | std::views::keys)
                keys.push_back(key);
            return keys;
        }

        template<typename NbtTag>
        void putNbt(const std::string_view key, const NbtTag &tag) {
            entries[key] = std::make_shared<NbtTag>(tag);
        }

        template<typename T>
        void put(const std::string_view key, const T &value) {
            putNbt(key, typename UnderlyingType<T>::TypeNbt{value});
        }

        [[nodiscard]]
        OptionCRef<NbtElement> at(const std::string_view key) const {
            if (!contains(key)) return None;
            return *entries.at(key);
        }

        [[nodiscard]]
        OptionRef<NbtElement> get(const std::string_view key) {
            if (!contains(key)) return None;
            return *entries[key];
        }

        [[nodiscard]]
        Option<NbtType> getType(const std::string_view key) const {
            return at(key).transform([](const NbtElement &element) { return element.getType(); });
        }

        [[nodiscard]]
        bool contains(const std::string_view key) const {
            return entries.contains(key);
        }

        [[nodiscard]]
        bool contains(const std::string_view key, const NbtType type) const {
            return getType(key)
                .transform([type](const NbtType elemType) { return type == elemType; })
                .value_or(false);
        }

        template<typename T>
        [[nodiscard]]
        T get(const std::string_view key) const {
            if (!contains(key, UnderlyingType<T>::TypeEnum)) return {};
            const auto *element = dynamic_cast<const UnderlyingType<T>::TypeNbt*>(entries.at(key).get());
            if (!element) return {};

            return element->value;
        }

        [[nodiscard]]
        NbtCompound getCompound(const std::string_view key) const {
            if (!contains(key, NbtType::COMPOUND)) return {};
            const auto *element = dynamic_cast<const NbtCompound*>(entries.at(key).get());
            if (!element) return {};

            return *element;
        }

        [[nodiscard]]
        NbtList getList(const std::string_view key) const {
            if (!contains(key, NbtType::LIST)) return {};
            const auto *element = dynamic_cast<const NbtList*>(entries.at(key).get());
            if (!element) return {};

            return *element;
        }

    };

    template<typename T>
    [[nodiscard]]
    std::string stringifyNbtArray(const NbtElement &element, const char arrayPrefix, const std::string_view valueSuffix) {
        std::string result = "[" + std::to_string(arrayPrefix) + ";";
        const std::vector<T> &array = dynamic_cast<const UnderlyingType<std::vector<T>>::TypeNbt*>(&element)->value;
        for (size_t i = 0; i < array.size(); i++) {
            if (i != 0) result += ',';
            result += std::to_string(array[i]) + std::string(valueSuffix);
        }
        result += ']';
        return result;
    }

    [[nodiscard]]
    inline std::string escapeNbtString(const std::string &str, const bool alwaysEscape = true) {
        std::string result = " ";
        char quoteChar{};

        for (size_t i = 0; i < str.length(); i++) {
            const auto ch = str[i];
            if (ch == '\\') {
                result += "\\\\";
                continue;
            }
            if (ch == '"' || ch == '\'') {
                if (!quoteChar)
                    quoteChar = ch == '"' ? '\'' : '"';

                if (ch == quoteChar)
                    result += '\\';
            }
            result += ch;
        }
        if (!quoteChar) {
            if (!alwaysEscape) return result.substr(1);
            quoteChar = '"';
        }

        result[0] = quoteChar;
        result += quoteChar;
        return result;
    }

    [[nodiscard]]
    inline std::string stringifyNbt(const NbtElement &element, const bool alwaysEscapeCompoundKeys = true) {
        switch (element.getType()) {
            case NbtType::END:
                return "END";
            case NbtType::BYTE:
                return std::to_string(get<int8_t>(element)) + "b";
            case NbtType::SHORT:
                return std::to_string(get<int16_t>(element)) + "s";
            case NbtType::INT:
                return std::to_string(get<int32_t>(element));
            case NbtType::LONG:
                return std::to_string(get<int64_t>(element)) + "L";
            case NbtType::FLOAT:
                return std::to_string(get<float>(element)) + "f";
            case NbtType::DOUBLE:
                return std::to_string(get<double>(element)) + "d";
            case NbtType::BYTE_ARRAY:
                return stringifyNbtArray<int8_t>(element, 'B', "B");
            case NbtType::INT_ARRAY:
                return stringifyNbtArray<int32_t>(element, 'I', "");
            case NbtType::LONG_ARRAY:
                return stringifyNbtArray<int64_t>(element, 'L', "L");
            case NbtType::STRING:
                return escapeNbtString(get<std::string>(element));
            case NbtType::LIST: {
                const auto &list = getAsTag<NbtList>(element);
                std::string result = "[";
                for (size_t i = 0; i < list.length(); i++) {
                    if (i != 0) result += ',';
                    result += stringifyNbt(list.get(i), alwaysEscapeCompoundKeys);
                }
                result += ']';
                return result;
            }
            case NbtType::COMPOUND: {
                const auto &compound = getAsTag<NbtCompound>(element);
                auto keys = compound.getKeys();
                std::ranges::sort(keys);

                std::string result = "{";
                for (size_t i = 0; i < keys.size(); i++) {
                    const auto &key = keys[i];
                    if (i != 0) result += ',';
                    result += escapeNbtString(key, alwaysEscapeCompoundKeys) + ':' + stringifyNbt(compound.at(key).value(), alwaysEscapeCompoundKeys);
                }
                result += '}';
                return result;
            }
        }
        return "";
    }


}
