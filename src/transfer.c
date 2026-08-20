#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "transfer.h"

#define SERIAL_DEVICE "/dev/ttyUSB0"
#define TRANSFER_DELAY 5

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

    serial_termios.c_iflag &= ~(IXON | IXOFF | IXANY);

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
            ssize_t written = write(fd,
                                    output + total_written,
                                    output_length - total_written);

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