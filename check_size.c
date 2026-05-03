#include <stdio.h>
#include "include/types.h"
#include "include/item.h"

int main() {
    printf("InventoryItem size: %zu (expected 0x184 = %d)\n", sizeof(InventoryItem), 0x184);
    printf("CharacterInventory size: %zu (expected 0xb18 = %d)\n", sizeof(CharacterInventory), 0xb18);
    return 0;
}
