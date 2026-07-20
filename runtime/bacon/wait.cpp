//
// Created by XIaokang00010 on 2026/7/20.
//

#include "wait.h"

pthread_mutex_t g_safepoint_mutex;
pthread_cond_t g_safepoint_cond;