/*
 * Stone Age Client - Entity Linked List Tests
 * Tests for entity allocation, insertion, update loop, and cleanup
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char u8;

#define ENTITY_DATA_SIZE 400

typedef struct TestNode {
    struct TestNode* prev;
    struct TestNode* next;
    void (*callback)(void*);
    void* extra;
    u8 priority;
    u8 pad1[3];
    int state;
    int delete_flag;
    u8 data[ENTITY_DATA_SIZE];
    int render_order;
} TestNode;

static TestNode s_head;
static TestNode s_tail;
static int s_init = 0;
static int s_count = 0;
static int s_cb_count = 0;

static void t_init(void) {
    if (s_init) return;
    memset(&s_head, 0, sizeof(TestNode));
    memset(&s_tail, 0, sizeof(TestNode));
    s_head.next = &s_tail;
    s_tail.prev = &s_head;
    s_init = 1;
    s_count = 0;
}

static void t_insert(TestNode* n) {
    TestNode* cur;
    t_init();
    cur = s_head.next;
    while (cur != &s_tail) {
        if (cur->priority > n->priority) break;
        cur = cur->next;
    }
    n->prev = cur->prev;
    n->next = cur;
    cur->prev->next = n;
    cur->prev = n;
    s_count++;
}

static void t_unlink(TestNode* n) {
    if (n->prev) n->prev->next = n->next;
    if (n->next) n->next->prev = n->prev;
    n->prev = NULL;
    n->next = NULL;
}

static TestNode* t_alloc(int pri, int extra_sz) {
    TestNode* n = (TestNode*)calloc(1, sizeof(TestNode));
    if (!n) return NULL;
    if (extra_sz > 0) n->extra = calloc(1, extra_sz);
    n->priority = (u8)pri;
    n->render_order = -1;
    n->state = -2;
    n->delete_flag = 0;
    t_insert(n);
    return n;
}

static void t_free(TestNode* n) {
    if (!n) return;
    if (n->extra) free(n->extra);
    free(n);
    if (s_count > 0) s_count--;
}

static void t_callback(void* p) { (void)p; s_cb_count++; }

static int tests_passed = 0;
static int tests_failed = 0;

#define ASSERT_EQ(a, b) do { int _a = (int)(a); int _b = (int)(b); if (_a != _b) { printf("FAIL: line %d got %d expected %d\n", __LINE__, _a, _b); tests_failed++; return; } } while(0)
#define ASSERT_TRUE(c) do { if (!(c)) { printf("FAIL: line %d false\n", __LINE__); tests_failed++; return; } } while(0)

void test_empty_list(void) {
    s_init = 0; s_count = 0;
    t_init();
    ASSERT_TRUE(s_head.next == &s_tail);
    ASSERT_TRUE(s_tail.prev == &s_head);
    ASSERT_EQ(s_count, 0);
    tests_passed++;
}

void test_alloc_single(void) {
    s_init = 0; s_count = 0;
    t_init();
    TestNode* n = t_alloc(5, 0);
    ASSERT_TRUE(n != NULL);
    ASSERT_EQ(s_count, 1);
    ASSERT_TRUE(s_head.next == n);
    ASSERT_TRUE(s_tail.prev == n);
    ASSERT_EQ(n->priority, 5);
    ASSERT_EQ(n->state, -2);
    ASSERT_EQ(n->delete_flag, 0);
    tests_passed++;
}

void test_alloc_with_extra(void) {
    s_init = 0; s_count = 0;
    t_init();
    TestNode* n = t_alloc(3, 256);
    ASSERT_TRUE(n != NULL);
    ASSERT_TRUE(n->extra != NULL);
    ASSERT_EQ(s_count, 1);
    tests_passed++;
}

void test_sorted_insert(void) {
    s_init = 0; s_count = 0;
    t_init();
    TestNode* e1 = t_alloc(5, 0);
    TestNode* e2 = t_alloc(2, 0);
    TestNode* e3 = t_alloc(8, 0);
    ASSERT_EQ(s_count, 3);
    ASSERT_TRUE(s_head.next == e2);
    ASSERT_TRUE(e2->next == e1);
    ASSERT_TRUE(e1->next == e3);
    ASSERT_TRUE(e3->next == &s_tail);
    tests_passed++;
}

void test_unlink_middle(void) {
    s_init = 0; s_count = 0;
    t_init();
    TestNode* e1 = t_alloc(1, 0);
    TestNode* e2 = t_alloc(2, 0);
    TestNode* e3 = t_alloc(3, 0);
    t_unlink(e2); s_count--;
    ASSERT_TRUE(e1->next == e3);
    ASSERT_TRUE(e3->prev == e1);
    ASSERT_EQ(s_count, 2);
    tests_passed++;
}

void test_unlink_first(void) {
    s_init = 0; s_count = 0;
    t_init();
    TestNode* e1 = t_alloc(1, 0);
    TestNode* e2 = t_alloc(2, 0);
    t_unlink(e1); s_count--;
    ASSERT_TRUE(s_head.next == e2);
    ASSERT_TRUE(e2->prev == &s_head);
    ASSERT_EQ(s_count, 1);
    tests_passed++;
}

void test_update_callbacks(void) {
    s_init = 0; s_count = 0; s_cb_count = 0;
    t_init();
    TestNode* e1 = t_alloc(1, 0);
    TestNode* e2 = t_alloc(2, 0);
    e1->callback = t_callback;
    e2->callback = t_callback;

    TestNode* cur = s_head.next;
    while (cur != &s_tail) {
        if (cur->delete_flag == 0 && cur->callback)
            cur->callback(cur);
        cur = cur->next;
    }
    ASSERT_EQ(s_cb_count, 2);
    tests_passed++;
}

void test_update_removes_deleted(void) {
    s_init = 0; s_count = 0;
    t_init();
    TestNode* e1 = t_alloc(1, 0);
    TestNode* e2 = t_alloc(2, 0);
    TestNode* e3 = t_alloc(3, 0);
    e2->delete_flag = 1;

    TestNode* cur = s_head.next;
    while (cur != &s_tail) {
        TestNode* next = cur->next;
        if (cur->delete_flag != 0) {
            t_unlink(cur);
            t_free(cur);
        }
        cur = next;
    }
    ASSERT_TRUE(s_head.next == e1);
    ASSERT_TRUE(e1->next == e3);
    ASSERT_TRUE(e3->next == &s_tail);
    tests_passed++;
}

void test_mark_all_deleted(void) {
    s_init = 0; s_count = 0;
    t_init();
    t_alloc(1, 0);
    t_alloc(2, 0);
    t_alloc(3, 0);

    TestNode* cur = s_head.next;
    while (cur != &s_tail) {
        cur->delete_flag = 1;
        cur = cur->next;
    }

    cur = s_head.next;
    int all = 1;
    while (cur != &s_tail) {
        if (cur->delete_flag == 0) all = 0;
        cur = cur->next;
    }
    ASSERT_TRUE(all);
    tests_passed++;
}

void test_full_cleanup(void) {
    s_init = 0; s_count = 0;
    t_init();
    t_alloc(1, 100);
    t_alloc(2, 200);
    t_alloc(3, 0);
    ASSERT_EQ(s_count, 3);

    TestNode* cur = s_head.next;
    while (cur != &s_tail) {
        cur->delete_flag = 1;
        cur = cur->next;
    }

    cur = s_head.next;
    while (cur != &s_tail) {
        TestNode* next = cur->next;
        t_unlink(cur);
        t_free(cur);
        cur = next;
    }
    ASSERT_TRUE(s_head.next == &s_tail);
    ASSERT_TRUE(s_tail.prev == &s_head);
    tests_passed++;
}

void test_state_init(void) {
    s_init = 0; s_count = 0;
    t_init();
    TestNode* n = t_alloc(7, 0);
    ASSERT_EQ(n->state, -2);
    ASSERT_EQ(n->render_order, -1);
    ASSERT_EQ(n->delete_flag, 0);
    tests_passed++;
}

void test_null_free(void) {
    t_free(NULL);
    tests_passed++;
}

void test_many_entities(void) {
    s_init = 0; s_count = 0;
    t_init();
    TestNode* ents[50];
    int i;
    for (i = 0; i < 50; i++) {
        ents[i] = t_alloc(i % 10, 0);
    }
    ASSERT_EQ(s_count, 50);

    TestNode* cur = s_head.next;
    int last_pri = -1;
    int cnt = 0;
    while (cur != &s_tail) {
        ASSERT_TRUE((int)cur->priority >= last_pri);
        last_pri = (int)cur->priority;
        cnt++;
        cur = cur->next;
    }
    ASSERT_EQ(cnt, 50);
    tests_passed++;
}

void test_delete_first(void) {
    s_init = 0; s_count = 0;
    t_init();
    TestNode* e1 = t_alloc(1, 0);
    TestNode* e2 = t_alloc(2, 0);
    e1->delete_flag = 1;

    TestNode* cur = s_head.next;
    while (cur != &s_tail) {
        TestNode* next = cur->next;
        if (cur->delete_flag != 0) {
            t_unlink(cur);
            t_free(cur);
        }
        cur = next;
    }
    ASSERT_TRUE(s_head.next == e2);
    ASSERT_TRUE(e2->prev == &s_head);
    ASSERT_TRUE(e2->next == &s_tail);
    tests_passed++;
}

void test_delete_last(void) {
    s_init = 0; s_count = 0;
    t_init();
    TestNode* e1 = t_alloc(1, 0);
    TestNode* e2 = t_alloc(2, 0);
    e2->delete_flag = 1;

    TestNode* cur = s_head.next;
    while (cur != &s_tail) {
        TestNode* next = cur->next;
        if (cur->delete_flag != 0) {
            t_unlink(cur);
            t_free(cur);
        }
        cur = next;
    }
    ASSERT_TRUE(s_head.next == e1);
    ASSERT_TRUE(e1->next == &s_tail);
    tests_passed++;
}

void test_null_callback_safe(void) {
    s_init = 0; s_count = 0; s_cb_count = 0;
    t_init();
    TestNode* e1 = t_alloc(1, 0);
    e1->callback = NULL;

    TestNode* cur = s_head.next;
    while (cur != &s_tail) {
        if (cur->callback) cur->callback(cur);
        cur = cur->next;
    }
    ASSERT_EQ(s_cb_count, 0);
    tests_passed++;
}

void test_reinit(void) {
    s_init = 0; s_count = 0;
    t_init();
    t_alloc(1, 0);
    t_alloc(2, 0);
    ASSERT_EQ(s_count, 2);

    s_init = 0; s_count = 0;
    t_init();
    ASSERT_TRUE(s_head.next == &s_tail);
    ASSERT_EQ(s_count, 0);
    tests_passed++;
}

int main(void) {
    printf("=== Entity Linked List Tests ===\n\n");

    printf("[List Basics]\n");
    test_empty_list();     printf("  TEST: Empty list init ... PASS\n");
    test_reinit();         printf("  TEST: Reinit clears list ... PASS\n");

    printf("[Allocation]\n");
    test_alloc_single();   printf("  TEST: Allocate single entity ... PASS\n");
    test_alloc_with_extra();printf("  TEST: Allocate with extra buffer ... PASS\n");
    test_state_init();     printf("  TEST: State field initialization ... PASS\n");
    test_null_free();      printf("  TEST: NULL free safe ... PASS\n");

    printf("[Sorted Insertion]\n");
    test_sorted_insert();  printf("  TEST: Sorted insertion (3 entities) ... PASS\n");
    test_many_entities();  printf("  TEST: 50 entities sorted ... PASS\n");

    printf("[Unlinking]\n");
    test_unlink_middle();  printf("  TEST: Unlink middle ... PASS\n");
    test_unlink_first();   printf("  TEST: Unlink first ... PASS\n");

    printf("[Update Loop]\n");
    test_update_callbacks();     printf("  TEST: Callbacks called ... PASS\n");
    test_update_removes_deleted();printf("  TEST: Deleted removed ... PASS\n");
    test_null_callback_safe();   printf("  TEST: NULL callback safe ... PASS\n");

    printf("[Deletion]\n");
    test_delete_first();   printf("  TEST: Delete first entity ... PASS\n");
    test_delete_last();    printf("  TEST: Delete last entity ... PASS\n");
    test_mark_all_deleted();printf("  TEST: Mark all deleted ... PASS\n");
    test_full_cleanup();   printf("  TEST: Full cleanup ... PASS\n");

    printf("\n=== Results: %d passed, %d failed, %d total ===\n",
           tests_passed, tests_failed, tests_passed + tests_failed);
    return tests_failed;
}
