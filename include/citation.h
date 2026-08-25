#ifndef CITATION_H
#define CITATION_H

#include <stddef.h>

#include "bibliography.h"

#define CITATION_MAX_LENGTH 2048

int citation_generate(const BibliographyRecord *record,
                      char *output,
                      size_t output_size);

#endif