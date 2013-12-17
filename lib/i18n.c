#include "i18n.h"

void init_i18n()
{
    setlocale(LC_MESSAGES, "");
    textdomain("DDE");
}

