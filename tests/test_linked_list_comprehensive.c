/*
 * Stone Age Client - Linked List Function Unit Tests
 * Tests for FUN_00444920 (insert after) and FUN_00444950 (append to tail)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Type definitions */
typedef unsigned int u32;
typedef int s32;

/* ========================================
 * Test Data and Constants
 * ======================================== */

/* Test node structure matching game's layout */
typedef struct TestNode {
    char data[36];          /* 0x00 - 0x23: padding/data */
    void* prev;             /* 0x24: prev pointer */
    void* next;             /* 0x28: next pointer */
} TestNode;

static int test_passed = 0;
static int test_failed = 0;

#define TEST(name) static void test_##name(void)
#define RUN_TEST(name) do { \
    printf("  Testing: %s... ", #name); \
    test_##name(); \
    printf("PASS\n"); \
    test_passed++; \
} while(0)

#define ASSERT(cond) do { \
    if (!(cond)) { \
        printf("FAIL at line %d: %s\n", __LINE__, #cond); \
        test_failed++; \
        return; \
    } \
} while(0)

/*
 * FUN_00444920 - Insert Node After
 */
static void FUN_00444920(void* existing, void* new_node) {
    TestNode* existing_node = (TestNode*)existing;
    TestNode* new_node_ptr = (TestNode*)new_node;

    if (existing == NULL || new_node == NULL) {
        return;
    }

    /* Set new node's prev to existing */
    new_node_ptr->prev = existing;

    /* Set new node's next to existing's next */
    new_node_ptr->next = existing_node->next;

    /* Update existing's next's prev to new node */
    if (existing_node->next != NULL) {
        TestNode* next_ptr = (TestNode*)existing_node->next;
        next_ptr->prev = new_node;
    }

    /* Set existing's next to new node */
    existing_node->next = new_node;
}

/*
 * FUN_00444950 - Append Node to Tail
 */
static void FUN_00444950(void* tail, void* new_node) {
    TestNode* tail_node = (TestNode*)tail;
    TestNode* new_node_ptr = (TestNode*)new_node;

    if (tail == NULL || new_node == NULL) {
        return;
    }

    /* Set new node's prev to tail */
    new_node_ptr->prev = tail;

    /* Set new node's next to tail's next (usually NULL) */
    new_node_ptr->next = tail_node->next;

    /* Update tail's next to new node */
    tail_node->next = new_node;
}

/* ========================================
 * Test Cases
 * ======================================== */

TEST(insert_after_null_existing) {
    TestNode new_node = {0};
    FUN_00444920(NULL, &new_node);
    ASSERT(new_node.prev == NULL);
    ASSERT(new_node.next == NULL);
}

TEST(insert_after_null_new) {
    TestNode existing = {0};
    FUN_00444920(&existing, NULL);
    ASSERT(existing.prev == NULL);
    ASSERT(existing.next == NULL);
}

TEST(insert_after_single_node) {
    TestNode node1 = {0};
    TestNode node2 = {0};

    FUN_00444920(&node1, &node2);

    ASSERT(node1.next == &node2);
    ASSERT(node2.prev == &node1);
    ASSERT(node2.next == NULL);
    ASSERT(node1.prev == NULL);
}

TEST(insert_after_middle) {
    TestNode node1 = {0};
    TestNode node2 = {0};
    TestNode node3 = {0};

    /* Create list: node1 -> node2 */
    FUN_00444920(&node1, &node2);

    /* Insert node3 after node1 (before node2) */
    FUN_00444920(&node1, &node3);

    ASSERT(node1.next == &node3);
    ASSERT(node3.prev == &node1);
    ASSERT(node3.next == &node2);
    ASSERT(node2.prev == &node3);
}

TEST(insert_after_chain) {
    TestNode nodes[5] = {0};

    /* Build chain: nodes[0] -> nodes[1] -> nodes[2] -> nodes[3] -> nodes[4] */
    for (int i = 0; i < 4; i++) {
        FUN_00444920(&nodes[i], &nodes[i+1]);
    }

    /* Verify chain */
    for (int i = 0; i < 5; i++) {
        if (i > 0) {
            ASSERT(nodes[i].prev == &nodes[i-1]);
        }
        if (i < 4) {
            ASSERT(nodes[i].next == &nodes[i+1]);
        }
    }
    ASSERT(nodes[0].prev == NULL);
    ASSERT(nodes[4].next == NULL);
}

TEST(append_null_tail) {
    TestNode new_node = {0};
    FUN_00444950(NULL, &new_node);
    ASSERT(new_node.prev == NULL);
    ASSERT(new_node.next == NULL);
}

TEST(append_null_new) {
    TestNode tail = {0};
    FUN_00444950(&tail, NULL);
    ASSERT(tail.prev == NULL);
    ASSERT(tail.next == NULL);
}

TEST(append_single_node) {
    TestNode node1 = {0};
    TestNode node2 = {0};

    FUN_00444950(&node1, &node2);

    ASSERT(node1.next == &node2);
    ASSERT(node2.prev == &node1);
    ASSERT(node2.next == NULL);
}

TEST(append_multiple_nodes) {
    TestNode nodes[5] = {0};

    /* Build list by appending */
    for (int i = 0; i < 4; i++) {
        FUN_00444950(&nodes[i], &nodes[i+1]);
    }

    /* Verify list */
    for (int i = 0; i < 5; i++) {
        if (i > 0) {
            ASSERT(nodes[i].prev == &nodes[i-1]);
        }
        if (i < 4) {
            ASSERT(nodes[i].next == &nodes[i+1]);
        }
    }
    ASSERT(nodes[0].prev == NULL);
    ASSERT(nodes[4].next == NULL);
}

TEST(insert_preserves_existing_next) {
    TestNode node1 = {0};
    TestNode node2 = {0};
    TestNode node3 = {0};

    /* Create list: node1 -> node2 */
    FUN_00444920(&node1, &node2);

    /* Insert node3 after node1, should preserve node1->node2 */
    FUN_00444920(&node1, &node3);

    /* node1 -> node3 -> node2 */
    ASSERT(node1.next == &node3);
    ASSERT(node3.next == &node2);
    ASSERT(node2.prev == &node3);
    ASSERT(node3.prev == &node1);
}

TEST(append_to_list_with_next) {
    TestNode node1 = {0};
    TestNode node2 = {0};
    TestNode node3 = {0};

    /* Create: node1 -> node2 */
    FUN_00444950(&node1, &node2);

    /* Append node3 to node1 (even though node1 is not tail) */
    FUN_00444950(&node1, &node3);

    /* Result: node1 -> node3 -> node2 (insertion in middle) */
    ASSERT(node1.next == &node3);
    ASSERT(node3.prev == &node1);
    ASSERT(node3.next == &node2);  /* node3.next is node2 (from node1.next) */
    ASSERT(node2.prev == &node1);  /* node2.prev unchanged (still node1) */
}

TEST(list_integrity_after_operations) {
    TestNode nodes[10] = {0};

    /* Build a list of 10 nodes */
    for (int i = 0; i < 9; i++) {
        FUN_00444920(&nodes[i], &nodes[i+1]);
    }

    /* Verify all links are correct */
    int count = 1;
    TestNode* current = &nodes[0];
    while (current->next != NULL) {
        current = (TestNode*)current->next;
        count++;
    }
    ASSERT(count == 10);
}

/* ========================================
 * Main Test Runner
 * ======================================== */

int main(void) {
    printf("=== Linked List Function Unit Tests ===\n\n");

    printf("Insert After - Null Tests:\n");
    RUN_TEST(insert_after_null_existing);
    RUN_TEST(insert_after_null_new);

    printf("\nInsert After - Basic Tests:\n");
    RUN_TEST(insert_after_single_node);
    RUN_TEST(insert_after_middle);
    RUN_TEST(insert_after_chain);

    printf("\nAppend - Null Tests:\n");
    RUN_TEST(append_null_tail);
    RUN_TEST(append_null_new);

    printf("\nAppend - Basic Tests:\n");
    RUN_TEST(append_single_node);
    RUN_TEST(append_multiple_nodes);

    printf("\nComplex Operation Tests:\n");
    RUN_TEST(insert_preserves_existing_next);
    RUN_TEST(append_to_list_with_next);
    RUN_TEST(list_integrity_after_operations);

    printf("\n=== Results ===\n");
    printf("Tests run: %d\n", test_passed + test_failed);
    printf("Tests passed: %d\n", test_passed);
    printf("Tests failed: %d\n", test_failed);

    return test_failed > 0 ? 1 : 0;
}
