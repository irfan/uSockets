/* This example, or test, is a moron test where the library is being hammered in all the possible ways randomly over time */

#include <libusockets.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int opened_connections, closed_connections, operations_done;
struct us_socket_context *http_context;
struct us_listen_socket *listen_socket;

void *long_buffer;
int long_length = 5 * 1024 * 1024;

void perform_random_operation(struct us_socket *s) {
    switch (rand() % 4) {
        case 0: {
            us_socket_close(s);
        }
        break;
        case 1: {
            us_socket_write(s, "short", 5, 0);
        }
        break;
        case 2: {
            us_socket_shutdown(s);
        }
        break;
        case 3: {
            us_socket_write(s, long_buffer, long_length, 0);
        }
        break;
    }
}

struct http_socket {

};

struct http_context {

};

void on_wakeup(struct us_loop *loop) {
    // test these also
}

void on_pre(struct us_loop *loop) {
    printf("PRE\n");
}

void on_post(struct us_loop *loop) {

}

void on_http_socket_writable(struct us_socket *s) {
    perform_random_operation(s);
}

void on_http_socket_close(struct us_socket *s) {
    closed_connections++;
    printf("Opened: %d\nClosed: %d\n\n", opened_connections, closed_connections);

    if (closed_connections == 10000) {
        us_listen_socket_close(listen_socket);
    } else {
        perform_random_operation(s);
    }
}

void on_http_socket_end(struct us_socket *s) {
    // we need to close on shutdown
    us_socket_close(s);
    perform_random_operation(s);
}

void on_http_socket_data(struct us_socket *s, char *data, int length) {
    perform_random_operation(s);
}

void on_http_socket_open(struct us_socket *s) {
    opened_connections++;
    printf("Opened: %d\nClosed: %d\n\n", opened_connections, closed_connections);

    if (opened_connections % 2 == 0 && opened_connections < 10000) {
        us_socket_context_connect(http_context, "localhost", 3000, 0, sizeof(struct http_socket));
    }

    perform_random_operation(s);
}

void on_http_socket_timeout(struct us_socket *s) {
    perform_random_operation(s);
}

// todo: a separate thread which hammers the wakeup which tries to mess up as much as possible
int main() {
    srand(time(0));
    long_buffer = calloc(long_length, 1);

    struct us_loop *loop = us_create_loop(1, on_wakeup, on_pre, on_post, 0);

    http_context = us_create_socket_context(loop, sizeof(struct http_context));

    us_socket_context_on_open(http_context, on_http_socket_open);
    us_socket_context_on_data(http_context, on_http_socket_data);
    us_socket_context_on_writable(http_context, on_http_socket_writable);
    us_socket_context_on_close(http_context, on_http_socket_close);
    us_socket_context_on_timeout(http_context, on_http_socket_timeout);
    us_socket_context_on_end(http_context, on_http_socket_end);

    listen_socket = us_socket_context_listen(http_context, 0, 3000, 0, sizeof(struct http_socket));

    us_socket_context_connect(http_context, "localhost", 3000, 0, sizeof(struct http_socket));

    if (listen_socket) {
        printf("Running hammer test\n");
        us_loop_run(loop);
    } else {
        printf("Cannot listen to port 3000!\n");
    }

    us_socket_context_free(http_context);
    us_loop_free(loop);
    free(long_buffer);
    printf("Done, shutting down now\n");
}
