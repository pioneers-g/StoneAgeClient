/*
 * Stone Age Client - Config File Parser Header
 * Reverse engineered from sa_9061.exe
 */

#ifndef CONFIGFILE_H
#define CONFIGFILE_H

#include "types.h"

/* Constants */
#define MAX_CONFIG_ENTRIES    256

/* Config entry */
typedef struct {
    char* key;
    char* value;

} ConfigEntry;

/* Config file */
typedef struct {
    ConfigEntry entries[MAX_CONFIG_ENTRIES];
    int entry_count;
    int modified;

} ConfigFile;

/* Initialization */
int configfile_init(ConfigFile* config);
void configfile_shutdown(ConfigFile* config);

/* Load/Save */
int configfile_load(ConfigFile* config, const char* filename);
int configfile_save(ConfigFile* config, const char* filename);
int configfile_reload(ConfigFile* config, const char* filename);

/* String values */
int configfile_set(ConfigFile* config, const char* key, const char* value);
const char* configfile_get(ConfigFile* config, const char* key, const char* default_value);

/* Integer values */
int configfile_get_int(ConfigFile* config, const char* key, int default_value);
int configfile_set_int(ConfigFile* config, const char* key, int value);

/* Float values */
float configfile_get_float(ConfigFile* config, const char* key, float default_value);
int configfile_set_float(ConfigFile* config, const char* key, float value);

/* Boolean values */
int configfile_get_bool(ConfigFile* config, const char* key, int default_value);
int configfile_set_bool(ConfigFile* config, const char* key, int value);

/* Key management */
int configfile_has_key(ConfigFile* config, const char* key);
int configfile_remove(ConfigFile* config, const char* key);
void configfile_clear(ConfigFile* config);

/* Query */
int configfile_get_count(ConfigFile* config);
ConfigEntry* configfile_get_entry(ConfigFile* config, int index);
int configfile_is_modified(ConfigFile* config);

#endif /* CONFIGFILE_H */
