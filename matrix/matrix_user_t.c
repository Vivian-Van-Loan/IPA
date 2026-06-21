#include "matrix_user_t.h"

void matrix_user_destroy(matrix_user_t* user) {
    free(user->id);
    free(user->display_name);
    free(user->avatar_url);
    memset(user, 0, sizeof(*user));
}
