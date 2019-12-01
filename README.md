# BinaryLove
BinaryLove is a simple header only binary data parsing library. It accepts the data in the form of `std::vector<unsigned char>` and parses it to `std::tuple` of specified types and vice versa. Header also provides simple data loader and writer that utilise `<fstream>` header.. 

# Examples
This section shows the propper usage of all of library's functions.

## Reading and Writing files.
```cpp
#include "BinaryLove.hpp"

int main()
{
	// Reads the data and puts in inside "data" buffer
	BinaryLove::binary_data_t data = BinaryLove::FileStream::ReadFile("in.wav");

	// Writes the data from "data" to specified file
	BinaryLove::FileStream::WriteFile("out.wav", data);

	return 0;
}
```
## Parsing and streaming the data
```cpp
#include "BinaryLove.hpp"

int main()
{
	// Reads the data and put's in inside "data" buffer
	BinaryLove::binary_data_t in_data = BinaryLove::FileStream::ReadFile("in.wav");

	// Current offset we are at, used to identify from where we want to read the data
	// Read and Write functions set offset to next read/write position
	BinaryLove::buffer_offset_t in_offset = 0;
	
	// The Canonical WAVE file format
	// source: http://soundfile.sapp.org/doc/WaveFormat/

	// Reading the header
	auto wav_header = BinaryLove::Read<
		// RIFF Header
		char[4],			// Chunk ID
		std::uint32_t,		// Chunk Size
		char[4],			// Format

		// The fmt subchunk
		char[4],			// Subchunk 1 ID
		std::uint32_t,		// Subchunk 1 Size
		std::uint16_t,		// Audio Format
		std::uint16_t,		// Number of Channels
		std::uint32_t,		// Sample Rate
		std::uint32_t,		// Byte Rate
		std::uint16_t,		// Block Align
		std::uint16_t,		// Bits per Sample

		// The data subchunk
		char[4],			// Subchunk 2 ID
		std::uint32_t		// Subchunk 2 Size
	>(in_data, in_offset);

	// Reading the data
	auto wav_data = BinaryLove::ReadStream<
		std::int16_t		// Mono-channel 16 bits per sample
	>(in_data, in_offset, std::get<12>(wav_header));
	
	// Current offset we are at, used to identify from where we want to write the data
	BinaryLove::buffer_offset_t out_offset = 0;

	// Buffer we want to write the data to
	BinaryLove::binary_data_t out_data;

	// We need to resize our buffer to fit all bytes
	// GetTupleSize(std::tuple<>) returns the size of tuple in bytes
	out_data.resize(BinaryLove::GetTupleSize(wav_header) + BinaryLove::GetTupleSize(wav_data[0]) * wav_data.size());

	// Writes tuple to buffer
	BinaryLove::Write(out_data, wav_header, out_offset);
	// Writes vector of tuples to buffer
	BinaryLove::WriteStream(out_data, wav_data, out_offset, BinaryLove::GetTupleSize(wav_data[0]) * wav_data.size());
	
	// Writes the data from "data" to specified file
	BinaryLove::FileStream::WriteFile("out.wav", out_data);

	return 0;
}
```
# Documentation

## BinaryLove namespace
---
`template<class ...T> std::size_t GetTupleSize(const std::tuple<T...>& _tuple)`

Purpose: Returns the size of tuple in bytes

`_tuple` - tuple, whose size will be returned

---
`template<class ...Types> std::tuple<Types...> Read(const binary_data_t& _buffer, buffer_offset_t& _offset)`

Purpose: Read buffer at specified offset and return the specified tuple

`_buffer` - buffer we are reading data from

`_offset` - offset of the data from the beginning of the buffer

---
`template<class ...Types> std::vector<std::tuple<Types...>> ReadStream(const binary_data_t& _buffer, buffer_offset_t& _offset, const buffer_size_t& _buffer_size)`

Purpose: Read the buffer at specified offset and return vector of specified tuples

`_buffer` - buffer we are reading data from

`_offset` - offset of the data from the beginning of the buffer

`_buffer_size` - amount of bytes we want to read

---
`template<class ...Types> void Write(binary_data_t& _buffer, const std::tuple<Types...>& _tuple, buffer_offset_t& _offset)`

Purpose: Writes specified tuple to specified buffer at give offset

`_buffer` - buffer we are writing to

`_tuple` - tuple we are reading from

`_offset` - offset we are writing at

---
`template<class ...Types> void WriteStream(binary_data_t& _buffer, const std::vector<std::tuple<Types...>>& _tuple_buffer, buffer_offset_t& _offset, const buffer_size_t& _buffer_size)`

Purpose: Writes specified tuple vector to specified buffer at give offset

`_buffer` - buffer we are writing to

`_tuple_buffer` - tuple vector we are reading from

`_offset` - offset we are writing at

`_buffer_size` - amount of bytes we want to read from tuple buffer

---
## BinaryLove::FileStream namespace

---
`binary_data_t ReadFile(const char* _filename)`

Purpose: Reads the data from specified file and returns vector of bytes

`_filename` - name of the file we want to read from

---
`void WriteFile(const char* _filename, const binary_data_t& _data)`

Purpose: Writes vector of bytes to the file with specified name

`_filename` - name of the file we want to write to

`_data` - data we want to write to the file

---

# Exceptions

Exceptions are enabled by default. BinaryLove can throw two types of excpetions: `BinaryLoveFileOutOfRange` and `BinaryLoveOutOfRangeException`.

To disable exceptions do `#undef BINARY_LOVE_EXCEPTIONS`.
