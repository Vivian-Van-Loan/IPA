#ifndef MATRIX_CLIENT_LOGIN_H
#define MATRIX_CLIENT_LOGIN_H

typedef struct matrix_login_t {
    bool logged_in;
    char* user_id;
    char* homeserver;
    char* homeserver_resolved;
    char* device_id;
    char* access_token;
} matrix_login_t;

matrix_login_t matrix_login_pass(char const* homeserver_base, char const* user, char const* pass, char const* device_id_str);

#endif //MATRIX_CLIENT_LOGIN_H
