#ifndef MEMORY_H
#define MEMORY_H

#include <stdbool.h>
#include "pcb.h"

#define MEMORY_SIZE 60  // حجم الذاكرة بالضبط زي ما قلنا

// تعريف كلمة الذاكرة
typedef struct {
    char* name;       // اسم المتغير أو instruction أو أي بيانات تانية
    char* data;       // القيمة بتاعته
    int process_id;   // ID بتاع العملية اللي مالكة للذاكرة دي
} MemoryWord;

// هيكل الذاكرة بالكامل
typedef struct {
    MemoryWord words[MEMORY_SIZE];  // مصفوفة من الكلمات
    int next_free_word;             // ممكن تستخدمها لتسريع البحث لو حبيت
} Memory;

// ✅ التهيئة
void init_memory(Memory* memory);

// ✅ تخصيص مساحة للـ PCB
int allocate_memory(Memory* memory, PCB* pcb, int size);

// ✅ تحرير الذاكرة لما العملية تخلص
void deallocate_memory(Memory* memory, PCB* pcb);

// ✅ الكتابة داخل الذاكرة (اسم + قيمة)
void write_memory(Memory* memory, int address, const char* name, const char* data, int process_id);

// ✅ قراءة من الذاكرة
void read_memory(const Memory* memory, int address, char** name, char** data, int* process_id);

// ✅ طباعة الذاكرة بالكامل
void print_memory(const Memory* memory);

// ✅ التأكد إذا كان فيه مساحة متاحة (قبل الحجز)
bool is_memory_available(const Memory* memory, int size);

#endif  // MEMORY_H