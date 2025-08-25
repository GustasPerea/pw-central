#pragma once

void storage_init(void);
void storage_deinit(void);
float storage_load_hidrometer(void);
void storage_save_hidrometer(float liters);
void storage_reset_hidrometer(void);
