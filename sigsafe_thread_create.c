#include <pthread.h>
#include <signal.h>

typedef struct {
	const struct sigaction *sa;
	const sigset_t *sm;
	void *(*routine)(void *);
	void *arg;
	const int *signals;
	int nsignals;
} sigsafe_arg_t;

static sigset_t thread_create_sigsafe__mask;

__attribute__((constructor))
static void blocking_mask_constructor__(void)
{
	sigfillset(&thread_create_sigsafe__mask);
	sigdelset(&thread_create_sigsafe__mask, SIGBUS);
	sigdelset(&thread_create_sigsafe__mask, SIGFPE);
	sigdelset(&thread_create_sigsafe__mask, SIGILL);
	sigdelset(&thread_create_sigsafe__mask, SIGSEGV);
}

/* helper */
static void *thread_create_sigsafe__impl(void *arg)
{
	sigsafe_arg_t *sarg = arg;

	// first set handlers, then unblock - order matters!
	for (int i = 0; i < sarg->nsignals; ++i) {
		sigaction(sarg->signals[i], sarg->sa, NULL);
	}
	pthread_sigmask(SIG_SETMASK, sarg->sm, NULL);

	return sarg->routine(sarg->arg);
}

/*!
 * \brief Spawn a new thread with custom signal handling parameters without
 * risk of signal related race conditions.
 *
 * @param thr pthread_t structure
 * @param attr pthread_attr_t structure
 * @param sa struct sigaction handler to be used within the thread
 * @param sm sigset_t signal mask to be used within the thread
 * @param signals an array of signal numbers to which sigaction should be applied
 * @param nsignals ARR_LEN(signals)
 * @param routine thread entry function
 * @param arg thread argument
 * @returns return value of pthread_create
 */
int thread_create_sigsafe(pthread_t *restrict thr,
			  const pthread_attr_t *restrict attr,
			  const struct sigaction *sa,
			  const sigset_t *sm,
			  const int *signals,
			  int nsignals,
			  void *(*routine)(void *),
			  void *restrict arg)
{
	// block all blockable signals
	sigset_t oldmask;
	pthread_sigmask(SIG_SETMASK, &thread_create_sigsafe__mask, &oldmask);

	// set desired sigmask and signal handler inside the thread through a wrapper function
	sigsafe_arg_t sarg = { sa, sm, routine, arg, signals, nsignals };
	int ret = pthread_create(thr, attr, thread_create_sigsafe__impl, &sarg);

	// restore original sigmask
	pthread_sigmask(SIG_SETMASK, &oldmask, NULL);

	return ret;
}
