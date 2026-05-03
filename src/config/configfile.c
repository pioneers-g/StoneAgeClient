/*
 * Stone Age Client - Config File Parser Implementation
 * Reverse engineered from sa_9061.exe
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "types.h"
#include "configfile.h"
#include "logger.h"

/*
 * Initialize config file
 */
int configfile_init(ConfigFile* config) {
    if (!config) return 0;

    memset(config, 0, sizeof(ConfigFile));
    return 1;
}

/*
 * Shutdown config file
 */
void configfile_shutdown(ConfigFile* config) {
    int i;

    if (!config) return;

    for (i = 0; i < config->entry_count; i++) {
        if (config->entries[i].key) free(config->entries[i].key);
        if (config->entries[i].value) free(config->entries[i].value);
    }

    memset(config, 0, sizeof(ConfigFile));
}

/*
 * Load config from file
 */
int configfile_load(ConfigFile* config, const char* filename) {
    FILE* fp;
    char line[512];
    char* key;
    char* value;
    char* p;
    int line_num = 0;
    char current_section[64] = "";

    if (!config || !filename) return 0;

    fp = fopen(filename, "r");
    if (!fp) {
        LOG_DEBUG("Config file not found: %s", filename);
        return 0;
    }

    while (fgets(line, sizeof(line), fp)) {
        line_num++;

        /* Remove trailing whitespace */
        p = line + strlen(line) - 1;
        while (p >= line && (*p == '\n' || *p == '\r' || isspace(*p))) {
            *p-- = '\0';
        }

        /* Skip empty lines and comments */
        p = line;
        while (*p && isspace(*p)) p++;

        if (*p == '\0' || *p == '#' || *p == ';') continue;

        /* Check for section */
        if (*p == '[') {
            char* end = strchr(p, ']');
            if (end) {
                *end = '\0';
                strncpy(current_section, p + 1, sizeof(current_section) - 1);
            }
            continue;
        }

        /* Parse key=value */
        key = p;
        value = strchr(p, '=');

        if (value) {
            *value = '\0';
            value++;

            /* Trim key */
            p = key + strlen(key) - 1;
            while (p >= key && isspace(*p)) *p-- = '\0';

            /* Trim value */
            while (*value && isspace(*value)) value++;
            p = value + strlen(value) - 1;
            while (p >= value && isspace(*p)) *p-- = '\0';

            /* Remove quotes from value */
            if (*value == '"' || *value == '\'') {
                char quote = *value;
                value++;
                p = value + strlen(value) - 1;
                if (p >= value && *p == quote) *p = '\0';
            }

            /* Store entry */
            if (config->entry_count < MAX_CONFIG_ENTRIES) {
                char full_key[128];

                if (current_section[0]) {
                    snprintf(full_key, sizeof(full_key), "%s.%s", current_section, key);
                } else {
                    strncpy(full_key, key, sizeof(full_key) - 1);
                }

                configfile_set(config, full_key, value);
            }
        }
    }

    fclose(fp);
    config->modified = 0;

    LOG_DEBUG("Loaded config file: %s (%d entries)", filename, config->entry_count);
    return 1;
}

/*
 * Save config to file
 */
int configfile_save(ConfigFile* config, const char* filename) {
    FILE* fp;
    int i;
    char section[64] = "";
    char* dot;

    if (!config || !filename) return 0;

    fp = fopen(filename, "w");
    if (!fp) {
        LOG_ERROR("Failed to save config: %s", filename);
        return 0;
    }

    fprintf(fp, "# Config file\n\n");

    for (i = 0; i < config->entry_count; i++) {
        ConfigEntry* entry = &config->entries[i];

        /* Check for section change */
        dot = strchr(entry->key, '.');
        if (dot) {
            char new_section[64];
            int len = dot - entry->key;
            if (len >= sizeof(new_section)) len = sizeof(new_section) - 1;
            strncpy(new_section, entry->key, len);
            new_section[len] = '\0';

            if (strcmp(section, new_section) != 0) {
                strcpy(section, new_section);
                fprintf(fp, "\n[%s]\n", section);
            }

            fprintf(fp, "%s=%s\n", dot + 1, entry->value ? entry->value : "");
        } else {
            if (section[0]) {
                section[0] = '\0';
                fprintf(fp, "\n");
            }
            fprintf(fp, "%s=%s\n", entry->key, entry->value ? entry->value : "");
        }
    }

    fclose(fp);
    config->modified = 0;

    LOG_DEBUG("Saved config file: %s", filename);
    return 1;
}

/*
 * Set config value
 */
int configfile_set(ConfigFile* config, const char* key, const char* value) {
    ConfigEntry* entry;
    int i;

    if (!config || !key) return 0;

    /* Find existing entry */
    for (i = 0; i < config->entry_count; i++) {
        if (strcmp(config->entries[i].key, key) == 0) {
            if (config->entries[i].value) free(config->entries[i].value);
            config->entries[i].value = value ? strdup(value) : NULL;
            config->modified = 1;
            return 1;
        }
    }

    /* Add new entry */
    if (config->entry_count >= MAX_CONFIG_ENTRIES) return 0;

    entry = &config->entries[config->entry_count];
    entry->key = strdup(key);
    entry->value = value ? strdup(value) : NULL;

    config->entry_count++;
    config->modified = 1;

    return 1;
}

/*
 * Get config value
 */
const char* configfile_get(ConfigFile* config, const char* key, const char* default_value) {
    int i;

    if (!config || !key) return default_value;

    for (i = 0; i < config->entry_count; i++) {
        if (strcmp(config->entries[i].key, key) == 0) {
            return config->entries[i].value ? config->entries[i].value : default_value;
        }
    }

    return default_value;
}

/*
 * Get integer value
 */
int configfile_get_int(ConfigFile* config, const char* key, int default_value) {
    const char* value = configfile_get(config, key, NULL);

    if (value) {
        return atoi(value);
    }

    return default_value;
}

/*
 * Set integer value
 */
int configfile_set_int(ConfigFile* config, const char* key, int value) {
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%d", value);
    return configfile_set(config, key, buffer);
}

/*
 * Get float value
 */
float configfile_get_float(ConfigFile* config, const char* key, float default_value) {
    const char* value = configfile_get(config, key, NULL);

    if (value) {
        return (float)atof(value);
    }

    return default_value;
}

/*
 * Set float value
 */
int configfile_set_float(ConfigFile* config, const char* key, float value) {
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%f", value);
    return configfile_set(config, key, buffer);
}

/*
 * Get boolean value
 */
int configfile_get_bool(ConfigFile* config, const char* key, int default_value) {
    const char* value = configfile_get(config, key, NULL);

    if (value) {
        if (strcmp(value, "true") == 0 ||
            strcmp(value, "yes") == 0 ||
            strcmp(value, "1") == 0 ||
            strcmp(value, "on") == 0) {
            return 1;
        }

        if (strcmp(value, "false") == 0 ||
            strcmp(value, "no") == 0 ||
            strcmp(value, "0") == 0 ||
            strcmp(value, "off") == 0) {
            return 0;
        }
    }

    return default_value;
}

/*
 * Set boolean value
 */
int configfile_set_bool(ConfigFile* config, const char* key, int value) {
    return configfile_set(config, key, value ? "true" : "false");
}

/*
 * Check if key exists
 */
int configfile_has_key(ConfigFile* config, const char* key) {
    int i;

    if (!config || !key) return 0;

    for (i = 0; i < config->entry_count; i++) {
        if (strcmp(config->entries[i].key, key) == 0) {
            return 1;
        }
    }

    return 0;
}

/*
 * Remove key
 */
int configfile_remove(ConfigFile* config, const char* key) {
    int i, j;

    if (!config || !key) return 0;

    for (i = 0; i < config->entry_count; i++) {
        if (strcmp(config->entries[i].key, key) == 0) {
            if (config->entries[i].key) free(config->entries[i].key);
            if (config->entries[i].value) free(config->entries[i].value);

            for (j = i; j < config->entry_count - 1; j++) {
                config->entries[j] = config->entries[j + 1];
            }

            config->entry_count--;
            config->modified = 1;
            return 1;
        }
    }

    return 0;
}

/*
 * Clear all entries
 */
void configfile_clear(ConfigFile* config) {
    int i;

    if (!config) return;

    for (i = 0; i < config->entry_count; i++) {
        if (config->entries[i].key) free(config->entries[i].key);
        if (config->entries[i].value) free(config->entries[i].value);
    }

    config->entry_count = 0;
    config->modified = 1;
}

/*
 * Get entry count
 */
int configfile_get_count(ConfigFile* config) {
    return config ? config->entry_count : 0;
}

/*
 * Get entry by index
 */
ConfigEntry* configfile_get_entry(ConfigFile* config, int index) {
    if (!config || index < 0 || index >= config->entry_count) {
        return NULL;
    }

    return &config->entries[index];
}

/*
 * Check if modified
 */
int configfile_is_modified(ConfigFile* config) {
    return config ? config->modified : 0;
}

/*
 * Reload from file
 */
int configfile_reload(ConfigFile* config, const char* filename) {
    if (!config || !filename) return 0;

    configfile_clear(config);
    return configfile_load(config, filename);
}
