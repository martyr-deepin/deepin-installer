#ifndef _MIME_ACTIONS_H
#define _MIME_ACTIONS_H

gboolean activate_file (GFile* file, const char* content_type, 
                    gboolean is_executable, GFile* _file_arg);

#endif
