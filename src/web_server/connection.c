#include "connection.h"
#include "../utils/log.h"
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>

static ssize_t raw_write(connection_ctx_t *conn, const char *buf, size_t count);

ssize_t connection_write(void *ctx, const char *buf, size_t count) {
    connection_ctx_t *conn = (connection_ctx_t *)ctx;
    if (!conn) return -1;

    if (conn->simple_response && conn->header_end_matched < 4) {
        static const char END[] = "\r\n\r\n";
        size_t i = 0;
        while (i < count && conn->header_end_matched < 4) {
            if (buf[i] == END[conn->header_end_matched]) conn->header_end_matched++;
            else conn->header_end_matched = (buf[i] == '\r') ? 1 : 0;
            i++;
        }
        if (i == count) return (ssize_t)count;      // all of it was header
        ssize_t sent = raw_write(conn, buf + i, count - i);
        return sent > 0 ? (ssize_t)i + sent : sent;
    }
    return raw_write(conn, buf, count);
}

static ssize_t raw_write(connection_ctx_t *conn, const char *buf, size_t count) {

    if (conn->ssl) {
        int ret = SSL_write(conn->ssl, buf, (int)count);
        if (ret > 0) return (ssize_t)ret;

        int ssl_err = SSL_get_error(conn->ssl, ret);
        switch (ssl_err) {
            case SSL_ERROR_ZERO_RETURN:
                LOG_DEBUG("SSL_write: connection closed by peer");
                return 0;
            case SSL_ERROR_WANT_READ:
            case SSL_ERROR_WANT_WRITE:
                return -2;
            case SSL_ERROR_SYSCALL:
                if (errno == EPIPE || errno == ECONNRESET || ret == 0) {
                    LOG_WARN("SSL_write: client disconnected (errno=%d)", errno);
                    return 0;
                }
                LOG_ERROR("SSL_write: syscall error (%s)", strerror(errno));
                return -1;
            default:
                LOG_ERROR("SSL_write: error %d", ssl_err);
                return -1;
        }
    }

    ssize_t sent;
#ifdef MSG_NOSIGNAL
    sent = send(conn->client_socket, buf, count, MSG_NOSIGNAL);
#else
    sent = write(conn->client_socket, buf, count);
#endif
    if (sent > 0) return sent;
    if (sent == 0) return 0;
    if (errno == EPIPE || errno == ECONNRESET) {
        LOG_WARN("write: client disconnected (%s)", strerror(errno));
        return 0;
    }
    if (errno == EINTR) return -2;
    LOG_ERROR("write: error (%s)", strerror(errno));
    return -1;
}

void connection_close(void *ctx) {
    if (!ctx) return;
    connection_ctx_t *conn = (connection_ctx_t *)ctx;

    if (conn->ssl) {
        int s = SSL_shutdown(conn->ssl);
        if (s == 0) s = SSL_shutdown(conn->ssl);
        if (s < 0) {
            int e = SSL_get_error(conn->ssl, s);
            LOG_WARN("connection_close: SSL_shutdown error %d", e);
        }
        SSL_free(conn->ssl);
        conn->ssl = NULL;
    }

    if (conn->client_socket >= 0) {
        shutdown(conn->client_socket, SHUT_RDWR);
        close(conn->client_socket);
        conn->client_socket = -1;
    }
}

ssize_t plain_read(void *ctx, char *buf, size_t count) {
    connection_ctx_t *conn = (connection_ctx_t *)ctx;
    if (!conn) return -1;

    ssize_t ret = read(conn->client_socket, buf, count);
    if (ret > 0) return ret;
    if (ret == 0) { LOG_DEBUG("plain_read: EOF"); return 0; }
    if (errno == EINTR) return -2;
    if (errno == EPIPE || errno == ECONNRESET) { LOG_WARN("plain_read: disconnected"); return 0; }
    LOG_ERROR("plain_read: %s", strerror(errno));
    return -1;
}

ssize_t ssl_read(void *ctx, char *buf, size_t count) {
    connection_ctx_t *conn = (connection_ctx_t *)ctx;
    if (!conn || !conn->ssl) return -1;

    int n = SSL_read(conn->ssl, buf, (int)count);
    if (n > 0) return n;

    int err = SSL_get_error(conn->ssl, n);
    switch (err) {
        case SSL_ERROR_ZERO_RETURN:
            LOG_DEBUG("ssl_read: closed by peer");
            return 0;
        case SSL_ERROR_WANT_READ:
        case SSL_ERROR_WANT_WRITE:
            return -2;
        case SSL_ERROR_SYSCALL:
            if (errno == EINTR) return -2;
            if (errno == EPIPE || errno == ECONNRESET) { LOG_WARN("ssl_read: disconnected"); return 0; }
            LOG_ERROR("ssl_read: syscall error (%s)", strerror(errno));
            return -1;
        default:
            LOG_ERROR("ssl_read: SSL error %d", err);
            return -1;
    }
}
