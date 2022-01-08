// From: https://docs.oracle.com/cd/E23824_01/html/819-2145/pam-1.html
//	 https://docs.oracle.com/cd/E23824_01/html/819-2145/emrbk.html#scrolltoc

// link with -lpam

// To add a test user:
//   useradd --no-create-home --user-group --shell /sbin/nologin -K UID_MIN=5000 ka_test
//   	might add --passwd $(crypt passwd)
//   passwd ka_test (set to ka_test_ka)
//
//   usermod --expiredate $(date +%Y-%m-%d" --date="yesterday" ka_test
//   usermod --inactive n ka_test


#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <errno.h>
#include <security/pam_appl.h>
#include <string.h>
#include <malloc.h>


static char *passwd;

/* Service modules do not clean up responses if an error is returned.
 * Free responses here.
 */
static void
free_resp(int num_msg, struct pam_response *pr)
{
    int i;
    struct pam_response *r = pr;

    if (pr == NULL)
        return;

    for (i = 0; i < num_msg; i++, r++) {
        if (r->resp) {
            /* clear before freeing -- may be a password */
            bzero(r->resp, strlen(r->resp));
            free(r->resp);
            r->resp = NULL;
        }
    }
    free(pr);
}

int
pam_tty_conv(int num_msg, const struct pam_message **mess,
    struct pam_response **resp, void *my_data)
{
    const struct pam_message *m = *mess;
    struct pam_response *r;
    int i;

    if (num_msg <= 0 || num_msg >= PAM_MAX_NUM_MSG) {
        (void) fprintf(stderr, "bad number of messages %d "
            "<= 0 || >= %d\n",
            num_msg, PAM_MAX_NUM_MSG);
        *resp = NULL;
        return (PAM_CONV_ERR);
    }
    if ((*resp = r = calloc(num_msg,
        sizeof (struct pam_response))) == NULL)
        return (PAM_BUF_ERR);

errno = 0; /* don't propogate possible EINTR */

    /* Loop through messages */
    for (i = 0; i < num_msg; i++) {
        /* bad message from service module */
        if (m->msg == NULL) {
            (void) fprintf(stderr, "message[%d]: %d/NULL\n",
                i, m->msg_style);
            goto err;
        }

        /*
         * fix up final newline:
         *     removed for prompts
         *     added back for messages
         */
#ifdef BOLLOCKS
        if (m->msg[strlen(m->msg)] == '\n')
            m->msg[strlen(m->msg)] = '\0';
#endif

        r->resp = NULL;
        r->resp_retcode = 0;
        switch (m->msg_style) {

        case PAM_PROMPT_ECHO_OFF:
printf("PAM_PROMPT_ECHO_OFF %s\n", m->msg);
            /*FALLTHROUGH*/

        case PAM_PROMPT_ECHO_ON:
printf("PAM_PROMPT_ECHO_ON %s\n", m->msg);
            (void) fputs(m->msg, stdout);

            r->resp = strdup(passwd);
            break;

        case PAM_ERROR_MSG:
printf("PAM_ERROR_MSG %s\n", m->msg);
            (void) fputs(m->msg, stderr);
            (void) fputc('\n', stderr);
            break;

        case PAM_TEXT_INFO:
printf("PAM_TEXT_INFO %s\n", m->msg);
            (void) fputs(m->msg, stdout);
            (void) fputc('\n', stdout);
            break;

        default:
            (void) fprintf(stderr, "message[%d]: unknown type "
                "%d/val=\"%s\"\n",
                i, m->msg_style, m->msg);
            /* error, service module won't clean up */
            goto err;
        }
        if (errno == EINTR)
            goto err;

        /* next message/response */
        m++;
        r++;
    }
    return (PAM_SUCCESS);

err:
    free_resp(i, r);
    *resp = NULL;
    return (PAM_CONV_ERR);
}

int
main(int argc, char **argv)
{
    struct pam_conv conv = { pam_tty_conv, NULL };
    pam_handle_t *pamh;
    struct passwd *pw;
    int err;

    if ((pw = getpwuid(getuid())) == NULL) {
        (void) fprintf(stderr, "plock: Can't get username: %s\n",
            strerror(errno));
        exit(1);
    }
    
    /* Initialize PAM framework */
//    err = pam_start("plock", pw->pw_name, &conv, &pamh);
    err = pam_start("login", argv[1], &conv, &pamh);
    if (err != PAM_SUCCESS) {
        (void) fprintf(stderr, "plock: pam_start failed: %s\n",
            pam_strerror(pamh, err));
        exit(1);
    }

    /* Authenticate user in order to unlock screen */
passwd = argv[2];
printf("Calling pam_authenticate\n");fflush(stdout);
    do {
//        (void) fprintf(stderr, "Terminal locked for %s. ", pw->pw_name);
        err = pam_authenticate(pamh, 0);
        if (err == PAM_USER_UNKNOWN) {
printf("PAM_USER_UNKNOWN returned\n");
//            logout();
            break;
        } else if (err != PAM_SUCCESS) {
            (void) fprintf(stderr, "Invalid password.\n");
        }
    } while (err != PAM_SUCCESS);
 printf("Returned from pam_authenticate\n"); fflush(stdout);

    /* Make sure account and password are still valid */
    switch (err = pam_acct_mgmt(pamh, PAM_SILENT | PAM_DISALLOW_NULL_AUTHTOK)) {
    case PAM_SUCCESS:
	    printf("pam_acct_mgmt success\n");
        break;
    case PAM_USER_UNKNOWN:
	    printf("pam_acct_mgmt user unknown\n");
	    break;
    case PAM_ACCT_EXPIRED:
        /* User not allowed in anymore */
	    printf("pam_acct_mgmt account expired\n");
//        logout();
        break;
    case PAM_NEW_AUTHTOK_REQD:
	    printf("pam_acct_mgmt new auth tok required\n");
        /* The user's password has expired. Get a new one */
#if 0
        do {
            err = pam_chauthtok(pamh, 0);
        } while (err == PAM_AUTHTOK_ERR);
        if (err != PAM_SUCCESS)
            logout();
#endif
        break;
    case PAM_AUTH_ERR:
	    printf("pam_acct_mgmt authentication failed\n");
	    break;
    case PAM_PERM_DENIED:
	    printf("pam_acct_mgmt permission denied\n");
	    break;
    default:
	    printf("pam_acct_mgmt reason unknown %d\n", err);
	    break;
//        logout();
    }

#if 0
if (pam_setcred(pamh, PAM_REFRESH_CRED) != PAM_SUCCESS){
    logout();
}
#endif

    (void) pam_end(pamh, err);

    printf("I have got to the end\n");
    return(0);
}
