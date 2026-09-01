#ifndef MATRIX_CLIENT_LOGIN_H
#define MATRIX_CLIENT_LOGIN_H

#include <pthread.h>

#include "../work_queue.h"

typedef struct matrix_login_t {
    pthread_mutex_t mutex;
    bool logged_in;
    char* user_id;
    char* homeserver;
    char* homeserver_resolved;
    char* device_id;
    char* access_token;
    char* refresh_token;
} matrix_login_t;

matrix_login_t make_login();

//notably bad idea to call these from outside the work queue
void matrix_login_from_save(matrix_login_t* login, bool force_refresh);
void matrix_login_pass(matrix_login_t* login, char const* homeserver_base, char const* user, char const* pass, char const* device_id_str);
void matrix_login_destroy(matrix_login_t* login); //requires locked mutex, destroying it, no need to unlock

struct matrix_login_wrapper_t {
    matrix_login_t* login;
    bool force_refresh;
};

triple$bool$alloc_void$size_t$ matrix_login_from_save_wrapper(ipa_queue_entry_t entry);
ssize_t queue_matrix_login_from_save(matrix_login_t* login, bool force_refresh);

#endif //MATRIX_CLIENT_LOGIN_H
