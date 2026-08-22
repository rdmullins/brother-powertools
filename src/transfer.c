#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "transfer.h"

#define SERIAL_DEVICE "/dev/ttyUSB0"
#define TRANSFER_DELAY 5

int split_file(const char *input_filename,
               const char *output_prefix,
               size_t max_size)
{
    FILE *input;
    FILE *output;
    char output_filename[512];
    char line[4096];
    size_t part_size = 0;
    int part_number = 1;

    if (max_size == 0) {
        fprintf(stderr, "Maximum part size must be greater than zero.\n");
        return -1;
    }

    input = fopen(input_filename, "r");

    if (input == NULL) {
        perror("Unable to open input file");
        return -1;
    }

    output = NULL;

    while (fgets(line, sizeof(line), input) != NULL) {
        size_t line_size = strlen(line);

        /*
         * If this line would exceed the maximum and we already
         * have content in the current part, start a new part.
         *
         * This means we prefer breaking at line/paragraph
         * boundaries rather than cutting text in half.
         */
        if (output != NULL &&
            part_size > 0 &&
            part_size + line_size > max_size) {

            fclose(output);
            output = NULL;

            part_number++;
            part_size = 0;
        }

        /*
         * If the line itself is larger than the requested
         * maximum, we have to split it. This is our last-resort
         * boundary.
         */
        if (line_size > max_size) {
            size_t offset = 0;

            while (offset < line_size) {
                size_t remaining = line_size - offset;
                size_t amount = remaining < max_size
                              ? remaining
                              : max_size;

                snprintf(output_filename,
                         sizeof(output_filename),
                         "%s_%02d.txt",
                         output_prefix,
                         part_number);

                output = fopen(output_filename, "w");

                if (output == NULL) {
                    perror("Unable to create output file");
                    fclose(input);
                    return -1;
                }

                fwrite(line + offset, 1, amount, output);
                fclose(output);

                output = NULL;
                part_number++;
                offset += amount;
            }

            part_size = 0;
            continue;
        }

        if (output == NULL) {
            snprintf(output_filename,
                     sizeof(output_filename),
                     "%s_%02d.txt",
                     output_prefix,
                     part_number);

            output = fopen(output_filename, "w");

            if (output == NULL) {
                perror("Unable to create output file");
                fclose(input);
                return -1;
            }
        }

        fwrite(line, 1, line_size, output);
        part_size += line_size;
    }

    if (output != NULL) {
        fclose(output);
    }

    fclose(input);

    return part_number;
}

int send_transfer_part(const char *filename,
                              int part_number,
                              int total_parts)
{
    printf("\n");
    printf("+---------------------------------------------+\n");
    printf("|              Transfer Part                 |\n");
    printf("+---------------------------------------------+\n");
    printf("| Part %d of %d                                |\n",
           part_number, total_parts);
    printf("+---------------------------------------------+\n");
    printf("\n");

    printf("\n");

    if (send_ascii_file(filename) != 0) {
        printf("\nTransfer failed.\n");
        return -1;
    }

    printf("\n");
    printf("Part %d of %d has been sent.\n", part_number, total_parts);
    printf("\n");
    printf("Save this file on the Brother before continuing.\n");

    return 0;
}

int send_ascii_file(const char *filename)
{
    int fd;
    FILE *input;
    struct termios old_termios;
    struct termios serial_termios;
    char buffer[4096];
    size_t bytes_read;

    input = fopen(filename, "rb");
    if (input == NULL) {
        fprintf(stderr, "Unable to open '%s': %s\n",
                filename, strerror(errno));
        return -1;
    }

    printf("\n");
    printf("Prepare the Brother to receive an ASCII file:\n");
    printf("\n");
    printf("  Communications\n");
    printf("  -> Receive ASCII File\n");
    printf("  -> No Protocol\n");
    printf("\n");
    printf("Enter the filename on the Brother.\n");
    printf("When the Brother is ready to receive, press ENTER here.\n");

    getchar();

    /*
     * From this point until the Brother has had time to finish
     * receiving, PowerTools must send NO user-interface text.
     */

    for (int i = TRANSFER_DELAY; i > 0; i--) {
        sleep(1);
    }

    fd = open(SERIAL_DEVICE, O_WRONLY | O_NOCTTY);
    if (fd == -1) {
        fprintf(stderr, "Unable to open %s: %s\n",
                SERIAL_DEVICE, strerror(errno));
        fclose(input);
        return -1;
    }

    if (tcgetattr(fd, &old_termios) == -1) {
        fprintf(stderr, "Unable to read serial settings: %s\n",
                strerror(errno));
        close(fd);
        fclose(input);
        return -1;
    }

    serial_termios = old_termios;

    cfsetispeed(&serial_termios, B9600);
    cfsetospeed(&serial_termios, B9600);

    serial_termios.c_cflag &= ~PARENB;
    serial_termios.c_cflag &= ~CSTOPB;
    serial_termios.c_cflag &= ~CSIZE;
    serial_termios.c_cflag |= CS8;
    serial_termios.c_cflag |= CLOCAL | CREAD;

    serial_termios.c_iflag |= IXON;
    serial_termios.c_iflag &= ~(IXOFF | IXANY);

#ifdef CRTSCTS
    serial_termios.c_cflag &= ~CRTSCTS;
#endif

    serial_termios.c_lflag = 0;
    serial_termios.c_oflag = 0;

    if (tcsetattr(fd, TCSANOW, &serial_termios) == -1) {
        fprintf(stderr, "Unable to configure serial port: %s\n",
                strerror(errno));
        close(fd);
        fclose(input);
        return -1;
    }

    /*
     * Send the file, translating Unix LF line endings into the
     * carriage returns expected by the PowerNote.
     */
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), input)) > 0) {
        char output[4096];
        size_t output_length = 0;

        for (size_t i = 0; i < bytes_read; i++) {
            if (buffer[i] == '\n') {
                output[output_length++] = '\r';
            } else {
                output[output_length++] = buffer[i];
            }
        }

size_t total_written = 0;

while (total_written < output_length) {
    size_t chunk_size = output_length - total_written;

    if (chunk_size > 128) {
        chunk_size = 128;
    }

    ssize_t written = write(fd,
                            output + total_written,
                            chunk_size);

    if (written < 0) {
                fprintf(stderr, "Serial write failed: %s\n",
                        strerror(errno));

                tcsetattr(fd, TCSANOW, &old_termios);
                close(fd);
                fclose(input);
                return -1;
            }

            total_written += (size_t)written;
        }
    }

    if (ferror(input)) {
        fprintf(stderr, "Error reading '%s'.\n", filename);

        tcsetattr(fd, TCSANOW, &old_termios);
        close(fd);
        fclose(input);
        return -1;
    }

    /*
     * Make sure every byte has physically left the serial port.
     */
    tcdrain(fd);

    /*
     * Give the operator time to press FILE on the PowerNote and
     * finish the receive operation. Do not print anything during
     * this interval.
     */
    sleep(5);

    tcsetattr(fd, TCSANOW, &old_termios);

    close(fd);
    fclose(input);

    printf("\nTransfer complete.\n");
    return 0;
}

void transfer_menu(void)
{
    int choice;
    char filename[512];

    while (1) {
        printf(
            "\n"
            "+---------------------------------------------+\n"
            "|               Transfer Text                 |\n"
            "+---------------------------------------------+\n"
            "|  1. Receive file on PowerNote               |\n"
            "|  2. Back                                    |\n"
            "+---------------------------------------------+\n"
            "\n"
            "Enter your choice: "
        );

        if (scanf("%d", &choice) != 1) {
            printf("Invalid input.\n");

            int c;
            while ((c = getchar()) != '\n' && c != EOF) {
                /* discard invalid input */
            }

            continue;
        }

        getchar();

        switch (choice) {
            case 1:
                printf("\nRemote filename to send: ");

                if (fgets(filename, sizeof(filename), stdin) == NULL) {
                    printf("Input error.\n");
                    break;
                }

                filename[strcspn(filename, "\n")] = '\0';

                if (filename[0] == '\0') {
                    printf("No filename entered.\n");
                    break;
                }

                send_ascii_file(filename);
                break;

            case 2:
                return;

            default:
                printf("Invalid option. Please try again.\n");
                break;
        }
    }
}