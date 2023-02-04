/*
CRC-6 used in Analog Devices AD4134
Copyright (c) 2023, SAGE team s.r.o., Samuel Kupka

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

Function calc_crc6_0x67_direct based on:
Copyright: Levent Ozturk crc@leventozturk.com
https://leventozturk.com/engineering/crc/

poly = 1100111 (0x67) (x6 + x5 + x2 + x + 1)
seed = 100101 (0x25)

These functions take 3 bytes (24 bit) inputs and return boolean value. Modify them to fit your needs.

To compiles, use parameters (for example):
gcc -std=c11 -Wall -Wextra -Wshadow -Werror crcad.c -o crcad
*/

#include <stdio.h>
#include <inttypes.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

const uint8_t known_good[][3] = {
{0b11111101, 0b01000010, 0b11011001},
{0b11111101, 0b01101111, 0b11011100},
{0b11111111, 0b11111101, 0b11011010},
{0b11111101, 0b01011101, 0b11000000},
{0b11111101, 0b10000110, 0b11100000},
{0,0,0}
};

const uint8_t known_bad[][3] = {
{0b11111101, 0b01000000, 0b11011001},
{0b11111101, 0b01101101, 0b11011100},
{0b11111111, 0b11111111, 0b11011010},
{0b11111001, 0b01011101, 0b11000000},
{0b11111101, 0b10000110, 0b11100010},
{0,0,0}
};

static uint8_t crc6_0x67_table[256];

void generate_crc6_0x67_table (void)
{
    const uint16_t poly = 0x67;
    int i, j;
    uint16_t val;

    for (i = 0; i < 256; ++i) {
		val = i;
        for (j = 0; j < 8; ++j) {
            val = (val & 0x80) ? ((val << 1) ^ (poly << 2)) : (val << 1);
        }
		crc6_0x67_table[i] = (val >> 2) & 0x3f;
    }
}

bool calc_crc6_0x67_table (const uint8_t data[3])
{
	uint8_t crc = 0x25;
	crc = crc6_0x67_table[(crc << 2) ^ data[0]];
	crc = crc6_0x67_table[(crc << 2) ^ data[1]];
	crc = crc6_0x67_table[(crc << 2) ^ data[2]];
	return (0 == crc);
}

bool calc_crc6_0x67_direct (const uint8_t data[3])
{
	bool c[7] = {true, false, true, false, false, true, false};
	bool ch[7];
	bool d[6];
	int i;

	for (i = 0; i < 4; ++i) {
		switch (i) {
			case 0:
				d[0] = (data[0] >> 7) & 1;
				d[1] = (data[0] >> 6) & 1;
				d[2] = (data[0] >> 5) & 1;
				d[3] = (data[0] >> 4) & 1;
				d[4] = (data[0] >> 3) & 1;
				d[5] = (data[0] >> 2) & 1;
				break;

			case 1:
				d[0] = (data[0] >> 1) & 1;
				d[1] = (data[0] >> 0) & 1;
				d[2] = (data[1] >> 7) & 1;
				d[3] = (data[1] >> 6) & 1;
				d[4] = (data[1] >> 5) & 1;
				d[5] = (data[1] >> 4) & 1;
				break;

			case 2:
				d[0] = (data[1] >> 3) & 1;
				d[1] = (data[1] >> 2) & 1;
				d[2] = (data[1] >> 1) & 1;
				d[3] = (data[1] >> 0) & 1;
				d[4] = (data[2] >> 7) & 1;
				d[5] = (data[2] >> 6) & 1;
				break;

			case 3:
				d[0] = (data[2] >> 5) & 1;
				d[1] = (data[2] >> 4) & 1;
				d[2] = (data[2] >> 3) & 1;
				d[3] = (data[2] >> 2) & 1;
				d[4] = (data[2] >> 1) & 1;
				d[5] = (data[2] >> 0) & 1;
				break;
		}

		memcpy (ch, c, sizeof (c));

		c[6] = ch[5];
		c[5] = ch[0] ^ ch[1] ^ ch[2] ^ ch[5] ^ d[0] ^ d[3] ^ d[4] ^ d[5];
		c[4] = ch[2] ^ ch[4] ^ ch[5] ^  d[0] ^ d[1] ^ d[3];
		c[3] = ch[1] ^ ch[3] ^ ch[4] ^  d[1] ^ d[2] ^ d[4];
		c[2] = ch[0] ^ ch[2] ^ ch[3] ^ ch[5] ^ d[0] ^ d[2] ^ d[3] ^ d[5];
		c[1] = ch[0] ^ ch[4] ^  d[1] ^  d[5];
		c[0] = ch[0] ^ ch[1] ^ ch[2] ^ ch[3] ^ d[2] ^ d[3] ^ d[4] ^ d[5];
	}

	for (i = 0; i < 6; ++i) {
		if (c[i]) return false;
	}

	return true;
}

int main (void)
{
	uint32_t tests_passed = 0;
	uint32_t tests_failed = 0;
	uint8_t data[3];

	generate_crc6_0x67_table ();

	/* Testing known good vectors */
	for (size_t i = 0; known_good[i][0]; ++i) {
		true == calc_crc6_0x67_direct (known_good[i]) ? ++tests_passed : ++tests_failed;
		true == calc_crc6_0x67_table (known_good[i]) ? ++tests_passed : ++tests_failed;
	}

	/* Testing known bad vectors */
	for (size_t i = 0; known_bad[i][0]; ++i) {
		false == calc_crc6_0x67_direct (known_bad[i]) ? ++tests_passed : ++tests_failed;
		false == calc_crc6_0x67_table (known_bad[i]) ? ++tests_passed : ++tests_failed;
	}

	/* Testing random vectors and comparing results from both functions */
	srand (42);
	for (size_t i = 0; i < 100000; ++i) {
		data[0] = rand () & 0xff;
		data[1] = rand () & 0xff;
		data[2] = rand () & 0xff;
		calc_crc6_0x67_table (data) == calc_crc6_0x67_direct (data) ? ++tests_passed : ++tests_failed;
	}

	printf ("Passed: %" PRIu32 "\n", tests_passed);
	printf ("Failed: %" PRIu32 "\n", tests_failed);

	return 0;
}
