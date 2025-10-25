#pragma once

namespace Smasher {
	// Used by "ISerializeable" as a helper class to manage stream containing serialized class
	class OutputArchive {
	public:
		explicit OutputArchive(std::ostream& os) : _os(os) {}

		// Writes "value" to stream as a string (ex. integer 255 is written as "255" not byte 0xff
		template<class T>
		void Write(const T& value) {
			_os << value;
		}

		// Reinterprets value as raw bytes and writes those bytes to the stream
		template<class T>
		void WriteBytes(const T& value) {
			_os.write(reinterpret_cast<const char*>(&value), sizeof(T));
		}

		// Reinterprets value as raw bytes and writes those bytes to the stream
		void WriteBytes(const char* ptr, std::size_t count) {
			_os.write(ptr, count);
		}

	private:
		std::ostream& _os;
	};

	// Used by "ISerializeable" as a helper class to manage stream containing serialized class
	class InputArchive {
	public:
		explicit InputArchive(std::istream& is) : _is(is) {}

		template<class T>
		void Read(T& value) {
			_is >> value;
		}

		template<class T>
		void ReadBytes(T& value) {
			_is.read(reinterpret_cast<char*>(&value), sizeof(T));
		}

		// Reinterprets value as raw bytes and writes those bytes to the stream
		void ReadBytes(char* ptr, std::size_t count) {
			_is.read(ptr, count);
		}

	private:
		std::istream& _is;
	};

	// Serializeable Interface
	class ISerializeable {
	public:
		virtual ~ISerializeable() = default;
		virtual void Serialize(OutputArchive& out) = 0;
		virtual void Deserialize(InputArchive& in) = 0;
	};
}