#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define DEVICE_PATH "/dev/my_device"
#define IOCTL_SET_VALUE _IOR('k', 0, int)
#define IOCTL_GET_VALUE _IOR('k', 1, int)
#define IOCTL_SHOW_VALUE _IOR('k', 2, int)

int main()
{
    int fd;
    int value;

    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0)
    {
        perror("Failed to open the device");
        return EXIT_FAILURE;
    }

    // Set value using ioctl
    value = 100;
    if (ioctl(fd, IOCTL_SET_VALUE, &value) < 0)
    {
        perror("IOCTL SET failed");
        close(fd);
        return EXIT_FAILURE;
    }

    // Get value using ioctl
    if (ioctl(fd, IOCTL_GET_VALUE, &value) < 0)
    {
        perror("IOCTL GET failed");
        close(fd);
        return EXIT_FAILURE;
    }

    printf("Value retrieved from the device: %d\n", value);


    for (unsigned int i = 0; i < 10; i++)
    {
        if (ioctl(fd, IOCTL_SHOW_VALUE, &value) < 0)
        {
            perror("IOCTL GET failed");
            close(fd);
            return EXIT_FAILURE;
        }
    }

    close(fd);
    return EXIT_SUCCESS;
}