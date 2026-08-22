#ifndef TRANSFER_H
#define TRANSFER_H

#include <stddef.h>

int send_ascii_file(const char *filename);
void transfer_menu(void);

int split_file(const char *input_filename,
               const char *output_prefix,
               size_t max_size);

int send_transfer_part(const char *filename,
                       int part_number,
                       int total_parts);

#endif