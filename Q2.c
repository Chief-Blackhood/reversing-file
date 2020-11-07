#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

extern int error;

void strrev(char *str, long long len)
{
    for (int i = 0; i < len / 2; i++)
    {
        char temp = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = temp;
    }
}

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        errno = 1;
        perror("Error: Insufficient number of arguments!");
        exit(1);
    }

    if (argc > 4)
    {
        errno = 7;
        perror("Error");
        exit(1);
    }

    int buf_size = 1e6, RW_count = 0, flag_for_rev = 0, cond_new = 0, cond_old = 0, cond_dir = 0;
    char read_buf[buf_size + 1], check_buf[buf_size + 1], console_buf[100];

    struct stat info_dir, info_new, info_old;
    if (stat(argv[3], &info_dir) < 0 || (info_dir.st_mode & S_IFDIR) == 0)
    {
        sprintf(console_buf, "Directory is created: No\n");
        write(1, console_buf, strlen(console_buf));
        cond_dir = 1;
    }
    else
    {
        sprintf(console_buf, "Directory is created: Yes\n");
        write(1, console_buf, strlen(console_buf));
    };

    if (stat(argv[1], &info_new) < 0)
    {
        sprintf(console_buf, "Newfile doesn't exist.");
        write(1, console_buf, strlen(console_buf));
        cond_new = 1;
    }

    if (!S_ISREG(info_new.st_mode))
    {
        sprintf(console_buf, "Not a regular file.");
        write(1, console_buf, strlen(console_buf));
        cond_new = 1;
    }

    if (stat(argv[2], &info_old) < 0)
    {
        sprintf(console_buf, "Oldfile doesn't exist.");
        write(1, console_buf, strlen(console_buf));
        cond_old = 1;
    }

    if (!S_ISREG(info_old.st_mode))
    {
        sprintf(console_buf, "Not a regular file.");
        write(1, console_buf, strlen(console_buf));
        cond_old = 1;
    }

    int f_readNew = open(argv[1], O_RDONLY);

    if (cond_new == 0 && f_readNew < 0)
        perror("Program");

    int f_readOld = open(argv[2], O_RDONLY);

    if (cond_old == 0 && f_readOld < 0)
        perror("Program");

    for (int i = 0; i < buf_size + 1; i++)
    {
        read_buf[i] = '\0';
        check_buf[i] = '\0';
    }

    if (cond_new == 0 && cond_old == 0 && f_readNew >= 0 && f_readOld >= 0)
    {

        if (info_new.st_size == info_old.st_size)
        {
            long long left_to_copy = info_new.st_size, bytes_to_read;
            if (lseek(f_readOld, 0, SEEK_SET) < 0)
            {
                perror("Error");
                exit(1);
            }
            if (lseek(f_readNew, 0, SEEK_END) < 0)
            {
                perror("Error");
                exit(1);
            }

            while (left_to_copy > 0)
            {
                RW_count = 0;
                bytes_to_read = (left_to_copy > buf_size) ? buf_size : left_to_copy;
                if (lseek(f_readNew, -bytes_to_read, SEEK_CUR) < 0)
                {
                    perror("Error");
                    exit(1);
                };
                while (read(f_readNew, read_buf, bytes_to_read) < 0)
                {
                    if (RW_count > 100)
                    {
                        sprintf(console_buf, "Could not read file. File maybe corrupt!");
                        write(1, console_buf, strlen(console_buf));
                        exit(1);
                    }
                    else
                    {
                        RW_count++;
                    }
                };
                if (lseek(f_readNew, -bytes_to_read, SEEK_CUR) < 0)
                {
                    perror("Error");
                    exit(1);
                };
                while (read(f_readOld, check_buf, bytes_to_read) < 0)
                {
                    if (RW_count > 100)
                    {
                        sprintf(console_buf, "Could not read file. File maybe corrupt!");
                        write(1, console_buf, strlen(console_buf));
                        exit(1);
                    }
                    else
                    {
                        RW_count++;
                    }
                };
                strrev(read_buf, bytes_to_read);

                for (long long i = 0; i < bytes_to_read + 1; i++)
                {
                    if (read_buf[i] != check_buf[i])
                    {
                        flag_for_rev = 1;
                        break;
                    }
                }
                if (flag_for_rev == 1)
                    break;
                left_to_copy -= bytes_to_read;
                sprintf(console_buf, "\rProgress: %.2lf %c", 100 * (double)(info_new.st_size - left_to_copy) / info_new.st_size, '%');
                write(1, console_buf, strlen(console_buf));
                fflush(stdout);
            }

            sprintf(console_buf, "\rProgress: 100.00 %c\n", '%');
            write(1, console_buf, strlen(console_buf));
            fflush(stdout);

            if (flag_for_rev == 1)
            {
                sprintf(console_buf, "Whether file contents are reversed in newfile: No\n");
                write(1, console_buf, strlen(console_buf));
            }
            else
            {
                sprintf(console_buf, "Whether file contents are reversed in newfile: Yes\n");
                write(1, console_buf, strlen(console_buf));
            }
        }
        else
        {
            sprintf(console_buf, "Whether file contents are reversed in newfile: No\n");
            write(1, console_buf, strlen(console_buf));
        }
    }

    if (cond_new == 0)
    {
        sprintf(console_buf, "Permissions for newfile:\n\n");
        write(1, console_buf, strlen(console_buf));
        sprintf(console_buf, "User has read permissions on newfile: %s\n", (info_new.st_mode & S_IRUSR) == 0 ? "No" : "Yes");
        write(1, console_buf, strlen(console_buf));
        sprintf(console_buf, "User has write permission on newfile: %s\n", (info_new.st_mode & S_IWUSR) == 0 ? "No" : "Yes");
        write(1, console_buf, strlen(console_buf));
        sprintf(console_buf, "User has execute permissions on newfile: %s\n", (info_new.st_mode & S_IXUSR) == 0 ? "No" : "Yes");
        write(1, console_buf, strlen(console_buf));
        sprintf(console_buf, "Group has read permissions on newfile: %s\n", (info_new.st_mode & S_IRGRP) == 0 ? "No" : "Yes");
        write(1, console_buf, strlen(console_buf));
        sprintf(console_buf, "Group has write permission on newfile: %s\n", (info_new.st_mode & S_IWGRP) == 0 ? "No" : "Yes");
        write(1, console_buf, strlen(console_buf));
        sprintf(console_buf, "Group has execute permissions on newfile: %s\n", (info_new.st_mode & S_IXGRP) == 0 ? "No" : "Yes");
        write(1, console_buf, strlen(console_buf));
        sprintf(console_buf, "Others has read permissions on newfile: %s\n", (info_new.st_mode & S_IROTH) == 0 ? "No" : "Yes");
        write(1, console_buf, strlen(console_buf));
        sprintf(console_buf, "Others has write permission on newfile: %s\n", (info_new.st_mode & S_IWOTH) == 0 ? "No" : "Yes");
        write(1, console_buf, strlen(console_buf));
        sprintf(console_buf, "Others has execute permissions on newfile: %s\n\n", (info_new.st_mode & S_IXOTH) == 0 ? "No" : "Yes");
        write(1, console_buf, strlen(console_buf));

        sprintf(console_buf, "---------------------------------------------------------------\n\n");
        write(1, console_buf, strlen(console_buf));
    }

    if (cond_old == 0)
    {
        sprintf(console_buf, "Permissions for oldfile:\n\n");
        write(1, console_buf, strlen(console_buf));
        sprintf(console_buf, "User has read permissions on newfile: %s\n", (info_old.st_mode & S_IRUSR) == 0 ? "No" : "Yes");
        write(1, console_buf, strlen(console_buf));
        sprintf(console_buf, "User has write permission on newfile: %s\n", (info_old.st_mode & S_IWUSR) == 0 ? "No" : "Yes");
        write(1, console_buf, strlen(console_buf));
        sprintf(console_buf, "User has execute permissions on newfile: %s\n", (info_old.st_mode & S_IXUSR) == 0 ? "No" : "Yes");
        write(1, console_buf, strlen(console_buf));
        sprintf(console_buf, "Group has read permissions on newfile: %s\n", (info_old.st_mode & S_IRGRP) == 0 ? "No" : "Yes");
        write(1, console_buf, strlen(console_buf));
        sprintf(console_buf, "Group has write permission on newfile: %s\n", (info_old.st_mode & S_IWGRP) == 0 ? "No" : "Yes");
        write(1, console_buf, strlen(console_buf));
        sprintf(console_buf, "Group has execute permissions on newfile: %s\n", (info_old.st_mode & S_IXGRP) == 0 ? "No" : "Yes");
        write(1, console_buf, strlen(console_buf));
        sprintf(console_buf, "Others has read permissions on newfile: %s\n", (info_old.st_mode & S_IROTH) == 0 ? "No" : "Yes");
        write(1, console_buf, strlen(console_buf));
        sprintf(console_buf, "Others has write permission on newfile: %s\n", (info_old.st_mode & S_IWOTH) == 0 ? "No" : "Yes");
        write(1, console_buf, strlen(console_buf));
        sprintf(console_buf, "Others has execute permissions on newfile: %s\n\n", (info_old.st_mode & S_IXOTH) == 0 ? "No" : "Yes");
        write(1, console_buf, strlen(console_buf));

        sprintf(console_buf, "---------------------------------------------------------------\n\n");
        write(1, console_buf, strlen(console_buf));
    }

    if (cond_dir == 0)
    {
        sprintf(console_buf, "Permissions for directory:\n\n");
        write(1, console_buf, strlen(console_buf));
        sprintf(console_buf, "User has read permissions on newfile: %s\n", (info_dir.st_mode & S_IRUSR) == 0 ? "No" : "Yes");
        write(1, console_buf, strlen(console_buf));
        sprintf(console_buf, "User has write permission on newfile: %s\n", (info_dir.st_mode & S_IWUSR) == 0 ? "No" : "Yes");
        write(1, console_buf, strlen(console_buf));
        sprintf(console_buf, "User has execute permissions on newfile: %s\n", (info_dir.st_mode & S_IXUSR) == 0 ? "No" : "Yes");
        write(1, console_buf, strlen(console_buf));
        sprintf(console_buf, "Group has read permissions on newfile: %s\n", (info_dir.st_mode & S_IRGRP) == 0 ? "No" : "Yes");
        write(1, console_buf, strlen(console_buf));
        sprintf(console_buf, "Group has write permission on newfile: %s\n", (info_dir.st_mode & S_IWGRP) == 0 ? "No" : "Yes");
        write(1, console_buf, strlen(console_buf));
        sprintf(console_buf, "Group has execute permissions on newfile: %s\n", (info_dir.st_mode & S_IXGRP) == 0 ? "No" : "Yes");
        write(1, console_buf, strlen(console_buf));
        sprintf(console_buf, "Others has read permissions on newfile: %s\n", (info_dir.st_mode & S_IROTH) == 0 ? "No" : "Yes");
        write(1, console_buf, strlen(console_buf));
        sprintf(console_buf, "Others has write permission on newfile: %s\n", (info_dir.st_mode & S_IWOTH) == 0 ? "No" : "Yes");
        write(1, console_buf, strlen(console_buf));
        sprintf(console_buf, "Others has execute permissions on newfile: %s\n", (info_dir.st_mode & S_IXOTH) == 0 ? "No" : "Yes");
        write(1, console_buf, strlen(console_buf));
    }

    if (close(f_readNew) < 0)
    {
        sprintf(console_buf, "Could not close New file");
        if (write(1, console_buf, strlen(console_buf)) < 0)
        {
            perror("Error");
            exit(1);
        };
    };
    if (close(f_readOld) < 0)
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