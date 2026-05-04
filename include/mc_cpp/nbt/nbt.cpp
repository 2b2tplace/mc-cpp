#include <fstream>
#include <ostream>
#include <mc_cpp/nbt/nbt.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filter/zlib.hpp>

namespace mc {
	auto NbtElement::read(std::istream &stream) -> NbtElement& {
		return *this;
	}

	auto NbtElement::write(std::ostream &stream, const bool writeType, const bool writeName,
		const std::string &name) const -> void {
		if (writeType) writeBE(stream, static_cast<int8_t>(getType()));
		if (writeName) writeBEString(stream, name);
	}

	auto NbtElement::writeWithType(std::ostream &stream) const -> void {
		write(stream, true, false, DEFAULT_NBT_ELEMENT_NAME);
	}

	auto read(std::istream &stream) -> NbtElementPtr {
		const auto type = static_cast<NbtType>(readBE<int8_t>(stream));
		auto result = createTag(type);
		result->read(stream);
		return result;
	}

    auto encodeToModifiedUTF8(const std::string &input) -> std::string {
        std::string output;
        output.reserve(input.size() * 2);

        for (size_t i = 0; i < input.size();) {
            const auto c = static_cast<uint8_t>(input[i]);
            uint32_t codepoint = 0;

            if (c <= 0x7F) {
                codepoint = c;
                i += 1;
            } else if ((c & 0xE0) == 0xC0) {
                codepoint = (c & 0x1F) << 6
                            | static_cast<uint8_t>(input[i + 1]) & 0x3F;
                i += 2;
            } else if ((c & 0xF0) == 0xE0) {
                codepoint = (c & 0x0F) << 12
                            | (static_cast<uint8_t>(input[i + 1]) & 0x3F) << 6
                            | static_cast<uint8_t>(input[i + 2]) & 0x3F;
                i += 3;
            } else {
                codepoint = (c & 0x07) << 18
                            | (static_cast<uint8_t>(input[i + 1]) & 0x3F) << 12
                            | (static_cast<uint8_t>(input[i + 2]) & 0x3F) << 6
                            | static_cast<uint8_t>(input[i + 3]) & 0x3F;
                i += 4;
            }
            if (codepoint == 0) {
                output.push_back(static_cast<char>(0xC0));
                output.push_back(static_cast<char>(0x80));
            } else if (codepoint <= 0x7F) {
                output.push_back(static_cast<char>(codepoint));
            } else if (codepoint <= 0x7FF) {
                output.push_back(static_cast<char>(0xC0 | codepoint >> 6));
                output.push_back(static_cast<char>(0x80 | codepoint & 0x3F));
            } else if (codepoint <= 0xFFFF) {
                output.push_back(static_cast<char>(0xE0 | codepoint >> 12));
                output.push_back(static_cast<char>(0x80 | codepoint >> 6 & 0x3F));
                output.push_back(static_cast<char>(0x80 | codepoint & 0x3F));
            } else {
                const auto cp = codepoint - 0x10000;
                const auto high = static_cast<uint16_t>(0xD800 | cp >> 10);
                const auto low  = static_cast<uint16_t>(0xDC00 | cp & 0x3FF);

                const auto encode3 = [&](const uint16_t surrogate) {
                    output.push_back(static_cast<char>(0xE0 | surrogate >> 12));
                    output.push_back(static_cast<char>(0x80 | surrogate >> 6 & 0x3F));
                    output.push_back(static_cast<char>(0x80 | surrogate & 0x3F));
                };
                encode3(high);
                encode3(low);
            }
        }
        return output;
    }

    auto decodeToRegularUTF8(const std::string &input) -> std::string {
        std::string output;
        output.reserve(input.size());

        for (size_t i = 0; i < input.size();) {
            if (i + 5 >= input.size()) {
                output.push_back(input[i++]);
                continue;
            }
            const auto b0 = static_cast<uint8_t>(input[i]);
            const auto b1 = static_cast<uint8_t>(input[i + 1]);
            const auto b2 = static_cast<uint8_t>(input[i + 2]);
            const auto b3 = static_cast<uint8_t>(input[i + 3]);
            const auto b4 = static_cast<uint8_t>(input[i + 4]);
            const auto b5 = static_cast<uint8_t>(input[i + 5]);

            const auto isHigh = b0 == 0xED && b1 >= 0xA0 && b1 <= 0xAF && (b2 & 0xC0) == 0x80;
            const auto isLow  = b3 == 0xED && b4 >= 0xB0 && b4 <= 0xBF && (b5 & 0xC0) == 0x80;

            if (!isHigh || !isLow) {
                output.push_back(input[i++]);
                continue;
            }
            const auto high = static_cast<uint16_t>((b0 & 0x0F) << 12 | (b1 & 0x3F) << 6 | b2 & 0x3F);
            const auto low = static_cast<uint16_t>((b3 & 0x0F) << 12 | (b4 & 0x3F) << 6 | b5 & 0x3F);

            if (high < 0xD800 || high > 0xDBFF || low < 0xDC00 || low > 0xDFFF) {
                output.push_back(input[i++]);
                continue;
            }
            const auto codepoint = static_cast<uint32_t>(0x10000 + ((high - 0xD800) << 10) + (low - 0xDC00));

            output.push_back(static_cast<char>(0xF0 | codepoint >> 18));
            output.push_back(static_cast<char>(0x80 | codepoint >> 12 & 0x3F));
            output.push_back(static_cast<char>(0x80 | codepoint >> 6 & 0x3F));
            output.push_back(static_cast<char>(0x80 | codepoint & 0x3F));

            i += 6;
        }
        return output;
    }

    NbtString::NbtString(std::string value): value(std::move(value)) {}

    auto NbtString::operator=(const NbtString &other) -> NbtString& {
        value = other.value;
        return *this;
    }

    auto NbtString::getType() const -> NbtType {
		return TypeEnum;
	}

	auto NbtString::byteSize() const -> size_t {
		return SIZE + 2 * value.length();
	}

	auto NbtString::read(std::istream &stream) -> NbtString& {
		value = decodeToRegularUTF8(readBEString(stream));
		return *this;
	}

	void NbtString::write(std::ostream &stream, const bool writeType, const bool writeName,
		const std::string &name) const {
		NbtElement::write(stream, writeType, writeName, name);
		writeBEString(stream, encodeToModifiedUTF8(value));
	}

    auto NbtString::clone() const -> NbtElementPtr {
        return std::make_unique<NbtString>(value);
    }

    auto NbtEnd::operator=(const NbtEnd&) -> NbtEnd& {
        return *this;
    }

    auto NbtEnd::getType() const -> NbtType {
		return TypeEnum;
	}

	auto NbtEnd::byteSize() const -> size_t {
		return SIZE;
	}

    auto NbtEnd::clone() const -> NbtElementPtr {
        return std::make_unique<NbtEnd>();
    }

    NbtList::NbtList(const NbtList &other): type(other.type) {
        *this = other;
    }

    NbtList::NbtList(const NbtType type): type(type) {}

    auto NbtList::operator=(const NbtList &other) -> NbtList& {
        values.clear();
        values.reserve(other.values.size());
        for (const auto &value : other.values) {
            if (value) {
                values.push_back(value->clone());
            } else {
                values.push_back(nullptr);
            }
        }
        return *this;
    }

    auto NbtList::getType() const -> NbtType {
		return TypeEnum;
	}

	auto NbtList::byteSize() const -> size_t {
		auto size = SIZE + 4 * values.size();
		for (const auto &element : values)
			size += element->byteSize();
		return size;
	}

	auto NbtList::length() const -> size_t {
		return values.size();
	}

	auto NbtList::getHeldType() const -> NbtType {
		return type;
	}

	auto NbtList::read(std::istream &stream) -> NbtList& {
		type = static_cast<NbtType>(readBE<int8_t>(stream));
		const auto length = readBE<int32_t>(stream);

		for (int32_t i = 0; i < length; i++) {
			auto tag = createTag(type);
			if (tag == nullptr)
				throw std::runtime_error(std::string("unknown tag type with id ") + std::to_string(static_cast<int>(type)));

			tag->read(stream);
			values.push_back(std::move(tag));
		}
		return *this;
	}

	auto NbtList::write(std::ostream &stream, const bool writeType,
						const bool writeName, const std::string &name) const -> void {
		NbtElement::write(stream, writeType, writeName, name);
		writeBE<int8_t>(stream, static_cast<int8_t>(type));
		writeBE<int32_t>(stream, static_cast<int32_t>(values.size()));
		for (const auto &value : values) {
			value->write(stream, false, false, DEFAULT_NBT_ELEMENT_NAME);
		}
	}

    auto NbtList::clone() const -> NbtElementPtr {
        return std::make_unique<NbtList>(*this);
    }

    NbtCompound::NbtCompound(const NbtCompound &other) {
        *this = other;
    }

    auto NbtCompound::operator=(const NbtCompound &other) -> NbtCompound& {
        entries.clear();
        for (const auto &[key, value] : other.entries) {
            if (value) {
                entries.emplace(key, value->clone());
            } else {
                entries.emplace(key, nullptr);
            }
        }
        return *this;
    }

    auto NbtCompound::getType() const -> NbtType {
		return TypeEnum;
	}

	auto NbtCompound::byteSize() const -> size_t {
		auto size = SIZE + 36 * entries.size();
		for (const auto &[key, value] : entries)
			size += 28 + 2 * key.length() + value->byteSize();
		return size;
	}

	auto NbtCompound::getKeys() const -> std::vector<std::string> {
		std::vector<std::string> keys;
		keys.reserve(entries.size());
		for (const auto &key : entries | std::views::keys)
			keys.push_back(key);
		return keys;
	}

	auto NbtCompound::at(const std::string_view key) const -> result::OptionCRef<NbtElement> {
		if (!contains(key)) return result::None;
		return *entries.at(key);
	}

	auto NbtCompound::get(const std::string_view key) -> result::OptionRef<NbtElement> {
		if (!contains(key)) return result::None;
		return *entries[key];
	}

	auto NbtCompound::getType(const std::string_view key) const -> result::Option<NbtType> {
		return at(key).transform([](const NbtElement &element) { return element.getType(); });
	}

	auto NbtCompound::contains(const std::string_view key) const -> bool {
		return entries.contains(key);
	}

	auto NbtCompound::contains(const std::string_view key, const NbtType type) const -> bool {
		return getType(key)
				.transform([type](const NbtType elemType) { return type == elemType; })
				.value_or(false);
	}

	auto NbtCompound::read(std::istream& stream) -> NbtCompound& {
		while (true) {
			const auto type = static_cast<NbtType>(readBE<int8_t>(stream));
			if (type == NbtType::END) break;

			auto name = readBEString(stream);
		    auto tag = createTag(type);
			if (tag == nullptr)
				throw std::runtime_error(std::string("Unknown tag type with id ") + std::to_string(static_cast<int>(type)));

			tag->read(stream);
			entries[name] = std::move(tag);
		}
		return *this;
	}

	auto NbtCompound::write(std::ostream& stream, const bool writeType,
							const bool writeName, const std::string &name) const -> void {
		NbtElement::write(stream, writeType, writeName, name);
		for (const auto &[tagName, tag] : entries) {
			tag->write(stream, true, true, tagName);
		}
		writeBE<int8_t>(stream, static_cast<int8_t>(NbtType::END));
	}

    auto NbtCompound::clone() const -> NbtElementPtr {
        return std::make_unique<NbtCompound>(*this);
    }

    auto escapeNbtString(const std::string &str, const bool alwaysEscape) -> std::string {
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

	auto stringifyNbt(const NbtElement &element, const bool alwaysEscapeCompoundKeys) -> std::string {
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
					result += stringifyNbt(*list.values[i], alwaysEscapeCompoundKeys);
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

	auto createTag(const NbtType type) -> NbtElementPtr {
		switch (type) {
			case NbtType::BYTE:
				return std::make_unique<NbtByte>();
			case NbtType::SHORT:
				return std::make_unique<NbtShort>();
			case NbtType::INT:
				return std::make_unique<NbtInt>();
			case NbtType::LONG:
				return std::make_unique<NbtLong>();
			case NbtType::FLOAT:
				return std::make_unique<NbtFloat>();
			case NbtType::DOUBLE:
				return std::make_unique<NbtDouble>();
			case NbtType::BYTE_ARRAY:
				return std::make_unique<NbtByteArray>();
			case NbtType::STRING:
				return std::make_unique<NbtString>();
			case NbtType::LIST:
				return std::make_unique<NbtList>();
			case NbtType::COMPOUND:
				return std::make_unique<NbtCompound>();
			case NbtType::INT_ARRAY:
				return std::make_unique<NbtIntArray>();
			case NbtType::LONG_ARRAY:
				return std::make_unique<NbtLongArray>();
			case NbtType::END:
				return std::make_unique<NbtEnd>();
			default:
				std::unreachable();
		}
	}

	void NbtFile::decompressStream(std::istream& stream, std::stringstream& decompressed,
	                               const Compression compression) {
		if (compression == Compression::NO_COMPRESSION) {
			decompressed << stream.rdbuf();
			return;
		}
		boost::iostreams::filtering_streambuf<boost::iostreams::input> in;
		if (compression == Compression::GZIP) {
			in.push(boost::iostreams::gzip_decompressor());
		} else if (compression == Compression::ZLIB) {
			in.push(boost::iostreams::zlib_decompressor());
		}
		try {
			in.push(stream);
			boost::iostreams::copy(in, decompressed);
		} catch (boost::iostreams::gzip_error &e) {
			throw std::runtime_error(
					"error while decompressing gzip data: " + std::string(e.what()) + " ("
							+ std::to_string(e.error()) + ")");
		} catch (boost::iostreams::zlib_error &e) {
			throw std::runtime_error(
					"error while decompressing zlib data: " + std::string(e.what()) + " ("
							+ std::to_string(e.error()) + ")");
		}
	}

	void NbtFile::readCompressed(std::istream& stream, const Compression compression) {
		std::stringstream decompressed(std::ios::in | std::ios::out | std::ios::binary);
		decompressStream(stream, decompressed, compression);
		if (const auto type = NbtByte{}.read(decompressed).value;
			static_cast<NbtType>(type) != NbtType::COMPOUND)
			throw std::runtime_error("first tag is not a tag compound");

		compoundName = NbtString{}.read(decompressed).value;
		NbtCompound::read(decompressed);
	}

	void NbtFile::readNBT(std::istream& stream, const Compression compression) {
		readCompressed(stream, compression);
	}

	void NbtFile::readNBT(const char* filename, const Compression compression) {
		std::ifstream file(filename, std::ios::binary);
		if (!file)
			throw std::runtime_error(std::string("unable to open file '") + filename + "'");
		readCompressed(file, compression);
		file.close();
	}

	void NbtFile::readNBT(const char* buffer, const size_t len, const Compression compression) {
		std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
		stream.write(buffer, static_cast<std::streamsize>(len));
		readCompressed(stream, compression);
	}

	void NbtFile::writeNBT(std::ostream& stream, const Compression compression) const {
		std::stringstream in(std::ios::in | std::ios::out | std::ios::binary);
		boost::iostreams::filtering_streambuf<boost::iostreams::input> out;
		if (compression == Compression::GZIP) {
			out.push(boost::iostreams::gzip_compressor());
		} else if (compression == Compression::ZLIB) {
			out.push(boost::iostreams::zlib_compressor());
		} else {
			writeWithType(stream);
			return;
		}
		out.push(in);
		writeWithType(in);
		boost::iostreams::copy(out, stream);
	}

	void NbtFile::writeNBT(const char* filename, const Compression compression) const {
		std::ofstream file(filename, std::ios::binary);
		if (!file)
			throw std::runtime_error(std::string("unable to open file '") + filename + "'");
		writeNBT(file, compression);
		file.close();
	}

	auto NbtFile::write(std::ostream& stream, const bool writeType,
						const bool writeName, const std::string &name) const -> void {
		NbtCompound::write(stream, writeType, true, writeName ? name : compoundName);
	}
}
