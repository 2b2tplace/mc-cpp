#pragma once

#include <memory>
#include <result.hpp>
#include <string>
#include <utility>
#include <vector>
#include <mc_cpp/endian.hpp>
#include <absl/container/flat_hash_map.h>
#include <fmt/format.h>

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

    enum class Compression {
        NO_COMPRESSION = 0, GZIP = 1, ZLIB = 2
    };

    static constexpr size_t END_SIZE = 0;
    static constexpr size_t BYTE_SIZE = 1;
    static constexpr size_t SHORT_SIZE = 2;
    static constexpr size_t INT_SIZE = 4;
    static constexpr size_t LONG_SIZE = 8;
    static constexpr size_t FLOAT_SIZE = 4;
    static constexpr size_t DOUBLE_SIZE = 8;

    static const std::string DEFAULT_NBT_ELEMENT_NAME;

    class NbtElement {
    public:
        virtual ~NbtElement() = default;

        [[nodiscard]]
        virtual auto getType() const -> NbtType = 0;

        [[nodiscard]]
        virtual auto byteSize() const -> size_t = 0;

        virtual auto read(std::istream& stream) -> NbtElement&;

        virtual auto write(std::ostream& stream, bool writeType, bool writeName, const std::string &name) const -> void;

        virtual auto writeWithType(std::ostream& stream) const -> void;
    };

    using NbtElementPtr = std::shared_ptr<NbtElement>;

    class AbstractNbtList : public NbtElement {

        [[nodiscard]]
        virtual auto getHeldType() const -> NbtType = 0;

    };

    template<typename T, NbtType TYPE, size_t SIZE>
    class NbtPrimitive final : public NbtElement {
        static constexpr auto FULL_SIZE = 8 + SIZE;
    public:
        static constexpr auto TypeEnum = TYPE;

        T value{};

        NbtPrimitive() = default;
        explicit NbtPrimitive(const T value): value(value) {}

        [[nodiscard]]
        auto getType() const -> NbtType override {
            return TypeEnum;
        }

        [[nodiscard]]
        auto byteSize() const -> size_t override {
            return FULL_SIZE;
        }

        auto read(std::istream& stream) -> NbtPrimitive& override {
            value = readBE<T>(stream);
            return *this;
        }

        auto write(std::ostream& stream, const bool writeType, const bool writeName, const std::string &name) const -> void override {
            NbtElement::write(stream, writeType, writeName, name);
            writeBE<T>(stream, value);
        }

    };

    class NbtString final : public NbtElement {
        static constexpr size_t SIZE = 36;

    public:
        static constexpr auto TypeEnum = NbtType::STRING;

        std::string value;

        NbtString() = default;
        explicit NbtString(std::string value);

        [[nodiscard]]
        auto getType() const -> NbtType override;

        [[nodiscard]]
        auto byteSize() const -> size_t override;

        auto read(std::istream& stream) -> NbtString& override;

        auto write(std::ostream& stream, bool writeType, bool writeName, const std::string &name) const -> void override;
    };

    class NbtEnd final : public NbtElement {
        static constexpr size_t SIZE = 8;

    public:
        static constexpr auto TypeEnum = NbtType::END;
        NbtEnd() = default;

        [[nodiscard]]
        auto getType() const -> NbtType override;

        [[nodiscard]]
        auto byteSize() const -> size_t override;
    };

    template<typename T, NbtType TYPE, NbtType HELD_TYPE, size_t HELD_SIZE>
    class NbtPrimitiveArray final : public AbstractNbtList {
        static constexpr auto SIZE = 24;

    public:
        static constexpr auto TypeEnum = TYPE;
        std::vector<T> value;

        NbtPrimitiveArray() = default;
        explicit NbtPrimitiveArray(const std::vector<T> &value): value(value) {}

        [[nodiscard]]
        auto getType() const -> NbtType override {
            return TypeEnum;
        }

        [[nodiscard]]
        auto byteSize() const -> size_t override {
            return SIZE + HELD_SIZE * value.size();
        }

        [[nodiscard]]
        auto getHeldType() const -> NbtType override {
            return HELD_TYPE;
        }

        auto read(std::istream& stream) -> NbtPrimitiveArray& override {
            const auto length = readBE<int32_t>(stream);
            value.resize(length);
            if (std::is_same_v<T, int8_t>) {
                stream.read(reinterpret_cast<char*>(&value[0]), length * sizeof(T));
            } else {
                for (int32_t i = 0; i < length; i++)
                    value[i] = readBE<T>(stream);
            }
            return *this;
        }

        auto write(std::ostream& stream, const bool writeType, const bool writeName, const std::string &name) const -> void override {
            NbtElement::write(stream, writeType, writeName, name);
            writeBE<int32_t>(stream, value.size());
            if (std::is_same_v<T, int8_t>) {
                stream.write(reinterpret_cast<const char*>(&value[0]), value.size() * sizeof(T));
            } else {
                for (size_t i = 0; i < value.size(); i++)
                    writeBE<T>(stream, value[i]);
            }
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

    template<typename Tag>
    [[nodiscard]]
    auto getAsTag(const NbtElement &element) -> const Tag& {
        return dynamic_cast<const Tag&>(element);
    }

    template<typename Tag>
    [[nodiscard]]
    auto getAsTag(NbtElement *element) -> Tag* {
        return dynamic_cast<Tag*>(element);
    }

    template<typename T>
    [[nodiscard]]
    auto get(const NbtElement &element) -> const T& {
        return getAsTag<typename UnderlyingType<T>::TypeNbt>(element).value;
    }

    class NbtList final : public AbstractNbtList {
        static constexpr auto SIZE = 37;

    public:
        std::vector<NbtElementPtr> values{};
        static constexpr auto TypeEnum = NbtType::LIST;
        NbtType type{NbtType::END};

        NbtList() = default;
        explicit NbtList(const NbtType type);

        [[nodiscard]]
        auto getType() const -> NbtType override;

        [[nodiscard]]
        auto byteSize() const -> size_t override;

        template<typename NbtTag>
        auto add(const NbtTag &tag) -> bool {
            const auto heldType = tag.getType();
            if (heldType == NbtType::END || (type != NbtType::END && type != heldType)) return false;

            type = heldType;
            values.push_back(std::make_shared<NbtTag>(tag));
            return true;
        }

        template<typename T>
        [[nodiscard]]
        auto read(const size_t index) const -> T {
            if (values[index]->getType() != UnderlyingType<T>::TypeEnum) return {};
            const auto *element = getAsTag<typename UnderlyingType<T>::TypeNbt>(values[index].get());
            if (!element) return {};

            return element->value;
        }

        template<typename T>
        [[nodiscard]]
        auto readNbt(const size_t index) const -> T {
            if (values[index]->getType() != T::TypeEnum) return {};
            const auto *element = getAsTag<T>(values[index].get());
            if (!element) return {};

            return *element;
        }

        template<typename T>
        [[nodiscard]]
        auto get(const size_t index) -> UnderlyingType<T>::TypeNbt* {
            if (values[index]->getType() != UnderlyingType<T>::TypeEnum) return nullptr;
            return getAsTag<typename UnderlyingType<T>::TypeNbt>(values[index].get());
        }

        template<typename T>
        [[nodiscard]]
        auto getNbt(const size_t index) -> T* {
            if (values[index]->getType() != T::TypeEnum) return nullptr;
            return getAsTag<T>(values[index].get());
        }

        [[nodiscard]]
        auto length() const -> size_t;

        [[nodiscard]]
        auto getHeldType() const -> NbtType override;

        auto read(std::istream &stream) -> NbtList& override;

        auto write(std::ostream &stream, bool writeType, bool writeName, const std::string &name) const -> void override;

    };

    class NbtCompound : public NbtElement {
        static constexpr size_t SIZE = 48;

    public:
        static constexpr auto TypeEnum = NbtType::COMPOUND;
        absl::flat_hash_map<std::string, NbtElementPtr> entries;

        NbtCompound() = default;

        [[nodiscard]]
        auto getType() const -> NbtType override;

        [[nodiscard]]
        auto byteSize() const -> size_t override;

        [[nodiscard]]
        auto getKeys() const -> std::vector<std::string>;

        template<typename NbtTag>
        auto putNbt(const std::string_view key, const NbtTag &tag) -> void {
            entries[key] = std::make_shared<NbtTag>(tag);
        }

        template<typename T>
        auto put(const std::string_view key, const T &value) -> void {
            putNbt(key, typename UnderlyingType<T>::TypeNbt{value});
        }

        [[nodiscard]]
        auto at(std::string_view key) const -> result::OptionCRef<NbtElement>;

        [[nodiscard]]
        auto get(std::string_view key) -> result::OptionRef<NbtElement>;

        [[nodiscard]]
        auto getType(std::string_view key) const -> result::Option<NbtType>;

        [[nodiscard]]
        auto contains(std::string_view key) const -> bool;

        [[nodiscard]]
        auto contains(std::string_view key, NbtType type) const -> bool;

        template<typename T>
        [[nodiscard]]
        auto containsNbt(const std::string_view key) const -> bool {
            return contains(key, T::TypeEnum);
        }

        template<typename T>
        [[nodiscard]]
        auto contains(const std::string_view key) const -> bool {
            return containsNbt<typename UnderlyingType<T>::TypeNbt>(key);
        }

        template<typename T>
        auto read(const std::string_view key, T &into) const -> void {
            into = read<T>(key);
        }

        template<typename T>
        auto readNbt(const std::string_view key, T &into) const {
            into = readNbt<T>(key);
        }

        template<typename T>
        [[nodiscard]]
        auto read(const std::string_view key) const -> T {
            if (!contains(key, UnderlyingType<T>::TypeEnum)) return {};
            const auto *element = getAsTag<typename UnderlyingType<T>::TypeNbt>(entries.at(key).get());
            if (!element) return {};

            return element->value;
        }

        template<typename T>
        [[nodiscard]]
        auto readNbt(const std::string_view key) const -> T {
            if (!contains(key, T::TypeEnum)) return {};
            const auto *element = getAsTag<T>(entries.at(key).get());
            if (!element) return {};

            return *element;
        }

        template<typename T>
        [[nodiscard]]
        auto get(const std::string_view key) -> UnderlyingType<T>::TypeNbt* {
            if (!contains(key, UnderlyingType<T>::TypeEnum)) return nullptr;
            return getAsTag<typename UnderlyingType<T>::TypeNbt>(entries.at(key).get());
        }

        template<typename T>
        [[nodiscard]]
        auto getNbt(const std::string_view key) -> T* {
            if (!contains(key, T::TypeEnum)) return {};
            return getAsTag<T>(entries.at(key).get());
        }

        auto read(std::istream &stream) -> NbtCompound& override;

        auto write(std::ostream &stream, bool writeType, bool writeName, const std::string &name) const -> void override;

    };

    template<typename T>
    [[nodiscard]]
    auto stringifyNbtArray(const NbtElement &element, const char arrayPrefix,
                           const std::string_view valueSuffix) -> std::string {
        std::string result = fmt::format("[{};", arrayPrefix);
        const std::vector<T> &array = dynamic_cast<const UnderlyingType<std::vector<T>>::TypeNbt*>(&element)->value;
        for (size_t i = 0; i < array.size(); i++) {
            if (i != 0) result += ',';
            result += std::to_string(array[i]) + std::string(valueSuffix);
        }
        result += ']';
        return result;
    }

    [[nodiscard]]
    auto escapeNbtString(const std::string &str, bool alwaysEscape = true) -> std::string;

    [[nodiscard]]
    auto stringifyNbt(const NbtElement &element, bool alwaysEscapeCompoundKeys = true) -> std::string;

    auto createTag(NbtType type) -> NbtElementPtr;

    class NbtFile final : public NbtCompound {
        static void decompressStream(std::istream& stream, std::stringstream& decompressed,
                                     Compression compression);

        std::string compoundName{};
    public:
        void readCompressed(std::istream& stream, Compression compression = Compression::GZIP);
        void readNBT(std::istream& stream, Compression compression = Compression::GZIP);
        void readNBT(const char* filename, Compression compression = Compression::GZIP);
        void readNBT(const char* buffer, size_t len, Compression compression = Compression::GZIP);

        void writeNBT(std::ostream& stream, Compression compression = Compression::GZIP) const;
        void writeNBT(const char* filename, Compression compression = Compression::GZIP) const;

        auto write(std::ostream &stream, bool writeType, bool writeName, const std::string &name) const -> void override;
    };

}
