#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

char *strrev(char *str, long long len)
{
    for (int i = 0; i < len / 2; i++)
    {
        char temp = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = temp;
    }
    return str;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        errno = 1;
        perror("Error: Insufficient number of arguments!");
        exit(1);
    }

    if (argc >= 3)
    {
        errno = 7;
        perror("Error");
        exit(1);
    }

    int path_len = strlen(argv[1]), j = 0, buf_size = 1e6, RW_count = 0;
    char read_buf[buf_size + 1], console_buf[100], dir[101] = "./Assignment/", filename[51];

    struct stat info;
    if (stat(argv[1], &info) < 0)
    {
        perror("Error");
        exit(1);
    };

    if ((info.st_mode & S_IFDIR) != 0)
    {
        errno = 21;
        perror("Error");
        exit(1);
    }

    if (!S_ISREG(info.st_mode))
    {
        sprintf(console_buf, "Not a regular file.");
        write(1, console_buf, strlen(console_buf));
        exit(1);
    }

    int f_read = open(argv[1], O_RDONLY);
    if (f_read < 0)
    {
        perror("Error");
        exit(1);
    }

    mkdir("./Assignment", 0700);

    char *last_slash = strrchr(argv[1], '/');

    for (int i = 0; i < 51; i++)
        filename[i] = '\0';

    if (last_slash != NULL)
    {
        int index = last_slash - argv[1];
        for (int i = index + 1; i < path_len; i++, j++)
            filename[j] = argv[1][i];
    }
    else
    {
        for (int i = 0; i < path_len; i++)
            filename[i] = argv[1][i];
    }
    int file_len = strlen(filename);
    j = 0;
    for (int i = strlen(dir); j < file_len; i++, j++)
        dir[i] = filename[j];

    int f_write = open(dir, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (f_write < 0)
    {
        perror("Error");
        exit(1);
    }

    long long left_to_copy = info.st_size, bytes_to_read;
    if (lseek(f_read, 0, SEEK_END) < 0)
    {
        perror("Error");
        exit(1);
    };

    while (left_to_copy > 0)
    {
        RW_count = 0;
        bytes_to_read = (left_to_copy > buf_size) ? buf_size : left_to_copy;
        if (lseek(f_read, -bytes_to_read, SEEK_CUR) < 0)
        {
            perror("Error");
            exit(1);
        };
        while (read(f_read, read_buf, bytes_to_read) < 0)
        {
            if (RW_count > 100)
            {
                sprintf(console_buf, "Could not read file. File maybe corrupt!");
                write(1, console_buf, strlen(console_buf));
                exit(1);
            }
            else
                RW_count++;
        };
        if (lseek(f_read, -bytes_to_read, SEEK_CUR) < 0)
        {
            perror("Error");
            exit(1);
        };
        if (write(f_write, strrev(read_buf, bytes_to_read), bytes_to_read) < 0)
        {
            perror("Error");
            exit(1);
        };
        left_to_copy -= bytes_to_read;
        sprintf(console_buf, "\rProgress: %.2lf %c", 100 * (double)(info.st_size - left_to_copy) / info.st_size, '%');
        if (write(1, console_buf, strlen(console_buf)) < 0)
        {
            perror("Error");
            exit(1);
        };
        fflush(stdout);
    }

    sprintf(console_buf, "\rProgress: 100.00 %c", '%');
    if (write(1, console_buf, strlen(console_buf)) < 0)
    {
        perror("Error");
        exit(1);
    };
    fflush(stdout);

    if (close(f_write) < 0)
    {
        sprintf(console_buf, "Could not close New file");
        if (write(1, console_buf, strlen(console_buf)) < 0)
        {
            perror("Error");
            exit(1);
        };
    };
    if (close(f_read) < 0)
    {
        sprintf(console_buf, "Could not close Old file");
        if (write(1, console_buf, strlen(console_buf)) < 0)
        {
            perror("Error");
            exit(1);
        };
    };
    return 0;
}