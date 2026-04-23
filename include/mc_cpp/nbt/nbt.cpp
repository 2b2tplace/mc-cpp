#include <fstream>
#include <ostream>
#include <mc_cpp/nbt/nbt.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filter/zlib.hpp>

namespace mc {
	auto NbtElement::read(std::istream &stream) -> NbtElement & {
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
		NbtElementPtr result;
		switch (type) {
			case NbtType::BYTE:		  { result = std::make_shared<NbtByte>();      break; }
			case NbtType::SHORT:	  { result = std::make_shared<NbtShort>();     break; }
			case NbtType::INT:		  { result = std::make_shared<NbtInt>();       break; }
			case NbtType::LONG:		  { result = std::make_shared<NbtLong>();      break; }
			case NbtType::FLOAT:      { result = std::make_shared<NbtFloat>();	   break; }
			case NbtType::DOUBLE:	  { result = std::make_shared<NbtDouble>();	   break; }
			case NbtType::BYTE_ARRAY: { result = std::make_shared<NbtByteArray>(); break; }
			case NbtType::STRING:	  { result = std::make_shared<NbtString>();    break; }
			case NbtType::LIST:       { result = std::make_shared<NbtList>();      break; }
			case NbtType::COMPOUND:   { result = std::make_shared<NbtCompound>();  break; }
			case NbtType::INT_ARRAY:  { result = std::make_shared<NbtIntArray>();  break; }
			case NbtType::LONG_ARRAY: { result = std::make_shared<NbtLongArray>(); break; }
			default:				  { result = std::make_shared<NbtEnd>();       break; }
		}
		result->read(stream);
		return result;
	}

	NbtString::NbtString(std::string value): value(std::move(value)) {}

	auto NbtString::getType() const -> NbtType {
		return TypeEnum;
	}

	auto NbtString::byteSize() const -> size_t {
		return SIZE + 2 * value.length();
	}

	NbtString & NbtString::read(std::istream &stream) {
		value = readBEString(stream);
		return *this;
	}

	void NbtString::write(std::ostream &stream, const bool writeType, const bool writeName,
		const std::string &name) const {
		NbtElement::write(stream, writeType, writeName, name);
		writeBEString(stream, value);
	}

	auto NbtEnd::getType() const -> NbtType {
		return TypeEnum;
	}

	auto NbtEnd::byteSize() const -> size_t {
		return SIZE;
	}

	NbtList::NbtList(const NbtType type): type(type) {}

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
			const auto tag = createTag(type);
			if (tag == nullptr)
				throw std::runtime_error(std::string("unknown tag type with id ") + std::to_string(static_cast<int>(type)));

			tag->read(stream);
			values.push_back(tag);
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
			const auto tag = createTag(type);
			if (tag == nullptr)
				throw std::runtime_error(std::string("Unknown tag type with id ") + std::to_string(static_cast<int>(type)));

			tag->read(stream);
			entries[name] = tag;
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
				return std::make_shared<NbtByte>();
			case NbtType::SHORT:
				return std::make_shared<NbtShort>();
			case NbtType::INT:
				return std::make_shared<NbtInt>();
			case NbtType::LONG:
				return std::make_shared<NbtLong>();
			case NbtType::FLOAT:
				return std::make_shared<NbtFloat>();
			case NbtType::DOUBLE:
				return std::make_shared<NbtDouble>();
			case NbtType::BYTE_ARRAY:
				return std::make_shared<NbtByteArray>();
			case NbtType::STRING:
				return std::make_shared<NbtString>();
			case NbtType::LIST:
				return std::make_shared<NbtList>();
			case NbtType::COMPOUND:
				return std::make_shared<NbtCompound>();
			case NbtType::INT_ARRAY:
				return std::make_shared<NbtIntArray>();
			case NbtType::LONG_ARRAY:
				return std::make_shared<NbtLongArray>();
			case NbtType::END:
				return std::make_shared<NbtEnd>();
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
