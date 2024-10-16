#pragma once
#include <stddef.h>

#define TEXT_REGION_BUFFER_SIZE 128

typedef struct _TextRegion
{

	char buffer[TEXT_REGION_BUFFER_SIZE];
	size_t pos;
	struct _TextRegion *prev;
	struct _TextRegion *next;

} TextRegion, *PTextRegion;

typedef struct _LineTextRegion
{

	struct _TextRegion r;
	struct _LineTextRegion *prev_line;
	struct _LineTextRegion *next_line;
	size_t line;

} LineTextRegion, *PLineTextRegion;

PTextRegion alloc_text_region();
PLineTextRegion alloc_line_text_region();