#ifndef MATRIX_CLIENT_LOGIN_H
#define MATRIX_CLIENT_LOGIN_H

typedef struct matrix_login_t {
    bool logged_in;
    char* user_id;
    char* homeserver;
    char* homeserver_resolved;
    char* device_id;
    char* access_token;
    char* refresh_token;
} matrix_login_t;

void matrix_login_from_save(matrix_login_t* login);
void matrix_login_refresh(matrix_login_t* login, char const* homeserver_base, char const* user, char const* refresh_token);
void matrix_login_pass(matrix_login_t* login, char const* homeserver_base, char const* user, char const* pass, char const* device_id_str);
void matrix_login_destroy(matrix_login_t* login);

#endif //MATRIX_CLIENT_LOGIN_H
