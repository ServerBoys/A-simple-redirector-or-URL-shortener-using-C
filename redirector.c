#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <microhttpd.h>
#include <mysql/mysql.h>

#define DB_HOST "localhost"
#define DB_USER "root"
#define DB_PASS "0000"
#define DB_NAME "redirector"

#define MAX_SLUG_LENGTH 100
#define MAX_URL_LENGTH 200
#define MAX_QUERY_LENGTH 150

void internalServerError(struct MHD_Response *response, struct MHD_Connection *connection) {
    response = MHD_create_response_from_buffer(21, (void *)"Internal Server Error", MHD_RESPMEM_PERSISTENT);
    MHD_queue_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
}

void redirect(struct MHD_Connection *connection, const char *slug) {
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;
    struct MHD_Response *response;

    char query[MAX_QUERY_LENGTH];
    char url[MAX_URL_LENGTH];

    conn = mysql_init(NULL);
    if (conn == NULL) {
        internalServerError(response, connection);
        goto d_response;
    }

    snprintf(query, MAX_QUERY_LENGTH, "SELECT url FROM urls WHERE slug='%s'", slug);
    if (mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, 0, NULL, 0) == NULL || mysql_query(conn, query)) {
        internalServerError(response, connection);
        goto d_conn;
    }

    res = mysql_store_result(conn);
    if (res == NULL) {
        internalServerError(response, connection);
        goto d_conn;
    }

    row = mysql_fetch_row(res);
    if (row == NULL) {
        response = MHD_create_response_from_buffer(9, (void *)"Not Found", MHD_RESPMEM_PERSISTENT);
        MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
        goto d_result;
    }

    
    snprintf(url, MAX_URL_LENGTH, "%s", row[0]);

    response = MHD_create_response_from_buffer(strlen(url), (void *)url, MHD_RESPMEM_PERSISTENT);
    MHD_add_response_header(response, "Location", url);
    MHD_queue_response(connection, MHD_HTTP_MOVED_PERMANENTLY, response);

d_result:
    mysql_free_result(res);
d_conn:
    mysql_close(conn);

d_response:
    MHD_destroy_response(response);
}

static int handle_request(void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version, const char *upload_data, size_t *upload_data_size, void **con_cls) {
    const char *slug = strstr(url, "/");
        if (slug != NULL) {
            slug++;  // Move past the '/' character

            // Check if slug is within max length
            if (strlen(slug) > MAX_SLUG_LENGTH) {
                struct MHD_Response *response = MHD_create_response_from_buffer(strlen("Bad Request"), (void *)"Bad Request", MHD_RESPMEM_PERSISTENT);
                MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, response);
                MHD_destroy_response(response);
                return MHD_YES;
            }

            redirect(connection, slug);
            return MHD_YES;
        }

    struct MHD_Response *response = MHD_create_response_from_buffer(strlen("Not Found"), (void *)"Not Found", MHD_RESPMEM_PERSISTENT);
    MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
    MHD_destroy_response(response);
    return MHD_YES;
}

int main() {
    struct MHD_Daemon *daemon;

    daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, 8080, NULL, NULL,
                          (MHD_AccessHandlerCallback)&handle_request, NULL, MHD_OPTION_END);
    if (daemon == NULL) {
        fprintf(stderr, "Error starting server\n");
        return 1;
    }

    getchar();  // Keep the server running until user presses Enter

    MHD_stop_daemon(daemon);
    return 0;
}

