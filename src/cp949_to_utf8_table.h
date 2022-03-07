#pragma once
#ifndef __CP949_TO_UTF8_TABLE_H__
#define __CP949_TO_UTF8_TABLE_H__

#include <stdint.h>

struct CP949Data
{
    uint16_t cp949;
    uint32_t utf8;
};


extern CP949Data cp949_to_utf8_table[17304];

#endif