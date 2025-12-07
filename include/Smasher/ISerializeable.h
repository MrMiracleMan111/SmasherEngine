#pragma once

namespace Smasher {
	// Used by "ISerializeable" as a helper class to manage stream containing serialized class
	class OutputArchive {
	public:
		explicit OutputArchive(std::ostream &outputStream) : m_OutputStream(outputStream) {}

		// Writes "value" to stream as a string (ex. integer 255 is written as "255" not byte 0xff
		template<class T>
		void Write(const T &value) {
			m_OutputStream << value;
		}

		// Reinterprets value as raw bytes and writes those bytes to the stream
		template<class T>
		void WriteBytes(const T &value) {
			m_OutputStream.write(reinterpret_cast<const char*>(&value), sizeof(T));
		}

		// Reinterprets value as raw bytes and writes those bytes to the stream
		void WriteBytes(const char *ptr, std::size_t count) {
			m_OutputStream.write(ptr, count);
		}

	private:
		std::ostream& m_OutputStream;
	};

	// Used by "ISerializeable" as a helper class to manage stream containing serialized class
	class InputArchive {
	public:
		explicit InputArchive(std::istream &inputStream) : m_InputStream(inputStream) {}

		template<class T>
		void Read(T &value) {
			m_InputStream >> value;
		}

		template<class T>
		void ReadBytes(T &value) {
			m_InputStream.read(reinterpret_cast<char*>(&value), sizeof(T));
		}

		// Reinterprets value as raw bytes and writes those bytes to the stream
		void ReadBytes(char *ptr, std::size_t count) {
			m_InputStream.read(ptr, count);
		}

	private:
		std::istream &m_InputStream;
	};

	// Serializeable Interface
	class ISerializeable {
	public:
		virtual ~ISerializeable() = default;
		virtual void Serialize(OutputArchive &out) = 0;
		virtual void Deserialize(InputArchive &in) = 0;
	};
}