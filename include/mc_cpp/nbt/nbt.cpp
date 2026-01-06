#include <fstream>
#include <ostream>
#include <mc_cpp/nbt/nbt.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filter/zlib.hpp>

namespace mc {
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
