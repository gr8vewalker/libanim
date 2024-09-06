#include "provider.h"

void provider_free(_animProvider *ptr) { free(ptr->data); }
