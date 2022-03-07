/*
* I assume that this program will be used in a little-endian architecture.
*/

#include <iostream>
#include <unordered_map>
#include <stdint.h>

#if defined(_MSC_VER)

#pragma warning(disable:4996)

#if !defined(UNICODE)
#define UNICODE
#endif

#if !defined(_UNICODE)
#define _UNICODE
#endif

#include <Windows.h>
#include <malloc.h>
#define MY_ALLOCA _alloca
#elif defined(__GNUC__) || defined(__clang__)
#include <alloca.h>
#define MY_ALLOCA alloca
#endif

#include "cp949_to_utf8_table.h"

std::unordered_map<uint16_t, uint32_t> map;

int32_t process_cp949_buffer(uint8_t* target_buffer, uint8_t* source_buffer, int32_t source_buffer_len);
static inline FILE* my_file_open(const char* file_address);
static inline FILE* my_file_open_write(const char* file_address);
static inline void my_file_read(FILE* f, int32_t read_size, uint8_t* buffer);
static inline void my_file_write(FILE* f, int32_t write_size, uint8_t* buffer);
static inline int32_t my_str_cmp(const char* s1, const char* s2);

/*
* argv[0] : program name
* argv[1] : input file address
* argv[2] : output file address
* 
* file addresses should be in the utf-8 format
*/
int main(int argc, char* argv[])
{
	using namespace std;

	if (argc < 3)
	{
		printf("Wrong Input Count\n");
		return -1;
	}

	const bool should_overwrite = my_str_cmp(argv[1], argv[2]) == 0;
	
	int32_t table_count = sizeof(cp949_to_utf8_table) / sizeof(CP949Data);
	for (int32_t i = 0; i < table_count; ++i)
	{
		CP949Data d = cp949_to_utf8_table[i];
		map[d.cp949] = d.utf8;
	}

	FILE* source_file = NULL;
	FILE* target_file = NULL;

	source_file = my_file_open(argv[1]);
	if (source_file == NULL)
	{
		printf("Fail to open the source file : %s\n", argv[1]);
		return -1;
	}

	fseek(source_file, 0, SEEK_END);
	
	int32_t source_buffer_size = ftell(source_file);
	int32_t target_buffer_size = source_buffer_size << 1;

	fseek(source_file, 0, SEEK_SET);
	
	uint8_t* buffer = (uint8_t*)malloc(source_buffer_size + target_buffer_size);
	uint8_t* source_buffer = buffer;
	uint8_t* target_buffer = buffer + source_buffer_size;
	
	my_file_read(source_file, source_buffer_size, source_buffer);

	if (should_overwrite == true)
	{
		fclose(source_file);
		source_file = NULL;
	}

	target_file = my_file_open_write(argv[2]);
	if (target_file == NULL)
	{
		printf("Fail to open the target file : %s\n", argv[2]);
		return -1;
	}

	int32_t target_processed_buffer_size = process_cp949_buffer(target_buffer, source_buffer, source_buffer_size);
		
	my_file_write(target_file, target_processed_buffer_size, target_buffer);

	free(buffer);

	if (target_file != NULL)
	{
		fclose(target_file);
		target_file = NULL;
	}

	if (source_file != NULL)
	{
		fclose(source_file);
		source_file = NULL;
	}
	
	return 0;
}

static inline int32_t get_utf8_byte_size(uint8_t first_value)
{
#if defined(_MSC_VER)
	// To start checking from the most significant bit, we should shift it
	int32_t shift_count = sizeof(unsigned long) * 8 - sizeof(char) * 8;

	unsigned long index = 500;
	unsigned long mask = first_value;

	/*
	* Explanation
	* mask = (mask << shift_count) statement is to move 8bit into msb
	* mask = ~mask is to convert 110xx xxxx into 001xx xxx to find the non-zero bit index with _BitScanReverse
	*/
	mask = (mask << shift_count);
	mask = ~mask;
	_BitScanReverse(&index, mask);	// The index means first zero from Most Significant Bit

	switch (index)
	{
	case 31: return 1;
	case 29: return 2;
	case 28: return 3;
	case 27: return 4;
	default: return -1;
	}
#elif defined(__GNUC__) || defined(__clang__)
	uint8_t is_msb_set = (first_value >> 7);
	if (is_msb_set)
	{
		// this indicates that we can detect how many redundant bits there are on the bits from msb
		int32_t shift_count = sizeof(int) * 8 - sizeof(uint8_t) * 8;
		int shifted_bit = ((int)first_value << shift_count);
		return __builtin_clrsb(shifted_bit) + 1;
	}
	else
	{
		return 1;
	}
#endif
}

static inline uint8_t get_byte_utf8(uint32_t sc, int32_t index)
{
	switch (index)
	{
	case 0: return (0x000000ff) & (sc);
	case 1: return (0x000000ff) & (sc >> 8);
	case 2: return (0x000000ff) & (sc >> 16);
	case 3: return (sc >> 24);
	default: return 0xff;
	}
}

// return : target_buffer size
int32_t process_cp949_buffer(uint8_t* target_buffer, uint8_t* source_buffer, int32_t source_buffer_len)
{
	int32_t source_i = 0;
	int32_t target_i = 0;
	uint32_t utf8_v = 0;
	int32_t utf8_size = 0;
	int32_t utf8_index = 0;
	uint16_t cp949_v = 0;
	uint8_t data = 0;

	while (source_i < source_buffer_len)
	{
		data = source_buffer[source_i];

		if (data >= 0x81 && data <= 0xFE && source_i + 1 < source_buffer_len)
		{
			// check the trail byte
			++source_i;

			uint8_t trail_data = source_buffer[source_i];
			bool is_cp949 = false;
			if (trail_data >= 0x41 && trail_data <= 0x5A)
			{
				is_cp949 = true;
			}
			else if (trail_data >= 0x61 && trail_data <= 0x7A)
			{
				is_cp949 = true;
			}
			else if (trail_data >= 0x81 && trail_data <= 0xFE)
			{
				is_cp949 = true;
			}

			if (is_cp949 == true)
			{
				cp949_v = (data << 8) | trail_data;

				utf8_v = map[cp949_v];

				utf8_size = get_utf8_byte_size((uint8_t)((0x000000ff) & (utf8_v)));

				for (utf8_index = 0; utf8_index < utf8_size; ++utf8_index)
				{
					target_buffer[target_i] = get_byte_utf8(utf8_v, utf8_index);
					++target_i;
				}
			}

			++source_i;
		}
		else
		{
			// only process ASCII-case here
			if (data <= 0x7F)
			{
				// crlf -> lf
				if (data == '\r' && source_i + 1 < source_buffer_len)
				{
					if (source_buffer[source_i + 1] == '\n')
					{
						++source_i; // eat '\r'
						data = source_buffer[source_i]; // feed new line
					}
				}

				target_buffer[target_i] = data;
				++target_i;
			}

			++source_i;
		}
	}

	return target_i;
}

// s is assumed as UTF-8
// t is assumed as wide char version
// target_byte_size is byte size of the t array
static inline int32_t str_widen(const char* s, wchar_t* t, int32_t target_byte_size)
{
#if defined(_WIN32) || defined(__WIN64)
	if (target_byte_size <= 0)
		return -1;

	int32_t s_len_with_null = (int32_t)strlen(s) + 1;

	int32_t target_size = MultiByteToWideChar(CP_UTF8, 0, s, s_len_with_null, NULL, 0);

	if (target_size > target_byte_size)
		return -1;

	return MultiByteToWideChar(CP_UTF8, 0, s, s_len_with_null, t, target_size);
#else 
	return -1;
#endif
}

static inline FILE* my_file_open(const char* file_address)
{
#if defined(_WIN32) || defined(__WIN64)
	int32_t path_len_with_null = ((int32_t)strlen(file_address)) + 1;
	wchar_t* temp_buffer = (wchar_t*)MY_ALLOCA(sizeof(wchar_t) * path_len_with_null);
	str_widen(file_address, temp_buffer, path_len_with_null);
	return _wfopen(temp_buffer, L"rb");
#else
	return fopen(file_address, "rb");
#endif
}

static inline FILE* my_file_open_write(const char* file_address)
{
#if defined(_WIN32) || defined(__WIN64)
	int32_t path_len_with_null = ((int32_t)strlen(file_address)) + 1;
	wchar_t* temp_buffer = (wchar_t*)MY_ALLOCA(sizeof(wchar_t) * path_len_with_null);
	str_widen(file_address, temp_buffer, path_len_with_null);
	return _wfopen(temp_buffer, L"wb");
#else
	return fopen(file_address, "wb");
#endif
}

static inline void my_file_read(FILE* f, int32_t read_size, uint8_t* buffer)
{
	int32_t residue_read_size = read_size;
	int32_t cur_read_size = 0;
	while (residue_read_size > 0)
	{
		cur_read_size = (int32_t)fread(buffer, 1, residue_read_size, f);

		residue_read_size -= cur_read_size;
		buffer += cur_read_size;
	}
}

static inline void my_file_write(FILE* f, int32_t write_size, uint8_t* buffer)
{
	int32_t cur_write_size = 0;
	int32_t residue_write_size = write_size;

	while (residue_write_size > 0)
	{
		cur_write_size = (int32_t)fwrite(buffer, 1, residue_write_size, f);

		residue_write_size -= cur_write_size;
		buffer += cur_write_size;
	}
}

static inline int32_t my_str_cmp(const char* s1, const char* s2)
{
	const unsigned char* c1 = (const unsigned char*)s1;
	const unsigned char* c2 = (const unsigned char*)s2;

	while (*c1 && *c1 == *c2)
	{
		++c1; ++c2;
	}

	return (*c1 - *c2);
}