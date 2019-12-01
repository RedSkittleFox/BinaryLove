//		MIT License
//		
//		Copyright(c) 2019 Marcin Poloczek (RedSkittleFox)
//		
//		Permission is hereby granted, free of charge, to any person obtaining a copy
//		of this softwareand associated documentation files(the "Software"), to deal
//		in the Software without restriction, including without limitation the rights
//		to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
//		copies of the Software, and to permit persons to whom the Software is
//		furnished to do so, subject to the following conditions :
//		
//		The above copyright noticeand this permission notice shall be included in all
//		copies or substantial portions of the Software.
//		
//		THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//		IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//		FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
//		AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//		LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//		OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//		SOFTWARE.

#ifndef BINARY_LOVE_H
#pragma once
#define BINARY_LOVE_H

#define BINARY_LOVE_EXCEPTIONS

#include <tuple>
#include <vector>
#include <utility>
#include <type_traits>
#include <fstream>

namespace BinaryLove
{
	using byte_t = std::uint8_t;
	using binary_data_t = std::vector<byte_t>;
	using buffer_size_t = std::size_t;
	using buffer_offset_t = std::uint32_t;

	namespace FileStream
	{
		namespace details
		{
#ifdef BINARY_LOVE_EXCEPTIONS
			// Purpose: Binary-Love file doesn't exist
			class BinaryLoveFileOutOfRange : public std::exception
			{
			public:
				const char* what() const noexcept override {
					return "Failed to access the file.\n";
				}
			};
#endif
		}

		binary_data_t ReadFile(const char* _filename)
		{
			std::ifstream in(_filename, std::ios::binary);
#ifdef BINARY_LOVE_EXCEPTIONS
			if (in.bad())
			{
				in.close();
				throw details::BinaryLoveFileOutOfRange();
				return binary_data_t();
			}
#endif
			binary_data_t bl_return(std::istreambuf_iterator<char>(in), {});
			in.close();
			return bl_return;
		};

		void WriteFile(const char* _filename, const binary_data_t& _data)
		{
			std::ofstream out(_filename, std::ios::binary);
#ifdef BINARY_LOVE_EXCEPTIONS
			if (out.bad())
			{
				out.close();
				throw details::BinaryLoveFileOutOfRange();
				return;
			}
#endif
			out.write((char*)(_data.data()), _data.size() * sizeof(byte_t));
			out.close();
		}
	}

	namespace details
	{
		// Purpose: Provide implementation of is_bounded_array type
		template<class T>
		struct is_bounded_array : std::false_type {};

		template<class T, std::size_t N>
		struct is_bounded_array<T[N]> : std::true_type {};

		// Purpose: Overrides tuple with data from _buffer from a given _offset
		template<std::size_t I = 0, class ...T>
		void BufferToTuple(std::tuple<T...>& _tuple,
			const std::vector<byte_t>& _buffer,
			buffer_offset_t& _offset)
		{
			using current_type = typename std::tuple_element<I, std::tuple<T...>>::type;
			// We want to make sure that the type is arithmetic
			static_assert(std::is_arithmetic <current_type>::value
				|| (is_bounded_array<current_type>::value
					&& std::is_arithmetic<std::remove_all_extents<current_type>::type>::value
					),
				"Static assertion failed: BinaryLove Error: Trying to write to non-arithmetic type!\n");

			// If is not an array
			if constexpr (std::is_integral<current_type>::value || std::is_floating_point<current_type>::value)
			{
				std::memcpy(&std::get<I>(_tuple), &_buffer[_offset],
					sizeof(current_type));

				_offset += sizeof(current_type);
			}
			// If is an array
			else if constexpr ((is_bounded_array<current_type>::value
				&& std::is_integral<std::remove_all_extents<current_type>::type>::value)
				|| (is_bounded_array<current_type>::value
					&& std::is_floating_point<std::remove_all_extents<current_type>::type>::value))
			{
				for (std::size_t i = 0; i < std::extent<current_type>::value; ++i)
				{
					std::memcpy(&std::get<I>(_tuple)[i], &_buffer[_offset],
						sizeof(std::remove_all_extents<current_type>::type));

					_offset += sizeof(std::remove_all_extents<current_type>::type);
				}
			}

			if constexpr (I + 1 != std::tuple_size<std::tuple<T...>>::value)
				BufferToTuple<I + 1>(_tuple, _buffer, _offset);
		}

		// Purpose: Overrides _buffer with data from _tuple at a given _offset
		template<std::size_t I = 0, class ...T>
		void TupleToBuffer(const std::tuple<T...>& _tuple,
			std::vector<byte_t>& _buffer,
			buffer_offset_t& _offset)
		{
			using current_type = typename std::tuple_element<I, std::tuple<T...>>::type;
			// Make sure it's an arithmetic type
			static_assert(std::is_arithmetic <current_type>::value
				|| (is_bounded_array<current_type>::value
					&& std::is_arithmetic<std::remove_all_extents<current_type>::type>::value
					),
				"Static assertion failed: BinaryLove Error: Trying to write to non-arithmetic type!\n");

			// If is not an array
			if constexpr (std::is_integral<current_type>::value || std::is_floating_point<current_type>::value)
			{
				std::memcpy(&_buffer[_offset], &std::get<I>(_tuple),
					sizeof(current_type));

				_offset += sizeof(current_type);
			}
			// If is an array
			else if constexpr ((is_bounded_array<current_type>::value
				&& std::is_integral<std::remove_all_extents<current_type>::type>::value)
				|| (is_bounded_array<current_type>::value
					&& std::is_floating_point<std::remove_all_extents<current_type>::type>::value))
			{
				for (std::size_t i = 0; i < std::extent<current_type>::value; ++i)
				{
					std::memcpy(&_buffer[_offset], &std::get<I>(_tuple)[i],
						sizeof(std::remove_all_extents<current_type>::type));

					_offset += sizeof(std::remove_all_extents<current_type>::type);
				}
			}

			if constexpr (I + 1 != std::tuple_size<std::tuple<T...>>::value)
				TupleToBuffer<I + 1>(_tuple, _buffer, _offset);
		}

#ifdef BINARY_LOVE_EXCEPTIONS
		// Purpose: Binary-Love buffer out of range exception
		class BinaryLoveOutOfRangeException : public std::exception
		{
		public:
			const char* what() const noexcept override {
				return "Trying to access out of range buffer.\n";
			}
		};
#endif // BINARY_LOVE_EXCEPTIONS
	}

	namespace Utility
	{
		// Purpose: Returns the size of the tuple in bytes
		template<class ...T>
		std::size_t GetTupleSize(const std::tuple<T...>& _tuple)
		{
			std::size_t bl_return = 0;
			Utility::details::TupleToSize(_tuple, bl_return);
			return bl_return;
		}

		namespace details
		{
			// Purpose: Overrides _size parameter with size of the tuple in bytes
			template<std::size_t I = 0, class ...T>
			void TupleToSize(const std::tuple<T...>& _tuple,
				std::size_t& _size)
			{
				// Make sure we start with the size of 0
				if constexpr (I == 0)
					_size = 0;

				_size += sizeof(std::tuple_element<I, std::tuple<T...>>::type);

				if constexpr (I + 1 != std::tuple_size<std::tuple<T...>>::value)
					TupleToSize<I + 1>(_tuple, _size);
			}
		}
	}

	template<
		class ...Types>
		std::tuple<Types...>
		Read(const binary_data_t& _buffer,
			buffer_offset_t& _offset)
	{
		// Tuple with values we are returning
		std::tuple<Types...> bl_return;
#ifdef BINARY_LOVE_EXCEPTIONS
		// Check if our buffer is big enough to read from
		if (Utility::GetTupleSize(bl_return) > _buffer.size() - _offset)
		{
			throw details::BinaryLoveOutOfRangeException();
			return bl_return;
		}
#endif // BINARY_LOVE_EXCEPTIONS
		// Fill our tuple with values
		details::BufferToTuple(bl_return, _buffer, _offset);
		return bl_return;
	}

	template<class ...Types>
	std::vector<std::tuple<Types...>>
		ReadStream(const binary_data_t& _buffer,
			buffer_offset_t& _offset,
			const buffer_size_t& _buffer_size)
	{
		std::vector<std::tuple<Types...>> bl_return;
#ifdef BINARY_LOVE_EXCEPTIONS
		// Check if our buffer is big enough to read from
		if (_buffer_size > _buffer.size() * sizeof(byte_t) - _offset)
		{
			throw details::BinaryLoveOutOfRangeException();
			return bl_return;
		}
#endif // BINARY_LOVE_EXCEPTIONS


		std::tuple<Types...> bl_tpl;
		for (std::size_t i = 0; i < _buffer_size / Utility::GetTupleSize(bl_tpl); ++i)
		{
			details::BufferToTuple(bl_tpl, _buffer, _offset);
			bl_return.push_back(bl_tpl);
		}

		return bl_return;
	}

	template<class ...Types>
	void
		Write(binary_data_t& _buffer,
			const std::tuple<Types...>& _tuple,
			buffer_offset_t& _offset)
	{
#ifdef BINARY_LOVE_EXCEPTIONS
		// Check if our buffer is big enough to read from
		if (Utility::GetTupleSize(_tuple) > _buffer.size() - _offset)
		{
			throw details::BinaryLoveOutOfRangeException();
			return;
		}
#endif // BINARY_LOVE_EXCEPTIONS

		details::TupleToBuffer(_tuple, _buffer, _offset);
	}

	template<class ...Types>
	void
		WriteStream(binary_data_t& _buffer,
			const std::vector<std::tuple<Types...>>& _tuple_buffer,
			buffer_offset_t& _offset,
			const buffer_size_t& _buffer_size)
	{
#ifdef BINARY_LOVE_EXCEPTIONS
		// Check if our buffer is big enough to read from
		if (_buffer_size > _buffer.size() * sizeof(byte_t) - _offset)
		{
			throw details::BinaryLoveOutOfRangeException();
			return;
		}
#endif // BINARY_LOVE_EXCEPTIONS
		for (std::size_t i = 0; i < _tuple_buffer.size(); ++i)
		{
			details::TupleToBuffer(_tuple_buffer[i], _buffer, _offset);
		}
	}
}

#endif
