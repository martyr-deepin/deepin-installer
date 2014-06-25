#ifndef __ESP_H__
#define __ESP_H__

#include <parted/parted.h>

PedPartition* create_esp_by_split(PedDisk* disk, PedPartition* old);

#endif
