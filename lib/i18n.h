#ifndef __I18N_H__
#define __I18N_H__

#include <locale.h>
#include <libintl.h>

#define _(STRING) gettext(STRING)

void init_i18n();

#endif
